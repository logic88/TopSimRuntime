#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#include <boost/foreach.hpp>

#if !defined(XSIM3_3) && !defined(XSIM3_2)
#include <TopSimRuntime/TSLogsUtil.h>
#endif // !defined(XSIM3_3) && !defined(XSIM3_2)


#include "TSLocalDataManager.h"
#include "TSDomain.h"
#include "TSDomain_p.h"
#include "TSDataInteractManager.h"
#include "TSDiscovery.h"
#include "TSSerializer.h"
#include "TSNTPTimeHolder.h"
#include "TSQosStruct.h"

extern "C" TSTOPICHANDLE BuiltinTopicRemovedHandle = 0;

BEGIN_METADATA(TSAbstractTimeHolder)
END_METADATA()

BEGIN_METADATA(TSITopicFindSetPlugin)
END_METADATA()

template<typename InheritCallbackMap_T,typename InheritCallback>
void BuildInheritCallbackMap(InheritCallbackMap_T& InheritArray)
{
	std::vector<TSTOPICHANDLE> Types = TSTopicTypeManager::Instance()->GetRegisteredTopics();

	BOOST_FOREACH(TSTOPICHANDLE Type,Types)
	{
		if(InheritArray.size() <= Type._value)
		{
			InheritArray.resize(Type + 100);
		}

		boost::shared_ptr<InheritCallback> & Callback = InheritArray[Type];

		if(!Callback)
		{
			Callback = boost::make_shared<InheritCallback>();
			Callback->THandle = Type;
		}
	}

	BOOST_FOREACH(TSTOPICHANDLE Type,Types)
	{
		if(TSTopicHelperPtr Desc = TSTopicTypeManager::Instance()->GetTopicHelperByTopic(Type))
		{
			boost::shared_ptr<InheritCallback> & ChildCallback = InheritArray[Type];

			std::vector<TSTOPICHANDLE> SupperTypes = Desc->GetTopicParents();

			std::reverse(SupperTypes.begin(),SupperTypes.end());

			BOOST_FOREACH(TSTOPICHANDLE SuperType,SupperTypes)
			{
				boost::shared_ptr<InheritCallback> & SuperCallback = InheritArray[SuperType];

				if(std::find(ChildCallback->Parents.begin(),ChildCallback->Parents.end(),SuperCallback)
					== ChildCallback->Parents.end())
				{
					ChildCallback->Parents.push_back(SuperCallback);
				}
			}
		}
	}
}

TSDomain::TSDomain(const TSDomainId& DomainId)
    :_Private_Ptr(new PrivateType)
{
    T_D();

	d->_CollectMode = TSDomainPrivate::None;

	d->_DomainId = DomainId;
	d->_AppId = DataManager->GetFrontAppUniquelyId();
	d->_Matcher = boost::make_shared<TSPSMatcher>();

	d->_UserFlag = 0;
	d->_WasInitialized = 0;

	TSTopicTypeManager::Instance()->InitNoCacheTopicSet(d->_NoCacheTopicSet);

	TSString Mode = TSTopicTypeManager::Instance()->GetExtendCfg("CollectMode");
	if(!Mode.empty())
	{
		d->_CollectMode = (TSDomainPrivate::CollectMode)TSValue_Cast<UINT32>(Mode);

		if(d->_CollectMode != TSDomainPrivate::None)
		{
			TSString Includes = TSTopicTypeManager::Instance()->GetExtendCfg("Includes");
			TSString Excludes = TSTopicTypeManager::Instance()->GetExtendCfg("Excludes");
            TSString CollectName = TSTopicTypeManager::Instance()->GetExtendCfg("Collect");

#ifdef _WIN32
			TSString MaxStdioSize = TSTopicTypeManager::Instance()->GetExtendCfg("MaxStdioSize");

			if (!MaxStdioSize.empty())
			{
				UINT32 StdioSize = TSValue_Cast<UINT32>(MaxStdioSize);

				if (StdioSize > 512)  //系统默认512 ，其他系统可能略小一点
				{
					_setmaxstdio(StdioSize);
				}
			}
#endif
            if (CollectName.empty())
            {
                CollectName = "TSDataCollection";
            }

			TSTOPICHANDLE MaxRegisterdTopic = TSTopicTypeManager::Instance()->GetMaxRegisterdTopic();
			
			d->_Collectors.resize(MaxRegisterdTopic + 1);

			if(!Includes.empty())
			{
				std::vector<TSString> IncludeTopics = TSStringUtil::Split(Includes,",",false);

				for(size_t i = 0; i < IncludeTopics.size(); ++i)
				{
					TSTOPICHANDLE TopicId = TSTopicTypeManager::Instance()->GetTopicByTopicName(IncludeTopics[i]);

					if(!TS_INVALID_HANDLE_VALUE(TopicId))
					{
                        int Type = TSMetaType::GetType(CollectName.c_str());
                        if(!Type)
                        {
                            DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_115) << CollectName;
                        }
                        else if(TSMetaType::IsAbstract(Type))
                        {
                            DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_119) << CollectName;
                        }
                        else
                        {
#ifndef  _WRS_KERNEL
                            TSDataCollectionPtr DataCollect = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(Type),TSDataCollectionPtr);
                            DataCollect->SetTopicName(IncludeTopics[i]);
                            DataCollect->Init();
                            d->_Collectors[TopicId] = DataCollect;
#endif //#ifndef  _WRS_KERNEL
                        }

					}
				}
			}
			else if(!Excludes.empty())
			{
				std::vector<TSString> ExcludeTopics = TSStringUtil::Split(Excludes,",",false);

				for(size_t i = 1; i <= MaxRegisterdTopic; ++i)
				{
					TSString TopicName = TSTopicTypeManager::Instance()->GetTopicNameByTopic((UINT32)i);

					if(std::find(ExcludeTopics.begin(),ExcludeTopics.end(),TopicName)
						== ExcludeTopics.end())
					{
                        int Type = TSMetaType::GetType(CollectName.c_str());
                        if(!Type)
                        {
                            DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_146) << CollectName;
                        }
                        else if(TSMetaType::IsAbstract(Type))
                        {
                            DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_150) << CollectName;
                        }
                        else
                        {
#ifndef  _WRS_KERNEL
                            TSDataCollectionPtr DataCollect = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(Type),TSDataCollectionPtr);
                            DataCollect->SetTopicName(TopicName);
                            d->_Collectors[i] = DataCollect;
#endif //#ifndef  _WRS_KERNEL
                        }

					}
				}
			}
			else
			{
				for(size_t i = 1; i <= MaxRegisterdTopic; ++i)
				{
					TSString TopicName = TSTopicTypeManager::Instance()->GetTopicNameByTopic((UINT32)i);
                    int Type = TSMetaType::GetType(CollectName.c_str());
                    if(!Type)
                    {
                        DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_170) << CollectName;
                    }
                    else if(TSMetaType::IsAbstract(Type))
                    {
                        DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_174) << CollectName;
                    }
                    else
                    {
#ifndef  _WRS_KERNEL
                        TSDataCollectionPtr DataCollect = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(Type),TSDataCollectionPtr);
                        DataCollect->SetTopicName(TopicName);
                        d->_Collectors[i] = DataCollect;
