#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"


#ifndef  _WRS_KERNEL

#if !defined(XSIM3_3) && !defined(XSIM3_2)
#include <TopSimRuntime/TSLogsUtil.h>
#endif // !defined(XSIM3_3) && !defined(XSIM3_2)

#include <boost/interprocess/smart_ptr/shared_ptr.hpp>

#include "TSSharedMemoryMiddleware.h"
#include "TSSegment.h"
#include "TSSharedMemoryMiddleware_p.h"
#include "TSDataInteractManager.h"
#include "TSSerializer.h"

BEGIN_METADATA(TSSharedMemoryMiddleware)
	REG_BASE(TSITransferMiddleware);
END_METADATA()


TSSharedMemoryMiddlewarePrivate::TSSharedMemoryMiddlewarePrivate()
{
	_FrontExchangedItem = 0;
	_BackendExchangedItem = 0;
}


TSSharedMemoryMiddleware::TSSharedMemoryMiddleware()
	:SuperType(new PrivateType)
{
	T_D();

	d->_Stoped = false;
}

TSSharedMemoryMiddleware::TSSharedMemoryMiddleware( PrivateType * p )
	:SuperType(p)
{
	T_D();
	#if defined(XSIM3_2) || defined(XSIM3_3)
	d->_EventDispatcherThread->join();
	#endif
	d->_Stoped = false;
}

bool TSSharedMemoryMiddleware::Initialize( int argc,char ** argv )
{
	T_D();

	try
	{
		TSSegment::Instance()->Init(TSSegment::OpenOnly);

		d->_AppId = DataManager->GetFrontAppUniquelyId();

		d->_Stoped = false;

		d->_GlobalMq = boost::make_shared<MessageQueue_T>(boost::interprocess::open_only_t(),
			"Golbal-MessageQueue");

		if(!d->_GlobalMq)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSHAREDMEMORYMIDDLEWARE_CPP_60);
			return false;
		}

		//创建接收后端消息的共享内存消息队列
		MessageQueue_T::remove((TSValue_Cast<TSString>(d->_AppId) + "-BackendMessageQueue").c_str());

		d->_ListenMq = boost::make_shared<MessageQueue_T>(boost::interprocess::create_only_t(),
			(TSValue_Cast<TSString>(d->_AppId) + "-BackendMessageQueue").c_str(),1,sizeof(TSMessageQueueSendItem));

		if(!d->_ListenMq)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSHAREDMEMORYMIDDLEWARE_CPP_71);
			return false;
		}

		//创建前端发送消息的共享内存消息队列
		MessageQueue_T::remove((TSValue_Cast<TSString>(d->_AppId) + "-FrontMessageQueue").c_str());

		d->_SendMq = boost::make_shared<MessageQueue_T>(boost::interprocess::create_only_t(),
			(TSValue_Cast<TSString>(d->_AppId) + "-FrontMessageQueue").c_str(),1,sizeof(TSMessageQueueSendItem));

		if(!d->_SendMq)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSHAREDMEMORYMIDDLEWARE_CPP_82);
			return false;
		}

		//创建交换内存
		d->_BackendExchangedItem = 
			GetSegment()->construct<TSSharedMemoryExchangedItem>((TSValue_Cast<TSString>(d->_AppId) + "-BackendExchangedItem").c_str())();

		if(!d->_BackendExchangedItem)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSHAREDMEMORYMIDDLEWARE_CPP_101) << d->_AppId;
			return false;
		}

		d->_FrontExchangedItem = 
			GetSegment()->construct<TSSharedMemoryExchangedItem>((TSValue_Cast<TSString>(d->_AppId) + "-FrontExchangedItem").c_str())();

		if(!d->_FrontExchangedItem)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSHAREDMEMORYMIDDLEWARE_CPP_92) << d->_AppId;
			return false;
		}
	}
	catch(const boost::interprocess::interprocess_exception & e)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSHAREDMEMORYMIDDLEWARE_CPP_107) << e.what();
	}

	//启动监听线程
	d->_ListenThread = boost::make_shared<boost::thread>(boost::bind(&TSSharedMemoryMiddleware::Listen,this));

	#if defined(XSIM3_2) || defined(XSIM3_3)
	d->_EventDispatcherThread = boost::make_shared<boost::thread>(boost::bind(&TSSharedMemoryMiddleware::EventDispatcherThread_,this));
	d->_Thread =  boost::make_shared<TSAsyncEventDispatcher>();
	#else
	d->_Thread = TSThread::CreateInstance();
	d->_Thread->Start();
	#endif
	d->_HeartbeatTimer = d->_Thread->CreateTimer();

	boost::system::error_code ec;

	d->_Thread->Post(boost::bind(&TSSharedMemoryMiddleware::Heartbeat,this,ec));

	//向全局的共享内存消息队列中发送该前端的消息，在后端进行注册（后端起一个实例监听前端创建的发送消息共享内存消息队列，后端发送数据写入前端监听的共享内存消息队列中）
	TSMessageQueueSendItem Item;
	Item.Flag = TSMessageQueueSendItem::kInit;
	Item.Source = d->_AppId;

	d->_GlobalMq->send(&Item,sizeof(Item),0);

	return true;
}

