#ifndef __TSGENSUPPORT_H__
#define __TSGENSUPPORT_H__

#include "TSBuiltinPlugin.h"
#include "TSIDLUtils.h"

struct TSGenSupportPluginPrivate;

struct TSCPTopicFlag
{
	bool buildTopic;
	bool buildSupport;
	bool buildDefine;
	bool buildPython;
};

struct TSCPServiceFlag
{
	bool buildTopic;
	bool buildSupport;
	bool buildDefine;
	bool buildService;
	bool buildProxy;
	bool buildMethod;
	bool buildMethodImpl;
	bool buildPythonService;
};

struct TSCPServiceVector
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

struct TSCPTopicVector
{
	std::vector<TSString> buildTopicVec;
	std::vector<TSString> buildSupportVec;
	std::vector<TSString> buildDefineVec;
	std::vector<TSString> buildPythonVec;
};

struct TSCPStoresRecords
{
	std::vector<TSString>           buildTopicRec;
	std::vector<TSString>           buildSupportRec;
	std::vector<TSString>           buildDefineRec;
};

class TOPSIMIDL_DECL TSGenSupportPlugin : public TSIIDLPlugin
{
	TS_MetaType(TSGenSupportPlugin,TSIIDLPlugin);
public:
	TSGenSupportPlugin(void);
	~TSGenSupportPlugin(void);

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
    void GenerateTopicHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);

    void BuildSupport();
    void GenerateSupportHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);
    void GenerateSupportCPP(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);

    void BuildPython();
    TSString GeneratePython(TSProjFilePtr ProjFile, TSProjFilePtr ProjHFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSTopicTypeDefPtr topicPtr = TSTopicTypeDefPtr());

    void BuildPythonService();
    void GeneratePythonService(TSProjFilePtr ProjFile, std::vector<TSTypeDefWithFuncAndFiledPtr> TypeDef, std::vector<TSString>& serviceName);
    void GeneratePythonServerPy(TSProjFilePtr ProjFile, TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GeneratePythonProxyPy(TSProjFilePtr ProjFile, TSTypeDefWithFuncAndFiledPtr TypeDef);

    void BuildDefine();
    void GenerateDefineHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, TSTopicTypeDefPtr topicPtr = TSTopicTypeDefPtr());
    void GenerateDefineCPP(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, TSTopicTypeDefPtr topicPtr = TSTopicTypeDefPtr());

    void BuildTypeDefine();
    void GenerateTypeDefineHeader(TSProjFilePtr ProjFile,TSTypeDefineLinkPtr TypeDef);

    void BuildService();
    void GenerateServiceHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);
    void GenerateServiceCPP(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);
    void GenerateServerSkelHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateServerSkelCpp(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, bool buildServiceFlag);

    void BuildRemoteMethod();
    void GenerateRemoteMethodHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateRemoteMethodCPP(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);

    void BuildProxy();
    void GenerateProxyHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);
    void GenerateProxyCPP(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, bool buildProxyFlag);
    void GenerateProxyFactoryHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateProxyFactoryCPP(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);

    void BuildMethod();
    void GenerateMethod(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);
    void GenerateMethodFactory(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateMethodArchiveHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateMethodArchiveCPP(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);

    void BuildMethodImpl();
    void GenerateMethodImplHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);
    void GenerateMethodImplCPP(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);
    void GenerateMethodFactoryImplHeader(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateMethodFactoryImplCPP(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateCallBackImpl(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateCallBackHeaderImpl(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateSubscribeFactoryImpl(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);
    void GenerateSubscribeFactoryHeaderImpl(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef);

    void ReplaceSerializeStatic( TSString &Code,TSTypeDefWithFiledPtr TypeDef,TSString SuperName = "");
    TSString GetTopicFlag( TSTypeDef::EnumTypeDef DataType );
    TSString GetAttributes(TSTypeDefWithFiledPtr TypeDef,const TSString & NameSpace = "");
    TSString GetAttributesImpl(TSTypeDefWithFiledPtr TypeDef,TSString NameSpace);
    TSString GetFunctions(TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, const TSString & Namespace = "");
    TSString GetFunctionsAddHandler( TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, TSString NameSpace );
    TSString GetFunctionsImpl(TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, TSString NameSpace);
    TSString GetFunctionsAsync(TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, const TSString & Namespace = "");
    TSString GetFunctionsAsyncImpl(TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, TSString NameSpace);
    TSString GetFunctionsAsyncHandle(TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName);
    TSString GetFunctionsAsyncHandleImpl(TSTypeDefWithFuncAndFiledPtr TypeDef, TSString NameSpace);
    TSString GetFunCode(TSFunctionDefPtr Func,const TSString & Namespace,bool IsService = false);
    TSString GetFunAndHandleCode(TSFunctionDefPtr Func,const TSString & Namespace);
    TSString GetRecourseCode(TSTypeDefPtr Type,TSString Name, bool & IsSimple);
    TSString GetArchiveCode(TSTypeDefWithFuncAndFiledPtr TypeDef);
    TSString RecourseParam(TSParamDefPtr Param);
    TSString RecourseGetCode(TSParamDefPtr Param);
    TSString GenTopicWithName(TSTypeDefWithFuncAndFiledPtr TypeDef,TSString TopicName, std::vector<TSString>& serviceName);
    TSString GenSupportWithName(TSTypeDefWithFuncAndFiledPtr TypeDef,TSString TopicName, std::vector<TSString>& serviceName);
    TSString GenSupportCPPWithName(TSTypeDefWithFuncAndFiledPtr TypeDef,TSString TopicName, std::vector<TSString>& serviceName);
    TSString GenDefineWithName(TSTypeDefWithFuncAndFiledPtr TypeDef,TSString TopicName, std::vector<TSString>& serviceName, bool IsHeader = false);
    TSString GetServiceFunction(TSTypeDefWithFunctionPtr TypeDef, std::vector<TSString>& serviceName);
    TSString GetIncludeHeadFile(TSTypeDefWithFuncAndFiledPtr TypeDef,const TSString & PathName);
    TSString InvokerAndInitializeFunc(TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, bool IsService = true, bool InvokerAndInitializeFunc = false);
    TSString GetFuncParamsString(TSFunctionDefPtr Function);
    TSString GetFuncOutParamsString(TSFunctionDefPtr Function);
    TSString GetFuncInParamsString(TSFunctionDefPtr Function);

    TSString GetPythonStructDef(std::vector<TSTypeDefWithFuncAndFiledPtr> TypeDef);
	std::vector<TSString> GetVectorDifference(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& topicVec);
	void                             DeleteStringOfFile(TSProjFilePtr ProjFile, TSString& str);
	std::vector<TSString> GetTopicInVec(std::vector<TSString>& tempTopicVec, std::vector<TSTypeDefWithFuncAndFiledPtr>& TypeDefs);
	std::vector<TSString> GetTopicInVec(std::vector<TSString>& tempTopicVec, std::vector<TSTopicTypeDefPtr>& TypeDefs);
	void                             GetServiceAndStructMap(std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs, std::map<TSString, TSString>& serviceMap);
	void                             StoreServiceAboutStruct(std::map<TSString, TSString>& serviceMap, std::vector<TSString>& serviceVec, std::vector<TSString>& indexServiceVec, std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec);
	bool                             FilterNotSelectNode(TSTypeDefPtr TypeDef, std::vector<TSString>& indexVec, std::vector<TSString>& structVec, std::vector<TSString>& storeStructRec);
	bool                             GetGenerateService(std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec, std::vector<TSString>& serviceVecStruct, bool serviceFlag);
	bool                             GetGenerateTopic(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& tempTopicVec, std::vector<TSString>& topicVecStruct, bool topicFlag);
private:
	TSGenSupportPluginPrivate * _p;
	TSCPTopicVector                         _topicVecStruct;
	TSCPTopicFlag*                           _topicFlagStruct;
	TSCPServiceFlag*                        _serviceFlagStruct;
	TSCPServiceVector                     _serviceVecStruct;
	TSCPStoresRecords                    _storeStructRec;
};
CLASS_PTR_DECLARE(TSGenSupportPlugin);

#endif