#endif //#ifndef  _WRS_KERNEL
                    }

				}
			}
		}
	}

    TSString TimeHolder = TSTopicTypeManager::Instance()->GetExtendCfg("TimeHolder");
    if (!TimeHolder.empty())
    {
        if(int MetaType = TSMetaType::GetType(TimeHolder.c_str()))
        {
            d->_TimeHolder = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(MetaType),TSAbstractTimeHolderPtr);
        }
    }
}

TSDomain::~TSDomain(void)
{
	T_D();

	d->_Matcher.reset();
	d->_Discovery.reset();

	delete _Private_Ptr;
}

const TSDomainId& TSDomain::GetDomainId()
{
    T_D();

    return d->_DomainId;
}

/* 订阅主题 */
TopSimDataInterface::ReturnCode TSDomain::SubscribeTopic( TSTOPICHANDLE Handle, bool LocalOnly )
{
	//返回 TSDomainPrivate* 
    T_D();

	// 调用 TSPSMatcher 类中的成员函数 Subscribe
	TopSimDataInterface::ReturnCode Ret = d->_Matcher->Subscribe(Handle);

	if (Ret == TopSimDataInterface::Failed)
	{
		return Ret;
	}

    else if(Ret == TopSimDataInterface::Ok)
    {
		Ret = DataManager->Subscribe(d->_DomainId,Handle,LocalOnly);
	}
	
	return TopSimDataInterface::Ok;
}

/* 发布主题 */
TopSimDataInterface::ReturnCode TSDomain::PublishTopic( TSTOPICHANDLE Handle )
{
    T_D();

	// 调用 TSPSMatcher 中的成员函数 Publish
    TopSimDataInterface::ReturnCode Ret = d->_Matcher->Publish(Handle);

	if (Ret == TopSimDataInterface::Failed)
	{
		return Ret;
	}

	else if (Ret == TopSimDataInterface::Ok)
	{
		Ret = DataManager->Publish(d->_DomainId, Handle);
	}

	return TopSimDataInterface::Ok;

}

/* 取消订阅主题 */
TopSimDataInterface::ReturnCode TSDomain::UnsubscribeTopic( TSTOPICHANDLE THandle )
{
    T_D();

    TopSimDataInterface::ReturnCode Ret = d->_Matcher->Unsubscribe(THandle);
    if(Ret == TopSimDataInterface::Ok)
    {
		Ret = DataManager->Unsubscribe(d->_DomainId,
			THandle);
	}

    return Ret;
}

/* 取消主题发布 */
TopSimDataInterface::ReturnCode TSDomain::UnpublishTopic( TSTOPICHANDLE Handle )
{
    T_D();

    TopSimDataInterface::ReturnCode pubState = d->_Matcher->Unpublish(Handle);
    if(pubState == TopSimDataInterface::Ok) 
    {
		DataManager->Unpublish(d->_DomainId,
			Handle);

		return TopSimDataInterface::Ok;
    }

    return TopSimDataInterface::Exist;
}

/* 是否是主题的订阅者 */
bool TSDomain::IsTopicSubscriber( TSTOPICHANDLE THandle )
{
    T_D();

    return d->_Matcher->IsSubscriber(THandle);
}

/* 是否是主题发布者 */
bool TSDomain::IsTopicPublisher( TSTOPICHANDLE THandle )
{
    T_D();

    return d->_Matcher->IsPublisher(THandle);
}

/* 推送主题数据 */
TSITopicContextPtr TSDomain::UpdateTopic( TSTOPICHANDLE Handle,const TSInterObject * Data )
{
	//返回 TSDomainPrivate* 
    T_D();

	//判断是否初始化
	if(d->_WasInitialized)
	{
		//判断是否是主题的发布者
		if(IsTopicPublisher(Handle))
		{
			if(TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(Handle))
			{
#ifdef DEBUG || defined(_DEBUG)
				ASSERT(Support->IsTypeMatched(Data));
#else
				if (Support->IsTypeMatched(Data))
#endif
				{
					TSTime Time = GetTimeHolder()->GetTime(d->_DomainId,Handle,NULL);

					bool IsNeedSerializer = (d->_CollectMode == TSDomainPrivate::PublisherCollect);

					if(TSTopicContextPtr Ctx = DataManager->UpdateTopic(d->_DomainId,
						Handle,
						Data,Time,IsNeedSerializer))
					{
						d->_FindSetPluginsLock.BeginRead();
						BOOST_FOREACH(TSITopicFindSetPluginPtr Plugin,d->_FindSetPlugins)
						{
							Plugin->OnTopicUpdate(Ctx);
						}
						d->_FindSetPluginsLock.EndRead();

						if(d->_CollectMode == TSDomainPrivate::PublisherCollect)
						{
							CollectData(Ctx);
						}

						return Ctx;
					}
				}
				else
				{
					DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_355) << TSTopicTypeManager::Instance()->GetTopicNameByTopic(Handle);
				}
			}
		}
	}

    return TSTopicContextPtr();
}


