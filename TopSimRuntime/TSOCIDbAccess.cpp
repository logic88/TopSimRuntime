#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimUtil/pugixml.hpp>
#include <TopSimUtil/TSDynamicLib.h>
#else
#include <TopSimRuntime/TSProductLicense.h>
#include <TopSimRuntime/TSLogsUtil.h>
#include <TopSimRuntime/pugixml.hpp>
#include <TopSimRuntime/TSDynamicLib.h>

#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TSCmd/Defined.h"

#include "TSDataInteractManager.h"
#include "TSDataInteractManager_p.h"
#include "TSTopicTypeManager.h"
#include "TSITransferMiddleware.h"

const TSFrontAppUniquelyId& TSDataNotifyDelegate::GetFrontAppUniquelyId()
{
    return _p->_AppId;
}

void TSDataNotifyDelegate::OnReceivedTopic(TSDomainPtr Domain,TSTopicContextPtr Ctx)
{
    if(ReceivedTopicCallback)
    {
        ReceivedTopicCallback(Domain,Ctx);
    }
}


void TSDataNotifyDelegate::OnReceivedTopicRemove( TSDomainPtr Domain,TSTopicContextPtr Ctx )
{
	if(ReceivedTopicRemoveCallback)
	{
		ReceivedTopicRemoveCallback(Domain,Ctx);
	}
}

void TSDataNotifyDelegate::OnReceivedTopicMatched(TSDomainPtr Domain, const TSDomainId& DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub)
{
	if(ReceivedTopicMatchedCallback)
	{
		ReceivedTopicMatchedCallback(Domain, DomainId, Thandle, AppId, IsPub);
	}
}

void TSDataNotifyDelegate::OnReceivedLeaveDomain(TSDomainPtr Domain, const TSDomainId& DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub)
{
	if (ReceivedLeaveDomainCallback)
	{
		ReceivedLeaveDomainCallback(Domain, DomainId, Thandle, AppId, IsPub);
	}
}


bool TSDataNotifyDelegate::OnContentFilterTopic(TSDomainPtr Domain, TSTopicContextPtr Ctx, UINT32 AppId)
{
	if (ContentFilterCallback)
	{
		return ContentFilterCallback(Domain, Ctx, AppId);
	}

	return true;
}

SINGLETON_IMPLEMENT(TSDataInteractManager);

TSDataInteractManager::TSDataInteractManager()
    :_Private_Ptr(new PrivateType)
{

}

/* DataManager初始化 */
bool TSDataInteractManager::Initialize(int argc,char ** argv,const TSFrontAppUniquelyId& FrontAppId)
{
    T_D();

	//是否初始化过
    if(!d->_HasInit)
    {
		//标记为初始化过
        d->_HasInit = true;

		//设置进程应用ID
        TSFrontAppUniquelyId AppId = FrontAppId;  //TSFrontAppUniquelyId()
        if(AppId == 0 || AppId == TS_INVALID)
        {
			if(d->_AppName.empty())
			{
				d->_AppName = TSValue_Cast<TSString>(GetCRC32(TSOSMisc::GenerateUUID()));
			}
			
			AppId = GetCRC32(TSString2Utf8(d->_AppName));
        }

        d->_AppId = AppId;

        TSOutDbgPrintf(TS_TEXT("FrontAppUniquelyId is %u\n"),AppId._value);

        d->_DataDelegate = boost::make_shared<TSDataNotifyDelegate>(_Private_Ptr);
        d->_DataDelegate->ReceivedTopicCallback = boost::bind(&TSDataInteractManager::OnDomainReceivedTopic_p,this,_1,_2);
		d->_DataDelegate->ReceivedTopicRemoveCallback = boost::bind(&TSDataInteractManager::OnDomainReceivedTopicRemoved_p, this, _1, _2);
		d->_DataDelegate->ReceivedTopicMatchedCallback = boost::bind(&TSDataInteractManager::OnDomainReceivedTopicMatched_p, this, _1, _2, _3, _4, _5);
		d->_DataDelegate->ReceivedLeaveDomainCallback = boost::bind(&TSDataInteractManager::OnDomainReceivedLeaveDomain_p, this, _1, _2, _3, _4, _5);
		d->_DataDelegate->ContentFilterCallback = boost::bind(&TSDataInteractManager::OnDomainContentFilter_p, this, _1, _2, _3);

        try
        {
			//加载Config/Middleware配置
            if(!Configure(argc,argv))
            {
                d->_HasInit = false;

                return false;
            }

			BuiltinTopicRemovedHandle = __topic_internal_remove__;

            d->_NotifyIOS = boost::make_shared<TSIOSStruct>();

			//循环初始化所有Monitor
            BOOST_FOREACH(TSIMonitorPtr Mointor,d->_Monitors)
            {
                Mointor->Initialize();
            }			
            return true;
        }
        catch(TSException &exp)
        {
            DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_104)<< exp.what();
        }
        catch(...)
        {
            DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_108) ;
        }        

        d->_HasInit = false;

        return false;
    }

    return true;
}

