#include "stdafx.h"
#if defined(XSIM3_2) || defined(XSIM3_3)
#else
#include "TopSimRuntime/TSLogsUtil.h"
#endif

#include <TopSimDataInterface/TSTopicTypeManager.h>
#include "TSLocalDataManager_p.h"
#include "TSLocalDataManager.h"
#include "TSSerializer.h"


TSLocalDataManager::TSLocalDataManager(void)
	:_Private_Ptr(new PrivateType)
{
}

TSLocalDataManager::TSLocalDataManager(PrivateType * p)
	: _Private_Ptr(p)
{

}


TSLocalDataManager::~TSLocalDataManager(void)
{
	delete _Private_Ptr;
}

bool TSLocalDataManager::Init()
{
	T_D();

	d->_Init = true;
	TSTopicTypeManager::Instance()->InitNoCacheTopicSet(d->_NoCacheTopicSet);
	return true;
}

void TSLocalDataManager::Clean()
{
	T_D();

	if (d->_Init)
	{
		TSRWSpinLock::WriteLock TopicCacheLock(d->_TopicCacheLock);
		for (size_t i = 0; i < TopicCacheMaxSize; ++i)
		{
			d->_TopicCache[i].clear();
		}

		d->_Init = false;

		d->_NoCacheTopicSet.clear();
	}
}