TSTopicContextPtr TSDomain::SendTopicDirect( TSTOPICHANDLE THandle,const TSInterObject * Parameter, const TSFrontAppUniquelyId& ReceiverId /*= TSFrontAppUniquelyId()*/ )
{
   T_D();

    TSTopicContextPtr Ctx;

   if(TS_INVALID_HANDLE_VALUE(ReceiverId))
   {
		Ctx = UpdateTopic(THandle,Parameter);
   }
   else
   {
	   if(d->_WasInitialized)
	   {
		   if(IsTopicPublisher(THandle))
		   {
			   if(TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(THandle))
			   {
				   if(Support->IsTypeMatched(Parameter))
				   {
					   TSTime Time = d->_TimeHolder ? d->_TimeHolder->GetTime(d->_DomainId,THandle,NULL):TSMinTimeValue;
					   Ctx = DataManager->SendData(d->_DomainId,THandle,Parameter,Time,ReceiverId);
				   }
				   else
				   {
					   DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_390) << TSTopicTypeManager::Instance()->GetTopicNameByTopic(THandle);
				   }
			   }
		   }
	   }
   }
  
	return Ctx;
}

TSTopicFindSetType TSDomain::CreateTopicFindSet( TSTOPICHANDLE THandle )
{
    T_D();

	TSTopicQuerySet Query;

	if (THandle)
	{
		Query.push_back(THandle);
	}

	return DataManager->CreateTopicFindSet(Query,d->_DomainId);
}

TSTopicFindSetType TSDomain::CreateTopicFindSet( DOUBLE CenterLon,DOUBLE CenterLat,FLOAT SpacialRange,TSTOPICHANDLE THandle )
{
    T_D();

	TSTopicQuerySet Query;

	if (THandle)
	{
		Query.push_back(THandle);
	}

	return DataManager->CreateTopicFindSet(CenterLon,CenterLat,SpacialRange,Query,d->_DomainId);
}

TSTopicFindSetType TSDomain::CreateTopicFindSet( TSHANDLE HKey)
{
	T_D();

	TSTopicQuerySet Query;
	return DataManager->CreateTopicFindSet(HKey,Query,d->_DomainId);
}

TSTopicContextPtr TSDomain::GetFirstTopicByHandle( TSTOPICHANDLE THandle )
{
	T_D();

	if (THandle)
	{
		return DataManager->GetFirstTopicByHandle(d->_DomainId, THandle);
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSDomain::GetFirstTopicByHandle( TSHANDLE Handle,TSTOPICHANDLE THandle )
{
	T_D();
#if defined(XSIM3_2) || defined(XSIM3_3)
	if (TSHandleFirstTopicCache * TopicCache = (d->_FirstTopicHandleTable.GetObjectFromHandle(Handle)).get())
#else
	if (TSHandleFirstTopicCache * TopicCache = d->_FirstTopicHandleTable.GetObjectFromHandleUnsafe(Handle))
#endif
	{
		return TopicCache->GetFirstTopic(THandle,true);
	}

	return TSTopicContextPtr();
}

TSTopicFindSetType TSDomain::CreateTopicFindSet( TSHANDLE Handle,const TSTopicQuerySet & Query )
{
	T_D();

	return DataManager->CreateTopicFindSet(Handle,Query,d->_DomainId);
}

TSTopicFindSetType TSDomain::CreateTopicFindSet( TSHANDLE Handle,TSTOPICHANDLE Topic )
{
	TSTopicQuerySet Set;

	if (Topic)
	{
		Set.push_back(Topic);
	}

	return CreateTopicFindSet(Handle,Set);
}

TSTopicFindSetType TSDomain::CreateTopicFindSet( DOUBLE CenterLon,DOUBLE CenterLat,FLOAT SpacialRange,const TSTopicQuerySet & Query )
{
	T_D();

	return DataManager->CreateTopicFindSet(CenterLon,CenterLat,SpacialRange,Query,d->_DomainId);
}

TSTopicContextPtr TSDomain::GetFirstTopic(TSTopicFindSetType FindSet)
{
    T_D();

    return DataManager->GetFirstObject(FindSet);
}

TSTopicContextPtr TSDomain::GetNextTopic( TSTopicFindSetType FindSet )
{
	T_D();

	return DataManager->GetNextObject(FindSet);
}

template<typename Array,typename T>
UINT32 RegisterCallback(Array& Callbacks,T cb,bool AsyncCallbackThread)
{
	UINT32 stub = TS_INVALID;

	for(size_t i = 0; i < Callbacks.size(); ++i)
	{
		if(Callbacks[i]->Removed)
		{
			Callbacks[i]->Functor = cb;
			Callbacks[i]->AsyncCallbackThread = AsyncCallbackThread;
			Callbacks[i]->Removed = false;

			stub = UINT32(i + 1);

			break;
		}
	}

	if(stub == TS_INVALID)
	{
		boost::shared_ptr<TSCallbackWapper<T> > cbWrapper = boost::make_shared<TSCallbackWapper<T> >(cb);
		cbWrapper->AsyncCallbackThread = AsyncCallbackThread;
		Callbacks.push_back(cbWrapper);

		stub = UINT32(Callbacks.size());
	}

	Callbacks.Update();

	return stub;
}

template<typename Array>
void UnregisterCallback(Array& Callbacks,UINT32 stub)
{
	UINT32 Index = stub - 1;
	if(Index < Callbacks.size())
	{
		Callbacks[Index]->Removed = true;
		Callbacks[Index]->Functor = NULL;
	}

	Callbacks.Update();
}

// boost::function<void(TSTopicContextPtr Ctx)> cb = boost::bind(CBReceiveTestTopic, _1);
TSTopicCallbackStubType TSDomain::RegisterTopicCallback( TSTOPICHANDLE THandle,TSTopicCallbackType cb ,bool AsyncCallbackThread)
{
	//返回 TSDomainPrivate* 
    T_D();
   
	if(d->_WasInitialized)
	{
		if(d->_TopicCallbackArray.size() > THandle._value)
		{
			TSTopicCallbackPtr Callback = d->_TopicCallbackArray[THandle];

			if(Callback)
			{
				TSRWSpinLock::WriteLock lock(Callback->CallbackLock);

				TSTopicCallbackStubType Stub = RegisterCallback(Callback->Callbacks,cb,AsyncCallbackThread);

				if(AsyncCallbackThread)
				{
					Callback->AsyncCallbackStub.push_back(Stub);
				}
				else
				{
					Callback->SyncCallbackStub.push_back(Stub);
				}

				return Stub;
			}
			else
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_552) << THandle;
			}
		}

		BuildInheritCallbackMap<InheritTopicCallbackArray_T,TSTopicCallback>(d->_TopicCallbackArray);

		if(d->_TopicCallbackArray.size() > THandle._value)
		{
			TSTopicCallbackPtr Callback = d->_TopicCallbackArray[THandle];

			if(Callback)
			{
				TSRWSpinLock::WriteLock lock(Callback->CallbackLock);

				TSTopicCallbackStubType Stub = RegisterCallback(Callback->Callbacks,cb,AsyncCallbackThread);

				if(AsyncCallbackThread)
				{
					Callback->AsyncCallbackStub.push_back(Stub);
				}
				else
				{
					Callback->SyncCallbackStub.push_back(Stub);
				}

				return Stub;
			}
			else
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_581) << THandle;
			}
		}
		else
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_586) << THandle;
		}
	}
	
	return 0;
  
}