bool TSDataInteractManager::Initialize(const TSFrontAppUniquelyId& FrontAppId /*= TSFrontAppUniquelyId()*/)
{
	return Initialize(0, 0, FrontAppId);
}

void TSDataInteractManager::Cleanup(const TSDomainId& Id)
{

    T_D();

	if(d->_HasInit)
	{
		BOOST_FOREACH(TSIMonitorPtr Mointor,d->_Monitors)
		{
			Mointor->Clean();
		}

		d->_Monitors.clear();

		if(d->_TransferLinker)
		{
			d->_TransferLinker->Clean();
		}

		d->_HasInit = false;
	}
}

TopSimDataInterface::ReturnCode
TSDataInteractManager::Subscribe(const TSDomainId& DomainId, TSTOPICHANDLE THandle, bool LocalOnly)
{
    T_D();

    if(d->_TransferLinker)
    {
        if(!d->_TransferLinker->Subscribe(DomainId,THandle,LocalOnly))
        {
			return TopSimDataInterface::Failed;
        }

        BOOST_FOREACH(TSIMonitorPtr Monitor,d->_Monitors)
        {
			if (Monitor->IsEnable())
			{
				Monitor->OnSubscribeTopic(DomainId, THandle);
			}
        }

        return TopSimDataInterface::Ok;
    }

    return TopSimDataInterface::Failed;
}

TopSimDataInterface::ReturnCode 
TSDataInteractManager::Publish( const TSDomainId& DomainId,TSTOPICHANDLE THandle)
{
    T_D();

    if(d->_TransferLinker)
    {
        if(!d->_TransferLinker->Publish(DomainId,THandle))
        {
            return TopSimDataInterface::Failed;
        }

        BOOST_FOREACH(TSIMonitorPtr Monitor,d->_Monitors)
        {
			if (Monitor->IsEnable())
			{
				Monitor->OnPublishTopic(DomainId, THandle);
			}
        }

        return TopSimDataInterface::Ok;
    }

    return TopSimDataInterface::Failed;
}

TopSimDataInterface::ReturnCode 
TSDataInteractManager::Unsubscribe( const TSDomainId& DomainId,TSTOPICHANDLE THandle)
{
    T_D();

    if(d->_TransferLinker)
    {
        d->_TransferLinker->Unsubscribe(DomainId,THandle);

        BOOST_FOREACH(TSIMonitorPtr Monitor,d->_Monitors)
        {
			if (Monitor->IsEnable())
			{
				Monitor->OnUnsubscribeTopic(DomainId, THandle);
			}
        }

        return TopSimDataInterface::Ok;
    }

    return TopSimDataInterface::Failed;
}

