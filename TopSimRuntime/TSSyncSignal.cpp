#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#ifndef  _WRS_KERNEL

#include <boost/filesystem.hpp>
#include <boost/asio/high_resolution_timer.hpp>

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimUtil/pugixml.hpp>
#include <TopSimUtil/TSDynamicLib.h>
#else
#include <TopSimRuntime/pugixml.hpp>
#include <TopSimRuntime/TSDynamicLib.h>
#include <TopSimRuntime/TSLogsUtil.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#include <TopSimDataInterface/TSTopicTypeManager.h>
#include <TopSimDataInterface/TSRDGTypes.h>
#include <TopSimDataInterface/TSSegment.h>

#include "TSSerializer.h"
#include "TSSMBackendInteract_p.h"
#include "TSSMBackendInteract.h"


TSFrontAppUniquelyId TSFrontAppDataInteract::GetAppId()
{
	return _AppId;
}

bool TSFrontAppDataInteract::IsStoped()
{
	return _Stoped;
}

boost::atomic_uint8_t & TSFrontAppDataInteract::KeepLive()
{
	return _keepLive;
}

bool TSFrontAppDataInteract::Init( TSFrontAppUniquelyId AppId ,TSSMBackendInteractPrivate * p)
{
	DEF_LOG_KERNEL(GB18030ToTSString("与前端建立通讯的实例初始化->{AppId=[%1]}\n")) << AppId;
	_Stoped = false;
	_AppId = AppId;
	_p = p;
	_keepLive = true;

	_sendMqIOSPtr = boost::make_shared<TSIOSStruct>();
	_sendMqIOSPtr->RunAlways();

	_ListenMq = boost::make_shared<MessageQueue_T>(boost::interprocess::open_only_t(),
		(TSValue_Cast<TSString>(_AppId) + "-FrontMessageQueue").c_str());

	if(!_ListenMq)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_57) << _AppId;
		return false;
	}

	_SendMq = boost::make_shared<MessageQueue_T>(boost::interprocess::open_only_t(),
		(TSValue_Cast<TSString>(_AppId) + "-BackendMessageQueue").c_str());

	if(!_SendMq)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_66) << _AppId;
		return false;
	}

	_BackendExchangedItem = 
		GetSegment()->find<TSSharedMemoryExchangedItem>((TSValue_Cast<TSString>(_AppId) + "-BackendExchangedItem").c_str()).first;

	if(!_BackendExchangedItem)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_75) << _AppId;
		return false;
	}

	_FrontExchangedItem = 
		GetSegment()->find<TSSharedMemoryExchangedItem>((TSValue_Cast<TSString>(_AppId) + "-FrontExchangedItem").c_str()).first;

	if(!_FrontExchangedItem)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_84) << _AppId;
		return false;
	}

	_ListenThread = boost::make_shared<boost::thread>(boost::bind(&TSFrontAppDataInteract::Listen,this));

	return true;
}


bool TSFrontAppDataInteract::IsSubscriber( UINT32 DomainId,TSTOPICHANDLE Topic )
{
	TSPSMatcherMap_T::iterator it = _Matchers.find(DomainId);
	if(it != _Matchers.end())
	{
		if(it->second)
		{
			return it->second->IsSubscriber(Topic);
		}
	}

	return false;
}


bool TSFrontAppDataInteract::IsKeepLive()
{
	return _keepLive > 0;
}