void TSDomain::UnregisterTopicCallback( TSTOPICHANDLE THandle,UINT32 stub )
{
    T_D();

	if(d->_WasInitialized)
	{
		if(d->_TopicCallbackArray.size() > THandle._value)
		{
			TSTopicCallbackPtr Callback = d->_TopicCallbackArray[THandle];

			if(Callback)
			{
				TSRWSpinLock::WriteLock lock(Callback->CallbackLock);

				UnregisterCallback(Callback->Callbacks,stub);

				std::vector<UINT32>::iterator AsyncIt = 
					std::find(Callback->AsyncCallbackStub.begin(),Callback->AsyncCallbackStub.end(),stub);

				if(AsyncIt != Callback->AsyncCallbackStub.end())
				{
					Callback->AsyncCallbackStub.erase(AsyncIt);
				}
				else
				{
					std::vector<UINT32>::iterator SyncIt = 
						std::find(Callback->SyncCallbackStub.begin(),Callback->SyncCallbackStub.end(),stub);

					if(SyncIt != Callback->SyncCallbackStub.end())
					{
						Callback->SyncCallbackStub.erase(SyncIt);
					}
				}
			}
		}
	}
}

void InovkeCallback(TSDomainPtr Domain,TSDomainPrivate * _p,TSTopicCallbackPtr Callback,TSTopicContextPtr Ctx)
{
	TSRWSpinLock::ReadLock lock(Callback->CallbackLock);

	if(Callback->Callbacks.IsValid())
	{
		BOOST_FOREACH(UINT32 Stub,Callback->SyncCallbackStub)
		{
			if(TSTopicCBWapperPtr CallbackWp = Callback->Callbacks[Stub - 1])
			{
				if(!CallbackWp->Removed
					&& CallbackWp->Functor)
				{
					CallbackWp->Functor(Ctx);
				}
			}
		}

		if(Callback->AsyncCallbackStub.size())
		{
			Domain->Post(boost::bind(&TSDomainPrivate::InvokeTopicCallback,_p,Domain,Callback,Ctx));
		}
	}
}

void InovkeMatchedCallback(TSDomainPtr Domain, TSDomainPrivate * _p, TSTopicMatchedCallbackPtr Callback, TSDomainId DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub)
{
	TSRWSpinLock::ReadLock lock(Callback->CallbackLock);

	if (Callback->Callbacks.IsValid())
	{
		BOOST_FOREACH(UINT32 Stub, Callback->SyncCallbackStub)
		{
			if (TSTopicMTWapperPtr CallbackWp = Callback->Callbacks[Stub - 1])
			{
				if (!CallbackWp->Removed
					&& CallbackWp->Functor)
				{
					CallbackWp->Functor(DomainId, Thandle, AppId, IsPub);
				}
			}
		}

		if (Callback->AsyncCallbackStub.size())
		{
			Domain->Post(boost::bind(&TSDomainPrivate::InvokeTopicMatchedCallback, _p, Domain, Callback, DomainId, Thandle, AppId, IsPub));
		}
	}
}

void TSDomain::OnRecieveTopicCallback(TSTOPICHANDLE TopicHandle,TSITopicContextPtr Ctx)
{
	T_D();

	if(d->_WasInitialized)
	{
		d->OnTopic(shared_from_this(),Ctx);

		TSTopicCallbackPtr Callback = _Private_Ptr->_TopicCallbackArray[TopicHandle];
		
		if(Callback)
		{
			//先回调子类，后回调父类
			//子类回调
			InovkeCallback(shared_from_this(),_Private_Ptr,Callback,Ctx);

			//父类回调

			BOOST_FOREACH(TSTopicCallbackPtr ParentCallback,Callback->Parents)
			{
				TSInstanceHandle CopyHandle = Ctx->GetInstanceHandle();
				CopyHandle.SetTopicHandle(ParentCallback->THandle);

				if(ParentCallback->Callbacks.IsValid())
				{
					TSTopicContextPtr CopyCtx = new TSTopicContext;

					CopyCtx->SetInstanceHandle(CopyHandle);
					CopyCtx->SetTime(Ctx->GetTime());
					CopyCtx->SetTopic(Ctx->GetTopic());

					InovkeCallback(shared_from_this(),_Private_Ptr,ParentCallback,CopyCtx);
				}
			}
		}
		else
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_695);
		}
	}
}