TopSimDataInterface::ReturnCode 
TSDataInteractManager::Unpublish( const TSDomainId& DomainId,TSTOPICHANDLE THandle)
{
    T_D();

    if(d->_TransferLinker)
    {
        d->_TransferLinker->Unpublish(DomainId,THandle);

        BOOST_FOREACH(TSIMonitorPtr Monitor,d->_Monitors)
        {
			if (Monitor->IsEnable())
			{
				Monitor->OnUnpublishTopic(DomainId, THandle);
			}
        }

        return TopSimDataInterface::Ok;
    }

    return TopSimDataInterface::Failed;
}

TSTopicContextPtr TSDataInteractManager::UpdateTopic( const TSDomainId& DomainId,TSTOPICHANDLE THandle,
    const void * Data,const TSTime & Time, bool IsNeedSerializer)
{
	//TSDataInteractManagerPrivate
	T_D();

    if(d->_TransferLinker)
    {
        TSTopicContextPtr Ctx = d->_TransferLinker->UpdateTopic(DomainId,THandle,Data,Time,IsNeedSerializer);

        BOOST_FOREACH(TSIMonitorPtr Monitor,d->_Monitors)
        {
			if (Monitor->IsEnable())
			{
				if (!Monitor->IsUpdateTopicCycle(THandle))
				{
					Monitor->OnUpdateTopic(DomainId, Ctx);
				}
			}
        }

        return Ctx;
    }

    return TSTopicContextPtr();
}

TSTopicContextPtr TSDataInteractManager::SendData( const TSDomainId& DomainId,TSTOPICHANDLE THandle,
    const void * Parameter,const TSTime & Time, const TSFrontAppUniquelyId& ReceiverId)
{
    T_D();

    if(d->_TransferLinker)
    {
        TSTopicContextPtr Ctx = d->_TransferLinker->SendData(DomainId,THandle,Parameter,Time,ReceiverId);

        BOOST_FOREACH(TSIMonitorPtr Monitor,d->_Monitors)
        {
			if (Monitor->IsEnable())
			{
				Monitor->OnUpdateTopic(DomainId, Ctx);
			}
        }

        return Ctx;
    }

	return TSTopicContextPtr();
}

bool TSDataInteractManager::Configure(int argc,char ** argv)
{
    T_D();

	//反射构建监控对象
	{
		//遍历监控发布端
		std::vector<TSString> Mointors;

		pugi::xpath_node_set pNodes = GetCfgDoc().select_nodes(TS_TEXT("Config/MonitorPS"));
		for (pugi::xpath_node_set::const_iterator it = pNodes.begin(); it != pNodes.end(); ++it)
		{
			if (it->node() && it->node().attribute("Enabled").as_bool())
			{
				TSString Realized = it->node().attribute("Realized").as_string();
				if (!Realized.empty()
					&& std::find(Mointors.begin(), Mointors.end(), Realized)
					== Mointors.end())
				{
					Mointors.push_back(Realized);
				}
			}
		}

		//类反射构建监控对象
		BOOST_FOREACH(TSString& Realized, Mointors)
		{
			int Type = TSMetaType::GetType(Realized.c_str());
			if (!Type)
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_299) << Realized;
			}
			else if (TSMetaType::IsAbstract(Type))
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_303) << Realized;
			}
			else
			{
				if (TSIMonitorPtr Mointor = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(Type), TSIMonitorPtr))
				{
					DEF_LOG_KERNEL(GB18030ToTSString("创建监控对象成功：%1")) << Realized;
					d->_Monitors.push_back(Mointor);
				}
			}
		}
	}

	TSString RealizedTypeName; 
	//反射构建Link对象
	if(!d->_TransferLinker)
	{
		//RealizedTypeName = TSTopicTypeManager::Instance()->GetExtendCfg("TransferLinker");
		if(RealizedTypeName.empty())
		{
			if(pugi::xml_node node = GetCfgDoc().select_single_node(TS_TEXT("Config/TransferLinker")).node())
			{
				RealizedTypeName = node.attribute(TS_TEXT("Realized")).as_string();
			}
		}

		if(RealizedTypeName.empty())
		{
			RealizedTypeName = TS_TEXT("TSTransferProxy");
		}

		if(!RealizedTypeName.empty())
		{
			if(int MetaType = TSMetaType::GetType(RealizedTypeName.c_str()))
			{
				DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_334) << RealizedTypeName;
				d->_TransferLinker = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(MetaType),TSITransferLinkerPtr);
			}
		}
	}

    if(d->_TransferLinker)
    {
        if(d->_TransferLinker->Initialize(argc,argv,d->_DataDelegate.get()))
        {
			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_344) << RealizedTypeName;
            return true;
        }
        else
        {
            d->_TransferLinker.reset();
            throw TSException(TSString(TS_TEXT("TransferLinker ")) + RealizedTypeName + TS_TEXT(CN_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_350) );
        }
    }
    else
    {
        if(RealizedTypeName.empty())
        {
            throw TSException(TS_TEXT(CN_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_357));
        }
        else
        {
            throw TSException(TSString(TS_TEXT(CN_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_361)) + RealizedTypeName + TS_TEXT(CN_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_361_1));
        }
    }

    return false;
}