void TSFrontAppDataInteract::Listen()
{
	DEF_LOG_KERNEL(GB18030ToTSString("创建的前端实例开启监听->{AppId=[%1]}\n")) << _AppId;
	try
	{
		while(!_Stoped)
		{
			TSMessageQueueSendItem Item;

			size_t ReceivedSize;

			UINT32 Pri;

			try
			{
				size_t count = 0;

				while(!_ListenMq->try_receive(&Item,sizeof(TSMessageQueueSendItem),ReceivedSize,Pri))
				{
					if(_Stoped)
					{
						break;
					}

					TSSchedYield(count++);

					continue;
				}
			}
			catch(const boost::interprocess::interprocess_exception & e)
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_144) << e.what();
				_Stoped = true;
				break;
			}

			if(_Stoped)
			{
				break;
			}

			ASSERT(ReceivedSize == sizeof(TSMessageQueueSendItem));
			ASSERT(_FrontExchangedItem);

			if(Item.Flag == TSMessageQueueSendItem::kClean)
			{
				DEF_LOG_KERNEL(GB18030ToTSString("创建的前端实例接收到清理消息->{Flag:[kClean], AppId=[%1]}\n")) << _AppId;
				_FrontExchangedItem->_ExchangedData1.reset();
				_FrontExchangedItem->_ExchangedData2.reset();
				_FrontExchangedItem->_ExchangedData3.reset();
				break;
			}
			else if(Item.Flag == TSMessageQueueSendItem::kData)
			{
				TSTopicDataMqItemSharedPtr & ExchangedData = 
					Item.ExchangedDataIndex == 1 ? 
					_FrontExchangedItem->_ExchangedData1 : Item.ExchangedDataIndex == 2 ? 
					_FrontExchangedItem->_ExchangedData2 : _FrontExchangedItem->_ExchangedData3;

				ASSERT(ExchangedData);

				if(Item.SubFlag == TSMessageQueueSendItem::kPublish
					|| Item.SubFlag == TSMessageQueueSendItem::kSublish
					|| Item.SubFlag == TSMessageQueueSendItem::kUnpublish
					|| Item.SubFlag == TSMessageQueueSendItem::kUnsublish
					|| Item.SubFlag == TSMessageQueueSendItem::kCreateDomain
					|| Item.SubFlag == TSMessageQueueSendItem::kDestoryDomain)
				{
					TSPSMatcherPtr & Matcher = _Matchers[ExchangedData->DomainId];
					if(!Matcher)
					{
						Matcher = boost::make_shared<TSPSMatcher>();
					}

					if(Item.SubFlag == TSMessageQueueSendItem::kPublish)
					{
						DEF_LOG_KERNEL(GB18030ToTSString("创建的前端实例接收到发布主题消息->{Flag:[kData], SubFlag:[kPublish], AppId:[%1], DomainId:[%2], Topic:[%3]}\n")) << _AppId << ExchangedData->DomainId << ExchangedData->Topic;
						TopSimDataInterface::ReturnCode Ret = Matcher->Publish(ExchangedData->Topic);
						if(Ret == TopSimDataInterface::Ok)
						{
							if(_p->_TransferMiddleware)
							{
								_p->_TransferMiddleware->Publish(TSValue_Cast<TSString>(ExchangedData->DomainId),ExchangedData->Topic);
							}
						}
					}
					else if(Item.SubFlag == TSMessageQueueSendItem::kSublish)
					{
						DEF_LOG_KERNEL(GB18030ToTSString("创建的前端实例接收到订阅主题消息->{Flag:[kData], SubFlag:[kSubscribe], AppId:[%1], DomainId:[%2], Topic:[%3]}\n")) << _AppId << ExchangedData->DomainId << ExchangedData->Topic;
						TopSimDataInterface::ReturnCode Ret = Matcher->Subscribe(ExchangedData->Topic);
						if(Ret == TopSimDataInterface::Ok)
						{
							if(TSSharedMemoryTopicDataCachePtr DataCache = _p->GetTopicCache(ExchangedData->DomainId,ExchangedData->Topic))
							{
								TSRWSpinLock::ReadLock lock(DataCache->Lock);
								DEF_LOG_KERNEL(GB18030ToTSString("发送缓存数据给订阅端->{AppId:[%1], DomainId:[%2], Topic:[%3], DataCount:[%4]}\n")) << _AppId << ExchangedData->DomainId << ExchangedData->Topic << DataCache->Cache.size();
								TSTopicDataMqItemMapType::iterator it = DataCache->Cache.begin();
								while(it != DataCache->Cache.end())
								{
									SendMq(TSMessageQueueSendItem::kData,TSMessageQueueSendItem::kUpdateTopic,it->second);

									++it;
								}
							}

							if(_p->_TransferMiddleware)
							{
								_p->_TransferMiddleware->Subscribe(TSValue_Cast<TSString>(ExchangedData->DomainId),ExchangedData->Topic,Item.LocalOnly);
							}
						}
					}
					else if(Item.SubFlag == TSMessageQueueSendItem::kUnpublish)
					{
						DEF_LOG_KERNEL(GB18030ToTSString("创建的前端实例接收到取消发布主题消息->{Flag:[kData], SubFlag:[kUnpublish], AppId:[%1], DomainId:[%2], Topic:[%3]}\n")) << _AppId << ExchangedData->DomainId << ExchangedData->Topic._value;
						TopSimDataInterface::ReturnCode Ret = Matcher->Unpublish(ExchangedData->Topic);
						if(Ret == TopSimDataInterface::Ok)
						{
							if(_p->_TransferMiddleware)
							{
								_p->_TransferMiddleware->Unpublish(TSValue_Cast<TSString>(ExchangedData->DomainId),ExchangedData->Topic);
							}
						}
					}
					else if(Item.SubFlag == TSMessageQueueSendItem::kUnsublish)
					{
						DEF_LOG_KERNEL(GB18030ToTSString("创建的前端实例接收到取消订阅主题消息->{Flag:[kData], SubFlag:[kUnsubscribe], AppId:[%1], DomainId:[%2], Topic:[%3]}\n")) << _AppId << ExchangedData->DomainId << ExchangedData->Topic._value;
						TopSimDataInterface::ReturnCode Ret = Matcher->Unsubscribe(ExchangedData->Topic);
						if(Ret == TopSimDataInterface::Ok)
						{
							if(_p->_TransferMiddleware)
							{
								_p->_TransferMiddleware->Unsubscribe(TSValue_Cast<TSString>(ExchangedData->DomainId),ExchangedData->Topic);
							}
						}
					}
					else if(Item.SubFlag == TSMessageQueueSendItem::kCreateDomain)
					{
						DEF_LOG_KERNEL(GB18030ToTSString("创建的前端实例接收到创建域消息->{Flag:[kData], SubFlag:[kCreateDomain], AppId:[%1], DomainId:[%2], Topic:[%3]}\n")) << _AppId << ExchangedData->DomainId << ExchangedData->Topic._value;
						_Matchers.insert(std::make_pair(ExchangedData->DomainId,boost::make_shared<TSPSMatcher>()));

						if(strcmp(ExchangedData->DomainPartNames,"") == 0)
						{
							if(_p->_TransferMiddleware)
							{
								TSString DomainId = TSValue_Cast<TSString>(ExchangedData->DomainId) + "://" + ExchangedData->DomainPartNames;
								_p->_TransferMiddleware->CreateDomain(DomainId,_AppId);
							}
						}
						else
						{
							if(_p->_TransferMiddleware)
							{
								_p->_TransferMiddleware->CreateDomain(TSValue_Cast<TSString>(ExchangedData->DomainId),_AppId);
							}
						}
					}
					else if(Item.SubFlag == TSMessageQueueSendItem::kDestoryDomain)
					{
						DEF_LOG_KERNEL(GB18030ToTSString("创建的前端实例接收到创建域消息->{Flag:[kData], SubFlag:[kUnsubscribe], AppId:[%1], DomainId:[%2], Topic:[%3]}")) << _AppId << ExchangedData->DomainId << ExchangedData->Topic._value;
						_Matchers.unsafe_erase(ExchangedData->DomainId);

						if(_p->_TransferMiddleware)
						{
							_p->_TransferMiddleware->DestroyDomain(TSValue_Cast<TSString>(ExchangedData->DomainId));
						}
					}
				}
				//接收到发送主题消息
				else if(Item.SubFlag == TSMessageQueueSendItem::kUpdateTopic)
				{
					OnUpdateTopicCallback(shared_from_this(),ExchangedData);
				}
				//接收到删除主题消息
				else if(Item.SubFlag == TSMessageQueueSendItem::kDeleteTopic)
				{
					OnDeleteTopicCallback(shared_from_this(),ExchangedData);
				}
				//接收到直接发送主题消息
				else if(Item.SubFlag == TSMessageQueueSendItem::kUpdateTopicDirect)
				{
					OnUpdateTopicDirectCallback(shared_from_this(),ExchangedData);
				}

				//非常重要
				ExchangedData.reset();
			}
		}
	}
	catch(const std::exception & e)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_294) << e.what();
	}
}