void TSSharedMemoryMiddleware::Clean()
{
	T_D();

	TSMessageQueueSendItem Item;
	Item.Flag = TSMessageQueueSendItem::kClean;
	Item.Source = d->_AppId;

	d->_GlobalMq->send(&Item,sizeof(Item),0);

	d->_ListenThread->join();
	d->_ListenThread.reset();

	d->_Thread->Stop();
	#if defined(XSIM3_2) || defined(XSIM3_3)
	d->_EventDispatcherThread->join();
	d->_EventDispatcherThread.reset();
	#else
	d->_Thread->Join();
	d->_Thread.reset();
	#endif
	TSSegment::Instance()->Cleanup();
}

void TSSharedMemoryMiddleware::MakeAlive( const TSFrontAppUniquelyId& FrontAppId )
{
	UNUSED(FrontAppId);
}

void TSSharedMemoryMiddleware::MakeDead( const TSFrontAppUniquelyId& FrontAppId )
{
	UNUSED(FrontAppId);
}

bool TSSharedMemoryMiddleware::Subscribe( const TSDomainId& DomainId,TSTOPICHANDLE THandle,bool LocalOnly)
{
	T_D();
	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();
	Item->DomainId = TSValue_Cast<UINT32>(DomainId);
	Item->Topic = THandle;
	Item->LocalOnly = LocalOnly;
	d->SendMq(TSMessageQueueSendItem::kSublish,Item);

	return true;
}

bool TSSharedMemoryMiddleware::Publish( const TSDomainId& DomainId,TSTOPICHANDLE THandle )
{
	T_D();

	//构建了共享内存
	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();
	Item->DomainId = TSValue_Cast<UINT32>(DomainId);
	Item->Topic = THandle;
	
	d->SendMq(TSMessageQueueSendItem::kPublish,Item);

	return true;
}

void TSSharedMemoryMiddleware::Unsubscribe( const TSDomainId& DomainId,TSTOPICHANDLE THandle )
{
	T_D();

	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();
	Item->DomainId = TSValue_Cast<UINT32>(DomainId);
	Item->Topic = THandle;
	
	d->SendMq(TSMessageQueueSendItem::kUnsublish,Item);
}

void TSSharedMemoryMiddleware::Unpublish( const TSDomainId& DomainId,TSTOPICHANDLE THandle )
{
	T_D();

	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();
	Item->DomainId = TSValue_Cast<UINT32>(DomainId);
	Item->Topic = THandle;
	
	d->SendMq(TSMessageQueueSendItem::kUnpublish,Item);
}

bool TSSharedMemoryMiddleware::UpdateTopic( const TSDomainId& DomainId, TSTOPICHANDLE THandle, TSDataContextPtr Ctx )
{
	T_D();

	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();
	Item->DomainId = TSValue_Cast<UINT32>(DomainId);
	Item->Topic = THandle;
	MakeSharedMemoryDataContext(Item->Ctx,Ctx);

	d->SendMq(TSMessageQueueSendItem::kUpdateTopic,Item);

	return true;
	
}

