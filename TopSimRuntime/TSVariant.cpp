#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#ifndef  _WRS_KERNEL
#include <boost/filesystem.hpp>
#endif //#ifndef  _WRS_KERNEL

#if !defined(XSIM3_3) && !defined(XSIM3_2)
    #include <TopSimRuntime/TSLogsUtil.h>
	#include <TopSimRuntime/TSProductLicense.h>
#endif // !defined(XSIM3_3) && !defined(XSIM3_2)

#include <TopSimDataInterface/TSTopicTypeManager.h>
#include <TopSimDataInterface/TSDomain.h>

#include "TSTransferProxy_p.h"
#include "TSTransferProxy.h"



BEGIN_METADATA(TSTransferProxy)
	REG_BASE(TSITransferLinker);
END_METADATA()

TSTransferProxy::TSTransferProxy(void)
	:SuperType(new PrivateType)
{

}

TSTransferProxy::TSTransferProxy( PrivateType * p )
	:SuperType(p)
{

}


TSTransferProxy::~TSTransferProxy(void)
{
}

bool TSTransferProxy::Initialize(int argc,char ** argv,TSIDataDelegate * DataDelegate)
{
	T_D();

	ASSERT(DataDelegate);

	d->_Delegate = DataDelegate;
	d->_LocalMgr = boost::make_shared<TSLocalDataManager>();
	d->_LocalMgr->Init();

	if(Configure(argc,argv))
	{
		MakeAlive(d->_Delegate->GetFrontAppUniquelyId());
		if (!TSTopicTypeManager::Instance()->GetExtendCfg("CenterServer").empty())
		{
			d->_DiscoveryProxy = boost::make_shared<TSDiscoveryProxy>();
		}
		return true;
	}

	return false;
}

void TSTransferProxy::Clean()
{
	T_D();

	if(d->_Middleware)
	{
		d->_Middleware->Clean();
	}

	d->_LocalMgr->Clean();
	d->_LocalMgr.reset();

	if (d->_DiscoveryProxy)
	{
		d->_DiscoveryProxy->Clean();
		d->_DiscoveryProxy.reset();
	}

	d->_Domains.clear();
}

void TSTransferProxy::MakeAlive( const TSFrontAppUniquelyId& FrontAppId )
{
	T_D();

	if(d->_Middleware)
	{
		d->_Middleware->MakeAlive(FrontAppId);
	}
	
}

void TSTransferProxy::MakeDead( const TSFrontAppUniquelyId& FrontAppId )
{
	T_D();

	if(d->_Middleware)
	{
		d->_Middleware->MakeDead(FrontAppId);
	}
}

bool TSTransferProxy::Subscribe( const TSDomainId& DomainId,TSTOPICHANDLE THandle,bool LocalOnly)
{
	T_D();

	if(d->_Middleware)
	{
		return d->_Middleware->Subscribe(DomainId,THandle,LocalOnly);
	}

	return true;
}

bool TSTransferProxy::Publish(const TSDomainId& DomainId, TSTOPICHANDLE THandle)
{
	T_D();

	if(d->_Middleware)
	{
        return d->_Middleware->Publish(DomainId,THandle);
	}

	return true;
}

void TSTransferProxy::Unsubscribe(const TSDomainId& DomainId,TSTOPICHANDLE THandle)
{
	T_D();

	if(d->_Middleware)
	{
		d->_Middleware->Unsubscribe(DomainId,THandle);
	}
}

void TSTransferProxy::Unpublish( const TSDomainId& DomainId,TSTOPICHANDLE THandle)
{
	T_D();

	if(d->_Middleware)
	{
		d->_Middleware->Unpublish(DomainId,THandle);
	}
}

TSTopicContextPtr TSTransferProxy::UpdateTopic( const TSDomainId& DomainId,
	TSTOPICHANDLE Handle,
	const void * Data,const TSTime & Time, bool IsNeedSerializer)
{
	T_D();

	return UpdateLocal_p(DomainId,Handle,Data,Time, TS_INVALID,IsNeedSerializer);
}

TSTopicContextPtr TSTransferProxy::SendData( const TSDomainId& DomainId,
    TSTOPICHANDLE THandle,const void * Data,const TSTime & Time,const TSFrontAppUniquelyId& Receiver)
{
	T_D();
	return UpdateLocal_p(DomainId,THandle,Data,Time,Receiver,false);
}