bool TSFrontAppDataInteract::SendMq( TSMessageQueueSendItem::OpFlag Flag, 
	TSMessageQueueSendItem::SubOpType SubFlag,TSTopicDataMqItemSharedPtr Data)
{
#if defined(XSIM3_2) || defined(XSIM3_3)
	_sendMqIOSPtr->GetIOS()->post(boost::bind(&TSFrontAppDataInteract::_sendMq, this, Flag, SubFlag, Data));
#else
	_sendMqIOSPtr->GetIOS()->Post(boost::bind(&TSFrontAppDataInteract::_sendMq, this, Flag, SubFlag, Data));
#endif
	return true;
}


bool TSFrontAppDataInteract::SendMq( TSMessageQueueSendItem::OpFlag Flag )
{
#if defined(XSIM3_2) || defined(XSIM3_3)
	_sendMqIOSPtr->GetIOS()->post(boost::bind(&TSFrontAppDataInteract::_sendMq, this, Flag));
#else
	_sendMqIOSPtr->GetIOS()->Post(boost::bind(&TSFrontAppDataInteract::_sendMq, this, Flag));
#endif
	return true;
}

bool TSFrontAppDataInteract::_sendMq(TSMessageQueueSendItem::OpFlag Flag, TSMessageQueueSendItem::SubOpType SubFlag, TSTopicDataMqItemSharedPtr Data)
{
	if (_Stoped)
	{
		return false;
	}

	ASSERT(_BackendExchangedItem);

	boost::interprocess::scoped_lock<TSRDGRobustMutex> lock(_BackendExchangedItem->_Mutex);

	TSMessageQueueSendItem Item;
	Item.Flag = Flag;
	Item.SubFlag = SubFlag;
	Item.Source = _AppId;

	if (!_BackendExchangedItem->_ExchangedData1)
	{
		Item.ExchangedDataIndex = 1;
		_BackendExchangedItem->_ExchangedData1 = Data;
	}
	else if (!_BackendExchangedItem->_ExchangedData2)
	{
		Item.ExchangedDataIndex = 2;
		_BackendExchangedItem->_ExchangedData2 = Data;
	}
	else
	{
		ASSERT(!_BackendExchangedItem->_ExchangedData3);
		_BackendExchangedItem->_ExchangedData3 = Data;
		Item.ExchangedDataIndex = 3;
	}

	try
	{
		size_t count = 0;

		while (!_SendMq->try_send(&Item, sizeof(TSMessageQueueSendItem), 0))
		{
			if (_Stoped)
			{
				break;
			}

			TSSchedYield(++count);
		}
	}
	catch (const boost::interprocess::interprocess_exception & e)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_349) << e.what();
		_Stoped = true;
	}

	return _Stoped;
}
bool TSFrontAppDataInteract::_sendMq(TSMessageQueueSendItem::OpFlag Flag)
{
	TSMessageQueueSendItem Item;
	Item.Flag = Flag;
	Item.Source = _AppId;

	try
	{
		size_t count = 0;

		while (!_SendMq->try_send(&Item, sizeof(TSMessageQueueSendItem), 0))
		{
			if (_Stoped)
			{
				break;
			}

			TSSchedYield(++count);
		}
	}
	catch (const boost::interprocess::interprocess_exception & e)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_379) << e.what();
		_Stoped = true;
	}

	return _Stoped;
}