TSTopicContextPtr TSLocalDataManager::UpdateTopic(const TSDomainId& DomainId,
	TSTOPICHANDLE THandle,
	const void * Data, const TSTime & Time, bool IsFromNetwork, bool IsNeedSerializer)
{
	T_D();

	TSDataContextPtr Ctx;
	TSInterObjectPtr CopyData;

	if (d->_NoCacheTopicSet.size() == 0 || d->_NoCacheTopicSet.find(THandle._value) == d->_NoCacheTopicSet.end() || IsFromNetwork)
	{
		if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(THandle))
		{
			if (int type = TSMetaType::GetType(Support->GetTypeName()))
			{
#if defined(XSIM3_2) || defined(XSIM3_3)

				CopyData = TS_STATIC_CAST(TSMetaType::AssociateSP(type, TSMetaType::Construct(type, 0)), TSInterObjectPtr);
#else
				CopyData = TS_STATIC_CAST(TSMetaType::ConstructSmart(type, Data), TSInterObjectPtr);
#endif
			}
		}
	}

	if (!CopyData)
	{
		//csharp java
		IsNeedSerializer = true;
	}

	if (Ctx = MakeDataContext(THandle, Data, Time, IsNeedSerializer))
	{
		return UpdateTopicBinary(DomainId, THandle, Ctx, Time, IsFromNetwork, CopyData);
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSLocalDataManager::UpdateTopicBinary(const TSDomainId& DomainId,
	TSTOPICHANDLE THandle, TSDataContextPtr Ctx,
	const TSTime & Time, bool IsFromNetwork, TSInterObjectPtr CopyData)
{
	T_D();

	if (d->_Init)
	{
		int DomainInt = ToDomainInt(DomainId);

		if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(THandle))
		{
			if (TSTopicDataCachePtr TopicCache = d->GetOrCreateTopicCache(DomainInt, THandle))
			{
				TSTopicContextPtr LocalCtx = new TSTopicContext;
				LocalCtx->SetCtx(Ctx);
				LocalCtx->SetInstanceHandle(TSInstanceHandle(THandle, THandle, Ctx->KeyHash(), IsFromNetwork));
				LocalCtx->SetTime(Time);
				LocalCtx->SetTopic(CopyData);

				TSTopicSpacialObjectPtr SpacialObject;

				if (Support->HasFindSet(TopSimDataInterface::kHandleType))
				{
					if (!Ctx->IsChecked())
					{
						//未执行CheckInit，可能为非LinkStudio数据包，需要构建Keys

						TSInterObjectPtr Data = LocalCtx->GetTopic();
						TSSerializer Ser;
						if (Ser.PerpareForSerialize(Support->GetTypeVersionHashCode(), Time))
						{
							Support->Serialize((TSInterObject*)Data.get(), Ser);
							Ser.MarshalComplete();
							Ctx->SetKeys(Ser.GetDataContext()->GetKeys());
						}
					}

					LocalTopicContextArrayPtr  ArrayPtr = TopicCache->HandleTable.GetObjectFromHandle(Ctx->HandleHash());

					if (!ArrayPtr)
					{
						TSRWSpinLock::WriteLock lock(TopicCache->Lock);

						ArrayPtr = TopicCache->HandleTable.GetObjectFromHandle(Ctx->HandleHash());

						if (!ArrayPtr)
						{
							ArrayPtr = boost::make_shared<LocalTopicContextArray>();
#if defined(XSIM3_3) || defined(XSIM3_2)
							TSHANDLE h = TopicCache->HandleTable.VerifyHandle(ArrayPtr, Ctx->HandleHash());
							LocalTopicContextArrayPtr  Result = TopicCache->HandleTable.GetObjectFromHandle(h);
#else
							LocalTopicContextArrayPtr  Result = TopicCache->HandleTable.VerifyHandle(ArrayPtr, Ctx->HandleHash());
#endif

							ASSERT(Result && "!TSLocalDataManager::UpdateTopicBinary->VerifyHandle");
							ASSERT(Result == ArrayPtr);
						}
					}

					ASSERT(ArrayPtr);

					bool IsReplace = false;
					UINT32 NoUseIndex = (UINT32)-1;

					LocalTopicContextArray & ContextArray = *ArrayPtr;

					for (size_t i = 0; i < ContextArray.size(); ++i)
					{
						TSTopicSpacialObjectPtr & TempSpacialObject = ContextArray[i];
						TSTopicContextPtr TopicContext = TempSpacialObject->GetContext();
						if (TopicContext)
						{
							if (TopicContext->GetCtx()->KeyHash() == Ctx->KeyHash())
							{
								TempSpacialObject->SetContext(LocalCtx);

								SpacialObject = TempSpacialObject;

								IsReplace = true;

								break;
							}
						}

						if (NoUseIndex == (UINT32)-1 && !TopicContext)
						{
							NoUseIndex = (UINT32)i;
							SpacialObject = TempSpacialObject;
						}
					}

					if (!IsReplace)
					{
						if (!SpacialObject)
						{
							SpacialObject = boost::make_shared<TSTopicSpacialObject>(LocalCtx);

							if (NoUseIndex != (UINT32)-1)
							{
								ContextArray[NoUseIndex] = SpacialObject;
							}
							else
							{
								ContextArray.push_back(SpacialObject);
							}
						}
						else
						{
							SpacialObject->SetContext(LocalCtx);
						}
					}
				}
				else
				{
					UINT32 Index = TS_INVALID;

					TSRWSpinLock::ReadLock lock(TopicCache->Lock);

					LocalTopicContextMap_T::iterator it = TopicCache->NoHandleHashMap.find(Ctx->KeyHash());
					if (it != TopicCache->NoHandleHashMap.end())
					{
						it->second->SetContext(LocalCtx);

						SpacialObject = it->second;
					}
					else
					{
						SpacialObject = boost::make_shared<TSTopicSpacialObject>(LocalCtx);
						TopicCache->NoHandleHashMap.insert(std::make_pair(Ctx->KeyHash(), SpacialObject));
					}
				}

				if (SpacialObject)
				{
					if (Support->HasFindSet(TopSimDataInterface::kSpacialRangeType))
					{
						d->_TopicCache[DomainInt].Spacial->Update(SpacialObject);
					}
				}

				//删除Key相同的父类主题,保证父类订阅父类主题的域也能获取到更新的主题。
				if (!TopicCache->Helper)
				{
					TSRWSpinLock::WriteLock lock(TopicCache->Lock);
					if (!TopicCache->Helper)
					{
						TopicCache->Helper = TSTopicTypeManager::Instance()->GetTopicHelperByTopic(THandle);
					}
				}

				if (TopicCache->Helper)
				{
					const std::vector<TSTOPICHANDLE> & Parents = TopicCache->Helper->GetTopicParents();

					BOOST_FOREACH(TSTOPICHANDLE Parent, Parents)
					{
						DeleteTopic(DomainId, Parent, Ctx);
					}
				}

				return LocalCtx;
			}
		}
	}

	return TSTopicContextPtr();
}