bool TSTransferProxy::Configure(int argc,char ** argv)
{
	T_D();

	TSString RealizedTypeName = TSTopicTypeManager::Instance()->GetExtendCfg("TSIMiddleware");
	if(RealizedTypeName.empty())
	{
		if(pugi::xml_node node =GetCfgDoc().select_single_node(TS_TEXT("Config/Middleware")).node())
		{
			RealizedTypeName = node.attribute(TS_TEXT("Realized")).as_string();
		}
	}

	if(!RealizedTypeName.empty())
	{
		if(int MetaType = TSMetaType::GetType(RealizedTypeName.c_str()))
		{
			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTRANSFERPROXY_CPP_168) << RealizedTypeName;

			d->_Middleware = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(MetaType),TSITransferMiddlewarePtr);
		}
		else
		{
            DEF_LOG_WARNING(TRS_TOPSIMDATAINTERFACE_TSTRANSFERPROXY_CPP_174) << RealizedTypeName;
		}
	}
	else
	{
		DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTRANSFERPROXY_CPP_179);
	}

	if(d->_Middleware)
	{
		d->_Middleware->TopicCallback = boost::bind(&TSTransferProxy::OnTopicAvaliable, this, _1, _2, _3);
		d->_Middleware->RemoveCallback = boost::bind(&TSTransferProxy::OnDataRemove, this, _1, _2, _3);
		d->_Middleware->TopicMatchedCallback = boost::bind(&TSTransferProxy::OnTopicMatched, this, _1, _2, _3, _4);
		d->_Middleware->LeaveDomainCallback = boost::bind(&TSTransferProxy::OnLeaveDomain, this, _1, _2, _3, _4);
		d->_Middleware->ContentFilterCallback = boost::bind(&TSTransferProxy::OnContentFilterCallback, this, _1, _2, _3, _4);

		if (d->_Middleware->Initialize(argc, argv))
		{
			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTRANSFERPROXY_CPP_189) << RealizedTypeName;
		}
		else
		{
			DEF_LOG_WARNING(TRS_TOPSIMDATAINTERFACE_TSTRANSFERPROXY_CPP_193) << RealizedTypeName;
			d->_Middleware.reset();
		}

	}
	return true;
}

void TSTransferProxy::OnTopicAvaliable( const TSDomainId& DomainId,
   TSTOPICHANDLE THandle,TSDataContextPtr Ctx)
{
	T_D();

	TSRWSpinLock::WriteLock lock(d->_DataWriteLock);

	TSTopicContextPtr TopicCtx = d->_LocalMgr->UpdateTopicBinary(DomainId,THandle,Ctx,TSTime(),true);

	if(d->_Delegate && TopicCtx)
	{
		if(TSDomainPtr Domain = GetDomain(DomainId))
		{
			d->_Delegate->OnReceivedTopic(Domain,TopicCtx);
		}
	}
}

TSDomainPtr TSTransferProxy::CreateDomain( const TSDomainId& DomainId ,const TSFrontAppUniquelyId& FrontAppUniquelyId)
{
	T_D();

	TSDomainPtr Domain;
	TSDomainId  DomainInt;

	std::vector<TSString> Vec = TSStringUtil::Split(DomainId,"://",false);

	if(Vec.size() > 0)
	{
		DomainInt = Vec[0];

		TSConUnorderedMap<TSDomainId,TSDomainRefPtr>::iterator it = 
			d->_Domains.find(DomainInt);

		if(it != d->_Domains.end())
		{
			Domain = it->second->Domain;
			++it->second->Ref;
		}

		if(!Domain)
		{
			if(d->_Middleware)
			{
				if(d->_Middleware->CreateDomain(DomainId,FrontAppUniquelyId))
				{
					Domain = CreateDomain_p(DomainInt);
				}
			}
			else
			{
				Domain = CreateDomain_p(DomainInt);
			}
			if (d->_DiscoveryProxy)
			{//重复加入域只发一次
				d->_DiscoveryProxy->OnCreateDomain(DomainInt);
			}
		}

		return Domain;
	}

	return TSDomainPtr();
}

void TSTransferProxy::DestroyDomain( const TSDomainId& DomainId )
{
	T_D();

	TSConUnorderedMap<TSDomainId,TSDomainRefPtr>::iterator it = 
		d->_Domains.find(DomainId);

	if(it != d->_Domains.end())
	{
		if(--it->second->Ref <= 0)
		{
			if(d->_Middleware)
			{
				d->_Middleware->DestroyDomain(DomainId);
			}
			if (d->_DiscoveryProxy)
			{//最后一次销毁域时发一次
				d->_DiscoveryProxy->OnDestroyDomain(DomainId);
			}
			d->_Domains.unsafe_erase(DomainId);
            d->_LocalMgr->CleanDomainData(DomainId);
		}
	}
}