bool TSFrontAppDataInteract::TrySendHeartbeat()
{
	try
	{
		TSMessageQueueSendItem Item;
		Item.Flag = TSMessageQueueSendItem::kHeartbeat;
		Item.Source = _AppId;

		return _SendMq->try_send(&Item,sizeof(TSMessageQueueSendItem),0);
	}
	catch(const boost::interprocess::interprocess_exception & e)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_398) << e.what();
		_Stoped = true;
	}

	return false;
}

void TSFrontAppDataInteract::Clean()
{
	_Stoped = true;

	TSMessageQueueSendItem Item;
	Item.Flag = TSMessageQueueSendItem::kClean;

	try
	{
		_SendMq->try_send(&Item,sizeof(TSMessageQueueSendItem),0);
	}
	catch(const boost::interprocess::interprocess_exception & e)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_418) << e.what();
	}

	_ListenThread->join();
	_ListenThread.reset();
	
	_sendMqIOSPtr->Stop();
}


TSSMBackendInteractPrivate::TSSMBackendInteractPrivate()
	:_Public_Ptr(0),_Stoped(false)
{
	 
}

TSSharedMemoryTopicDataCachePtr TSSMBackendInteractPrivate::GetTopicCache( UINT32 DomainId,TSTOPICHANDLE Topic )
{
	UINT64 Key = DomainId;
	Key = Key << 32 | Topic;
	return _TopicMap[Key];
}

