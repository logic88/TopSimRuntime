#include "stdafx.h"
#include "TSScriptContext.h"

#if !defined(XSIM3_2) && !defined(XSIM3_3)
#include "TopSimRuntime/TSScriptInterpreter.h"
#endif

BEGIN_METADATA(TSTopicFindSetScriptContext)
END_METADATA()

BEGIN_METADATA(TSTopicScriptContext)
REG_PROP_FIELD2(TopicId);
REG_PROP_FIELD2(Data);
END_METADATA()

BEGIN_METADATA(TSScriptContext)
REG_FUNC(GetTypeFlag);
REG_FUNC(GetTypeName);
REG_FUNC(HasFindSet);
REG_FUNC(CanConvert);
REG_FUNC(GetTopicInherits);
REG_FUNC(GetTopicParents);
REG_FUNC(GetTopicByTopicName);
REG_FUNC(GetTopicNameByTopic);
REG_FUNC(GetTypeNameByTopic);
REG_FUNC(GetDescByTopic);
REG_FUNC(GetRegisteredTopics);
REG_FUNC(GetMaxRegisterdTopic);
REG_FUNC(GetConfigurePath);
REG_FUNC(GetWorkDirectoryPath);
REG_FUNC(GetExtendCfg);
REG_FUNC(CreateDomain);
REG_FUNC(DestoryDomain);
REG_FUNC(GetAppName);
REG_FUNC(GetFrontAppUniquelyId);
REG_FUNC(GetAppAuthor);
REG_FUNC(GetAppVersion);
REG_FUNC(GetAppDescription);
REG_FUNC(IsDomainCreated);
REG_FUNC(SubscribeTopic);
REG_FUNC(PublishTopic);
REG_FUNC(UnsubscribeTopic);
REG_FUNC(IsTopicPublisher);
REG_FUNC(IsTopicSubscriber);
REG_FUNC(UpdateTopic);
REG_FUNC(CreateTopicFindSetByHandleTopicSet);
REG_FUNC(CreateTopicFindSetByHandleTopic);
REG_FUNC(CreateTopicFindSetByTopicId);
REG_FUNC(CreateTopicFindSetBySpacialTopic);
REG_FUNC(CreateTopicFindSetBySpacialTopicSet);
REG_FUNC(CreateTopicFindSetByHandle);
REG_FUNC(GetFirstTopic);
REG_FUNC(GetNextTopic);
REG_FUNC(RegisterTopicCallbackFunction);
REG_FUNC(RegisterTopicCallbackFile);
END_METADATA()

struct TSTopicScriptCallback
{
	std::set<TSString> FunctionNames;
	std::set<TSString> ScriptFiles;
};

typedef TSConArray<TSTopicScriptCallback> TSTopicScriptCallbackArray;

struct TSScriptContextPrivate
{
#if !defined(XSIM3_2) && !defined(XSIM3_3)
	TSScriptInterpreter * _Interpreter;
#endif
	TSRWSpinLock                                            _CallbackMapsLock;
	TSConUnorderedMap<TSString, TSTopicScriptCallbackArray> _CallbackMaps;
};

static TSScriptContextPtr g_ScriptContext;

struct GetScriptContextFuncOverload
{
	static TSScriptContextPtr GetScriptContext(TSVariant & obj)
	{
		return g_ScriptContext;
	}
};

TSScriptContext::TSScriptContext()
	:_p(new TSScriptContextPrivate)
{

}

TSScriptContext::~TSScriptContext()
{
	delete _p;
}

bool TSScriptContext::Initialize(const std::vector<TSString> & ScriptFiles)
{
	g_ScriptContext = shared_from_this();

	if (!ScriptFiles.empty())
	{
#if !defined(XSIM3_2) && !defined(XSIM3_3)
		_p->_Interpreter = TSScriptInterpreter::CreateInstance();


		if (_p->_Interpreter)
		{
			_p->_Interpreter->RegisterAllMetaClass();
			_p->_Interpreter->RegisterNoClassMemberFunction(TSMakeFunctionData("GetScriptContext", GetScriptContextFuncOverload::GetScriptContext));

			for (size_t i = 0; i < ScriptFiles.size(); ++i)
			{
				if (!_p->_Interpreter->Load(ScriptFiles[i].c_str()))
				{
					DEF_LOG_WARNING("script engine load file %1 failed!") << ScriptFiles[i];
				}
			}
		}
#endif
	}

	return true;
}