void TSDomain::OnReceiveTopicMatched(TSDomainPtr Domain, const TSDomainId& DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub)
{
	if (_Private_Ptr->_TopicMatchedArray.size() > Thandle._value)
	{
		TSTopicMatchedCallbackPtr Callback = _Private_Ptr->_TopicMatchedArray[Thandle];
		if (Callback)
		{
			//先回调子类，后回调父类
			//子类回调
			TSDomainId Temp = DomainId;
			InovkeMatchedCallback(shared_from_this(), _Private_Ptr, Callback, Temp, Thandle, AppId, IsPub);
		}
		else
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_695);
		}
	}
}

void TSDomain::OnReceiveLeaveDomain(TSDomainPtr Domain, const TSDomainId& DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub)
{
	if (_Private_Ptr->_LeaveDomainArray.size() > Thandle._value)
	{
		TSTopicMatchedCallbackPtr Callback = _Private_Ptr->_LeaveDomainArray[Thandle];
		if (Callback)
		{
			//先回调子类，后回调父类
			//子类回调
			TSDomainId Temp = DomainId;
			InovkeMatchedCallback(shared_from_this(), _Private_Ptr, Callback, Temp, Thandle, AppId, IsPub);
		}
		else
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_695);
		}
	}
}

void TSDomain::OnReceiveTopic(TSITopicContextPtr Ctx)
{
    T_D();

   if(d->_WasInitialized)
   {
	   if (Ctx->GetIsFromNetwork())
	   {
		   TSTime Time = GetTimeHolder()->GetTime(d->_DomainId, Ctx->GetTopicHandle(), NULL);
		   Ctx->SetTime(Time);
	   }

	   if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(Ctx->GetDataTopicHandle()))
	   {
		   if (Support->HasFindSet(TopSimDataInterface::kHandleType))
		   {
			   TSHandleFirstTopicCachePtr FirstTopicCache = d->_FirstTopicHandleTable.GetObjectFromHandle(Ctx->GetCtx()->HandleHash());

			   if (!FirstTopicCache)
			   {
				   TSRWSpinLock::WriteLock lock(d->_FirstTopicHandleTableLock);

				   if (!FirstTopicCache)
				   {
					   FirstTopicCache = new TSHandleFirstTopicCache();
					   FirstTopicCache->_FirstSpacialObjects.resize(TSTopicTypeManager::Instance()->GetMaxRegisterdTopic() + 1);

					   for (size_t i = 1; i < FirstTopicCache->_FirstSpacialObjects.size(); ++i)
					   {
						   FirstTopicCache->_FirstSpacialObjects[i].Helper = TSTopicTypeManager::Instance()->GetTopicHelperByTopic((UINT32)i);
					   }

					   VERIFY(d->_FirstTopicHandleTable.VerifyHandle(FirstTopicCache, Ctx->GetCtx()->HandleHash()));
				   }
			   }

			   if (FirstTopicCache)
			   {
				   FirstTopicCache->UpdateFirstTopic(Ctx->GetDataTopicHandle(), Ctx);
			   }
		   }
	   }

	   if (IsTopicSubscriber(Ctx->GetDataTopicHandle()))
	   {
		   d->_FindSetPluginsLock.BeginRead();
		   BOOST_FOREACH(TSITopicFindSetPluginPtr Plugin, d->_FindSetPlugins)
		   {
			   Plugin->OnTopicUpdate(Ctx);
		   }
		   d->_FindSetPluginsLock.EndRead();

		   if (d->_CollectMode == TSDomainPrivate::SubscriberCollect)
		   {
			   CollectData(Ctx);
		   }

		   OnRecieveTopicCallback(Ctx->GetDataTopicHandle(), Ctx);
	   }
   }
}

TSDiscoveryPtr TSDomain::GetDiscovery()
{
	T_D();

	return d->_Discovery;
}

bool TSDomain::Initialize()
{
	T_D();

	if(!d->_WasInitialized++)
	{
		d->_WeakDomain = shared_from_this();
		d->_AppId = DataManager->GetFrontAppUniquelyId();
		d->_Matcher = boost::make_shared<TSPSMatcher>();

		//创建callbakmap
		BuildInheritCallbackMap<InheritTopicCallbackArray_T, TSTopicCallback>(d->_TopicCallbackArray);
		//创建Matchedcallbackmap 
		BuildInheritCallbackMap<InheritTopicMTCallbackArray_T, TSTopicMatchedCallback>(d->_TopicMatchedArray);
		//创建Matchedcallbackmap 
		BuildInheritCallbackMap<InheritTopicMTCallbackArray_T, TSTopicMatchedCallback>(d->_LeaveDomainArray);

		TSString Interval = TSTopicTypeManager::Instance()->GetExtendCfg("DiscoveryEnable");
		if(!Interval.empty())
		{
			bool IsEnable = TSValue_Cast<UINT32>(Interval);
			if (IsEnable)
			{
				d->_Discovery = boost::make_shared<TSDiscovery>(shared_from_this());
			}
			else
			{
				d->_Discovery = TSDiscoveryPtr();
			}

		}

		//创建查找集
		pugi::xpath_node_set PluginNodes = GetCfgDoc().select_nodes(TS_TEXT("Config/FindSetPlugins/Plugin"));
		for(pugi::xpath_node_set::const_iterator it = PluginNodes.begin();it != PluginNodes.end();++it)
		{
			TSString Realized = it->node().attribute(TS_TEXT("Realized")).as_string();
			if(!Realized.empty())
			{
				int Type = TSMetaType::GetType(Realized.c_str());
				if(Type)
				{
					if(TSMetaType::CanConvert(Type,TSITopicFindSetPlugin::GetMetaTypeIdStatic()))
					{
						if(!TSMetaType::IsAbstract(Type))
						{
							TSITopicFindSetPluginPtr Plugin = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(Type),TSITopicFindSetPluginPtr);
							if(Plugin)
							{
								Plugin->Initialize(d->_WeakDomain.lock());

								TSRWSpinLock::WriteLock lock(d->_FindSetPluginsLock);
								d->_FindSetPlugins.push_back(Plugin);
							}
						}
					}
				}

			}
		}
	}

	return true;
}