bool TSSharedMemoryMiddleware::SendData( const TSDomainId& DomainId, TSTOPICHANDLE THandle, TSDataContextPtr Ctx,const TSFrontAppUniquelyId& Receiver )
{
	T_D();

	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();
	Item->DomainId = TSValue_Cast<UINT32>(DomainId);
	Item->Topic = THandle;
	Item->Receiver = Receiver;
	MakeSharedMemoryDataContext(Item->Ctx,Ctx);

	d->SendMq(TSMessageQueueSendItem::kUpdateTopic,Item);

	return true;
}

bool TSSharedMemoryMiddleware::DeleteTopic( const TSDomainId & DomainId,TSTOPICHANDLE THandle,TSDataContextPtr Ctx,bool IsNotice )
{
	T_D();

	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();
	Item->DomainId = TSValue_Cast<UINT32>(DomainId);
	Item->Topic = THandle;
	Item->IsNotice = IsNotice;
	MakeSharedMemoryDataContext(Item->Ctx,Ctx);

	d->SendMq(TSMessageQueueSendItem::kDeleteTopic,Item);

	return true;
}

bool TSSharedMemoryMiddleware::CreateDomain( const TSDomainId& DomainId ,const TSFrontAppUniquelyId& FrontAppUniquelyId )
{
	T_D();
	DEF_LOG_KERNEL(GB18030ToTSString("前端创建域->{DomainId:[%1]}\n")) << DomainId;
	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();
	Item->DomainId = TSValue_Cast<UINT32>(DomainId);

	d->SendMq(TSMessageQueueSendItem::kCreateDomain,Item);

	return true;
}

void TSSharedMemoryMiddleware::DestroyDomain( const TSDomainId& DomainId )
{
	T_D();

	TSTopicDataMqItemSharedPtr Item = MakeTopicDataShared();

	std::vector<TSString> Vec = TSStringUtil::Split(DomainId,"://",false);

	if(Vec.size() > 1)
	{
		Item->DomainId = TSValue_Cast<UINT32>(Vec[0]);
		strcpy(Item->DomainPartNames,Vec[1].c_str());
	}
	else
	{
		Item->DomainId = TSValue_Cast<UINT32>(DomainId);
	}

	

	d->SendMq(TSMessageQueueSendItem::kDestoryDomain,Item);
}

void TSSharedMemoryMiddleware::OutputLogger( UINT32 Catalog,const TSString & LoggerMsg )
{
	T_D();

	//todo
}

/* 根据用户Qos参数创建域 */
bool TSSharedMemoryMiddleware::CreateDomainWithQos(const TSDomainId & domainId, const ParticipantQosPolicy & participantQosPolicy, const TSFrontAppUniquelyId& FrontAppUniquelyId)
{
	return true;
}

/* 获取参与者Qos */
bool TSSharedMemoryMiddleware::GetParticipantQosPolicy(const TSDomainId & domainId, ParticipantQosPolicy & participantQosPolicy)
{
	return true;
}
/* 设置参与者Qos */
bool TSSharedMemoryMiddleware::SetParticipantQosPolicy(const TSDomainId & domainId, const ParticipantQosPolicy & participantQosPolicy)
{
	return true;
}

/* 获取数据读者Qos */
bool TSSharedMemoryMiddleware::GetDataReaderQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy)
{
	return true;
}
/* 设置数据读者Qos */
bool TSSharedMemoryMiddleware::SetDataReaderQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy)
{
	return true;
}

/* 获取数据写者Qos */
bool TSSharedMemoryMiddleware::GetDataWriterQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy)
{
	return true;
}
/* 设置数据写者Qos */
bool TSSharedMemoryMiddleware::SetDataWriterQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, const DataWriterQosPolicy & dataWriterQosPolicy)
{
	return true;
}

