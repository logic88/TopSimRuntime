#ifndef __TSCSHARP_PLUGIN_H__
#define __TSCSHARP_PLUGIN_H__

#include "TopSimIDL.h"
#include "TSBuiltinPlugin.h"
#include "TSIDLUtils.h"
#include "TSSolutionMgr.h"

struct TSCSTopicFlag
{
	bool buildTopic;
	bool buildSupport;
	bool buildDefine;
};

struct TSCSServiceFlag
{
	bool buildTopic;
	bool buildSupport;
	bool buildDefine;
	bool buildServiceSkel;
	bool buildProxy;
	bool buildMethod;
	bool buildMethodImpl;
};

struct TSCSServiceVector
{
	std::vector<TSString> buildTopicVec;
	std::vector<TSString> buildSupportVec;
	std::vector<TSString> buildDefineVec;
	std::vector<TSString> buildServiceVec;
	std::vector<TSString> buildProxyVec;
	std::vector<TSString> buildMethodVec;
	std::vector<TSString> buildMethodImplVec;
	std::vector<TSString> buildPythonServiceVec;
};

struct TSCSTopicVector
{
	std::vector<TSString> buildTopicVec;
	std::vector<TSString> buildSupportVec;
	std::vector<TSString> buildDefineVec;
	std::vector<TSString> buildPythonVec;
};

struct TSCSStoresRecords
{
	std::vector<TSString>           buildTopicRec;
	std::vector<TSString>           buildSupportRec;
	std::vector<TSString>           buildDefineRec;
};


struct TSCSharpPluginPrivate;
class TOPSIMIDL_DECL TSCSharpPlugin : public TSIIDLPlugin
{
    TS_MetaType(TSCSharpPlugin,TSIIDLPlugin);
public:
    TSCSharpPlugin(void);
    ~TSCSharpPlugin(void);

public:
    virtual TSString         GetName();
    virtual void             OnPluginLoaded();
    virtual bool             DoParse();
    virtual bool             DoBuild();
    virtual void             Clean();
    void BuildCallBackImpl(TSTypeDefPtr TypeDef,const TSString & FileName);

private:
    void BuildExport(TSString FileName);

    void BuildTopic();
    void GenerateTopic(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateTopicWithService(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, bool FirstOrNot,const bool & Requst = true);
	bool FilterNotSelectNode(TSTypeDefPtr TypeDef, std::vector<TSString>& indexVec, std::vector<TSString>& structVec, std::vector<TSString>& storeStructRec);

    void BuildSupport();
    void GenerateSupport(TSProjFilePtr ProjFile,TSTypeDefWithFiledPtr TypeDef);
    void GenerateSupportWithService(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName,const bool & Requst = true);
    void BuildDefine();
    void GenerateDefine(TSProjFilePtr ProjFile,TSTypeDefWithFiledPtr TypeDef,TSTopicTypeDefPtr TopicDef);
    void GenerateDefineWithService(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,const TSString & Requst, std::vector<TSString>& serviceName);

    void BuildMethod();
    void GenerateMethod(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);

    void BuildProxy();
    void GenerateProxy(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);

    void BuildRemoteMethod();
    void GenerateRemoteMethod(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);

    void BuildServer();
    void GenerateServer(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);

    void BuildServiceSkel();
    void GenerateServiceSkel(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);

    void BuildMethodImpl();
    void GenerateMethodImpl(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);

    void GenerateCallBackImpl( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef );
    TSString GetCSharpValueType( TSTypeDefPtr tmpDef);

	void GetServiceAndStructMap(std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs, std::map<TSString, TSString>& serviceMap);
	bool GetGenerateService(std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec, std::vector<TSString>& serviceVecStruct, bool serviceFlag);
	void StoreServiceAboutStruct(std::map<TSString, TSString>& serviceMap, std::vector<TSString>& serviceVec, std::vector<TSString>& indexServiceVec, std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec);
	bool GetGenerateTopic(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& tempTopicVec, std::vector<TSString>& topicVecStruct, bool topicFlag);
	std::vector<TSString> GetTopicInVec(std::vector<TSString>& tempTopicVec, std::vector<TSTypeDefWithFuncAndFiledPtr>& TypeDefs);
	std::vector<TSString> GetTopicInVec(std::vector<TSString>& tempTopicVec, std::vector<TSTopicTypeDefPtr>& TypeDefs);
	std::vector<TSString> GetVectorDifference(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& topicVec);

private:
    TSCSharpPluginPrivate * _p;
	TSCSTopicVector                  _topicVecStruct;
	TSCSTopicFlag*                    _topicFlagStruct;
	TSCSServiceFlag*                 _serviceFlagStruct;
	TSCSServiceVector               _serviceVecStruct;
	TSCSStoresRecords              _storeStructRec;
};
CLASS_PTR_DECLARE(TSCSharpPlugin);
#endif // __TSCSHARP_PLUGIN_H__