void TSDomain::Cleanup()
{
	T_D();
    
	if(d->_WasInitialized > 0)
	{
		int WasInitialized = d->_WasInitialized;

		if(!(WasInitialized - 1))
		{
			DataManager->CleanDomainData(d->_DomainId);

#ifndef  _WRS_KERNEL
			for(size_t i = 0; i < d->_Collectors.size(); ++i)
			{
				if(TSDataCollectionPtr Collect = d->_Collectors[i])
				{
					Collect->Close();
				}
			}
#endif //#ifndef  _WRS_KERNEL
			if(d->_Discovery)
			{
				d->_Discovery->Stop();
				d->_Discovery.reset();
			}


			d->_FindSetPluginsLock.BeginWrite();
			for(size_t i = 0; i < d->_FindSetPlugins.size();++i)
			{
				d->_FindSetPlugins[i]->Clean();
			}
			d->_FindSetPlugins.clear();
			d->_FindSetPluginsLock.EndWrite();

			TSRWSpinLock::ReadLock lock(d->_MatchClearLock);
			TSTopicHandleArray_T & publish = d->_Matcher->GetPublish();

			for (int i = 0; i < publish.size(); i++)
			{
                if (publish[i])
                {
					TopSimDataInterface::ReturnCode Ret = d->_Matcher->Unpublish(i);
					if(Ret == TopSimDataInterface::Ok)
					{
						 DataManager->Unpublish(d->_DomainId,i);
					}
                   
                }  
			}

			TSTopicHandleArray_T & subscriber = d->_Matcher->GetSubscriber();
            for (int i = 0; i < subscriber.size(); i++)
            {
                if (subscriber[i])
                {
					TopSimDataInterface::ReturnCode Ret = d->_Matcher->Unsubscribe(i);
					if(Ret == TopSimDataInterface::Ok)
					{
						DataManager->Unsubscribe(d->_DomainId,i);
					}
                    
                }  
            }

			d->_ListenersLock.BeginWrite();
			d->_Listeners.clear();
			d->_ListenersLock.EndWrite();

			d->_WeakDomain.reset();
		}

		d->_WasInitialized = WasInitialized - 1;
	}
}

void TSDomain::ClearDomainData()
{
	T_D();

	TSRWSpinLock::WriteLock lock(d->_FirstTopicHandleTableLock);
	d->_FirstTopicHandleTable.Clear();
}

TSAbstractTimeHolderPtr TSDomain::GetTimeHolder()
{
	T_D();

	if (!d->_TimeHolder)
	{
		TSRWSpinLock::WriteLock lock(d->_TimeHolderLock);

		if (!d->_TimeHolder)
		{
			d->_TimeHolder = boost::make_shared<TSNTPTimeHolder>();
		}
	}

	return d->_TimeHolder;
}

void TSDomain::SetTimeHolder( TSAbstractTimeHolderPtr TimeHolder )
{
	T_D();

	d->_TimeHolder = TimeHolder;
}

void TSDomain::SetUserFlag( UINT64 Flag )
{
	T_D();

	d->_UserFlag = Flag;
}

UINT64 TSDomain::GetUserFlag()
{
	T_D();

	return d->_UserFlag;
}

void TSDomain::Post( TSDomainRoutine_T Routine )
{
	T_D();

	if(d->_WasInitialized)
	{
		DataManager->Post(Routine);
	}
}

bool TSDomain::WasInitialized()
{
	T_D();

	return d->_WasInitialized;
}

bool TSDomain::RegisterTopicListener( TSITopicListener * Listener )
{
	T_D();

	TSRWSpinLock::WriteLock lock(d->_ListenersLock);

	if(std::find(d->_Listeners.begin(),d->_Listeners.end(),Listener)
		== d->_Listeners.end())
	{
		d->_Listeners.push_back(Listener);
		return true;
	}

	return false;
}

void TSDomain::UnregisterTopicListener( TSITopicListener * Listener )
{
	T_D();

	TSRWSpinLock::WriteLock lock(d->_ListenersLock);

	std::vector<TSITopicListener*>::iterator it = std::find(d->_Listeners.begin(),d->_Listeners.end(),Listener);

	if(it != d->_Listeners.end())
	{
		d->_Listeners.erase(it);
	}
}

TSTopicContextPtr TSDomain::SendBinary( TSTOPICHANDLE THandle,const void * Data, const UINT32 Length )
{
    return SendBinaryDirect(THandle,Data,Length,TS_INVALID);
}

TSTopicContextPtr TSDomain::SendBinary( TSTOPICHANDLE THandle,TSByteBufferPtr DataBuffer )
{
	return SendBinaryDirect(THandle,DataBuffer,TS_INVALID);
}

TSTopicContextPtr TSDomain::SendBinary( TSTOPICHANDLE THandle,TSDataContextPtr DataContext )
{
	return SendBinaryDirect(THandle,DataContext,TS_INVALID);
}

void TSDomain::SetOutputPath(const TSString& OutPath )
{
    T_D();

	//关闭之前开始的采集
	d->_OutputPath = OutPath;

#ifndef  _WRS_KERNEL
	for (size_t i = 0; i < d->_Collectors.size(); ++i)
	{
		if (TSDataCollectionPtr Collect = d->_Collectors[i])
		{
			Collect->Close();
		}
	}
#endif //#ifndef  _WRS_KERNEL

	//重新开始新的采集

#ifndef  _WRS_KERNEL
    for(size_t i = 0; i < d->_Collectors.size(); ++i)
	{
		if(TSDataCollectionPtr Collect = d->_Collectors[i])
		{
			Collect->SetOutputPath(OutPath);
			Collect->Init();
		}
	}
#endif //#ifndef  _WRS_KERNEL
}

