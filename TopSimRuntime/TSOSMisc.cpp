#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#include "TSDiscovery.h"
#include "TSDataInteractManager.h"
#include "TSDomain.h"
#include "TSAppStatus/Topic.h"


#undef BOOST_ASIO_HAS_STD_CHRONO

#include <boost/asio/high_resolution_timer.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/chrono/duration.hpp>

#ifdef BOOST_ASIO_HAS_STD_CHRONO
using namespace std::chrono;
#elif defined(BOOST_ASIO_HAS_BOOST_CHRONO)
using namespace boost::chrono;
#endif

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimTypes/TSExitSpinLock.h>
#include <TopSimTypes/TSAsyncEventDispatcher.h>
typedef boost::shared_ptr<TSAsyncEventDispatcher> TSThreadPtr;
typedef boost::shared_ptr<TSIAsyncTimer> TSIAsyncTimerPtr;
#else
#include <TopSimRuntime/TSLogsUtil.h>
#include <TopSimRuntime/TSExitSpinLock.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#define BUILTIN_APPINFO "BUILTIN_APPINFO"

struct TSDiscoveryAppStruct
{
	TSAppStatusParam::DataType Param;
	TSDiscovery::TSDiscoveryCallback_T Callback;
};

STRUCT_PTR_DECLARE(TSDiscoveryAppStruct);

typedef TSConUnorderedMap<TSFrontAppUniquelyId,TSAppStatus::DataType> TSAppStatusMap_T;
typedef TSConUnorderedMap<TSString,TSDiscoveryAppStructPtr> TSDiscoveryAppStructMap_T;

struct TSDiscoveryPrivate
{
	TSDiscoveryPrivate()
		:_HeartbeatInterval(300000)
		,_EnabledHeartbeat(false)
		,_IsChanged(false)
	{
		
	}

	enum Status
	{
		Online,
		Offline,
	};

	void Update()
	{
		if(TSDomainPtr Domain = _Domain.lock())
		{
			_LocalStatus.Status = TSDiscoveryPrivate::Online;

			UpdateOSMisc();

			Domain->UpdateTopic(TOPIC_APP_STATUS,&_LocalStatus);
		}
	}

	void SendDirect(const TSFrontAppUniquelyId& FrontAppUniquelyId)
	{
		if(TSDomainPtr Domain = _Domain.lock())
		{
			_LocalStatus.Status = TSDiscoveryPrivate::Online;
			Domain->SendTopicDirect(TOPIC_APP_STATUS,&_LocalStatus,FrontAppUniquelyId);
		}
	}

	void Stop()
	{
		if(TSDomainPtr Domain = _Domain.lock())
		{
			_LocalStatus.Status = TSDiscoveryPrivate::Offline;
			Domain->UpdateTopic(TOPIC_APP_STATUS,&_LocalStatus);
		}
	}

	void UpdateHeartbeat()
	{
		Update();
	}

	void UpdateOSMisc()
	{
		static TSString Ip;
		static TSString Host;

		if(Ip.empty() && Host.empty())
		{
			boost::system::error_code ec;

			Host = boost::asio::ip::host_name(ec);

			if (!ec)
			{
			
				boost::asio::io_service io_service;

				boost::asio::ip::tcp::resolver resolver(io_service);

				boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(boost::asio::ip::tcp::resolver::query(Host,""),ec);


				if (!ec)
				{
					while (iterator != boost::asio::ip::tcp::resolver::iterator())
					{
						boost::asio::ip::address Addr = iterator->endpoint().address();

						if(Addr.is_v4() && !Addr.is_loopback() && !Addr.is_multicast() && !Addr.is_unspecified())
						{
							Ip = Addr.to_string(ec);

							if (!ec)
							{
								break;
							}
						}

						++iterator;
					}
				}
			}
		}



		static TSString ProcessName = TSOSMisc::GetProcessName();

		std::vector<UINT32> CPUUsages;
		TSOSMisc::GetCPUUsage(CPUUsages);

		UINT64 TotalPhysMemory;
		UINT64 AvailPhysMemory;

		TSOSMisc::GetPhysMemoryInfo(TotalPhysMemory,AvailPhysMemory);

		static UINT32 ProcessId = TSOSMisc::GetCurrentProcessId();

		TSByteBufferPtr WriteBuffer = MakeBufferPtr();

		WriteBuffer->WriteNetString(Ip);
		WriteBuffer->WriteNetString(Host);
		WriteBuffer->WriteUInt32(ProcessId);
		WriteBuffer->WriteUInt64(AvailPhysMemory);
		WriteBuffer->WriteUInt64(TotalPhysMemory);
		WriteBuffer->WriteNetString(ProcessName);
		WriteBuffer->WriteUInt32((UINT32)CPUUsages.size());
		for(size_t i = 0; i < CPUUsages.size(); ++i)
		{
			WriteBuffer->WriteUInt32(CPUUsages[i]);
		}

		std::vector<TSAppStatusParam::DataType>::iterator it = 
			_LocalStatus.Params.begin();

		bool IsFound = false;

		while(it != _LocalStatus.Params.end())
		{
			if(it->AppName == BUILTIN_APPINFO)
			{
				IsFound = true;

				BufferToU8(it->AppBuffer,WriteBuffer->Data(),WriteBuffer->Length());

				break;
			}

			++it;
		}

		if(!IsFound)
		{
			TSAppStatusParam::DataType Data;
			Data.AppName = BUILTIN_APPINFO;
			BufferToU8(Data.AppBuffer,WriteBuffer->Data(),WriteBuffer->Length());
			_LocalStatus.Params.push_back(Data);
		}
	}