TSDataContextPtr TSLocalDataManager::MakeDataContext(TSTOPICHANDLE Handle, const void * Data, const TSTime & Time, bool IsNeedSerializer)
{
	TSDataContextPtr DataCtx;

	if (TSTypeSupportPtr Support = HandleToSupport(Handle))
	{
		TSSerializer Ser;
		Ser.SetSerializerKeyFlag(!IsNeedSerializer);

		if (Ser.PerpareForSerialize(Support->GetTypeVersionHashCode(), Time))
		{
			Support->Serialize((TSInterObject*)Data, Ser);
			Ser.MarshalComplete();
			DataCtx = Ser.GetDataContext();
		}

	}

	return DataCtx;
}

void TSLocalDataManager::CleanDomainData(const TSDomainId& DomainId)
{
	T_D();

	if (d->_Init)
	{
		TSRWSpinLock::WriteLock TopicCacheLock(d->_TopicCacheLock);

		int Id = ToDomainInt(DomainId);

		ASSERT(Id < TopicCacheMaxSize);

		if (Id < TopicCacheMaxSize)
		{
			d->_TopicCache[Id].clear();
		}
	}
}

void TSLocalDataManager::DeleteTopic(const TSDomainId & DomainId, TSTOPICHANDLE Handle, TSDataContextPtr Ctx)
{
	T_D();

	if (d->_Init)
	{
		int DomainInt = ToDomainInt(DomainId);

		if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(Handle))
		{
			TSTopicDataCachePtr TopicCache = d->GetOrCreateTopicCache(DomainInt, Handle);

			if (Support->HasFindSet(TopSimDataInterface::kHandleType))
			{
#if defined(XSIM3_2) || defined(XSIM3_3)
				LocalTopicContextArray *  ArrayPtr = TopicCache->HandleTable.GetObjectFromHandle(Ctx->HandleHash()).get();
#else
				LocalTopicContextArray *  ArrayPtr = TopicCache->HandleTable.GetObjectFromHandleUnsafe(Ctx->HandleHash());
#endif
				if (ArrayPtr)
				{
					LocalTopicContextArray & ContextArray = *ArrayPtr;

					for (size_t i = 0; i < ContextArray.size(); ++i)
					{
						if (TSTopicSpacialObjectPtr SpacialObject = ContextArray[i])
						{
							TSTopicContextPtr TopicContext = SpacialObject->GetContext();

							if (TopicContext && TopicContext->GetCtx()->KeyHash() == Ctx->KeyHash())
							{
								SpacialObject->SetContext(TSTopicContextPtr());
								break;
							}
						}
					}
				}
			}
			else
			{
				TSRWSpinLock::WriteLock lock(TopicCache->Lock);

				LocalTopicContextMap_T::iterator it = TopicCache->NoHandleHashMap.find(Ctx->KeyHash());
				if (it != TopicCache->NoHandleHashMap.end())
				{
					TopicCache->NoHandleHashMap.unsafe_erase(it);
				}
			}
		}
	}
}

TSTopicContextPtr TSLocalDataManager::FindContextByHash(const TSDomainId & DomainId, TSTOPICHANDLE Handle, TSDataContextPtr Ctx)
{
	T_D();

	if (d->_Init)
	{
		int DomainInt = ToDomainInt(DomainId);

		if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(Handle))
		{
			TSTopicDataCachePtr TopicCache = d->GetOrCreateTopicCache(DomainInt, Handle);

			if (Support->HasFindSet(TopSimDataInterface::kHandleType))
			{
#if defined(XSIM3_2) || defined(XSIM3_3)
				LocalTopicContextArray * ArrayPtr = TopicCache->HandleTable.GetObjectFromHandle(Ctx->HandleHash()).get();
#else
				LocalTopicContextArray * ArrayPtr = TopicCache->HandleTable.GetObjectFromHandleUnsafe(Ctx->HandleHash());
#endif
				if (ArrayPtr)
				{
					LocalTopicContextArray & ContextArray = *ArrayPtr;

					for (size_t i = 0; i < ContextArray.size(); ++i)
					{
						if (TSTopicSpacialObjectPtr SpacialObject = ContextArray[i])
						{
							TSTopicContextPtr TopicContext = SpacialObject->GetContext();

							if (TopicContext && TopicContext->GetCtx()->KeyHash() == Ctx->KeyHash())
							{
								return TopicContext;
							}
						}
					}
				}
			}
			else
			{
				TSRWSpinLock::ReadLock lock(TopicCache->Lock);

				LocalTopicContextMap_T::iterator it = TopicCache->NoHandleHashMap.find(Ctx->KeyHash());
				if (it != TopicCache->NoHandleHashMap.end())
				{
					return it->second->GetContext();
				}
			}
		}
	}

	return TSTopicContextPtr();
}