TSDomainPtr TSTransferProxy::GetDomain( const TSDomainId& DomainId )
{
	T_D();

	TSConUnorderedMap<TSDomainId,TSDomainRefPtr>::iterator it = 
		d->_Domains.find(DomainId);

	if(it != d->_Domains.end())
	{
		return it->second->Domain;
	}

	return TSDomainPtr();
}

TSDomainPtr TSTransferProxy::CreateDomain_p( const TSDomainId& DomainId )
{
	T_D();

	TSDomainPtr Domain;

	TSConUnorderedMap<TSDomainId,TSDomainRefPtr>::iterator it = 
		d->_Domains.find(DomainId);

	if(it != d->_Domains.end())
	{
		Domain = it->second->Domain;
		++it->second->Ref;
	}
	else
	{
		TSDomainRefPtr DomainRef = boost::make_shared<TSDomainRef>();
		Domain = boost::make_shared<TSDomain>(DomainId);

		DomainRef->Domain = Domain;
		++DomainRef->Ref;

		d->_Domains.insert(std::make_pair(DomainId,DomainRef));
	}

	return Domain;
}

TSTopicContextPtr TSTransferProxy::UpdateLocal_p( const TSDomainId& DomainId, TSTOPICHANDLE Handle, 
	const void * Data ,const TSTime & Time,const TSFrontAppUniquelyId& Receiver, bool IsNeedSerializer)
{
	T_D();

	TSTopicContextPtr LocalCtx;
    TSDataContextPtr       DataCtx;

	if(d->_LocalMgr)
	{
		if (d->_Middleware)
		{
			IsNeedSerializer = true;
		}

		if(TSTypeSupportPtr Support = HandleToSupport(Handle))
		{
			//只有主题数据需要存储
			LocalCtx = d->_LocalMgr->UpdateTopic(DomainId,Handle,Data,Time,false, IsNeedSerializer);
		}

		if (LocalCtx && d->_Middleware)
		{
			if (TS_INVALID_HANDLE_VALUE(Receiver))
			{
				d->_Middleware->UpdateTopic(DomainId, Handle, LocalCtx->GetCtx());
			}
			else
			{
				d->_Middleware->SendData(DomainId, Handle, LocalCtx->GetCtx(), Receiver);
			}
		}

		if(LocalCtx && d->_Delegate)
		{
			if(TSDomainPtr Domain = GetDomain(DomainId))
			{
				if (!Domain->IsNoCacheTopicHandle(LocalCtx->GetTopicHandle()) || LocalCtx->GetIsFromNetwork())
				{
					d->_Delegate->OnReceivedTopic(Domain, LocalCtx);
				}
			}
		}
	}

	return LocalCtx;
}

TSTopicContextPtr TSTransferProxy::SendBinary( const TSDomainId& DomainId, TSTOPICHANDLE THandle, TSDataContextPtr DataCtx,const TSTime & Time ,const TSFrontAppUniquelyId& Receiver)
{
    T_D();

    return SendBinary_p(DomainId,THandle,DataCtx,Time,Receiver);

}

TSTopicContextPtr TSTransferProxy::SendBinary_p( const TSDomainId& DomainId, TSTOPICHANDLE Handle, 
    TSDataContextPtr DataCtx,const TSTime & Time,const TSFrontAppUniquelyId& Receiver)
{
    T_D();

	TSTopicContextPtr LocalCtx;

    if(TSTypeSupportPtr Support = HandleToSupport(Handle))
    {
		if(d->_LocalMgr)
		{
			LocalCtx = d->_LocalMgr->UpdateTopicBinary(DomainId,Handle,DataCtx,Time,false);

			if(LocalCtx && d->_Delegate)
			{
				if(TSDomainPtr Domain = GetDomain(DomainId))
				{
					d->_Delegate->OnReceivedTopic(Domain, LocalCtx);
				}
			}

			if(d->_Middleware && DataCtx)
			{
				if(TS_INVALID_HANDLE_VALUE(Receiver))
				{
					d->_Middleware->UpdateTopic(DomainId,Handle,DataCtx);
				}
				else
				{
					d->_Middleware->SendData(DomainId,Handle,DataCtx,Receiver);
				}
			}
		}
    }

    return LocalCtx;
}

void TSTransferProxy::CleanDomainData( const TSDomainId& Domain )
{
    T_D();

	if(d->_LocalMgr)
	{
		d->_LocalMgr->CleanDomainData(Domain);
	}
}

bool TSTransferProxy::DeleteTopic( const TSDomainId & DomainId,TSTopicContextPtr Context ,bool IsNotice)
{
	T_D();

	return DeleteTopic_p(DomainId, Context->GetDataTopicHandle(), Context->GetCtx(), IsNotice);
}