TSSMBackendInteract::TSSMBackendInteract(void)
	:_Private_Ptr(new PrivateType)
{
    T_D();

	d->_Public_Ptr = this;
}

TSSMBackendInteract::TSSMBackendInteract( PrivateType * p )
	:_Private_Ptr(p)
{
    T_D();

	d->_Public_Ptr = this;
}


TSSMBackendInteract::~TSSMBackendInteract(void)
{
	delete _Private_Ptr;
}

bool TSSMBackendInteract::Initialize(int argc,char ** argv)
{
	T_D();

	if(Configure(argc,argv))
	{
		d->_Stoped = false;

		try
		{
			MessageQueue_T::remove("Golbal-MessageQueue");

			d->_GlobalMq = boost::make_shared<MessageQueue_T>(boost::interprocess::create_only_t(),
				"Golbal-MessageQueue",100,sizeof(TSMessageQueueSendItem));

			if(!d->_GlobalMq)
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_478);
				return false;
			}
		}
		catch(const boost::interprocess::interprocess_exception & e)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_484) << e.what();
			return false;
		}
		#if defined(XSIM3_2) || defined(XSIM3_3)
		d->_EventDispatcherThread = boost::make_shared<boost::thread>(boost::bind(&TSSMBackendInteract::EventDispatcherThread_,this));
		d->_Thread =  boost::make_shared<TSAsyncEventDispatcher>();
		#else
		d->_Thread = TSThread::CreateInstance();
		d->_Thread->Start();
		#endif

		d->_HeartbeatTimer = d->_Thread->CreateTimer();

		boost::system::error_code ec;
		d->_Thread->Post(boost::bind(&TSSMBackendInteract::Heartbeat,this,ec));

		return true;
		
	}

	return false;
}

void TSSMBackendInteract::Cleanup()
{
	T_D();

	TSRWSpinLock::ReadLock lock(d->_InteractsLock);

	BOOST_FOREACH(TSFrontAppDataInteractPtr Interact,d->_Interacts)
	{
		Interact->Clean();
	}

	if(d->_HeartbeatTimer)
	{
		d->_HeartbeatTimer->CancelCurrentCall();
		d->_HeartbeatTimer->Reset();
		d->_HeartbeatTimer.reset();
	}

	if(d->_Thread)
	{
		d->_Thread->Stop();
		#if defined(XSIM3_2) || defined(XSIM3_3)
		d->_EventDispatcherThread->join();
		d->_EventDispatcherThread.reset();
		#else
		d->_Thread->Join();
		d->_Thread.reset();
		#endif
	}
}

bool TSSMBackendInteract::Configure(int argc,char ** argv)
{
    T_D();

	TSString RealizedTypeName;
	if(pugi::xml_node node =GetCfgDoc().select_single_node(TS_TEXT("Config/Middleware")).node())
	{
		RealizedTypeName = node.attribute(TS_TEXT("Realized")).as_string();
	}

	if(!RealizedTypeName.empty())
	{
		if(int MetaType = TSMetaType::GetType(RealizedTypeName.c_str()))
		{
			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_537) << RealizedTypeName;

			d->_TransferMiddleware = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(MetaType),TSITransferMiddlewarePtr);
			d->_TransferMiddleware->TopicCallback = boost::bind(&TSSMBackendInteract::OnTopicAvaliable,this,_1,_2,_3);
			d->_TransferMiddleware->RemoveCallback = boost::bind(&TSSMBackendInteract::OnTopicRemoved,this,_1,_2,_3);
		}
		else
		{
			DEF_LOG_WARNING(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_545) << RealizedTypeName;
		}
	}

	if(d->_TransferMiddleware)
	{
		if(d->_TransferMiddleware->Initialize(argc,argv))
		{
			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_553) << RealizedTypeName;
			return true;
		}
		else
		{
            DEF_LOG_WARNING(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_558) << RealizedTypeName;
			d->_TransferMiddleware.reset();
		}
	}

	return false;
}

