#ifndef __TSSCRIPTCONTENXT_H__
#define __TSSCRIPTCONTENXT_H__

#if defined(XSIM3_2) || defined(XSIM3_3)

#else
#include <TopSimRuntime/TSScriptInterpreter.h>
#endif

#include "TopSimDataInterface.h"
#include "TSDomain.h"
#include "TSDataInteractManager.h"
#include "TSTopicTypeManager.h"
#include "TSTopicHelper.h"

struct TOPSIMDATAINTERFACE_DECL TSTopicFindSetScriptContext
{
	TS_MetaType(TSTopicFindSetScriptContext);

	TSTopicFindSetType FindSet;
};

STRUCT_PTR_DECLARE(TSTopicFindSetScriptContext);

struct TOPSIMDATAINTERFACE_DECL TSTopicScriptContext
{
	TS_MetaType(TSTopicScriptContext);

	UINT32    TopicId;
	TSVariant Data;
};

STRUCT_PTR_DECLARE(TSTopicScriptContext);

STRUCT_PTR_DECLARE(TSTopicHelperScriptContext);

struct TSScriptContextPrivate;
struct TOPSIMDATAINTERFACE_DECL TSScriptContext : public boost::enable_shared_from_this<TSScriptContext>
{
	TS_MetaType(TSScriptContext);

	TSScriptContext();
	~TSScriptContext();

public:
	//只能调用一次
	bool Initialize(const std::vector<TSString> & ScriptFiles);
#if !defined(XSIM3_2) && !defined(XSIM3_3)
	TSScriptInterpreter * GetInterpreter();
#endif
public:
	//TypeSupport
	UINT32                        GetTypeFlag(UINT32 TopicId);
	TSString                      GetTypeName(UINT32 TopicId);
	bool                          HasFindSet(UINT32 TopicId,UINT32 Key);
public:
	//TopicHelper
	bool                          CanConvert(UINT32 From, UINT32 To);
	std::vector<UINT32>           GetTopicInherits(UINT32 TopicId);
	std::vector<UINT32>           GetTopicParents(UINT32 TopicId);
public:
	//TSTopicTypeManager
	UINT32                        GetTopicByTopicName(const TSString & TopicName);
	const TSString&               GetTopicNameByTopic(UINT32 TopicId);
	const TSString&               GetTypeNameByTopic(UINT32 TopicId);
	const TSString&               GetDescByTopic(UINT32 TopicHandle);
	std::vector<UINT32>           GetRegisteredTopics();
	UINT32                        GetMaxRegisterdTopic();
	const TSString&               GetConfigurePath();
	const TSString&               GetWorkDirectoryPath();
	const TSString&               GetExtendCfg(const TSString& Key);
public:
	//DataInteractManager
	bool                          CreateDomain(const TSString & DomainId);
	void                          DestoryDomain(const TSString & DomainId);
	TSString                      GetAppName();
	UINT32                        GetFrontAppUniquelyId();
	TSString                      GetAppAuthor();
	TSString                      GetAppVersion();
	TSString                      GetAppDescription();
	bool                          IsDomainCreated(const TSString & DomainId);
public:
	//TSDomain
	bool                          SubscribeTopic(const TSString & DomainId,UINT32 TopicId);
	bool                          PublishTopic(const TSString & DomainId, UINT32 TopicId);
	void                          UnsubscribeTopic(const TSString & DomainId, UINT32 TopicId);
	void                          UnpublishTopic(const TSString & DomainId, UINT32 TopicId);
	bool                          IsTopicPublisher(const TSString & DomainId, UINT32 TopicId);
	bool                          IsTopicSubscriber(const TSString & DomainId, UINT32 TopicId);
	bool                          UpdateTopic(const TSString & DomainId, UINT32 TopicId, const TSVariant & Val);
	TSVariant                     CreateTopicFindSetByHandleTopicSet(const TSString & DomainId,UINT32 Handle, const std::vector<UINT32> & Query);
	TSVariant                     CreateTopicFindSetByHandleTopic(const TSString & DomainId,UINT32 Handle, UINT32 TopicId);
	TSVariant                     CreateTopicFindSetByTopicId(const TSString & DomainId,UINT32 TopicId);
	TSVariant                     CreateTopicFindSetBySpacialTopic(const TSString & DomainId,DOUBLE CenterLon, DOUBLE CenterLat, FLOAT SpacialRange, UINT32 TopicId);
	TSVariant                     CreateTopicFindSetBySpacialTopicSet(const TSString & DomainId,DOUBLE CenterLon, DOUBLE CenterLat, FLOAT SpacialRange, const std::vector<UINT32> & Query);
	TSVariant                     CreateTopicFindSetByHandle(const TSString & DomainId,UINT32 Handle);
	TSTopicScriptContextPtr       GetFirstTopic(const TSString & DomainId, TSVariant FindSet);
	TSTopicScriptContextPtr       GetNextTopic(const TSString & DomainId, TSVariant FindSet);
	void                          RegisterTopicCallbackFunction(const TSString & DomainId, UINT32 TopicId, const TSString & CallbackFunctionName);
	void                          RegisterTopicCallbackFile(const TSString & DomainId, UINT32 TopicId, const TSString & FilePath);
private:
	void                          OnReceivedTopic(const TSDomainId & DomainId, TSTopicContextPtr Ctx);
	TSScriptContextPrivate * _p;
};

STRUCT_PTR_DECLARE(TSScriptContext);

#endif