TSDomainPtr TSDataInteractManager::CreateDomain( const TSDomainIdOrPartName& DomainId )
{
    T_D();

    if(d->_TransferLinker)
    {
		//使用LINK指针创建域
        if (TSDomainPtr Domain = d->_TransferLinker->CreateDomain(DomainId,d->_AppId))
        {
			//域创建成功后，设置用户标识
			Domain->SetUserFlag(GetUserFlag());
			//初始化域
			Domain->Initialize();

			BOOST_FOREACH(TSIMonitorPtr Monitor,d->_Monitors)
			{
				//判断是否启动监控功能
				if (Monitor->IsEnable())
				{
					Monitor->OnCreateDomain(Domain);
				}
			}
            return Domain;
        }
    }

    return TSDomainPtr();
}

void TSDataInteractManager::EvokeCallbacks()
{
    T_D();

	d->_IsStoped = false;

#if defined(XSIM3_3) || defined(XSIM3_2)
	if (d->_NotifyIOS
		&& d->_NotifyIOS->GetIOS())
	{
		if(d->_NotifyIOS->GetIOS()->stopped())
		{
			d->_NotifyIOS->GetIOS()->reset();
		}

		boost::system::error_code ec;
		d->_NotifyIOS->GetIOS()->poll(ec);

		if(ec)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_407) << ec.message();
		}
	}
#else
	if(d->_NotifyIOS
		&& d->_NotifyIOS->GetIOS())
	{
// 		if(!d->_NotifyIOS->GetIOS()->Runing())
// 		{
// 			d->_NotifyIOS->GetIOS()->Reset();
// 		}

		d->_NotifyIOS->GetIOS()->Poll();
	}
#endif // defined(XSIM3_3) || defined(XSIM3_2)
   
}

void TSDataInteractManager::EvokeCallback()
{
    T_D();

	d->_IsStoped = false;

#if defined(XSIM3_3) || defined(XSIM3_2)
	if (d->_NotifyIOS
		&& d->_NotifyIOS->GetIOS())
	{
		if(d->_NotifyIOS->GetIOS()->stopped())
		{
			d->_NotifyIOS->GetIOS()->reset();
		}

		boost::system::error_code ec;
		d->_NotifyIOS->GetIOS()->poll_one(ec);

		if(ec)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDATAINTERACTMANAGER_CPP_428) << ec.message();
		}
	}
#else
	if (d->_NotifyIOS
		&& d->_NotifyIOS->GetIOS())
	{
// 		if(!d->_NotifyIOS->GetIOS()->Runing())
// 		{
// 			d->_NotifyIOS->GetIOS()->Reset();
// 		}

		d->_NotifyIOS->GetIOS()->PollOne();
	}
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

void TSDataInteractManager::Stop()
{
    T_D();

    if(d->_NotifyIOS)
    {
        d->_NotifyIOS->Stop();
    }

	d->_IsStoped = true;
}