void TSSharedMemoryMiddleware::Listen()
{
	T_D();
	DEF_LOG_KERNEL(GB18030ToTSString("前端启动监听线程->{AppId:[%1]}\n")) << d->_AppId;

	while(!d->_Stoped)
	{
		try
		{
			TSMessageQueueSendItem Item;

			size_t ReceivedSize;
			UINT32 Pri;

			size_t count = 0;
			//循环去接收后端共享内存消息队列中数据
			while(!d->_ListenMq->try_receive(&Item,sizeof(TSMessageQueueSendItem),ReceivedSize,Pri))
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

			if(Item.Flag == TSMessageQueueSendItem::kClean)
			{
				DEF_LOG_KERNEL(GB18030ToTSString("前端收到清理消息->{Flag:[kClean], AppId:[%1]}\n")) << d->_AppId;
				//退出
				d->_Stoped = true;
				
				d->_BackendExchangedItem->_ExchangedData1.reset();
				d->_BackendExchangedItem->_ExchangedData2.reset();
				d->_BackendExchangedItem->_ExchangedData3.reset();

				d->SendMq(TSMessageQueueSendItem::kClean);

				break;
			}
			else if(Item.Flag == TSMessageQueueSendItem::kData)
			{
				ASSERT(d->_BackendExchangedItem);

				TSTopicDataMqItemSharedPtr & ExchangedData = 
					Item.ExchangedDataIndex == 1 ? 
					d->_BackendExchangedItem->_ExchangedData1 : Item.ExchangedDataIndex == 2 ? 
					d->_BackendExchangedItem->_ExchangedData2 : d->_BackendExchangedItem->_ExchangedData3;

				ASSERT(ExchangedData);

				//数据操作
				if(Item.SubFlag == TSMessageQueueSendItem::kUpdateTopic)
				{
					DEF_LOG_KERNEL(GB18030ToTSString("收到来自后端的数据消息->{Falg:[kData]，SubFlag:[kUpdateTopic]}\n"));
					TSSerializer Ser(MakeBufferPtr(ExchangedData->Ctx.Data.get(),ExchangedData->Ctx.Length));
					TopicCallback(TSValue_Cast<TSString>(ExchangedData->DomainId),ExchangedData->Topic,Ser.GetDataContext());

				}
				else if(Item.SubFlag == TSMessageQueueSendItem::kDeleteTopic)
				{
					DEF_LOG_KERNEL(GB18030ToTSString("收到来自后端的数据消息->{Falg:[kData]，SubFlag:[kDeleteTopic]}\n"));
					TSDataContextPtr Ctx = new TSDataContext();
					Ctx->SetKeyHash(ExchangedData->Ctx.KeyHash);
					RemoveCallback(TSValue_Cast<TSString>(ExchangedData->DomainId),ExchangedData->Topic,Ctx);
				}
				else if(Item.SubFlag == TSMessageQueueSendItem::kUpdateTopic)
				{
					if(ExchangedData->Receiver == d->_AppId)
					{
						TSSerializer Ser(MakeBufferPtr(ExchangedData->Ctx.Data.get(),ExchangedData->Ctx.Length));
						TopicCallback(TSValue_Cast<TSString>(ExchangedData->DomainId),ExchangedData->Topic,Ser.GetDataContext());
					}
				}

				//非常重要
				ExchangedData.reset();
			}
		}
		catch(const boost::interprocess::interprocess_exception & e)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSSHAREDMEMORYMIDDLEWARE_CPP_383) << e.what();
			d->_Stoped = true;
		}
	}
}

void TSSharedMemoryMiddleware::Heartbeat(const boost::system::error_code & ec)
{
	T_D();

	if(!ec)
	{
		TSMessageQueueSendItem Item;
		Item.Flag = TSMessageQueueSendItem::kHeartbeat;
		Item.Source = d->_AppId;
		d->_GlobalMq->send(&Item,sizeof(Item),0);

		d->_HeartbeatTimer->AsyncCall(100,boost::bind(&TSSharedMemoryMiddleware::Heartbeat,this,boost::asio::placeholders::error));
	}
}

bool TSSharedMemoryMiddleware::WaitForDiscoveryComplete( const TSDomainId & DomainId, const TSTopicQuerySet & QuerySet,const TSFrontAppUniquelyId & AppId,UINT32 Timeout ,TSWaitSet::WaitMode Mode)
{
	//todo
	return false;
}

void TSSharedMemoryMiddleware::EventDispatcherThread_()
{
	T_D();

#if defined(XSIM3_2) || defined(XSIM3_3)
	d->_Thread->Run();
#endif
}

#endif //#ifndef  _WRS_KERNEL