void TSSMBackendInteract::OnTopicAvaliable( const TSDomainId& DomainId,TSTOPICHANDLE THandle,TSDataContextPtr Ctx)
{
	T_D();

	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();
	Item->DomainId = TSValue_Cast<UINT32>(DomainId);
	Item->Topic = THandle;
	MakeSharedMemoryDataContext(Item->Ctx,Ctx);

	InsertToSharedMemory(Item);

	TSRWSpinLock::ReadLock lock(d->_InteractsLock);
	UINT32 DomainIdInt = TSValue_Cast<UINT32>(DomainId);

	BOOST_FOREACH(TSFrontAppDataInteractPtr Interact,d->_Interacts)
	{
		if(Interact->IsSubscriber(DomainIdInt,THandle))
		{
			Interact->SendMq(TSMessageQueueSendItem::kData,TSMessageQueueSendItem::kUpdateTopic,Item);
		}
	}
}


void TSSMBackendInteract::SetTimeHolder( TSAbstractTimeHolderPtr TimeHoder )
{
	T_D();

	d->_TimeHolder = TimeHoder;
}

TSAbstractTimeHolderPtr TSSMBackendInteract::GetTimeHolder()
{
	T_D();

	return d->_TimeHolder;
}

void TSSMBackendInteract::OnTopicRemoved( const TSDomainId& DomainId,TSTOPICHANDLE THandle,TSDataContextPtr Ctx )
{
	T_D();

	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();
	Item->DomainId = TSValue_Cast<UINT32>(DomainId);
	Item->Topic = THandle;
	Item->IsNotice = false;
	MakeSharedMemoryDataContext(Item->Ctx,Ctx);

	EraseFromSharedMemory(Item);

	TSRWSpinLock::ReadLock lock(d->_InteractsLock);

	BOOST_FOREACH(TSFrontAppDataInteractPtr Interact,d->_Interacts)
	{
		Interact->SendMq(TSMessageQueueSendItem::kData,TSMessageQueueSendItem::kDeleteTopic,Item);
	}
}

void TSSMBackendInteract::Listen()
{
	T_D();

	while(!d->_Stoped)
	{
		try
		{
			TSMessageQueueSendItem Item;

			size_t ReceivedSize;

			UINT32 Pri;

			size_t count = 0;

			while(!d->_GlobalMq->try_receive(&Item,sizeof(TSMessageQueueSendItem),ReceivedSize,Pri))
			{
				if(d->_Stoped)
				{
					break;
				}

				TSSchedYield(count++);

				continue;
			}

			if(d->_Stoped)
			{
				break;
			}

			ASSERT(ReceivedSize == sizeof(TSMessageQueueSendItem));

			if(Item.Flag == TSMessageQueueSendItem::kInit)
			{
				TSRWSpinLock::WriteLock lock(d->_InteractsLock);
				DEF_LOG_KERNEL(GB18030ToTSString("收到前端初始化信息->{Flag:[kInit], AppId:[%1]}\n")) << Item.Source._value;

				for(std::vector<TSFrontAppDataInteractPtr>::iterator it = d->_Interacts.begin();
					it != d->_Interacts.end();)
				{
					if(TSFrontAppDataInteractPtr & Interact = (*it))
					{
						if(Interact->IsStoped())
						{
							it = d->_Interacts.erase(it);
						}
						else if(Interact->GetAppId() == Item.Source)
						{
							Interact->Clean();
							d->_Interacts.erase(it);

							break;
						}
						else
						{
							++it;
						}
					}
				}

				TSFrontAppDataInteractPtr Interact = boost::make_shared<TSFrontAppDataInteract>();
				Interact->OnDeleteTopicCallback = boost::bind(&TSSMBackendInteract::OnDeleteTopic,this,_1,_2);
				Interact->OnUpdateTopicCallback = boost::bind(&TSSMBackendInteract::OnUpdateTopic,this,_1,_2);
				Interact->OnUpdateTopicDirectCallback = boost::bind(&TSSMBackendInteract::OnUpdateTopicDirect,this,_1,_2);

				if(Interact->Init(Item.Source,d))
				{
					d->_Interacts.push_back(Interact);
				}
			}
			else if(Item.Flag == TSMessageQueueSendItem::kClean)
			{
				TSRWSpinLock::WriteLock lock(d->_InteractsLock);
				DEF_LOG_KERNEL(GB18030ToTSString("收到前端清理信息->{Flag:[kClean], AppId:[%1]}\n")) << Item.Source._value;

				for(std::vector<TSFrontAppDataInteractPtr>::iterator it = d->_Interacts.begin();
					it != d->_Interacts.end(); ++it)
				{
					if(TSFrontAppDataInteractPtr & Interact = (*it))
					{
						if(Interact->GetAppId() == Item.Source)
						{
							if(!Interact->IsStoped())
							{
								Interact->Clean();
							}

							d->_Interacts.erase(it);
							break;
						}
					}
				}
			}
			else if(Item.Flag == TSMessageQueueSendItem::kHeartbeat)
			{
				TSRWSpinLock::ReadLock lock(d->_InteractsLock);
				//DEF_LOG_KERNEL(TSUtf8String("收到前端心跳信息->{Flag:[" + TSValue_Cast<TSString>(Item.Flag) + "], AppId:[" + TSValue_Cast<TSString>(Item.Source._value) + "]}\n"));

				for(std::vector<TSFrontAppDataInteractPtr>::iterator it = d->_Interacts.begin();
					it != d->_Interacts.end(); ++it)
				{
					if(TSFrontAppDataInteractPtr & Interact = (*it))
					{
						if(Interact->GetAppId() == Item.Source)
						{
							if(!Interact->IsStoped())
							{
								if(++Interact->KeepLive() > 5)
								{
									Interact->KeepLive() = 1;
								}
							}

							break;
						}
					}
				}
			}
		}
		catch(const boost::interprocess::interprocess_exception & e)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSMBACKENDINTERACT_CPP_745) << e.what();
			d->_Stoped = true;
		}
	}
}