const TSString & TSDomain::GetOutputPath()
{
	T_D();

	return d->_OutputPath;
}

TSTopicCallbackStubType TSDomain::RegisterTopicMatchedCallback(TSTOPICHANDLE THandle, TSTopicMatchedCallbackType cb, bool AsyncCallbackThread /*= true*/)
{
	T_D();

	if (d->_WasInitialized)
	{
		if (d->_TopicMatchedArray.size() > THandle._value)
		{
			TSTopicMatchedCallbackPtr Callback = d->_TopicMatchedArray[THandle];

			if (Callback)
			{
				TSRWSpinLock::WriteLock lock(Callback->CallbackLock);

				TSTopicCallbackStubType Stub = RegisterCallback(Callback->Callbacks, cb, AsyncCallbackThread);

				if (AsyncCallbackThread)
				{
					Callback->AsyncCallbackStub.push_back(Stub);
				}
				else
				{
					Callback->SyncCallbackStub.push_back(Stub);
				}

				return Stub;
			}
			else
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_552) << THandle;
			}
		}

		BuildInheritCallbackMap<InheritTopicMTCallbackArray_T, TSTopicMatchedCallback>(d->_TopicMatchedArray);

		if (d->_TopicMatchedArray.size() > THandle._value)
		{
			TSTopicMatchedCallbackPtr Callback = d->_TopicMatchedArray[THandle];

			if (Callback)
			{
				TSRWSpinLock::WriteLock lock(Callback->CallbackLock);

				TSTopicCallbackStubType Stub = RegisterCallback(Callback->Callbacks, cb, AsyncCallbackThread);

				if (AsyncCallbackThread)
				{
					Callback->AsyncCallbackStub.push_back(Stub);
				}
				else
				{
					Callback->SyncCallbackStub.push_back(Stub);
				}

				return Stub;
			}
			else
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_581) << THandle;
			}
		}
		else
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_586) << THandle;
		}
	}

	return 0;

}

TSTopicCallbackStubType TSDomain::RegisterLeaveDomainCallback(TSTOPICHANDLE THandle, TSTopicMatchedCallbackType cb, bool AsyncCallbackThread /*= true*/)
{
	T_D();

	if (d->_WasInitialized)
	{
		if (d->_LeaveDomainArray.size() > THandle._value)
		{
			TSTopicMatchedCallbackPtr Callback = d->_LeaveDomainArray[THandle];

			if (Callback)
			{
				TSRWSpinLock::WriteLock lock(Callback->CallbackLock);

				TSTopicCallbackStubType Stub = RegisterCallback(Callback->Callbacks, cb, AsyncCallbackThread);

				if (AsyncCallbackThread)
				{
					Callback->AsyncCallbackStub.push_back(Stub);
				}
				else
				{
					Callback->SyncCallbackStub.push_back(Stub);
				}

				return Stub;
			}
			else
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_552) << THandle;
			}
		}

		BuildInheritCallbackMap<InheritTopicMTCallbackArray_T, TSTopicMatchedCallback>(d->_LeaveDomainArray);

		if (d->_LeaveDomainArray.size() > THandle._value)
		{
			TSTopicMatchedCallbackPtr Callback = d->_LeaveDomainArray[THandle];

			if (Callback)
			{
				TSRWSpinLock::WriteLock lock(Callback->CallbackLock);

				TSTopicCallbackStubType Stub = RegisterCallback(Callback->Callbacks, cb, AsyncCallbackThread);

				if (AsyncCallbackThread)
				{
					Callback->AsyncCallbackStub.push_back(Stub);
				}
				else
				{
					Callback->SyncCallbackStub.push_back(Stub);
				}

				return Stub;
			}
			else
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_581) << THandle;
			}
		}
		else
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_586) << THandle;
		}
	}

	return 0;
}

TSDataContentFilterCallbackType TSDomain::SetContentFilterCallback(TSDataContentFilterCallbackType Callback)
{
	T_D();

	TSDataContentFilterCallbackType Result = d->_ContentFilterCallback;
	d->_ContentFilterCallback = Callback;
	return Result;
}

void TSDomain::ResetContentFilterCallback()
{
	T_D();

	d->_ContentFilterCallback = TSDataContentFilterCallbackType();
}

void TSDomain::CollectData(TSITopicContextPtr Ctx)
{
    T_D();

#ifndef  _WRS_KERNEL
	if(d->_Collectors.size() > Ctx->GetDataTopicHandle())
	{
		if(TSDataCollectionPtr Collect = d->_Collectors[Ctx->GetDataTopicHandle()])
		{
			Collect->Write(Ctx);
		}
	}
#endif //#ifndef  _WRS_KERNEL
}

bool TSDomain::OnContentFilterCallback(TSDomainPtr Domain, TSTopicContextPtr Ctx, UINT32 AppId)
{
	T_D();

	if (d->_ContentFilterCallback)
	{
		return d->_ContentFilterCallback(Ctx, AppId);
	}

	return true;
}

bool TSDomain::IsNoCacheTopicHandle(TSTOPICHANDLE topic)
{
	T_D();
	if (d->_NoCacheTopicSet.size() == 0)
	{
		return false;
	}
	return d->_NoCacheTopicSet.find(topic._value) != d->_NoCacheTopicSet.end();
}

bool TSDomain::DeleteTopic( TSTopicContextPtr Ctx ,bool IsNotice)
{
	T_D();

	if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(Ctx->GetDataTopicHandle()))
	{
		if (Support->HasFindSet(TopSimDataInterface::kHandleType))
		{
			if (TSHandleFirstTopicCachePtr FirstTopicCache = d->_FirstTopicHandleTable.GetObjectFromHandle(Ctx->GetCtx()->HandleHash()))
			{
				FirstTopicCache->DeleteFirstTopic(Ctx->GetDataTopicHandle());
			}
		}
	}

	return DataManager->DeleteTopic(d->_DomainId,Ctx,IsNotice);
	//return true;
}