bool TSTransferProxy::DeleteTopic( const TSDomainId & DomainId,TSTOPICHANDLE TopicHandle,void * KeyData ,bool IsNotice)
{
	T_D();
	TSTime Time = boost::posix_time::from_time_t(0);
	if(TSDataContextPtr Ctx = d->_LocalMgr->MakeDataContext(TopicHandle,KeyData,Time,true))
	{
		return DeleteTopic_p(DomainId,TopicHandle,Ctx,IsNotice);
	}

	return false;
}

bool TSTransferProxy::DeleteTopic_p( const TSDomainId & DomainId,TSTOPICHANDLE THandle,TSDataContextPtr Ctx ,bool IsNotice )
{
	T_D();

	if(d->_LocalMgr)
	{
		d->_LocalMgr->DeleteTopic(DomainId,THandle,Ctx);

		if(d->_Middleware)
		{
			return d->_Middleware->DeleteTopic(DomainId,THandle,Ctx,IsNotice);
		}

		return true;
	}

	return false;
}

void TSTransferProxy::OnDataRemove( const TSDomainId & DomainId,TSTOPICHANDLE Handle,TSDataContextPtr Ctx )
{
	T_D();

	if(TSTopicContextPtr Context = d->_LocalMgr->FindContextByHash(DomainId,Handle,Ctx))
	{
		 if(TSDomainPtr Domain = GetDomain(DomainId))
		 {
			 d->_Delegate->OnReceivedTopicRemove(Domain,Context);
		 }
	}
}

void TSTransferProxy::OnTopicMatched(const TSDomainId& DomainId, TSTOPICHANDLE THandle, UINT32 AppId, bool IsPub)
{
	T_D();

	if (d->_Delegate)
	{
		if (TSDomainPtr Domain = GetDomain(DomainId))
		{
			d->_Delegate->OnReceivedTopicMatched(Domain, DomainId, THandle, AppId, IsPub);
		}
	}
}

void TSTransferProxy::OnLeaveDomain(const TSDomainId& DomainId, TSTOPICHANDLE THandle, UINT32 AppId, bool IsPub)
{
	T_D();

	if (d->_Delegate)
	{
		if (TSDomainPtr Domain = GetDomain(DomainId))
		{
			d->_Delegate->OnReceivedLeaveDomain(Domain, DomainId, THandle, AppId, IsPub);
		}
	}
}

bool TSTransferProxy::OnContentFilterCallback(const TSDomainId & DomainId, TSTOPICHANDLE TopicId, TSDataContextPtr Ctx, UINT32 AppId)
{
	T_D();

	if (d->_Delegate)
	{
		if (TSDomainPtr Domain = GetDomain(DomainId))
		{
			TSITopicContextPtr TopicCtx = d->_LocalMgr->FindContextByHash(DomainId, TopicId, Ctx);

			if (TopicCtx)
			{
				return d->_Delegate->OnContentFilterTopic(Domain, TopicCtx,AppId);
			}
		}
	}

	return true;
}

TSTopicFindSetType TSTransferProxy::CreateTopicFindSet( TSHANDLE Handle,const TSTopicQuerySet & Query,const TSDomainId & DomainId )
{
	T_D();

	if(d->_LocalMgr)
	{
		return d->_LocalMgr->CreateTopicFindSet(Handle,Query,DomainId);
	}

	return TSTopicFindSetType();
}

TSTopicFindSetType TSTransferProxy::CreateTopicFindSet( const TSTopicQuerySet & Query,const TSDomainId & DomainId )
{
	T_D();

	if(d->_LocalMgr)
	{
		return d->_LocalMgr->CreateTopicFindSet(Query,DomainId);
	}

	return TSTopicFindSetType();
}

TSTopicFindSetType TSTransferProxy::CreateTopicFindSet( DOUBLE CenterLon,DOUBLE CenterLat,FLOAT SpacialRange,const TSTopicQuerySet & Query,const TSDomainId & DomainId )
{
	T_D();

	if(d->_LocalMgr)
	{
		return d->_LocalMgr->CreateTopicFindSet(CenterLon,CenterLat,SpacialRange,Query,DomainId);
	}

	return TSTopicFindSetType();
}

