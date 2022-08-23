#ifndef __TSJAVA_PLUGIN_H__
#define __TSJAVA_PLUGIN_H__

#include<boost/filesystem/fstream.hpp>
#include "TopSimIDL.h"
#include "TSBuiltinPlugin.h"
#include "TSIDLUtils.h"
#include "TSSolutionMgr.h"

struct TSTopicFlag
{
	bool buildTopic;
	bool buildProject;
	bool buildClassPath;
};

struct TSServiceFlag
{
	bool buildTopic;
	bool buildProject;
	bool buildClassPath;
	bool buildService;
};

struct TSServiceVector
{
	std::vector<TSString> buildTopicVec;
	std::vector<TSString> buildProjectVec;
	std::vector<TSString> buildClassPathVec;
	std::vector<TSString> buildServiceVec;
};

struct TSTopicVector
{
	std::vector<TSString> buildTopicVec;
	std::vector<TSString> buildProjectVec;
	std::vector<TSString> buildClassPathVec;
};

struct TSStoresRecords
{
	std::vector<TSString>           buildTopicRec;
	std::vector<TSString>           buildProjectRec;
	std::vector<TSString>           buildClassPathRec;
};

struct TSJAVAPluginPrivate;

class TOPSIMIDL_DECL TSJavaPlugin : public TSIIDLPlugin
{
    TS_MetaType(TSJavaPlugin,TSIIDLPlugin);
public:
    TSJavaPlugin(void);
    ~TSJavaPlugin(void);

public:
    virtual TSString       GetName();
    virtual void             OnPluginLoaded();
    virtual bool             DoParse();
    virtual bool             DoBuild();
    virtual void             Clean();
    void BuildCallBackImpl(TSTypeDefPtr TypeDef,const TSString & FileName);
    void GenerateClassPathFile(TSProjFilePtr ProjFile,bool HasPubSub);

private:
    void BuildClassPath();
    void BuildProject();
    void BuildTopic();
    void BuildService();

    void GenerateTOPICFile(TSProjFilePtr ProjFile,TSTypeDefPtr TypeDef, TSTypeDefWithFiledPtr  structPtr);
    void GenerateTOPICStructFile(TSProjFilePtr ProjFile,TSTypeDefPtr TypeDef);
    void GenerateTOPICEnumFile(TSProjFilePtr ProjFile,TSEnumDefPtr TypeDef,TSString filename);
    void GenerateTOPICSupportFile(TSProjFilePtr ProjFile,TSTypeDefPtr TypeDef );

    void GenerateSERVICEFile(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString className);
    void GenerateSERVICEStructFile(TSProjFilePtr ProjFile,TSFunctionDefPtr Function,TSString className,TSString ParseFileBaseName,bool IsinOrOut = true);
    void GenerateSERVICESupportFile(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString className,TSString HashVersionCode = "0");
    TSString GetJavaFunctions(TSFunctionDefPtr Func,TSString PackageName,bool IsAsync = false);
    TSString GetJavaFunctionsWithOutType(TSFunctionDefPtr Func,TSString PackageName,bool IsAsync = false);
    TSString GetJAVAFunctioinType( TSTypeDefPtr tmpDef);

    void GenerateMethodFile(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString className,TSString ParseFileBaseName, std::vector<TSString>& serviceName);
    void GenerateProxyFile(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString className,TSString ParseFileBaseName, std::vector<TSString>& serviceName);
    void GenerateProxyInterFile(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString className,TSString ParseFileBaseName, std::vector<TSString>& serviceName);
    void GenerateServerSkelsFile(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString className,TSString ParseFileBaseName, std::vector<TSString>& serviceName);
    void GenerateOutClassFile(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString className,TSString ParseFileBaseName);

    void GenerateProjectFile(TSProjFilePtr ProjFile,TSTypeDefPtr TypeDef);

    TSString GetImportJarNameFunc(TSFunctionDefPtr Function,TSString ParseFileBaseName);
   
    TSString GetImportJarNameTopic(TSTypeDefPtr Typedef,TSString ParseFileBaseName);

	bool GetGenerateService(std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec, std::vector<TSString>& serviceVecStruct, bool serviceFlag);
	std::vector<TSString> GetVectorDifference(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& topicVec);
	bool GetGenerateTopic(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& tempTopicVec, std::vector<TSString>& topicVecStruct, bool topicFlag);
	void StoreServiceAboutStruct(std::map<TSString, TSString>& serviceMap, std::vector<TSString>& serviceVec, std::vector<TSString>& indexServiceVec, std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec);
	bool FilterNotSelectNode(TSTypeDefPtr TypeDef, std::vector<TSString>& indexVec, std::vector<TSString>& structVec, std::vector<TSString>& storeStructRec);
	void GetServiceAndStructMap(std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs, std::map<TSString, TSString>& serviceMap);
	std::vector<TSString> GetTopicInVec(std::vector<TSString>& tempTopicVec, std::vector<TSTypeDefPtr>& TypeDefs);
private:

    TSJAVAPluginPrivate* _p;
	TSTopicVector            _topicVecStruct;
	TSTopicFlag*              _topicFlagStruct;
	TSServiceFlag*           _serviceFlagStruct;
	TSServiceVector         _serviceVecStruct;
	TSStoresRecords        _storeStructRec;
};
CLASS_PTR_DECLARE(TSJavaPlugin);
#endif // __TSJAVA_PLUGIN_H__