TSTopicFindSetType TSLocalDataManager::CreateTopicFindSet(TSHANDLE Handle, const TSTopicQuerySet & Query, const TSDomainId & DomainId)
{
	T_D();

	if (d->_Init)
	{
		TSHandleFindSetType FindSet = new TSHandleFindSet(d);
		FindSet->Handle = Handle;
		FindSet->DomainId = ToDomainInt(DomainId);

		if (Query.empty())
		{
			FindSet->Query = TSTopicTypeManager::Instance()->GetRegisteredTopics();
		}
		else
		{
			BOOST_FOREACH(TSTOPICHANDLE TopicId, Query)
			{
				FindSet->Query.push_back(TopicId);

				if (TSTopicHelperPtr Helper = TSTopicTypeManager::Instance()->GetTopicHelperByTopic(TopicId))
				{
					FindSet->Query.insert(FindSet->Query.end(), Helper->GetTopicInherits().begin(), Helper->GetTopicInherits().end());
				}
			}
		}

		FindSet->Reset();

		return FindSet;
	}

	return TSTopicFindSetType();
}

TSTopicFindSetType TSLocalDataManager::CreateTopicFindSet(const TSTopicQuerySet & Query, const TSDomainId & DomainId)
{
	T_D();

	if (d->_Init)
	{
		TSTopicFindSetPtr FindSet = new TSTopicFindSet(d);
		FindSet->DomainId = ToDomainInt(DomainId);

		if (Query.empty())
		{
			FindSet->Query = TSTopicTypeManager::Instance()->GetRegisteredTopics();
		}
		else
		{
			BOOST_FOREACH(TSTOPICHANDLE TopicId, Query)
			{
				FindSet->Query.push_back(TopicId);

				if (TSTopicHelperPtr Helper = TSTopicTypeManager::Instance()->GetTopicHelperByTopic(TopicId))
				{
					FindSet->Query.insert(FindSet->Query.end(), Helper->GetTopicInherits().begin(), Helper->GetTopicInherits().end());
				}
			}
		}

		FindSet->Reset();

		return FindSet;
	}

	return TSTopicFindSetType();
}

TSTopicFindSetType TSLocalDataManager::CreateTopicFindSet(DOUBLE CenterLon, DOUBLE CenterLat, FLOAT SpacialRange, const TSTopicQuerySet & Query, const TSDomainId & DomainId)
{
	T_D();

	if (d->_Init)
	{
		TSSpacialRangeFindSetType FindSet = new TSSpacialRangeFindSet(d);
		FindSet->DomainId = ToDomainInt(DomainId);
		FindSet->_Space = d->_TopicCache[FindSet->DomainId].Spacial;

		if (Query.empty())
		{
			FindSet->Query = TSTopicTypeManager::Instance()->GetRegisteredTopics();
		}
		else
		{
			BOOST_FOREACH(TSTOPICHANDLE TopicId, Query)
			{
				FindSet->Query.push_back(TopicId);

				if (TSTopicHelperPtr Helper = TSTopicTypeManager::Instance()->GetTopicHelperByTopic(TopicId))
				{
					FindSet->Query.insert(FindSet->Query.end(), Helper->GetTopicInherits().begin(), Helper->GetTopicInherits().end());
				}
			}
		}

		FindSet->Range = SpacialRange;

		TSConversion::LLAToECEF(CenterLat, CenterLon, 0, FindSet->Center);

		int CenterHashLat = d->_TopicCache[FindSet->DomainId].Spacial->_Spec->HashLatitude(CenterLat);
		int CenterHashLon = d->_TopicCache[FindSet->DomainId].Spacial->_Spec->HashLongitude(CenterLon);

		int LatRange = int(SpacialRange / d->_TopicCache[FindSet->DomainId].Spacial->_Spec->GetLatDistance()) + 1;
		int LonRange = int(SpacialRange / d->_TopicCache[FindSet->DomainId].Spacial->_Spec->GetLonDistance(CenterHashLat)) + 1;

		FindSet->Overtop = false;

		FindSet->StartHashLat = CenterHashLat - LatRange;

		if (FindSet->StartHashLat <= 0)
		{
			FindSet->Overtop = true;

			FindSet->StartHashLat = 1;
		}

		FindSet->StartHashLon = CenterHashLon - LonRange;
		if (FindSet->StartHashLon <= 0)
		{
			FindSet->Overtop = true;

			FindSet->StartHashLon = 1;
		}

		FindSet->EndHashLat = CenterHashLat + LatRange;
		if (FindSet->EndHashLat > d->_TopicCache[FindSet->DomainId].Spacial->_Spec->GetNumberOfRows())
		{
			FindSet->Overtop = true;

			FindSet->EndHashLat = d->_TopicCache[FindSet->DomainId].Spacial->_Spec->GetNumberOfRows();
		}

		FindSet->EndHashLon = CenterHashLon + LonRange;
		if (FindSet->EndHashLon > d->_TopicCache[FindSet->DomainId].Spacial->_Spec->GetNumberOfColumns())
		{
			FindSet->Overtop = true;

			FindSet->EndHashLon = d->_TopicCache[FindSet->DomainId].Spacial->_Spec->GetNumberOfColumns();
		}

		FindSet->CurHashLat = FindSet->StartHashLat;
		FindSet->CurHashLon = FindSet->StartHashLon;

		FindSet->Reset();

		return FindSet;
	}

	return TSTopicFindSetType();
}