	TSDomainWPtr _Domain; 
    TSRWSpinLock _LocalStatusLock;
	TSAppStatus::DataType _LocalStatus;
	UINT32 _TopicStub;
    UINT32 _HeartbeatInterval;
	TSExitSpinLock _ExitLock;
	bool _EnabledHeartbeat;
	boost::atomic_bool _IsChanged;
	TSAppStatusMap_T _AppStatusMap;
	TSConUnorderedMap<TSString,TSDiscovery::TSDiscoveryCallback_T> _Callbacks;

	TSThreadPtr      _Thread;
	TSIAsyncTimerPtr _Timer;
	boost::shared_ptr<boost::thread> _EventDispatcherThread;
    
};

TSDiscovery::TSDiscovery(TSDomainPtr Domain)
	:_p(new TSDiscoveryPrivate)
{
   _p->_Domain = Domain;

   if(TSDomainPtr Domain = _p->_Domain.lock())
   {
	   TSString EnabledString = TSTopicTypeManager::Instance()->GetExtendCfg("EnabledHeartbeat");
	   if(!EnabledString.empty())
	   {
		   _p->_EnabledHeartbeat = TSValue_Cast<bool>(EnabledString);
	   }

	   Domain->PublishTopic(TOPIC_APP_STATUS);
	   Domain->SubscribeTopic(TOPIC_APP_STATUS);

	   _p->_TopicStub = Domain->RegisterTopicCallback(TOPIC_APP_STATUS,boost::bind(&TSDiscovery::OnAppStatusUpdate,this,_1),false);

	   TSString Interval = TSTopicTypeManager::Instance()->GetExtendCfg("HeartbeatInterval");
	   if(!Interval.empty())
	   {
		   _p->_HeartbeatInterval = TSValue_Cast<UINT32>(Interval);
	   }

	   _p->_LocalStatus.Id = DataManager->GetFrontAppUniquelyId();
	   _p->_LocalStatus.DomainId = Domain->GetDomainId();
	   _p->_LocalStatus.Status = TSDiscoveryPrivate::Online;

	   _p->UpdateOSMisc();

	   Domain->UpdateTopic(TOPIC_APP_STATUS,&_p->_LocalStatus);

	   if(_p->_EnabledHeartbeat)
	   {
		   #if defined(XSIM3_2) || defined(XSIM3_3)
		   _p->_EventDispatcherThread = boost::make_shared<boost::thread>(boost::bind(&TSDiscovery::EventDispatcherThread_,this));
		   _p->_Thread =  boost::make_shared<TSAsyncEventDispatcher>();
		   #else
		   _p->_Thread = TSThread::CreateInstance();
		   _p->_Thread->Start();
		   #endif
		   
		   _p->_Timer = _p->_Thread->CreateTimer();

		   boost::system::error_code ec;
		   _p->_Thread->Post(boost::bind(&TSDiscovery::ProcessOnLineTimer,this,ec));
	   }
   }
}

TSDiscovery::~TSDiscovery(void)
{
	if(_p->_Thread)
	{
		if(_p->_Timer)
		{
			_p->_Timer->CancelCurrentCall();
			_p->_Timer->Reset();
		}

		_p->_Thread->Stop();
		#if defined(XSIM3_2) || defined(XSIM3_3)
		_p->_EventDispatcherThread->join();
		_p->_EventDispatcherThread.reset();
		#else
		_p->_Thread->Join();
		_p->_Thread.reset();
		#endif
	}

	delete _p;
}