void TSSMBackendInteract::OnDeleteTopic(TSFrontAppDataInteractPtr SourceInteract, TSTopicDataMqItemSharedPtr Item )
{
	T_D();

	if(d->_TransferMiddleware)
	{
		d->_TransferMiddleware->DeleteTopic(TSValue_Cast<TSString>(Item->DomainId),
			Item->Topic,
			MakeDataContextFromSharedMemory(Item->Ctx),
			Item->IsNotice);
	}

	EraseFromSharedMemory(Item);

	TSRWSpinLock::ReadLock lock(d->_InteractsLock);

	BOOST_FOREACH(TSFrontAppDataInteractPtr Interact,d->_Interacts)
	{
		if(!SourceInteract->IsStoped())
		{
			if(SourceInteract != Interact && Interact->IsSubscriber(Item->DomainId,Item->Topic))
			{
				Interact->SendMq(TSMessageQueueSendItem::kData,TSMessageQueueSendItem::kUpdateTopic,Item);
			}
		}
	}
}

void TSSMBackendInteract::OnUpdateTopicDirect(TSFrontAppDataInteractPtr SourceInteract, TSTopicDataMqItemSharedPtr Item )
{
	T_D();

	if(d->_TransferMiddleware)
	{
		d->_TransferMiddleware->SendData(TSValue_Cast<TSString>(Item->DomainId),
			Item->Topic,
			MakeDataContextFromSharedMemory(Item->Ctx),
			Item->Receiver);
	}

	TSRWSpinLock::ReadLock lock(d->_InteractsLock);

	BOOST_FOREACH(TSFrontAppDataInteractPtr Interact,d->_Interacts)
	{ 
		if(!SourceInteract->IsStoped())
		{
			if(SourceInteract != Interact && Interact->IsSubscriber(Item->DomainId,Item->Topic))
			{
				Interact->SendMq(TSMessageQueueSendItem::kData,TSMessageQueueSendItem::kUpdateTopic,Item);
			}
		}
	}
}