#if !defined(XSIM3_2) && !defined(XSIM3_3)
TSScriptInterpreter * TSScriptContext::GetInterpreter()
{
	return _p->_Interpreter;
}
#endif

UINT32 TSScriptContext::GetTypeFlag(UINT32 TopicId)
{
	if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(TopicId))
	{
		return Support->GetTypeFlag();
	}

	return 0;
}

TSString TSScriptContext::GetTypeName(UINT32 TopicId)
{
	if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(TopicId))
	{
		return Support->GetTypeName();
	}

	return "";
}

bool TSScriptContext::HasFindSet(UINT32 TopicId, UINT32 Key)
{
	if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(TopicId))
	{
		return Support->HasFindSet((TopSimDataInterface::FindSetFlag)Key);
	}

	return false;
}

bool TSScriptContext::CanConvert(UINT32 From, UINT32 To)
{
	if (TSTopicHelperPtr FromHelper = TSTopicTypeManager::Instance()->GetTopicHelperByTopic(From))
	{
		return FromHelper->CanConvert(To);
	}

	return false;
}

std::vector<UINT32> TSScriptContext::GetTopicInherits(UINT32 TopicId)
{
	if (TSTopicHelperPtr Helper = TSTopicTypeManager::Instance()->GetTopicHelperByTopic(TopicId))
	{
		std::vector<UINT32> Ret;
		Ret.resize(Helper->GetTopicInherits().size());
#ifndef __VXWORKS__
		memcpy(Ret.data(), Helper->GetTopicInherits().data(), Ret.size() * sizeof(UINT32));
#else
		if((Ret.size() > 0) && (Helper->GetTopicInherits().size() > 0))
		{
			memcpy(&(Ret.at(0)), &(Helper->GetTopicInherits().at(0)), Ret.size() * sizeof(UINT32));
		}

#endif

		return Ret;
	}

	return std::vector<UINT32>();
}

std::vector<UINT32> TSScriptContext::GetTopicParents(UINT32 TopicId)
{
	if (TSTopicHelperPtr Helper = TSTopicTypeManager::Instance()->GetTopicHelperByTopic(TopicId))
	{
		std::vector<UINT32> Ret;
		Ret.resize(Helper->GetTopicParents().size());
#ifndef __VXWORKS__
		memcpy(Ret.data(), Helper->GetTopicParents().data(), Ret.size() * sizeof(UINT32));
#else
		if((Ret.size() > 0) && (Helper->GetTopicParents().size() > 0))
		memcpy(&Ret.at(0), &Helper->GetTopicParents().at(0), Ret.size() * sizeof(UINT32));
#endif

		return Ret;
	}

	return std::vector<UINT32>();
}

UINT32 TSScriptContext::GetTopicByTopicName(const TSString & TopicName)
{
	return TSTopicTypeManager::Instance()->GetTopicByTopicName(TopicName);
}

const TSString& TSScriptContext::GetTopicNameByTopic(UINT32 TopicId)
{
	return TSTopicTypeManager::Instance()->GetTopicNameByTopic(TopicId);
}

const TSString& TSScriptContext::GetTypeNameByTopic(UINT32 TopicId)
{
	return TSTopicTypeManager::Instance()->GetTypeNameByTopic(TopicId);
}

const TSString& TSScriptContext::GetDescByTopic(UINT32 TopicId)
{
	return TSTopicTypeManager::Instance()->GetDescByTopic(TopicId);
}

std::vector<UINT32> TSScriptContext::GetRegisteredTopics()
{
	std::vector<TSTOPICHANDLE> RegisteredTopics = TSTopicTypeManager::Instance()->GetRegisteredTopics();

	std::vector<UINT32> Ret;
	Ret.resize(RegisteredTopics.size());
#ifndef __VXWORKS__
	memcpy(Ret.data(), RegisteredTopics.data(), Ret.size() * sizeof(UINT32));
#else
	if((Ret.size() > 0) && (RegisteredTopics.size() > 0))
	memcpy(&Ret.at(0), &RegisteredTopics.at(0), Ret.size() * sizeof(UINT32));
#endif

	return Ret;
}