bool TSDomain::DeleteTopic( TSTOPICHANDLE TopicHandle,const TSInterObject * KeyData ,bool IsNotice)
{
	T_D();

	if(TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(TopicHandle))
	{
		//todo
		TSDataContextPtr DataCtx;

		TSSerializer Ser;
		if (Ser.PerpareForSerialize(Support->GetTypeVersionHashCode(), TSTime()))
		{
			Support->Serialize((TSInterObject*)KeyData, Ser);
			Ser.MarshalComplete();
			DataCtx = Ser.GetDataContext();
		}

		if (DataCtx && Support->HasFindSet(TopSimDataInterface::kHandleType))
		{
			if (TSHandleFirstTopicCachePtr FirstTopicCache = d->_FirstTopicHandleTable.GetObjectFromHandle(DataCtx->HandleHash()))
			{
				FirstTopicCache->DeleteFirstTopic(TopicHandle);
			}
		}

		if(Support->IsTypeMatched(KeyData))
		{
			return DataManager->DeleteTopic(d->_DomainId,TopicHandle,(void*)KeyData,IsNotice);
		}
		else
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSDOMAIN_CPP_999) << TSTopicTypeManager::Instance()->GetTopicNameByTopic(TopicHandle);
		}
	}

	return false;
}

void TSDomain::OnTopicRemoved( TSTopicContextPtr Ctx )
{
	T_D();

	if(d->_WasInitialized)
	{
		OnRecieveTopicCallback(BuiltinTopicRemovedHandle,Ctx);
	}
}

TSTopicContextPtr TSDomain::SendBinaryDirect( TSTOPICHANDLE THandle,const void * Data, const UINT32 Length,const TSFrontAppUniquelyId & Receiver )
{
	TSSerializer Deser(MakeBufferPtr(Data, Length));
	TSDataContextPtr DataContext = Deser.GetDataContext();
	return SendBinaryDirect(THandle,DataContext,Receiver);
}

TSTopicContextPtr TSDomain::SendBinaryDirect( TSTOPICHANDLE THandle,TSByteBufferPtr DataBuffer,const TSFrontAppUniquelyId & Receiver )
{
	TSSerializer Deser(DataBuffer);
	TSDataContextPtr DataContext = Deser.GetDataContext();
	return SendBinaryDirect(THandle,DataContext,Receiver);
}

TSTopicContextPtr TSDomain::SendBinaryDirect( TSTOPICHANDLE THandle,TSDataContextPtr DataContext,const TSFrontAppUniquelyId & Receiver )
{
	T_D();

	if(d->_WasInitialized)
	{
		if(IsTopicPublisher(THandle))
		{
			TSTime Time = GetTimeHolder()->GetTime(d->_DomainId,THandle,NULL);
			if(TSTopicContextPtr Ctx = DataManager->SendBinary(d->_DomainId,
				THandle,
				DataContext,
				Time,
				Receiver))
			{
				d->_FindSetPluginsLock.BeginRead();
				BOOST_FOREACH(TSITopicFindSetPluginPtr Plugin,d->_FindSetPlugins)
				{
					Plugin->OnTopicUpdate(Ctx);
				}
				d->_FindSetPluginsLock.EndRead();

				if(d->_CollectMode == TSDomainPrivate::PublisherCollect)
				{
					CollectData(Ctx);
				}

				return Ctx;
			}
		}
	}

	return TSTopicContextPtr();
}

bool TSDomain::WaitForDiscoveryComplete(const TSTopicQuerySet & QuerySet,const TSFrontAppUniquelyId & AppId,UINT32 Timeout ,TSWaitSet::WaitMode Mode)
{
	T_D();

	return DataManager->WaitForDiscoveryComplete(d->_DomainId,QuerySet,AppId,Timeout,Mode);
}

void TSDomain::CollectTopicData(TSTOPICHANDLE TopicId, const TSInterObject *Obj, const TSTime & Timestamp)
{
	T_D();

#ifndef  _WRS_KERNEL
	if(d->_Collectors.size() > TopicId)
	{
		if(TSDataCollectionPtr Collect = d->_Collectors[TopicId])
		{
			Collect->Write(TopicId,Obj,Timestamp);
		}
	}
#endif //#ifndef  _WRS_KERNEL
}

void TSDomain::CollectTopicData(TSITopicContextPtr Ctx)
{
	CollectData(Ctx);
}

/* 获取域Qos */
bool TSDomain::GetDomainQosPolicy(ParticipantQosPolicy & participantQosPolicy)
{
	T_D();
	return DataManager->GetParticipantQosPolicy(d->_DomainId, participantQosPolicy);
}

/* 设置域Qos */
bool TSDomain::SetDomainQosPolicy(const ParticipantQosPolicy & participantQosPolicy)
{
	T_D();
	return DataManager->SetParticipantQosPolicy(d->_DomainId, participantQosPolicy);
}

/* 获取订阅者Qos */
bool TSDomain::GetSubscribeQos(TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy)
{
	T_D();
	return DataManager->GetDataReaderQos(d->_DomainId, tHandle, dataReaderQosPolicy);
}

/* 设置订阅者Qos */
bool TSDomain::SetSubscribeQos(TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy)
{
	T_D();
	return DataManager->SetDataReaderQos(d->_DomainId, tHandle, dataReaderQosPolicy);
}
/* 获取发布者Qos */
bool TSDomain::GetPublishQos(TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy)
{
	T_D();
	return DataManager->GetDataWriterQos(d->_DomainId, tHandle, dataWriterQosPolicy);
}

/* 设置发布者Qos */
bool TSDomain::SetPublishQos(TSTOPICHANDLE tHandle, const DataWriterQosPolicy & dataWriterQosPolicy)
{
	T_D();
	return DataManager->SetDataWriterQos(d->_DomainId, tHandle, dataWriterQosPolicy);
}