TSTopicContextPtr TSTransferProxy::GetFirstObject(TSTopicFindSetType FindSet )
{
	T_D();

	if(d->_LocalMgr)
	{
		return d->_LocalMgr->GetFirstObject(FindSet);
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSTransferProxy::GetNextObject(TSTopicFindSetType FindSet )
{
	T_D();

	if(d->_LocalMgr)
	{
		return d->_LocalMgr->GetNextObject(FindSet);
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSTransferProxy::GetFirstTopicByHandle(const TSDomainId & DomainId, TSTOPICHANDLE THandle )
{
	T_D();

	if(d->_LocalMgr)
	{
		return d->_LocalMgr->GetFirstTopicByHandle(DomainId,THandle);
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSTransferProxy::GetFirstTopicByHandle(const TSDomainId & DomainId, TSHANDLE Handle,TSTOPICHANDLE THandle )
{
	T_D();

	if(d->_LocalMgr)
	{
		return d->_LocalMgr->GetFirstTopicByHandle(DomainId,Handle,THandle);
	}

	return TSTopicContextPtr();
}

bool TSTransferProxy::WaitForDiscoveryComplete( const TSDomainId & DomainId, const TSTopicQuerySet & QuerySet,const TSFrontAppUniquelyId & AppId,UINT32 Timeout ,TSWaitSet::WaitMode Mode)
{
	T_D();

	if(d->_Middleware)
	{
		return d->_Middleware->WaitForDiscoveryComplete(DomainId,QuerySet,AppId,Timeout,Mode);
	}

	return false;
}

void TSTransferProxy::OutputLogger( UINT32 Catalog,const TSString & LoggerMsg )
{
	T_D();

	if(d->_Middleware)
	{
		d->_Middleware->OutputLogger(Catalog,LoggerMsg);
	}
}

/* 根据用户Qos参数创建域 */
TSDomainPtr TSTransferProxy::CreateDomainWithQos(const TSDomainId& domainId, const ParticipantQosPolicy & participantQosPolicy, const TSFrontAppUniquelyId& FrontAppUniquelyId)
{
	T_D();

	TSDomainPtr Domain;
	TSDomainId  DomainInt;

	std::vector<TSString> Vec = TSStringUtil::Split(domainId, "://", false);

	if (Vec.size() > 0)
	{
		DomainInt = Vec[0];

		TSConUnorderedMap<TSDomainId, TSDomainRefPtr>::iterator it =
			d->_Domains.find(DomainInt);

		if (it != d->_Domains.end())
		{
			Domain = it->second->Domain;
			++it->second->Ref;
		}

		if (!Domain)
		{
			if (d->_Middleware)
			{
				if (d->_Middleware->CreateDomainWithQos(domainId, participantQosPolicy, FrontAppUniquelyId))
				{
					Domain = CreateDomain_p(DomainInt);
				}
			}
			else
			{
				Domain = CreateDomain_p(DomainInt);
			}
		}

		return Domain;
	}

	return TSDomainPtr();
}
/* 获取参与者Qos */
bool TSTransferProxy::GetParticipantQosPolicy(const TSDomainId& domainId, ParticipantQosPolicy & participantQosPolicy)
{
	T_D();
	return d->_Middleware->GetParticipantQosPolicy(domainId, participantQosPolicy);
}
/* 设置参与者Qos */
bool TSTransferProxy::SetParticipantQosPolicy(const TSDomainId& domainId, const ParticipantQosPolicy & participantQosPolicy)
{
	T_D();
	return d->_Middleware->SetParticipantQosPolicy(domainId, participantQosPolicy);
}
/* 获取数据读者Qos */
bool TSTransferProxy::GetDataReaderQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy)
{
	T_D();
	return d->_Middleware->GetDataReaderQos(domainId, tHandle, dataReaderQosPolicy);
}
/* 设置数据读者Qos */
bool TSTransferProxy::SetDataReaderQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy)
{
	T_D();
	return d->_Middleware->SetDataReaderQos(domainId, tHandle, dataReaderQosPolicy);
}
/* 获取数据写者Qos */
bool TSTransferProxy::GetDataWriterQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy)
{
	T_D();
	return d->_Middleware->GetDataWriterQos(domainId, tHandle, dataWriterQosPolicy);
}
/* 设置数据写者Qos */
bool TSTransferProxy::SetDataWriterQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, const DataWriterQosPolicy & dataWriterQosPolicy)
{
	T_D();
	return d->_Middleware->SetDataWriterQos(domainId, tHandle, dataWriterQosPolicy);
}
/**
 * 获取创建的域列表,返回值为加入域的个数
 */
UINT8 TSTransferProxy::GetJoinedDomains(TSStringArray& domainIDVec)
{
	T_D();
	domainIDVec.clear();
	for (TSConUnorderedMap<TSDomainId, TSDomainRefPtr>::iterator it =d->_Domains.begin();it!=d->_Domains.end();++it)
	{
		domainIDVec.push_back(it->first);
	}
	return domainIDVec.size();
}