TSTopicContextPtr TSLocalDataManager::GetFirstObject(TSTopicFindSetType FindSet)
{
	if (FindSet)
	{
#ifndef __VXWORKS__
		TSIFindSetType FindSetType = boost::static_pointer_cast<BOOST_DEDUCED_TYPENAME TSIFindSetType::element_type>(FindSet); //TS_STATIC_CAST(FindSet,TSIFindSetType);
#else
		TSIFindSetType FindSetType = boost::static_pointer_cast<TSIFindSetType::element_type>(FindSet); //TS_STATIC_CAST(FindSet,TSIFindSetType);
#endif

		FindSetType->Reset();
		return FindSetType->GetNextObject();
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSLocalDataManager::GetNextObject(TSTopicFindSetType FindSet)
{
	if (FindSet)
	{
#ifndef __VXWORKS__
		TSIFindSetType FindSetType = boost::static_pointer_cast<BOOST_DEDUCED_TYPENAME TSIFindSetType::element_type>(FindSet);
#else
		TSIFindSetType FindSetType = boost::static_pointer_cast<TSIFindSetType::element_type>(FindSet);
#endif

		return FindSetType->GetNextObject();
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSLocalDataManager::GetFirstTopicByHandle(const TSDomainId & DomainId, TSTOPICHANDLE THandle)
{
	T_D();

	int DomainIdInt = ToDomainInt(DomainId);

	if (DomainIdInt >= TopicCacheMaxSize)
	{
		return TSTopicContextPtr();
	}

	if (TSTopicHelperPtr Helper = TSTopicTypeManager::Instance()->GetTopicHelperByTopic(THandle))
	{
		if (TSTopicDataCachePtr DataCache = d->GetOrCreateTopicCache(DomainIdInt, THandle))
		{
			if (const TSTypeSupportPtr & Support =
				TSTopicTypeManager::Instance()->GetTypeSupportByTopic(THandle))
			{
				if (Support->HasFindSet(TopSimDataInterface::kHandleType))
				{
#if defined(XSIM3_3) || defined(XSIM3_2) || defined(XSIM4_3)
					TSHANDLE Handle = TS_INVALID;

					while (true)
					{
						Handle = DataCache->HandleTable.GetNextValidHandle(Handle);
						if (!TS_INVALID_HANDLE_VALUE(Handle))
						{
							if (LocalTopicContextArrayPtr & ContextArrayPtr = DataCache->HandleTable.GetObjectFromHandle(Handle))
							{
								LocalTopicContextArray & ContextArray = *ContextArrayPtr;

								if (ContextArray.size())
								{
									int Index = 0;
									while (Index < ContextArray.size())
									{
										TSTopicSpacialObjectPtr SpacialObject = ContextArray[Index];

										if (SpacialObject)
										{
											if (TSTopicContextPtr TopicContext = SpacialObject->GetContext())
											{
												return TopicContext;
											}
										}

										++Index;
									}
								}
							}
						}
						else
						{
							break;
						}
					}

					return TSTopicContextPtr();
#else
					TopicContextTable::Iterator it = DataCache->HandleTable.Begin();

					while (it)
					{
						if (const LocalTopicContextArrayPtr & ContextArrayPtr = *it)
						{
							LocalTopicContextArray & ContextArray = *ContextArrayPtr;

							if (ContextArray.size())
							{
								int Index = 0;
								while (Index < ContextArray.size())
								{
									TSTopicSpacialObjectPtr SpacialObject = ContextArray[Index];

									if (SpacialObject)
									{
										if (TSTopicContextPtr TopicContext = SpacialObject->GetContext())
										{
											return TopicContext;
										}
									}

									++Index;
								}
							}
						}

						++it;
					}

					return TSTopicContextPtr();
#endif

				}
				else
				{
					TSRWSpinLock::ReadLock lock(DataCache->Lock);

					LocalTopicContextMap_T::iterator it = DataCache->NoHandleHashMap.begin();
					if (it != DataCache->NoHandleHashMap.end())
					{
						if (TSTopicSpacialObjectPtr Ret = it->second)
						{
							return Ret->GetContext();
						}
					}
				}
			}
		}

		const std::vector<TSTOPICHANDLE> & InheritsTopics = Helper->GetTopicInherits();

		for (size_t i = 0; i < InheritsTopics.size(); ++i)
		{
			if (TSTopicDataCachePtr DataCache = d->GetOrCreateTopicCache(DomainIdInt, InheritsTopics[i]))
			{
				if (TSTypeSupportPtr Support =
					TSTopicTypeManager::Instance()->GetTypeSupportByTopic(THandle))
				{
					if (Support->HasFindSet(TopSimDataInterface::kHandleType))
					{
#if defined(XSIM3_3) || defined(XSIM3_2) || defined(XSIM4_3)
						TSHANDLE Handle = TS_INVALID;

						while (true)
						{
							Handle = DataCache->HandleTable.GetNextValidHandle(Handle);
							if (!TS_INVALID_HANDLE_VALUE(Handle))
							{
								if (LocalTopicContextArrayPtr & ContextArrayPtr = DataCache->HandleTable.GetObjectFromHandle(Handle))
								{
									LocalTopicContextArray & ContextArray = *ContextArrayPtr;

									if (ContextArray.size())
									{
										int Index = 0;
										while (Index < ContextArray.size())
										{
											TSTopicSpacialObjectPtr SpacialObject = ContextArray[Index];

											if (SpacialObject)
											{
												if (TSTopicContextPtr TopicContext = SpacialObject->GetContext())
												{
													return TopicContext;
												}
											}

											++Index;
										}
									}
								}
							}
							else
							{
								break;
							}
						}
#else
						TopicContextTable::Iterator it = DataCache->HandleTable.Begin();

						while (it)
						{
							if (const LocalTopicContextArrayPtr & ContextArrayPtr = *it)
							{
								LocalTopicContextArray & ContextArray = *ContextArrayPtr;

								if (ContextArray.size())
								{
									int Index = 0;
									while (Index < ContextArray.size())
									{
										TSTopicSpacialObjectPtr SpacialObject = ContextArray[Index];

										if (SpacialObject)
										{
											if (TSTopicContextPtr TopicContext = SpacialObject->GetContext())
											{
												return TopicContext;
											}
										}

										++Index;
									}
								}
							}

							++it;
						}
#endif

					}
					else
					{
						TSRWSpinLock::ReadLock lock(DataCache->Lock);

						LocalTopicContextMap_T::iterator it = DataCache->NoHandleHashMap.begin();
						if (it != DataCache->NoHandleHashMap.end())
						{
							if (TSTopicSpacialObjectPtr Ret = it->second)
							{
								return Ret->GetContext();
							}
						}
					}
				}
			}
		}
	}

	return TSTopicContextPtr();
}

TSTopicContextPtr TSLocalDataManager::GetFirstTopicByHandle(const TSDomainId & DomainId, TSHANDLE Handle, TSTOPICHANDLE THandle)
{
	T_D();

	ASSERT(false);
	return TSTopicContextPtr();
}