bool TSDiscovery::RegisterApp( const TSString& AppName,TSDiscoveryCallback_T Callback ,TSByteBufferPtr AppBuffer/*= TSDiscoveryCallback_T()*/ )
{
	DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSDISCOVERY_CPP_267) << AppName;

	TSAppStatusParam::DataType Param;
	Param.AppName = AppName;

	if(AppBuffer)
	{
		BufferToU8(Param.AppBuffer,AppBuffer->Data(),AppBuffer->Length());
	}

	for(size_t i = 0; i < _p->_LocalStatus.Params.size(); ++i)
	{
		if(_p->_LocalStatus.Params[i].AppName == AppName)
		{
            DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDISCOVERY_CPP_281) << Param.AppName;
			return false;
		}
	}

	_p->_LocalStatus.Params.push_back(Param);
	_p->_Callbacks.insert(std::make_pair(AppName,Callback));

	if(Callback)
	{
		TSAppStatusMap_T::iterator it = _p->_AppStatusMap.begin();
		while(it != _p->_AppStatusMap.end())
		{
			Callback(TSDiscovery::AppOnline,it->second);
			++it;
		}
	}

	_p->_IsChanged = true;
	_p->Update();

	return true;
}

void TSDiscovery::UnregisterApp( const TSString& AppName )
{
	std::vector<TSAppStatusParam::DataType>::iterator it = 
		_p->_LocalStatus.Params.begin();
	while(it != _p->_LocalStatus.Params.end())
	{
		if(it->AppName == AppName)
		{
			it = _p->_LocalStatus.Params.erase(it);
			break;
		}
		else
		{
			++it;
		}
	}
	
	_p->_IsChanged = true;
	_p->Update();
}

bool TSDiscovery::UpdateApp( const TSString& AppName,TSByteBufferPtr AppBuffer )
{
    TSRWSpinLock::ReadLock lock(_p->_LocalStatusLock);
	std::vector<TSAppStatusParam::DataType>::iterator it = 
		_p->_LocalStatus.Params.begin();
	while(it != _p->_LocalStatus.Params.end())
	{
		if(it->AppName == AppName)
		{
			BufferToU8(it->AppBuffer,AppBuffer->Data(),AppBuffer->Length());
			break;
		}
		else
		{
			++it;
		}
	}

	_p->_IsChanged = true;

	return true;
}

void TSDiscovery::ProcessOnLineTimer(const boost::system::error_code& ec)
{
	if(!ec)
	{
		_p->UpdateHeartbeat();

		if(_p->_Timer)
		{
			_p->_Timer->AsyncCall(_p->_HeartbeatInterval,boost::bind(&TSDiscovery::ProcessOnLineTimer,shared_from_this(),boost::asio::placeholders::error));
		}
	}
}

void TSDiscovery::ProcessAppStatus_p(const TSAppStatus::DataType& Status )
{
	if(TSExitSpinLock::Lock lock = _p->_ExitLock)
	{
		if(TSDomainPtr Domain = _p->_Domain.lock())
		{
			if(Status.Id != DataManager->GetFrontAppUniquelyId()
				&& Status.DomainId == Domain->GetDomainId())
			{
				switch(Status.Status)
				{
				case TSDiscoveryPrivate::Online:
					{
						_p->_AppStatusMap[Status.Id] = Status;

						TSConUnorderedMap<TSString,TSDiscoveryCallback_T>::iterator it = 
							_p->_Callbacks.begin();

						while(it != _p->_Callbacks.end())
						{
							if(it->second)
							{
								it->second(TSDiscovery::AppOnline,Status);
							}

							++it;
						}
					}
					break;
				case TSDiscoveryPrivate::Offline:
					{
						TSAppStatusMap_T::iterator it = 
							_p->_AppStatusMap.find(Status.Id);

						if(it != _p->_AppStatusMap.end())
						{
							_p->_AppStatusMap.unsafe_erase(it);
						}

						TSConUnorderedMap<TSString,TSDiscovery::TSDiscoveryCallback_T>::iterator CallbackIt = 
							_p->_Callbacks.begin();

						while(CallbackIt != _p->_Callbacks.end())
						{
							if(CallbackIt->second)
							{
								CallbackIt->second(TSDiscovery::AppOffline,Status);
							}

							++CallbackIt;
						}

					}
					break;
				}
			}
		}

		
	}
}

void TSDiscovery::OnAppStatusUpdate(TSTopicContextPtr Ctx )
{
	ProcessAppStatus_p(*TS_STATIC_CAST(Ctx->GetTopic(),TSAppStatus::DataTypePtr));
}

void TSDiscovery::Stop()
{
	_p->_ExitLock.Exit();

	if(_p->_Timer)
	{
		_p->_Timer->Reset();
		_p->_Timer.reset();
	}

	_p->Stop();

	if(TSDomainPtr Domian = _p->_Domain.lock())
	{
		Domian->UnpublishTopic(TOPIC_APP_STATUS);
		Domian->UnsubscribeTopic(TOPIC_APP_STATUS);
		Domian->UnregisterTopicCallback(TOPIC_APP_STATUS,_p->_TopicStub);
	}
}

void TSDiscovery::EventDispatcherThread_()
{
#if defined(XSIM3_2) || defined(XSIM3_3)
	_p->_Thread->Run();
#endif
	
}