UINT32 TSScriptContext::GetMaxRegisterdTopic()
{
	return TSTopicTypeManager::Instance()->GetMaxRegisterdTopic();
}

const TSString& TSScriptContext::GetConfigurePath()
{
	return TSTopicTypeManager::Instance()->GetConfigurePath();
}

const TSString& TSScriptContext::GetWorkDirectoryPath()
{
	return TSTopicTypeManager::Instance()->GetWorkDirectoryPath();
}

const TSString& TSScriptContext::GetExtendCfg(const TSString& Key)
{
	return TSTopicTypeManager::Instance()->GetExtendCfg(Key);
}

bool TSScriptContext::CreateDomain(const TSString & DomainId)
{
	return TSDataInteractManager::Instance()->CreateDomain(DomainId).get();
}

void TSScriptContext::DestoryDomain(const TSString & DomainId)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		TSDataInteractManager::Instance()->DestroyDomain(Domain);
	}
}

TSString TSScriptContext::GetAppName()
{
	return TSDataInteractManager::Instance()->GetAppName();
}

UINT32 TSScriptContext::GetFrontAppUniquelyId()
{
	return TSDataInteractManager::Instance()->GetFrontAppUniquelyId();
}

TSString TSScriptContext::GetAppAuthor()
{
	return TSDataInteractManager::Instance()->GetAppAuthor();
}

TSString TSScriptContext::GetAppVersion()
{
	return TSDataInteractManager::Instance()->GetAppVersion();
}

TSString TSScriptContext::GetAppDescription()
{
	return TSDataInteractManager::Instance()->GetAppDescription();
}

bool TSScriptContext::IsDomainCreated(const TSString & DomainId)
{
	return TSDataInteractManager::Instance()->GetDomain(DomainId).get();
}

bool TSScriptContext::SubscribeTopic(const TSString & DomainId, UINT32 TopicId)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		return Domain->SubscribeTopic(TopicId) != TopSimDataInterface::Failed;
	}

	return false;
}

bool TSScriptContext::PublishTopic(const TSString & DomainId, UINT32 TopicId)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		return Domain->PublishTopic(TopicId) != TopSimDataInterface::Failed;
	}

	return false;
}

void TSScriptContext::UnsubscribeTopic(const TSString & DomainId, UINT32 TopicId)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		Domain->UnsubscribeTopic(TopicId);
	}
}

void TSScriptContext::UnpublishTopic(const TSString & DomainId, UINT32 TopicId)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		Domain->UnpublishTopic(TopicId);
	}
}

bool TSScriptContext::IsTopicPublisher(const TSString & DomainId, UINT32 TopicId)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		return Domain->IsTopicPublisher(TopicId);
	}

	return false;
}

bool TSScriptContext::IsTopicSubscriber(const TSString & DomainId, UINT32 TopicId)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		return Domain->IsTopicSubscriber(TopicId);
	}

	return false;
}

bool TSScriptContext::UpdateTopic(const TSString & DomainId, UINT32 TopicId, const TSVariant & Val)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		return Domain->UpdateTopic(TopicId,(TSInterObject*)Val.GetDataPtr()).get();
	}

	return false;
}

TSVariant TSScriptContext::CreateTopicFindSetByHandleTopicSet(const TSString & DomainId, UINT32 Handle, const std::vector<UINT32> & Query)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		std::vector<TSTOPICHANDLE> TopicQuery;
		TopicQuery.resize(Query.size());
#ifndef __VXWORKS__
		memcpy(TopicQuery.data(), Query.data(), Query.size() * sizeof(UINT32));
#else
		if((TopicQuery.size() > 0 ) && (Query.size() > 0))
		memcpy(&TopicQuery.at(0), &Query.at(0), Query.size() * sizeof(UINT32));
#endif


		TSTopicFindSetScriptContextPtr Script = boost::make_shared<TSTopicFindSetScriptContext>();
		Script->FindSet = Domain->CreateTopicFindSet(TSHANDLE(Handle), TopicQuery);
		return TSVariant::FromValue(Script);
	}

	return TSVariant();
}