void TSSMBackendInteract::OnUpdateTopic(TSFrontAppDataInteractPtr SourceInteract, TSTopicDataMqItemSharedPtr Item )
{
	T_D();
	
	DEF_LOG_KERNEL(GB18030ToTSString("转发主题OnUpdateTopic->{DomainId:[%1], Topic:[%2]}\n")) << Item->DomainId << Item->Topic._value;
	if(d->_TransferMiddleware)
	{
		d->_TransferMiddleware->UpdateTopic(TSValue_Cast<TSString>(Item->DomainId),
			Item->Topic,
			MakeDataContextFromSharedMemory(Item->Ctx));
	}

	//插入共享内存中
	InsertToSharedMemory(Item);

	TSRWSpinLock::ReadLock lock(d->_InteractsLock);

	//给所有订阅该域上主题的前端发送该数据
	BOOST_FOREACH(TSFrontAppDataInteractPtr Interact,d->_Interacts)
	{
		if(!SourceInteract->IsStoped())
		{
			if(SourceInteract != Interact && Interact->IsSubscriber(Item->DomainId,Item->Topic))
			{
				Interact->SendMq(TSMessageQueueSendItem::kData,TSMessageQueueSendItem::kUpdateTopic,Item);
			}
		}
	}
}

void TSSMBackendInteract::InsertToSharedMemory( TSTopicDataMqItemSharedPtr Item )
{
	T_D();

	UINT64 Key = Item->DomainId;
	Key = Key << 32 | Item->Topic._value;

	TSSharedMemoryTopicDataCachePtr & DataCache = d->_TopicMap[Key];

	if(!DataCache)
	{
		DataCache = boost::make_shared<TSSharedMemoryTopicDataCache>();
	}

	DataCache->Lock.BeginRead();

	TSTopicDataMqItemMapType::iterator it = DataCache->Cache.find(Item->Ctx.KeyHash);

	if(it != DataCache->Cache.end())
	{
		it->second = Item;
	}
	else
	{
		DataCache->Cache[Item->Ctx.KeyHash] = Item;
	}

	DataCache->Lock.EndRead();
}

void TSSMBackendInteract::EraseFromSharedMemory( TSTopicDataMqItemSharedPtr Item )
{
	T_D();

	UINT64 Key = Item->DomainId;
	Key = Key << 32 | Item->Topic._value;

	TSSharedMemoryTopicDataCachePtr & DataCache = d->_TopicMap[Key];

	if(DataCache)
	{
		DataCache->Lock.BeginWrite();

		TSTopicDataMqItemMapType::iterator it = DataCache->Cache.find(Item->Ctx.KeyHash);

		if(it != DataCache->Cache.end())
		{
			DataCache->Cache.unsafe_erase(it);
		}

		DataCache->Lock.EndWrite();
	}
}

void TSSMBackendInteract::Heartbeat( const boost::system::error_code & ec )
{
	T_D();

	if(!ec)
	{
		TSRWSpinLock::ReadLock lock(d->_InteractsLock);

		for(std::vector<TSFrontAppDataInteractPtr>::iterator it = d->_Interacts.begin();
			it != d->_Interacts.end(); ++it)
		{
			if(TSFrontAppDataInteractPtr & Interact = (*it))
			{
				if(!Interact->IsStoped())
				{
					if(Interact->IsKeepLive())
					{
						Interact->KeepLive() = 0;
					}
					else if(!Interact->TrySendHeartbeat())
					{
						//已掉线
						Interact->Clean();
					}
				}
			}
		}

		d->_HeartbeatTimer->AsyncCall(500,boost::bind(&TSSMBackendInteract::Heartbeat,this,boost::asio::placeholders::error));

	}
}

const TSString & TSSMBackendInteract::GetLocalIPAddress()
{
	static TSString Ip;

	if(Ip.empty())
	{
#ifdef _WIN32
		WSAData data;
		if(!WSAStartup(MAKEWORD(1,1),&data))
		{
			char host[255];
			gethostname(host,sizeof(host));

			hostent * p = gethostbyname(host);

			in_addr in;
			memcpy(&in,p->h_addr_list[0],sizeof(in));

			Ip = inet_ntoa(in);

			WSACleanup();
		}
#endif
	}

	return Ip;
}

void TSSMBackendInteract::EventDispatcherThread_()
{
	T_D();

#if defined(XSIM3_2) || defined(XSIM3_3)
	d->_Thread->Run();
#endif
}

#endif //#ifndef  _WRS_KERNEL