void TSDataInteractManager::EvokeCallbacksAlways()
{
    T_D();

	d->_IsStoped = false;

#if defined(XSIM3_3) || defined(XSIM3_2)
	if (d->_NotifyIOS)
	{
		if(d->_NotifyIOS->GetIOS() && d->_NotifyIOS->GetIOS()->stopped())
		{
			d->_NotifyIOS->GetIOS()->reset();
		}

		d->_NotifyIOS->RunAlwaysBlock();
	}
#else
	if (d->_NotifyIOS)
	{
// 		if(!d->_NotifyIOS->GetIOS()->Runing())
// 		{
// 			d->_NotifyIOS->GetIOS()->Reset();
// 		}

		d->_NotifyIOS->RunAlwaysBlock();
	}
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

void TSDataInteractManager::EvokeCallbacksAlwaysNoBlock()
{
    T_D();

	d->_IsStoped = false;

#if defined(XSIM3_3) || defined(XSIM3_2)
	if(d->_NotifyIOS)
	{
		if (d->_NotifyIOS->GetIOS() && d->_NotifyIOS->GetIOS()->stopped())
		{
			d->_NotifyIOS->GetIOS()->reset();
		}

		d->_NotifyIOS->RunAlways();
	}
#else
	if(d->_NotifyIOS)
	{
// 		if(!d->_NotifyIOS->GetIOS()->Runing())
// 		{
// 			d->_NotifyIOS->GetIOS()->Reset();
// 		}
		//启动一个线程非阻塞运行
		d->_NotifyIOS->RunAlways();
	}
#endif // defined(XSIM3_3) || defined(XSIM3_2)
  
}

TSFrontAppUniquelyId TSDataInteractManager::GetFrontAppUniquelyId()
{
    T_D();

    return d->_AppId;
}

/* 设置应用名称 */
void TSDataInteractManager::SetAppName( const TSString & AppName )
{
    T_D();

    d->_AppName = AppName;

	//更新主机和应用信息
	BOOST_FOREACH(TSIMonitorPtr Monitor, d->_Monitors)
	{
		if (Monitor->IsEnable())
		{
			Monitor->UpdateHostAppInfo();
		}
	}
}

const TSString& TSDataInteractManager::GetAppName()
{
    T_D();

    return d->_AppName;
}

void TSDataInteractManager::SetAppAuthor( const TSString & AppAuthor )
{
    T_D();

    d->_AppAuthor = AppAuthor;
}

const TSString& TSDataInteractManager::GetAppAuthor()
{
    T_D();

    return d->_AppAuthor;
}

void TSDataInteractManager::SetAppVersion( const TSString & AppVersion )
{
    T_D();

    d->_AppVersion = AppVersion;
}

const TSString& TSDataInteractManager::GetAppVersion()
{
    T_D();

    return d->_AppVersion;
}

void TSDataInteractManager::SetAppDescription( const TSString & Description )
{
    T_D();

    d->_Description = Description;
}

const TSString& TSDataInteractManager::GetAppDescription( )
{
    T_D();

    return d->_Description;
}

void TSDataInteractManager::SetUserData( TSVariant & UserData )
{
    T_D();

    d->_UserData = UserData;
}

const TSVariant & TSDataInteractManager::GetUserData()
{
    T_D();

    return d->_UserData;
}


TSDomainPtr TSDataInteractManager::GetDomain( const TSDomainId& Id )
{
    T_D();

    if(d->_TransferLinker)
    {
        return d->_TransferLinker->GetDomain(Id);
    }

    return TSDomainPtr();
}

void TSDataInteractManager::DestroyDomain( TSDomainPtr Domain )
{
    T_D();

    if(Domain)
    {
        TSDomainId DomainId = Domain->GetDomainId();

        Domain->Cleanup();

        if(d->_TransferLinker)
        {
			//LINK指针创建出来的域
            d->_TransferLinker->DestroyDomain(DomainId);
        }

		BOOST_FOREACH(TSIMonitorPtr Monitor,d->_Monitors)
		{
			if (Monitor->IsEnable())
			{
				Monitor->OnDestroyDomain(Domain);
			}
		}
    }
}

bool TSDataInteractManager::WasInitialize()
{
    T_D();

    return d->_HasInit; 
}

void TSDataInteractManager::OnDomainReceivedTopic_p( TSDomainPtr Domain,TSTopicContextPtr Ctx)
{
    T_D();
	
	//添加接受到主题的监控
	BOOST_FOREACH(TSIMonitorPtr Monitor, d->_Monitors)
	{
		if (Monitor->IsEnable())
		{
			if (!Monitor->IsUpdateTopicCycle(Ctx->GetTopicHandle()))
			{
				Monitor->OnReceivedTopic(Domain->GetDomainId(), Ctx);
			}
		}
	}

    Domain->OnReceiveTopic(Ctx);
}

void TSDataInteractManager::SetUserFlag( UINT64 Flag )
{
    T_D();

    d->_UserFlag = Flag;
}

UINT64 TSDataInteractManager::GetUserFlag()
{
    T_D();

    return d->_UserFlag;
}

void TSDataInteractManager::Post( TSDomainRoutine_T Routine )
{
    T_D();

#if defined(XSIM3_3) || defined(XSIM3_2)
	if(d->_NotifyIOS)
	{
		if(!d->_IsStoped)
		{
			try
			{
				d->_NotifyIOS->GetIOS()->post(Routine);
			}
			catch(...)
			{
				std::cerr << "TSDataInteractManager::Post error" << std::endl;
			}
		}
	}
#else
	if(d->_NotifyIOS)
	{
		if(!d->_IsStoped && d->_NotifyIOS->GetIOS())
		{
			try
			{
				d->_NotifyIOS->GetIOS()->Post(Routine);
			}
			catch(...)
			{
				
			}
		}
	}
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

/* 使用配置文件初始化DataManager */
bool TSDataInteractManager::InitializeWithConfigure(int argc, char ** argv, const TSString& ConfigureFileName, const TSFrontAppUniquelyId& FrontAppId)
{
#if !defined(XSIM3_3) && !defined(XSIM3_2)
	DefualtLogger->SetLoggingFlag(TSLogger::_LOG_FLAG_CONSOLE | TSLogger::_LOG_FLAG_FILE);
#endif
	TSTopicTypeManager::Instance()->SetCfg(ConfigureFileName.empty() ? TS_TEXT("config.ini") : ConfigureFileName);

	//真正初始化的函数
	return Initialize(argc, argv, FrontAppId);
}

bool TSDataInteractManager::InitializeWithConfigure(const TSString& ConfigureFileName /*= ""*/, const TSFrontAppUniquelyId& FrontAppId /*= TSFrontAppUniquelyId()*/)
{
	return InitializeWithConfigure(0, 0, ConfigureFileName, FrontAppId);
}

TSTopicContextPtr TSDataInteractManager::SendBinary( const TSDomainId& DomainId,TSTOPICHANDLE THandle,  TSDataContextPtr DataCtx,const TSTime & Time,const TSFrontAppUniquelyId& Receiver )
{
    T_D();

    if(d->_TransferLinker)
    {
        TSTopicContextPtr Ctx = d->_TransferLinker->SendBinary(DomainId,THandle,DataCtx,Time,Receiver);

        BOOST_FOREACH(TSIMonitorPtr Monitor,d->_Monitors)
        {
			if (Monitor->IsEnable())
			{
				Monitor->OnUpdateTopic(DomainId, Ctx);
			}
        }

        return Ctx;
    }

    return TSTopicContextPtr();
}

bool TSDataInteractManager::DeleteTopic( const TSDomainId & DomainId,TSTopicContextPtr Ctx ,bool IsNotice)
{
	T_D();

	if(d->_TransferLinker)
	{
		return d->_TransferLinker->DeleteTopic(DomainId,Ctx,IsNotice);
	}

	return false;
}

bool TSDataInteractManager::DeleteTopic( const TSDomainId & DomainId,TSTOPICHANDLE TopicHandle,void * KeyData ,bool IsNotice)
{
	T_D();

	if(d->_TransferLinker)
	{
		return d->_TransferLinker->DeleteTopic(DomainId,TopicHandle,KeyData,IsNotice);
	}

	return false;
}

void TSDataInteractManager::OnDomainReceivedTopicRemoved_p( TSDomainPtr Domain,TSTopicContextPtr Ctx )
{
	Domain->OnTopicRemoved(Ctx);
}

void TSDataInteractManager::OnDomainReceivedTopicMatched_p(TSDomainPtr Domain, const TSDomainId& DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub)
{
	Domain->OnReceiveTopicMatched(Domain, DomainId, Thandle, AppId, IsPub);
}

void TSDataInteractManager::OnDomainReceivedLeaveDomain_p(TSDomainPtr Domain, const TSDomainId& DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub)
{
	Domain->OnReceiveLeaveDomain(Domain, DomainId, Thandle, AppId, IsPub);
}

bool TSDataInteractManager::OnDomainContentFilter_p(TSDomainPtr Domain, TSTopicContextPtr Ctx, UINT32 AppId)
{
	return Domain->OnContentFilterCallback(Domain, Ctx, AppId);
}

void TSDataInteractManager::CleanDomainData( const TSDomainId & DomainId )
{
	T_D();

	if(d->_TransferLinker)
	{
		if (TSDomainPtr Domian = GetDomain(DomainId))
		{
			Domian->ClearDomainData();

			return d->_TransferLinker->CleanDomainData(DomainId);
		}
	}
}

void TSDataInteractManager::OutputLogger( UINT32 Catalog,const TSString & LoggerMsg )
{
	T_D();

	if(d->_TransferLinker)
	{
		d->_TransferLinker->OutputLogger(Catalog,LoggerMsg);
	}
}

/* 根据用户Qos参数创建域 */
TSDomainPtr TSDataInteractManager::CreateDomainWithQos(const TSDomainIdOrPartName& DomainId, const ParticipantQosPolicy & participantQosPolicy)
{
	T_D();

	if (d->_TransferLinker)
	{
		//使用LINK指针创建域
		if (TSDomainPtr Domain = d->_TransferLinker->CreateDomainWithQos(DomainId, participantQosPolicy, d->_AppId))
		{
			//域创建成功后，设置用户标识
			Domain->SetUserFlag(GetUserFlag());
			//初始化域
			Domain->Initialize();

			BOOST_FOREACH(TSIMonitorPtr Monitor, d->_Monitors)
			{
				//判断是否启动监控功能
				if (Monitor->IsEnable())
				{
					Monitor->OnCreateDomain(Domain);
				}
			}

			return Domain;
		}
	}

	return TSDomainPtr();
}

/* 获取参与者Qos */
bool TSDataInteractManager::GetParticipantQosPolicy(const TSDomainId& domainId, ParticipantQosPolicy & participantQosPolicy)
{
	T_D();

	if (d->_TransferLinker)
	{
		return d->_TransferLinker->GetParticipantQosPolicy(domainId, participantQosPolicy);
	}

	return false;
}

/* 设置参与者Qos */
bool TSDataInteractManager::SetParticipantQosPolicy(const TSDomainId& domainId, const ParticipantQosPolicy & participantQosPolicy)
{
	T_D();

	if (d->_TransferLinker)
	{
		return d->_TransferLinker->SetParticipantQosPolicy(domainId, participantQosPolicy);
	}

	return false;
}

/* 获取数据读者Qos */
bool TSDataInteractManager::GetDataReaderQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy)
{
	T_D();

	if (d->_TransferLinker)
	{
		return d->_TransferLinker->GetDataReaderQos(domainId, tHandle, dataReaderQosPolicy);
	}

	return false;
}

/* 设置数据读者Qos */
bool TSDataInteractManager::SetDataReaderQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy)
{
	T_D();

	if (d->_TransferLinker)
	{
		return d->_TransferLinker->SetDataReaderQos(domainId, tHandle, dataReaderQosPolicy);
	}

	return false;
}