TSVariant TSScriptContext::CreateTopicFindSetByHandleTopic(const TSString & DomainId, UINT32 Handle, UINT32 TopicId)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		TSTopicFindSetScriptContextPtr Script = boost::make_shared<TSTopicFindSetScriptContext>();
		Script->FindSet = Domain->CreateTopicFindSet(Handle, TSTOPICHANDLE(TopicId));
		return TSVariant::FromValue(Script);
	}

	return TSVariant();
}

TSVariant TSScriptContext::CreateTopicFindSetByHandle(const TSString & DomainId, UINT32 Handle)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		TSTopicFindSetScriptContextPtr Script = boost::make_shared<TSTopicFindSetScriptContext>();
		Script->FindSet = Domain->CreateTopicFindSet(TSHANDLE(Handle));
		return TSVariant::FromValue(Script);
	}

	return TSVariant();
}

TSVariant TSScriptContext::CreateTopicFindSetByTopicId(const TSString & DomainId, UINT32 TopicId)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		TSTopicFindSetScriptContextPtr Script = boost::make_shared<TSTopicFindSetScriptContext>();
		Script->FindSet = Domain->CreateTopicFindSet(TSTOPICHANDLE(TopicId));
		return TSVariant::FromValue(Script);
	}

	return TSVariant();
}

TSVariant TSScriptContext::CreateTopicFindSetBySpacialTopic(const TSString & DomainId, DOUBLE CenterLon, DOUBLE CenterLat, FLOAT SpacialRange, UINT32 TopicId)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		TSTopicFindSetScriptContextPtr Script = boost::make_shared<TSTopicFindSetScriptContext>();
		Script->FindSet = Domain->CreateTopicFindSet(CenterLon,CenterLat,SpacialRange,TSTOPICHANDLE(TopicId));
		return TSVariant::FromValue(Script);
	}

	return TSVariant();
}

TSVariant TSScriptContext::CreateTopicFindSetBySpacialTopicSet(const TSString & DomainId, DOUBLE CenterLon, DOUBLE CenterLat, FLOAT SpacialRange, const std::vector<UINT32> & Query)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		std::vector<TSTOPICHANDLE> TopicQuery;
		TopicQuery.resize(Query.size());
#ifndef __VXWORKS__
		memcpy(TopicQuery.data(), Query.data(), Query.size() * sizeof(UINT32));
#else
		if((TopicQuery.size() > 0) && (Query.size() > 0))
		memcpy(&TopicQuery.at(0), &Query.at(0), Query.size() * sizeof(UINT32));
#endif


		TSTopicFindSetScriptContextPtr Script = boost::make_shared<TSTopicFindSetScriptContext>();
		Script->FindSet = Domain->CreateTopicFindSet(CenterLon, CenterLat, SpacialRange, TopicQuery);
		return TSVariant::FromValue(Script);
	}

	return TSVariant();
}

TSTopicScriptContextPtr TSScriptContext::GetFirstTopic(const TSString & DomainId, TSVariant FindSet)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		TSTopicFindSetScriptContextPtr Script = FindSet.Value<TSTopicFindSetScriptContextPtr>();

		if (TSTopicContextPtr Ctx = Domain->GetFirstTopic(Script->FindSet))
		{
			TSTopicScriptContextPtr TopicScript = boost::make_shared<TSTopicScriptContext>();
			TopicScript->TopicId = Ctx->GetTopicHandle();

			if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(TopicScript->TopicId))
			{
			    TSInterObjectPtr Obj = Ctx->GetTopic();
				TopicScript->Data = TSVariant(TSMetaType::GetType(Support->GetTypeName()), &Obj, TSVariant::Is_SmartPointer);

				return TopicScript;
			}
		}
	}

	return TSTopicScriptContextPtr();
}

TSTopicScriptContextPtr TSScriptContext::GetNextTopic(const TSString & DomainId, TSVariant FindSet)
{
	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		TSTopicFindSetScriptContextPtr Script = FindSet.Value<TSTopicFindSetScriptContextPtr>();

		if (TSTopicContextPtr Ctx = Domain->GetNextTopic(Script->FindSet))
		{
			TSTopicScriptContextPtr TopicScript = boost::make_shared<TSTopicScriptContext>();
			TopicScript->TopicId = Ctx->GetTopicHandle();

			if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(TopicScript->TopicId))
			{
			    TSInterObjectPtr Obj = Ctx->GetTopic();
				TopicScript->Data = TSVariant(TSMetaType::GetType(Support->GetTypeName()), &Obj, TSVariant::Is_SmartPointer);

				return TopicScript;
			}
		}
	}

	return TSTopicScriptContextPtr();
}