/* 获取数据写者Qos */
bool TSDataInteractManager::GetDataWriterQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy)
{
	T_D();

	if (d->_TransferLinker)
	{
		return d->_TransferLinker->GetDataWriterQos(domainId, tHandle, dataWriterQosPolicy);
	}

	return false;
}

/* 设置数据写者Qos */
bool TSDataInteractManager::SetDataWriterQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, const DataWriterQosPolicy & dataWriterQosPolicy)
{
	T_D();

	if (d->_TransferLinker)
	{
		return d->_TransferLinker->SetDataWriterQos(domainId, tHandle, dataWriterQosPolicy);
	}

	return false;
}

UINT8 TSDataInteractManager::GetJoinedDomains(TSStringArray & domainIDVec)
{
	T_D();
	if (d->_TransferLinker)
	{
		return d->_TransferLinker->GetJoinedDomains(domainIDVec);
	}
	return 0;
}

TSTopicFindSetType TSDataInteractManager::CreateTopicFindSet( TSHANDLE Handle,const TSTopicQuerySet & Query,const TSDomainId & DomainId )
{
	T_D();

	if(d->_TransferLinker)
	{
		return d->_TransferLinker->CreateTopicFindSet(Handle,Query,DomainId);
	}

	return TSTopicFindSetType();
}