void TSScriptContext::RegisterTopicCallbackFunction(const TSString & DomainId, UINT32 TopicId, const TSString & CallbackFunctionName)
{
	TSRWSpinLock::WriteLock lock(_p->_CallbackMapsLock);

	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		Domain->RegisterTopicCallback(TopicId, boost::bind(&TSScriptContext::OnReceivedTopic, this, DomainId, _1));

		TSTopicScriptCallbackArray & Callbacks = _p->_CallbackMaps[DomainId];

		if (Callbacks.size() <= TopicId)
		{
			Callbacks.grow_to_at_least(TopicId + 1);
		}

		Callbacks[TopicId].FunctionNames.insert(CallbackFunctionName);
	}

	
}

void TSScriptContext::RegisterTopicCallbackFile(const TSString & DomainId, UINT32 TopicId, const TSString & FilePath)
{
	TSRWSpinLock::WriteLock lock(_p->_CallbackMapsLock);

	if (TSDomainPtr Domain = TSDataInteractManager::Instance()->GetDomain(DomainId))
	{
		Domain->RegisterTopicCallback(TopicId, boost::bind(&TSScriptContext::OnReceivedTopic, this, DomainId, _1));

		TSTopicScriptCallbackArray & Callbacks = _p->_CallbackMaps[DomainId];

		if (Callbacks.size() <= TopicId)
		{
			Callbacks.grow_to_at_least(TopicId + 1);
		}

		if (Callbacks[TopicId].ScriptFiles.find(FilePath)
			!= Callbacks[TopicId].ScriptFiles.end())
		{
#if defined(XSIM3_2) || defined(XSIM3_3)
			if (false)
#else
			if (!_p->_Interpreter->Load(FilePath.c_str()))
#endif
			
			{
				DEF_LOG_WARNING("script engine load file %1 failed!") << FilePath;
			}
			else
			{
				Callbacks[TopicId].ScriptFiles.insert(FilePath);
			}
		}
	}
}

void TSScriptContext::OnReceivedTopic(const TSDomainId & DomainId, TSTopicContextPtr Ctx)
{
	TSRWSpinLock::ReadLock lock(_p->_CallbackMapsLock);

#if !defined(XSIM3_2) && !defined(XSIM3_3)
	if (_p->_Interpreter)
	{
		TSTopicScriptContextPtr TopicScript = boost::make_shared<TSTopicScriptContext>();
		TopicScript->TopicId = Ctx->GetTopicHandle();

		if (TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(TopicScript->TopicId))
		{
		    TSInterObjectPtr InterObj = Ctx->GetTopic();
			TopicScript->Data = TSVariant(TSMetaType::GetType(Support->GetTypeName()), &InterObj, TSVariant::Is_SmartPointer);

			TSVariant Val1 = TSVariant::FromValue(DomainId);
			TSVariant Val2 = TSVariant::FromValue(TopicScript);

			TSTopicScriptCallbackArray & Callbacks = _p->_CallbackMaps[DomainId];

			if (Callbacks.size() > Ctx->GetTopicHandle())
			{
				TSTopicScriptCallback & Callback = Callbacks[Ctx->GetTopicHandle()];
				std::set<TSString>::iterator FunctionIter = Callback.FunctionNames.begin();
				while (FunctionIter != Callback.FunctionNames.end())
				{
					_p->_Interpreter->RunFunction((*FunctionIter).c_str(), 0, "TSVariant,TSVariant", &Val1,&Val2);

					++FunctionIter;
				}

				std::set<TSString>::iterator FileIter = Callback.ScriptFiles.begin();

				while (FileIter != Callback.ScriptFiles.end())
				{
					_p->_Interpreter->Load((*FileIter).c_str(), 0, 0, "TSVariant,TSVariant", &Val1, &Val2);

					++FileIter;
				}
			}
		}
	}
#endif
}