TSTopicFindSetType TSDataInteractManager::CreateTopicFindSet( const TSTopicQuerySet & Query,const TSDomainId & DomainId )
{
	T_D();

	if(d->_TransferLinker)
	{
		return d->_TransferLinker->CreateTopicFindSet(Query,DomainId);
	}

	return TSTopicFindSetType();
}

TSTopicFindSetType TSDataInteractManager::CreateTopicFindSet( DOUBLE CenterLon,DOUBLE CenterLat,FLOAT SpacialRange,const TSTopicQuerySet & Query,const TSDomainId & DomainId )
{
	T_D();

	if(d->_TransferLinker)
	{
		return d->_TransferLinker->CreateTopicFindSet(CenterLon,CenterLat,SpacialRange,Query,DomainId);
	}

	return TSTopicFindSetType();
}

TSTopicContextPtr TSDataInteractManager::GetFirstObject(TSTopicFindSetType FindSet )
{
	T_D();

	if(d->_TransferLinker)
	{
		return d->_TransferLinker->GetFirstObject(FindSet);
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSDataInteractManager::GetNextObject(TSTopicFindSetType FindSet )
{
	T_D();

	if(d->_TransferLinker)
	{
		return d->_TransferLinker->GetNextObject(FindSet);
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSDataInteractManager::GetFirstTopicByHandle(const TSDomainId & DomainId, TSTOPICHANDLE THandle )
{
	T_D();

	if(d->_TransferLinker)
	{
		return d->_TransferLinker->GetFirstTopicByHandle(DomainId,THandle);
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSDataInteractManager::GetFirstTopicByHandle( const TSDomainId & DomainId,TSHANDLE Handle,TSTOPICHANDLE THandle )
{
	T_D();

	if(d->_TransferLinker)
	{
		return d->_TransferLinker->GetFirstTopicByHandle(DomainId,Handle,THandle);
	}

	return TSTopicContextPtr();
}

bool TSDataInteractManager::WaitForDiscoveryComplete( const TSDomainId & DomainId,const TSTopicQuerySet & QuerySet,const TSFrontAppUniquelyId & AppId,UINT32 Timeout ,TSWaitSet::WaitMode Mode)
{
	T_D();

	if(d->_TransferLinker)
	{
		return d->_TransferLinker->WaitForDiscoveryComplete(DomainId,QuerySet,AppId,Timeout,Mode);
	}

	return false;
}

TOPSIMDATAINTERFACE_DECL TSFrontAppUniquelyId BuildFromString( const TSString& AppId )
{
    return GetCRC32(TSString2Utf8(AppId));
}
