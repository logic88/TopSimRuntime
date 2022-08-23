#include "stdafx.h"

#include <boost/foreach.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/filesystem.hpp>

#include "TSIDLUtils.h"
#include "TSJavaPlugin.h"

#include "temp/temp.h"
#include "temp/java.temp.h"
#include "TSCSharpPlugin.h"

using namespace TSIDLUtils;
BEGIN_METADATA(TSJavaPlugin)
    REG_BASE(TSIIDLPlugin);
END_METADATA()


struct TSJAVAPluginPrivate
{
    TSIBuiltinPluginPtr _BuiltinPlugin;
};

TSJavaPlugin::TSJavaPlugin(void)
    :_p(new TSJAVAPluginPrivate)
{
	_topicFlagStruct = new TSTopicFlag;
	_topicFlagStruct->buildTopic = false;
	_topicFlagStruct->buildProject = false;
	_topicFlagStruct->buildClassPath = false;

	_serviceFlagStruct = new TSServiceFlag;
	_serviceFlagStruct->buildClassPath = false;
	_serviceFlagStruct->buildProject = false;
	_serviceFlagStruct->buildService = false;
	_serviceFlagStruct->buildTopic = false;
}

TSJavaPlugin::~TSJavaPlugin(void)
{
    delete _p;
	_p = NULL;
	delete _topicFlagStruct;
	_topicFlagStruct = NULL;
	delete _serviceFlagStruct;
	_serviceFlagStruct = NULL;
}

TSString TSJavaPlugin::GetName()
{
    return GB18030ToTSString("Java 文件生成插件");
}

void TSJavaPlugin::OnPluginLoaded()
{
	_p->_BuiltinPlugin = GetPluginT<TSIBuiltinPlugin>();
	ASSERT(_p->_BuiltinPlugin && "TSJavaPlugin");
}

bool TSJavaPlugin::DoParse()
{
    return true;
}

bool TSJavaPlugin::DoBuild()
{
    TSString Java = TSIDLPluginMgr::Instance()->GetCodeType();
    if(Java != "java")
    {
        return true;
    }

    BuildProject();

    BuildClassPath();

    BuildTopic();

    BuildService();

    return true;
}



void TSJavaPlugin::Clean()
{

}

void TSJavaPlugin::BuildClassPath()
{
    TSProjFilePtr ProjectFile;
    std::vector<TSTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTypes();
	std::vector<TSServiceDefPtr> TypeDefServices = _p->_BuiltinPlugin->GetAllServiceDefs();

	//服务依赖的结构体
	std::map<TSString, TSString> serviceMap;
	TSString tempName;
	BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr typeDef, TypeDefServices)
	{
		if (typeDef->Type == TSTypeDef::Service)
		{
			BOOST_FOREACH(TSFunctionDefPtr Func, typeDef->Functions)
			{
				BOOST_FOREACH(TSParamDefPtr param, Func->Params)
				{
					if (tempName != param->Def->TypeName)
					{
						std::pair<TSString, TSString> servicePair(Func->ValName, param->Def->TypeName);
						serviceMap.insert(servicePair);
					}
					tempName = param->Def->TypeName;
				}
			}
		}
	}

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildClassPathVec, _serviceFlagStruct->buildClassPath);

	std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

	//接收选中的主题
	std::vector<TSString> vecSelectTopic = TSIDLPluginMgr::Instance()->GetSelectTopicVector();
	std::vector<TSString> tempTopicVec;
	//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
	bool emptyTopicFlag = GetGenerateTopic(vecSelectTopic, tempTopicVec, _topicVecStruct.buildClassPathVec, _topicFlagStruct->buildClassPath);

	std::vector<TSString> indexVec;
	if (!tempTopicVec.empty() && !vecSelectTopic.empty())
	{
		//用来存储没有被选中的主题
		//indexVec = GetTopicInVec(tempTopicVec, TypeDefs);
	}
	//if (!vecSelectTopic.empty())
	//{
	//	//根据选中的主题查找TypeDefs中不被使用的主题
	//	for (int i = 0; i < TypeDefs.size(); ++i)
	//	{
	//		if (TypeDefs[i]->Type == TSTypeDef::Topic)
	//		{
	//			TSString tempTypeName = TypeDefs[i]->TypeName;
	//			std::cout << "BuildDefine: " << tempTypeName << "类型：" << TypeDefs[i]->Type;
	//			boost::algorithm::replace_all(tempTypeName, "_", "::");
	//			std::vector<TSString>::iterator iterTopic = std::find(vecSelectTopic.begin(), vecSelectTopic.end(), tempTypeName);
	//			if (iterTopic == vecSelectTopic.end())
	//			{
	//				indexVec.push_back(TypeDefs[i]->TypeName);
	//			}
	//		}
	//	}
	//}

    bool init_projectfile = true;
    BOOST_FOREACH(TSTypeDefPtr TypeDef,TypeDefs)
    {
		//过滤掉没有选中的节点以及可能重复生成的节点
		if (FilterNotSelectNode(TypeDef, indexVec, structVec, _storeStructRec.buildClassPathRec))
		{
			continue;
		}

		//如果没有选中服务，不生成服务
		if (!vecSelectService.empty())
		{
			if (vecSelectService[0] == "N")
			{
				if (TypeDef->Type == TSTypeDef::Service)
				{
					continue;
				}
			}
		}

		//存储生成的结构体
		_storeStructRec.buildClassPathRec.push_back(TypeDef->TypeName);

        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                ProjectFile = Dir->GetOrCreateProjFile(".","classpath");
                GenerateClassPathFile(ProjectFile,false);            
                ProjectFile->SetFileType(None);

            }
        }
    }

}

void TSJavaPlugin::BuildProject()
{
    TSProjFilePtr ProjectFile;
    std::vector<TSTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTypes();
	std::vector<TSServiceDefPtr> TypeDefServices = _p->_BuiltinPlugin->GetAllServiceDefs();

	//服务依赖的结构体
	/*std::map<TSString, TSString> serviceMap;
	TSString tempName;
	BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr typeDef, TypeDefServices)
	{
		if (typeDef->Type == TSTypeDef::Service)
		{
			BOOST_FOREACH(TSFunctionDefPtr Func, typeDef->Functions)
			{
				BOOST_FOREACH(TSParamDefPtr param, Func->Params)
				{
					if (tempName != param->Def->TypeName)
					{
						std::pair<TSString, TSString> servicePair(Func->ValName, param->Def->TypeName);
						serviceMap.insert(servicePair);
					}
					tempName = param->Def->TypeName;
				}
			}
		}
	}*/

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildProjectVec, _serviceFlagStruct->buildProject);

	std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

	//接收选中的主题
	std::vector<TSString> vecSelectTopic = TSIDLPluginMgr::Instance()->GetSelectTopicVector();
	std::vector<TSString> tempTopicVec;
	//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
	bool emptyTopicFlag = GetGenerateTopic(vecSelectTopic, tempTopicVec, _topicVecStruct.buildProjectVec, _topicFlagStruct->buildProject);

	std::vector<TSString> indexVec;
	if (!tempTopicVec.empty() && !vecSelectTopic.empty())
	{
		//用来存储没有被选中的主题
		//indexVec = GetTopicInVec(tempTopicVec, TypeDefs);
	}
	//if (!vecSelectTopic.empty())
	//{
	//	//根据选中的主题查找TypeDefs中不被使用的主题
	//	for (int i = 0; i < TypeDefs.size(); ++i)
	//	{
	//		TSString tempTypeName = TypeDefs[i]->TypeName;
	//		std::vector<TSString>::iterator iterTopic = std::find(vecSelectTopic.begin(), vecSelectTopic.end(), tempTypeName);
	//		if (iterTopic == vecSelectTopic.end())
	//		{
	//			indexVec.push_back(TypeDefs[i]->TypeName);
	//		}
	//	}
	//}

    bool init_projectfile = true;
    BOOST_FOREACH(TSTypeDefPtr TypeDef,TypeDefs)
    {
		//过滤掉没有选中的节点以及可能重复生成的节点
		if (FilterNotSelectNode(TypeDef, indexVec, structVec, _storeStructRec.buildProjectRec))
		{
			continue;
		}

		//如果没有选中服务，不生成服务
		if (!vecSelectService.empty())
		{
			if (vecSelectService[0] == "N")
			{
				if (TypeDef->Type == TSTypeDef::Service)
				{
					continue;
				}
			}
		}

		//存储生成的结构体
		_storeStructRec.buildProjectRec.push_back(TypeDef->TypeName);

        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                ProjectFile = Dir->GetOrCreateProjFile(".","project");
                GenerateProjectFile(ProjectFile,TypeDef);        
                ProjectFile->SetFileType(None);

            }
        }
    }
   
}

void TSJavaPlugin::BuildTopic()
{
    TSProjFilePtr TOPICFile, TOPICStructFile, TOPICSupportFile;
    TSStructDefPtr  structPtr;

    std::vector<TSTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTypes();
	std::vector<TSServiceDefPtr> TypeDefServices = _p->_BuiltinPlugin->GetAllServiceDefs();

	//服务依赖的结构体
	/*std::map<TSString, TSString> serviceMap;
	TSString tempName;
	BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr typeDef, TypeDefServices)
	{
		if (typeDef->Type == TSTypeDef::Service)
		{
			BOOST_FOREACH(TSFunctionDefPtr Func, typeDef->Functions)
			{
				BOOST_FOREACH(TSParamDefPtr param, Func->Params)
				{
					if (tempName != param->Def->TypeName)
					{
						std::pair<TSString, TSString> servicePair(Func->ValName, param->Def->TypeName);
						serviceMap.insert(servicePair);
					}
					tempName = param->Def->TypeName;
				}
			}
		}
	}*/

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	//std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildTopicVec, _serviceFlagStruct->buildTopic);

	//std::vector<TSString> structVec;
	//std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

	//接收选中的主题
	std::vector<TSString> vecSelectTopic = TSIDLPluginMgr::Instance()->GetSelectTopicVector();
	//std::vector<TSString> tempTopicVec;
	//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
	//bool emptyTopicFlag = GetGenerateTopic(vecSelectTopic, tempTopicVec, _topicVecStruct.buildTopicVec, _topicFlagStruct->buildTopic);

	//std::vector<TSString> indexVec;
	//if (!tempTopicVec.empty() && !vecSelectTopic.empty())
	//{
	//	//用来存储没有被选中的主题
	//	indexVec = GetTopicInVec(tempTopicVec, TypeDefs);
	//}

	//if (!vecSelectTopic.empty())
	//{
	//	//根据选中的主题查找TypeDefs中不被使用的主题
	//	for (int i = 0; i < TypeDefs.size(); ++i)
	//	{
	//		if (TypeDefs[i]->Type == TSTypeDef::Topic)
	//		{
	//			TSString tempTypeName = TypeDefs[i]->TypeName;
	//			std::cout << "BuildDefine: " << tempTypeName << "类型：" << TypeDefs[i]->Type;
	//			boost::algorithm::replace_all(tempTypeName, "_", "::");
	//			std::vector<TSString>::iterator iterTopic = std::find(vecSelectTopic.begin(), vecSelectTopic.end(), tempTypeName);
	//			if (iterTopic == vecSelectTopic.end())
	//			{
	//				indexVec.push_back(TypeDefs[i]->TypeName);
	//			}
	//		}
	//	}
	//}

	////接收选中的主题
	//std::vector<TSString> vecSelectTopic = TSIDLPluginMgr::Instance()->GetSelectTopicVector();
	//std::vector<TSString> indexVec;

	//if (!vecSelectTopic.empty())
	//{
	//	//根据选中的主题查找TypeDefs中不被使用的主题
	//	for (int i = 0; i < TypeDefs.size(); ++i)
	//	{
	//		bool flag = false;
	//		if (TypeDefs[i]->Type == TSTypeDef::Topic)
	//		{
	//			TSString tempTypeName = TypeDefs[i]->TypeName;
	//			boost::algorithm::replace_all(tempTypeName, "_", "::");
	//			std::vector<TSString>::iterator iterTopic = std::find(vecSelectTopic.begin(), vecSelectTopic.end(), tempTypeName);
	//			if (iterTopic == vecSelectTopic.end())
	//			{
	//				indexVec.push_back(TypeDefs[i]->TypeName);
	//			}
	//		}
	//		else if (TypeDefs[i]->Type == TSTypeDef::Struct)
	//		{
	//			for (int j = 0; j < vecSelectTopic.size(); ++j)
	//			{
	//				int pos = vecSelectTopic[j].rfind(":");
	//				//获得结构体字符串
	//				TSString str = vecSelectTopic[j].substr(pos + 1, vecSelectTopic[j].size());

	//				if (TypeDefs[i]->TypeName == str)
	//				{
	//					flag = true;
	//					break;
	//				}
	//			}
	//			if (!flag)
	//			{
	//				indexVec.push_back(TypeDefs[i]->TypeName);
	//			}
	//		}
	//	}
	//}

    std::vector<TSEnumDefPtr> EnumTypeDefs = _p->_BuiltinPlugin->GetAllEnumDefs();
    BOOST_FOREACH(TSEnumDefPtr TypeDef,EnumTypeDefs)
    {
        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
            || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            TSString ParseFileBaseName = 
                GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                if(TSEnumDefPtr tempstructPtr = TS_CAST(TypeDef,TSEnumDefPtr))
                {
                    TSString filename;
                    if(TSIBuiltinPlugin::GetSuitFullTypeName(tempstructPtr,true) != "")
                    {
                        filename = TSIBuiltinPlugin::GetSuitFullTypeName(tempstructPtr,true) + "_" +  tempstructPtr->TypeName;
                    }
                    else
                    {
                       filename = TSIBuiltinPlugin::GetSuitFullTypeName(tempstructPtr,true) + tempstructPtr->TypeName;
                    }
                    
                    boost::algorithm::replace_all(filename,"::","_");
                    TOPICStructFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "4Java" + "/" + filename ,".java");
                    GenerateTOPICEnumFile( TOPICStructFile, TypeDef,filename);
                   // TSIDLUtils::GenerateEnum( TOPICStructFile, TypeDef);
                    TOPICStructFile->SetFileType(Generic);
                }  
            }
        }
    }
     BOOST_FOREACH(TSTypeDefPtr TypeDef,TypeDefs)
    {
		 //过滤掉没有选中的节点以及可能重复生成的节点
		 /*if (FilterNotSelectNode(TypeDef, indexVec, structVec, _storeStructRec.buildTopicRec))
		 {
			 continue;
		 }*/

		 //跳过没有选中的主题
		/* if (!indexVec.empty())
		 {
			 if (TypeDef->Type == TSTypeDef::Topic)
			 {
				 TSString tempTopic = TypeDef->TypeName;
				 int pos = tempTopic.rfind("_");
				 TSString tempStruct = tempTopic.substr(pos + 1);
				 std::vector<TSString>::iterator iterStruct = std::find(indexVec.begin(), indexVec.end(), tempStruct);
				 if (iterStruct != indexVec.end())
				 {
					 continue;
				 }
			 }
		 }*/

		 //如果没有选中服务，不生成服务
		if (!vecSelectService.empty())
		 {
			 if (vecSelectService[0] == "N")
			 {
				 if (TypeDef->Type == TSTypeDef::Service)
				 {
					 continue;
				 }
			 }
		 }

		 //存储生成的结构体
		 _storeStructRec.buildTopicRec.push_back(TypeDef->TypeName);

        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
            || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            TSString ParseFileBaseName = 
                GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
            if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
            {
                if(TSSolutionPtr Solution = 
                    SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
                {
                    TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

                    if(TSStructDefPtr tempstructPtr = TS_CAST(TypeDef,TSStructDefPtr))//对枚举例子处理有问题
                    {
                       TSString filename = TSIBuiltinPlugin::GetSuitFullTypeName(tempstructPtr,true);
                       boost::algorithm::replace_all(filename,"::","_");
                       TOPICStructFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "4Java" + "/" + filename,".java");
                       GenerateTOPICStructFile( TOPICStructFile, TypeDef);
                       TOPICStructFile->SetFileType(Generic);

                       TOPICSupportFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "4Java" + "/" + filename,"Support.java");
                       GenerateTOPICSupportFile(TOPICSupportFile, TypeDef);
                       TOPICSupportFile->SetFileType(Generic);

                       structPtr = tempstructPtr;
                    }                  
                   else  if(!TS_CAST(TypeDef,TSEnumDefPtr) && TypeDef->Type == TSTypeDef::Topic)
                    {
                      TSString filename = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
                      boost::algorithm::replace_all(filename,"::","_");
                      TOPICFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "4Java" + "/" + filename,"Topic.java");//TOPIC NAME

                      TSTopicTypeDefPtr topicDef = TS_CAST(TypeDef,TSTopicTypeDefPtr);
                      TSTypeDefWithFiledPtr structDef = TS_CAST(topicDef->Def.lock(),TSTypeDefWithFiledPtr);

                      GenerateTOPICFile( TOPICFile, TypeDef, structDef);
                      TOPICFile->SetFileType(Generic);
                    }
                }
           }
        }      
    } 
}

void TSJavaPlugin::BuildService()
{
    TSProjFilePtr ServiceFile, ServiceStructFile, ServiceSupportFile,MethodFile,ProxyFile,ProxyInterFile,RemoteMethodFile,ServerSkelsFile,OutClassFile;

    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();
	//std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	//GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	//std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildServiceVec, _serviceFlagStruct->buildService);

	//std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
		//if (TypeDef->Type != TSTypeDef::Service)
		//{
		//	continue;
		//}
		//else if (TypeDef->Type == TSTypeDef::Service)
		//{
		//	//服务不再重复生成
		//	/*if (_serviceFlagStruct->buildService)
		//	{
		//		continue;
		//	}*/
		//	//如果没有选中服务则不生成服务
		//	if (!vecSelectService.empty())
		//	{
		//		if (vecSelectService[0] == "N")
		//		{
		//			continue;
		//		}
		//	}
		//}

        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
            || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            TSString ParseFileBaseName = 
                GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
            TSString FilePackageName = "package " + TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true)+ "4Java;\n";
            boost::algorithm::replace_all(FilePackageName,"::","_");

            if(TypeDef->Type != TSTypeDef::Service)
            {
                continue;
            }

            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                TSString filename = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
                boost::algorithm::replace_all(filename,"::","_");

                ServiceStructFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/" + filename,".java");
                TSFunctionDefPtr Function = NULL;
                GenerateSERVICEStructFile(ServiceStructFile, Function,filename,ParseFileBaseName);
                ServiceStructFile->SetFileType(Generic);

                ServiceSupportFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl"+  "/" + filename,"Support.java");
                GenerateSERVICESupportFile(ServiceSupportFile, TypeDef,filename);
                ServiceSupportFile->SetFileType(Generic);

                ServiceFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/" + filename,"Topic.java");
                GenerateSERVICEFile(ServiceFile, TypeDef,filename);
                ServiceFile->SetFileType(Generic);         
                bool firstNotInFile = true;
                bool firstNotOutFile = true;

                TSString Path = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
                boost::algorithm::replace_all(Path,"::","_");
                MethodFile = Dir->GetOrCreateProjFile(Path + "4Java" + "/" , "Method.java");
                GenerateMethodFile(MethodFile,TypeDef, FilePackageName,ParseFileBaseName, indexServiceVec);
                MethodFile->SetFileType(Generic);

                ProxyFile = Dir->GetOrCreateProjFile(Path + "4Java" + "/" , "Proxy.java");
                GenerateProxyFile(ProxyFile,TypeDef, FilePackageName,ParseFileBaseName, indexServiceVec);
                ProxyFile->SetFileType(Generic);

                ProxyInterFile = Dir->GetOrCreateProjFile(Path + "4Java" + "/", "proxyInter.java");
                GenerateProxyInterFile(ProxyInterFile,TypeDef, FilePackageName,ParseFileBaseName, indexServiceVec);
                ProxyInterFile->SetFileType(Generic);

              /*  RemoteMethodFile = Dir->GetOrCreateProjFile(Path + "4Java" + "/" , "RemoteMethod.java");
                GenerateRemoteMethodFile(RemoteMethodFile,TypeDef, FilePackageName,ParseFileBaseName);
                RemoteMethodFile->SetFileType(Generic);*/

                ServerSkelsFile = Dir->GetOrCreateProjFile(Path + "4Java" + "/" , "ServerSkels.java");
                GenerateServerSkelsFile(ServerSkelsFile,TypeDef, FilePackageName,ParseFileBaseName, indexServiceVec);
                ServerSkelsFile->SetFileType(Generic);


                BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
                {
					/*TSString funName = Function->ValName;
					std::vector<TSString>::iterator iterTopic = std::find(indexServiceVec.begin(), indexServiceVec.end(), funName);
					if (iterTopic == indexServiceVec.end())
					{
						continue;
					}*/
                    //处理参数列表里面只有in或out或inout;
					TSString HashCodeVersion4Res = TSIDLUtils::GenerateTypeVersionNum4Service(Function,false);
                    TSString filenameResponse = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "_" + Function->ValName + "_Response"; 
                    boost::algorithm::replace_all(filenameResponse,"::","_");
                    ServiceStructFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/" + filenameResponse,".java");
                    GenerateSERVICEStructFile(ServiceStructFile, Function,filename,ParseFileBaseName,false);
                    ServiceStructFile->SetFileType(Generic);

                    ServiceSupportFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl"+  "/"  + filenameResponse,"Support.java");
                    GenerateSERVICESupportFile(ServiceSupportFile, TypeDef,filenameResponse,HashCodeVersion4Res);
                    ServiceSupportFile->SetFileType(Generic);

                    ServiceFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/" + filenameResponse,"Topic.java");
                    GenerateSERVICEFile(ServiceFile, TypeDef,filenameResponse);
                    ServiceFile->SetFileType(Generic);   

					TSString HashCodeVersion4Req = TSIDLUtils::GenerateTypeVersionNum4Service(Function,true);
                    TSString filenameRequest = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "_" + Function->ValName + "_Request"; 
                    boost::algorithm::replace_all(filenameRequest,"::","_");
                    ServiceStructFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/"  +  filenameRequest,".java");
                    GenerateSERVICEStructFile(ServiceStructFile, Function,filename,ParseFileBaseName,true);
                    ServiceStructFile->SetFileType(Generic);

                    ServiceSupportFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/"  +  filenameRequest,"Support.java");
                    GenerateSERVICESupportFile(ServiceSupportFile, TypeDef,filenameRequest,HashCodeVersion4Req);
                    ServiceSupportFile->SetFileType(Generic);

                    ServiceFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/"  +  filenameRequest,"Topic.java");
                    GenerateSERVICEFile(ServiceFile, TypeDef,filenameRequest);
                    ServiceFile->SetFileType(Generic);
                    for (unsigned int i = 0; i < Function->Params.size(); i++)
                    {
                        TSParamDefPtr field = Function->Params[i];
                        if(field->Type == TSParamDef::Out ||field->Type == TSParamDef::Inout)
                        {
                            TSString filename = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) ; 
                            boost::algorithm::replace_all(filename,"::","_");
                            OutClassFile = Dir->GetOrCreateProjFile(Path + "4Java" + "/" , filename + "_" + Function->ValName + "_" + field->ValName +".java");
                            TSString content = FilePackageName + "import java.util.Vector;\n" +"import java.util.HashMap;\n" + "import java.util.Map;\n";                               
                            content += GetImportJarNameFunc(Function,ParseFileBaseName);
                            content += "public class " + filename + "_" + Function->ValName + "_" + field->ValName + "\n";
                            //content += "{\n" + Table(1) + "public " + GetJAVAFunctioinType(field->Def) + " " + field->ValName; 
                            if(GetJAVAFunctioinType(field->Def) == "String")
                            {
                                content += "{\n" + Table(1) + "public " + GetJAVAFunctioinType(field->Def) + " " + field->ValName +" = new " + GetJAVAFunctioinType(field->Def) + "();\n}";
                            }
                            else if( field->Def->Type == TSTypeDef::Array)
                            {

                                TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(field->Def,TSVectorBuiltinTypeDefPtr);
                                content += "{\n" + Table(1) + "public Vector<" + GetJAVAFunctioinType(vectorField->Value) + "> " + field->ValName;
                                if (vectorField->Value->Type != TSTypeDef::Builtin||vectorField->Value->Type == TSTypeDef::Enum)
                                {
                                    content += " = new  Vector<" + GetJAVAFunctioinType(vectorField->Value) + ">();\n}"; 
                                }
                                else if(GetJAVAFunctioinType(field->Def) == "String[]")
                                {
                                    content += " = new String[1];\n}";
                                }
                                else
                                {
                                    content += " = new " + GetJAVAFunctioinType(field->Def) + "();\n}";
                                }
                               
                            }
                            else if ( field->Def->Type == TSTypeDef::Map)
                            {
                                content += "{\n" + Table(1) + "public " + GetJAVAFunctioinType(field->Def) + " " + field->ValName; 
                                TSMapBuiltinTypeDefPtr mapField = TS_CAST(field->Def,TSMapBuiltinTypeDefPtr);

                                TSString mapkey = "Number";
                                TSString mapvalue = "Number";
                                if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) ==  "String")
                                {
                                    mapkey = "String";
                                }
                                else if (mapField->Key->Type == TSTypeDef::Enum)
                                {
                                    mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key); 
                                }
                                else if( mapField->Key->Type == TSTypeDef::Struct)
                                {
                                    mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key,true); 
                                }
                                else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) == "boolean")
                                {
                                    mapkey = "Boolean";
                                }

                                if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "String")
                                {
                                    mapvalue = "String";
                                }
                                else if(mapField->Value->Type == TSTypeDef::Enum)
                                {
                                    mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value);
                                }
                                else if (mapField->Value->Type == TSTypeDef::Struct)
                                {
                                    mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value,true);
                                }
                                else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "boolean")
                                {
                                    mapvalue = "Boolean";
                                }

                                content +=  " = new HashMap<" +mapkey +"," 
                                    + mapvalue + ">"  + "();\n}";                 
                            }
                            else  if ( field->Def->Type != TSTypeDef::Builtin && field->Def->Type != TSTypeDef::Enum)
                            {
                                content += "{\n" + Table(1) + "public " + GetJAVAFunctioinType(field->Def) + " " + field->ValName; 
                                content +=  " = new " + GetJAVAFunctioinType(field->Def) + "();\n}";
                            }
                            else
                            {
                                content += "{\n" + Table(1) + "public " + GetJAVAFunctioinType(field->Def) + " " + field->ValName; 
                                content += ";\n}";
                            }
                            boost::algorithm::replace_all(content,"::","_");
                            boost::algorithm::replace_all(content,"Vector<int>","Vector<Integer>");
                            boost::algorithm::replace_all(content,"Vector<byte>","Vector<Byte>");
                            boost::algorithm::replace_all(content,"Vector<long>","Vector<Long>");
                            boost::algorithm::replace_all(content,"Vector<short>","Vector<Short>");
                            boost::algorithm::replace_all(content,"Vector<string>","Vector<String>");
                            boost::algorithm::replace_all(content,"Vector<double>","Vector<Double>");
                            boost::algorithm::replace_all(content,"Vector<boolean>","Vector<Boolean>");
                            boost::algorithm::replace_all(content,"Vector<float>","Vector<Float>");
                            boost::algorithm::replace_all(content,"(boolean)0","false");
                            OutClassFile->AppendLast(content);
                            OutClassFile->SetFileType(Generic);                      
                        }
                        if (field->Type != TSParamDef::In )
                        {
                            firstNotInFile = false;
                            TSString filenameResponse = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "_" + Function->ValName + "_Response"; 
                            boost::algorithm::replace_all(filenameResponse,"::","_");
                            ServiceStructFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/" + filenameResponse,".java");
                            GenerateSERVICEStructFile(ServiceStructFile, Function,filename,ParseFileBaseName,false);
                            ServiceStructFile->SetFileType(Generic);

                            //ServiceSupportFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl"+  "/"  + filenameResponse,"Support.java");
                            //GenerateSERVICESupportFile(ServiceSupportFile, TypeDef,filenameResponse);
                            //ServiceSupportFile->SetFileType(Generic);

                            ServiceFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/" + filenameResponse,"Topic.java");
                            GenerateSERVICEFile(ServiceFile, TypeDef,filenameResponse);
                            ServiceFile->SetFileType(Generic);                      
                        }
                        else if (field->Type != TSParamDef::Out )
                        {
                            firstNotOutFile = false;
                            TSString filenameRequest = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "_" + Function->ValName + "_Request"; 
                            boost::algorithm::replace_all(filenameRequest,"::","_");
                            ServiceStructFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/"  +  filenameRequest,".java");
                            GenerateSERVICEStructFile(ServiceStructFile, Function,filename,ParseFileBaseName,true);
                            ServiceStructFile->SetFileType(Generic);

                            //ServiceSupportFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/"  +  filenameRequest,"Support.java");
                            //GenerateSERVICESupportFile(ServiceSupportFile, TypeDef,filenameRequest);
                            //ServiceSupportFile->SetFileType(Generic);

                            ServiceFile = Dir->GetOrCreateProjFile(ParseFileBaseName + "_xidl" +  "/"  +  filenameRequest,"Topic.java");
                            GenerateSERVICEFile(ServiceFile, TypeDef,filenameRequest);
                            ServiceFile->SetFileType(Generic);
                        }
                    }                    
                }
            }
        }
    }
	//_serviceFlagStruct->buildService = true;
}



void TSJavaPlugin::GenerateProjectFile( TSProjFilePtr ProjFile,TSTypeDefPtr TypeDef )
{
    TSString content = _JAVA_PROJECT_;
    boost::algorithm::replace_all(content,"_JAVA_PROJECTNAME_", GetBaseName(TypeDef->Parse.lock()->GetFullFileName()));
    ProjFile->SetContent(TSString2Utf8(content));
}

void TSJavaPlugin::GenerateClassPathFile( TSProjFilePtr ProjFile,bool HasPubSub)
{
    TSString content = _JAVA_CLASSPATH_;
    boost::filesystem::path pfile;
    const char* link;
    TSString pubsubFile;
    if (HasOption(SetSDKKey))
    {
        TSString sdkName = Option<TSString>(SetSDKKey);
        link = getenv(sdkName.c_str());
    }
    else
    {
        link = getenv("LINKSDK");
    }

    if (!link)
    {
        pfile = boost::filesystem::current_path().string().c_str();
    }
    else
    {
        pfile = link;
    }
    if(HasPubSub)
    {
      pubsubFile = " <classpathentry kind=\"src\" path=\"impl\"/>\\n\\";
    }
    boost::algorithm::replace_all(content,"_JAVAPUBSUB_",pubsubFile);
    boost::algorithm::replace_all(content,"_JAVALINKSDK_",pfile.string());
    ProjFile->SetContent(TSString2Utf8(content));
}


/* 
* 生成主题的数据结构的以及序列化与反序列化文件
* TopicName.java
*/
 void TSJavaPlugin::GenerateTOPICStructFile(TSProjFilePtr ProjFile,TSTypeDefPtr TypeDef )
 {
    TSString content = _JAVA_TOPIC_STRUCT_FILE_;
    TSString ParseFileBaseName = 
        GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
    TSString importFileStruct = GetImportJarNameTopic(TypeDef,ParseFileBaseName);
    boost::algorithm::replace_all(content,"_JAVA_IMPORTFILE_",importFileStruct);
    boost::algorithm::replace_all(content,"_JAVA_PACKAGE_NAME_",ParseFileBaseName);

    TSTypeDefWithFiledPtr TypeField = TS_CAST(TypeDef,TSTypeDefWithFiledPtr);

	//是否有继承
    if(TypeField->Extend.lock())
    {
        boost::algorithm::replace_all(content,"_JAVA_STRUCT_EXTEND_NAME_",TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + " extends " + TSIBuiltinPlugin::GetSuitFullTypeName(TypeField->Extend.lock(),true));  
    }
    else
    {
        boost::algorithm::replace_all(content,"_JAVA_STRUCT_EXTEND_NAME_", TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true));  
    }

    boost::algorithm::replace_all(content,"_JAVA_STRUCT_NAME_", TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true));  

    TSString paramsInit, paramsTopicInit, paramsSerializeInit, paramsDeserializeInit;

	//如果有继承先构建继承关系，防止子类空字段不生成父类的调用
	if (TypeField->Extend.lock())
	{
		paramsTopicInit += TSIDL_2TABLE + "super()" + ";" + TSIDL_NEWLINE;
		paramsSerializeInit += TSIDL_2TABLE + "super.Serialize(buffer,MarshaledFlag)" + ";" + TSIDL_NEWLINE;
		paramsDeserializeInit += TSIDL_2TABLE + "super.Deserialize(buffer)" + ";" + TSIDL_NEWLINE;
	}
    
    BOOST_FOREACH(TSFieldDefPtr field,TypeField->Fileds)
    {   
        if (field->Def->Type == TSTypeDef::Array)//array类型
        {
            TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(field->Def,TSVectorBuiltinTypeDefPtr);
            if (vectorField->Value->Type != TSTypeDef::Builtin)
            {
                if(vectorField->Value->Type == TSTypeDef::Enum)
                {
                    paramsInit += TSIDL_TABLE + "public "  + " Vector<" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)+ "> " + field->ValName + ";"+TSIDL_NEWLINE;
                    paramsTopicInit += TSIDL_2TABLE + "this." + field->ValName +  " = new " + " Vector<" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)  + ">();\n ";
                    paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName +".size());" + TSIDL_NEWLINE +
                        TSIDL_2TABLE + "for(" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)+ " " + field->Def->TypeName  + "topic : " +  field->ValName + ")" + TSIDL_NEWLINE
                        +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_TABLE + TSIDL_2TABLE + "buffer.WriteInt32((int)" + field->Def->TypeName + "topic.getValue());" + TSIDL_NEWLINE + TSIDL_2TABLE
                        + "}" +  TSIDL_NEWLINE;

                    paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                        +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "capacity; i ++)"  + TSIDL_NEWLINE + TSIDL_2TABLE +"{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE                   
                        + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) +" topic = " + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)  + ".getEnumByValue(buffer.ReadInt32());"
                        + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + field->ValName + ".add(topic);" + TSIDL_NEWLINE + TSIDL_2TABLE
                        + "}" +  TSIDL_NEWLINE;

                }
                else
                {
                    paramsInit += TSIDL_TABLE + "public"  + " Vector<" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "> " + field->ValName + ";"+TSIDL_NEWLINE;
                    paramsTopicInit += TSIDL_2TABLE + "this." + field->ValName +  " = new " + " Vector<" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ">();\n ";
                    paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName +".size());" + TSIDL_NEWLINE +
                        TSIDL_2TABLE + "for(" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + " topic : " +  field->ValName + ")" + TSIDL_NEWLINE
                        +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE  + TSIDL_TABLE + "topic.Serialize(buffer,2);" + TSIDL_NEWLINE + TSIDL_2TABLE
                        + "}" +  TSIDL_NEWLINE;

                    paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                        +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "capacity; i ++)"  + TSIDL_NEWLINE + TSIDL_2TABLE +"{" + TSIDL_NEWLINE
                        +  TSIDL_2TABLE +TSIDL_TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + " topic = new " + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "();"  + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE
                        + "topic.Deserialize(buffer);" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + field->ValName + ".add(topic);" + TSIDL_NEWLINE + TSIDL_2TABLE
                        + "}" +  TSIDL_NEWLINE;
                
                }
            
            }
            else//基础数组数据类型
            {
                paramsInit += TSIDL_TABLE + "public"  + " " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true)+ " " + field->ValName + ";"+TSIDL_NEWLINE;
                paramsTopicInit += TSIDL_2TABLE + "this." + field->ValName +  " = new " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true) + "();\n";
	

				paramsSerializeInit += TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value, true) + "[]" + field->ValName + "Devan = new "
					+ TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value, true) + "[this." + field->ValName + ".size()];\n" + TSIDL_2TABLE + "for(int i = 0; i < this." + field->ValName + ".size(); i++)\n" + TSIDL_2TABLE + "{\n" + TSIDL_2TABLE
					+ TSIDL_TABLE + field->ValName + "Devan[i] = this." + field->ValName + ".get(i);\n" + TSIDL_2TABLE + "}\n";

                if(vectorField->ValueTypeName == "uint8" || vectorField->ValueTypeName == "UINT8" ||vectorField->ValueTypeName == "uint16" || vectorField->ValueTypeName == "UINT16" ||
                    vectorField->ValueTypeName == "uint32" || vectorField->ValueTypeName == "UINT32" ||vectorField->ValueTypeName == "uint64" || vectorField->ValueTypeName == "UINT64" ||
                    vectorField->ValueTypeName == "uint32" || vectorField->ValueTypeName == "UINT32" ||vectorField->ValueTypeName == "uint64" || vectorField->ValueTypeName == "UINT64" ||
                    vectorField->ValueTypeName == "ushort" || vectorField->ValueTypeName == "ulong" ||vectorField->ValueTypeName == "uint" ||
                    vectorField->ValueTypeName == "udouble")
                {
                     paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteArray" +  "(" + field->ValName +"Devan,false);" + TSIDL_NEWLINE;
                }
                else
                {
                    paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteArray" +  "(" + field->ValName +"Devan,true);" + TSIDL_NEWLINE;
                } 

                paramsDeserializeInit += TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[] " + field->ValName + "Bin = " + "buffer.Read" + vectorField->Value->TypeName + "ForArray();" + TSIDL_NEWLINE; 
				paramsDeserializeInit += TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "Bin.length; i++)\n" + TSIDL_2TABLE + "{\n" + TSIDL_2TABLE
					+ TSIDL_TABLE + "this." + field->ValName + ".add(" + field->ValName + "Bin[i]);\n" + TSIDL_2TABLE + "}\n";        
            }

        }
        else if (field->Def->Type == TSTypeDef::Map)//Map
        {
     
             TSMapBuiltinTypeDefPtr mapField = TS_CAST(field->Def,TSMapBuiltinTypeDefPtr);

             TSString mapkey = "Number";
             TSString mapvalue = "Number";
             if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) ==  "String")
             {
                 mapkey = "String";
             }
			 else if (mapField->Key->Type == TSTypeDef::Enum )
			 {
				 mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key); 
			 }
             else if ( mapField->Key->Type == TSTypeDef::Struct)
             {
                 mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key,true);
             }
			 else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) == "boolean")
			 {
				 mapkey = "Boolean";
			 }

             if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "String")
             {
                 mapvalue = "String";
             }
			 else if(mapField->Value->Type == TSTypeDef::Enum)
			 {
				 mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value);
			 }
             else if(mapField->Value->Type == TSTypeDef::Struct)
             {
                 mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value,true);
             }
			 else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "boolean")
			 {
				 mapvalue = "Boolean";
			 }

             paramsInit += TSIDL_TABLE + "public"  + " Map<" + mapkey +"," 
                        + mapvalue + ">" + field->ValName + ";"+TSIDL_NEWLINE;

             paramsTopicInit += TSIDL_2TABLE + field->ValName +  " = new " +"HashMap<" +mapkey +"," 
                        + mapvalue + ">"  + "();\n";

             paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName +".size());" + TSIDL_NEWLINE +
                 TSIDL_2TABLE + "for(" + mapkey + " obj : " +  field->ValName + ".keySet())" + TSIDL_NEWLINE
                 +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE+ "buffer.Write";
			 if (mapField->Key->Type == TSTypeDef::Enum)
			 {
				 paramsSerializeInit += "Int32";
				 paramsSerializeInit += "((int)((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + ")obj).getValue());" + TSIDL_NEWLINE;
			 }
			 else
			 {
				paramsSerializeInit += mapField->Key->TypeName;
				paramsSerializeInit += "((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + ")obj);" + TSIDL_NEWLINE; 
			 }

			 if (mapField->Value->Type == TSTypeDef::Enum)
			 {
                 paramsSerializeInit += TSIDL_2TABLE + TSIDL_TABLE + "buffer.Write";
				 paramsSerializeInit += "Int32";
				 paramsSerializeInit += "(((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + ")" + field->ValName +".get(obj)).getValue());" + TSIDL_NEWLINE 
					 +  TSIDL_2TABLE + "}" +  TSIDL_NEWLINE;
			 }
             else if(mapField->Value->Type == TSTypeDef::Struct)
             {
                 paramsSerializeInit += TSIDL_2TABLE + TSIDL_TABLE + field->ValName + ".get(obj).Serialize(buffer,2);" + TSIDL_NEWLINE 
                     +  TSIDL_2TABLE + "}" +  TSIDL_NEWLINE;
             }
			 else
			 {
                paramsSerializeInit += TSIDL_2TABLE + TSIDL_TABLE + "buffer.Write";
				paramsSerializeInit += mapField->Value->TypeName;
				paramsSerializeInit += "((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + ")" + field->ValName +".get(obj));" + TSIDL_NEWLINE 
					+  TSIDL_2TABLE + "}" +  TSIDL_NEWLINE;
			 }

			 paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
				 +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName +"capacity; i ++)"+ TSIDL_NEWLINE  + TSIDL_2TABLE  +"{" + TSIDL_NEWLINE;

             if(mapField->Key->Type == TSTypeDef::Enum)
			 {
				paramsDeserializeInit +=  TSIDL_2TABLE  + TSIDL_TABLE + field->ValName +".put(" + mapField->Key->TypeName + ".getEnumByValue(buffer.ReadInt32()),";
				if (mapField->Value->Type == TSTypeDef::Enum)
				{
					paramsDeserializeInit += "buffer.ReadInt32());" + TSIDL_NEWLINE
						+ TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
				}
				else
				{
					paramsDeserializeInit += " buffer.Read" + mapField->Value->TypeName + "());" + TSIDL_NEWLINE
						+ TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
				}
			 }
			 else
			 {
                if ( mapField->Value->Type == TSTypeDef::Struct )
                {
                     paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + mapkey + " tmpKey = buffer.Read" + mapField->Key->TypeName + "();\n";
                     paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + mapvalue + " tmpValue = new " + mapvalue + "();\n";
                     paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + "tmpValue.Deserialize(buffer);\n";
                     paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + field->ValName + ".put(tmpKey,tmpValue);\n        }\n";
                }
                else
                {
                    paramsDeserializeInit +=  TSIDL_2TABLE  + TSIDL_TABLE + field->ValName + ".put(buffer.Read" + mapField->Key->TypeName + "(),";
                    if (mapField->Value->Type == TSTypeDef::Enum)
                    {
                        paramsDeserializeInit += "buffer.ReadInt32());" + TSIDL_NEWLINE
                            + TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
                    }
                    else
                    {
                        paramsDeserializeInit += "buffer.Read" + mapField->Value->TypeName + "());" + TSIDL_NEWLINE
                            + TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
                    }
                }
			 }   
        }
        else//其他类型
        {
            
            if( field->Def->Type == TSTypeDef::Enum)
            {
                paramsInit += TSIDL_TABLE + "public " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def)+ " " + field->ValName + ";"+TSIDL_NEWLINE;
            }
            else
            {
                paramsInit += TSIDL_TABLE + "public"  + " " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true)+ " " + field->ValName + ";"+TSIDL_NEWLINE;
            }
              
            if( field->Def->Type == TSTypeDef::Struct)//里面含有其他Struct
            {
                paramsTopicInit += TSIDL_2TABLE + "this." + field->ValName + " = new " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true)  + "();" + TSIDL_NEWLINE;
				paramsSerializeInit += TSIDL_2TABLE + "long KeyType_" + field->ValName + " = " + boost::lexical_cast<TSString>(field->FindSetEnumType) + ";\n";
				paramsSerializeInit += TSIDL_2TABLE + "long shaledFlag_" + field->ValName + "= MarshaledFlag == 0 ? MarshaledFlag : KeyType_" + field->ValName + ";\n";
                paramsSerializeInit += TSIDL_2TABLE + "this." + field->ValName + ".Serialize(buffer,shaledFlag_" + field->ValName +");" + TSIDL_NEWLINE;
                paramsDeserializeInit += TSIDL_2TABLE + "this." + field->ValName + ".Deserialize(buffer)"  + ";" + TSIDL_NEWLINE;
            }
            else if( field->Def->Type == TSTypeDef::Enum)
            {
                //if(field->HaveKey)
                //{
                    paramsSerializeInit += TSIDL_2TABLE + "long KeyType_" + field->ValName + " = " + boost::lexical_cast<TSString>(field->FindSetEnumType) + ";\n";
                    paramsSerializeInit += TSIDL_2TABLE + "long Offset_" + field->ValName + " = buffer.TellWJava();\n";
                //}
                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32((int)this." +  field->ValName+ ".getValue());" + TSIDL_NEWLINE;
				paramsSerializeInit += TSIDL_2TABLE + "long shaledFlag_" + field->ValName + "= MarshaledFlag == 0 ? MarshaledFlag : KeyType_" + field->ValName + ";\n";
                //if (field->HaveKey)
                //{
                    paramsSerializeInit += TSIDL_2TABLE + "long Size_" + field->ValName + " = buffer.TellWJava() - Offset_" + field->ValName + ";\n";
                    paramsSerializeInit += TSIDL_2TABLE + "if (0 != shaledFlag_" + field->ValName +")\n";
                    paramsSerializeInit += TSIDL_2TABLE + "{\n";
                    paramsSerializeInit += TSIDL_2TABLE + Table() + "buffer.MarshalKeyJava(shaledFlag_" + field->ValName + ",Offset_" + 
                        field->ValName + ",Size_" + field->ValName + ");\n";
                    paramsSerializeInit += TSIDL_2TABLE + "}\n";

                //}
                paramsDeserializeInit += TSIDL_2TABLE +" this." + field->ValName + " = " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def) + ".getEnumByValue(buffer.ReadInt32());\n";
            }
            else 
            {
                if(TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true) == "String[]")
                {
                    paramsTopicInit += TSIDL_2TABLE + "this." + field->ValName + " = new " + "String[1];" + TSIDL_NEWLINE;
                    paramsSerializeInit += TSIDL_2TABLE + "buffer.Write" + "Array" + "(this." + field->ValName +");" + TSIDL_NEWLINE;
                }
                else //基础数据类型
                {
                    //if(field->HaveKey)
                    //{
                        paramsSerializeInit += TSIDL_2TABLE + "long KeyType_" + field->ValName + " = " + boost::lexical_cast<TSString>(field->FindSetEnumType) + ";\n";
                        paramsSerializeInit += TSIDL_2TABLE + "long Offset_" + field->ValName + " = buffer.TellWJava();\n";
                    //}
                    paramsSerializeInit += TSIDL_2TABLE + "buffer.Write" +  field->Def->TypeName + "(this." + field->ValName + ");" + TSIDL_NEWLINE;
					paramsSerializeInit += TSIDL_2TABLE + "long shaledFlag_" + field->ValName + "= MarshaledFlag == 0 ? MarshaledFlag : KeyType_" + field->ValName + ";\n";
                    //if (field->HaveKey)
                   // {
                        paramsSerializeInit += TSIDL_2TABLE + "long Size_" + field->ValName + " = buffer.TellWJava() - Offset_" + field->ValName + ";\n";
						paramsSerializeInit += TSIDL_2TABLE + "if (0 != shaledFlag_" + field->ValName + ")\n";
						paramsSerializeInit += TSIDL_2TABLE + "{\n";
						paramsSerializeInit += TSIDL_2TABLE + Table() + "buffer.MarshalKeyJava(shaledFlag_" + field->ValName + ",Offset_" + 
							field->ValName + ",Size_" + field->ValName + ");\n";
                        paramsSerializeInit += TSIDL_2TABLE + "}\n";

                    //}
                   
                    paramsDeserializeInit += TSIDL_2TABLE +"this." + field->ValName + " = " + "buffer.Read" + field->Def->TypeName + "();" + TSIDL_NEWLINE; 
                }
                if(TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true) == "String")
                {
                    paramsTopicInit += TSIDL_2TABLE + "this." + field->ValName + " =  " + "\"\";" + TSIDL_NEWLINE;
                }
            }

        }
    
    }

    paramsTopicInit += TSIDL_2TABLE +"//user content";
    boost::algorithm::replace_all(content,"_JAVA_CLASS_FUNC_", paramsInit);
    boost::algorithm::replace_all(content,"_JAVA_PUBILC_FUNC_", paramsTopicInit);
    boost::algorithm::replace_all(content,"_JAVA_SERIALIZE_FUNC_", paramsSerializeInit);
    boost::algorithm::replace_all(content,"_JAVA_DERIALIZE_FUNC_", paramsDeserializeInit);

	//替换简单类型的写接口
	boost::algorithm::replace_all(content, "Writebool",				"WriteBool");
	boost::algorithm::replace_all(content, "WriteBoolean",			"WriteBool");
	boost::algorithm::replace_all(content, "WriteBOOL",				"WriteBool");
														           
	boost::algorithm::replace_all(content, "Writesbyte",            "WriteInt8");
	boost::algorithm::replace_all(content, "WriteSBYTE",            "WriteInt8");
	boost::algorithm::replace_all(content, "Writebyte",             "WriteInt8");
	boost::algorithm::replace_all(content, "WriteBYTE",             "WriteInt8");
	boost::algorithm::replace_all(content, "Writechar",             "WriteInt8");
	boost::algorithm::replace_all(content, "WriteChar",             "WriteInt8");
	boost::algorithm::replace_all(content, "Writeuchar",            "WriteInt8");
	boost::algorithm::replace_all(content, "WriteUChar",            "WriteInt8");
	boost::algorithm::replace_all(content, "Writeint8",			    "WriteInt8");
	boost::algorithm::replace_all(content, "WriteInt8",			    "WriteInt8");
    boost::algorithm::replace_all(content, "WriteINT8",             "WriteInt8");
	boost::algorithm::replace_all(content, "Writeuint8",			"WriteInt8");
	boost::algorithm::replace_all(content, "WriteUInt8",			"WriteInt8");
	boost::algorithm::replace_all(content, "WriteUINT8",            "WriteInt8");
	
	boost::algorithm::replace_all(content, "Writeshort",            "WriteInt16");
	boost::algorithm::replace_all(content, "WriteShort",            "WriteInt16");
	boost::algorithm::replace_all(content, "WriteSHORT",            "WriteInt16");
	boost::algorithm::replace_all(content, "Writeushort",           "WriteInt16");
	boost::algorithm::replace_all(content, "WriteUShort",           "WriteInt16");
	boost::algorithm::replace_all(content, "WriteUSHORT",           "WriteInt16");
	boost::algorithm::replace_all(content, "Writeint16",            "WriteInt16");
	boost::algorithm::replace_all(content, "WriteInt16",            "WriteInt16");
	boost::algorithm::replace_all(content, "WriteINT16",            "WriteInt16");
	boost::algorithm::replace_all(content, "Writeuint16",           "WriteInt16");
	boost::algorithm::replace_all(content, "WriteUInt16",           "WriteInt16");
	boost::algorithm::replace_all(content, "WriteUINT16",           "WriteInt16");
														         
	boost::algorithm::replace_all(content, "Writeint(",				"WriteInt32(");
	boost::algorithm::replace_all(content, "WriteInt(",				"WriteInt32(");
	boost::algorithm::replace_all(content, "WriteINT(",				"WriteInt32(");
	boost::algorithm::replace_all(content, "Writeuint(",			"WriteInt32(");
	boost::algorithm::replace_all(content, "WriteUInt(",			"WriteInt32(");
	boost::algorithm::replace_all(content, "WriteUINT(",			"WriteInt32(");
	boost::algorithm::replace_all(content, "Writeint32",            "WriteInt32");
	boost::algorithm::replace_all(content, "WriteInt32",            "WriteInt32");
	boost::algorithm::replace_all(content, "WriteINT32",            "WriteInt32");
	boost::algorithm::replace_all(content, "Writeuint32",           "WriteInt32");
	boost::algorithm::replace_all(content, "WriteUInt32",           "WriteInt32");
	boost::algorithm::replace_all(content, "WriteUINT32",           "WriteInt32");
	boost::algorithm::replace_all(content, "Writelong",             "WriteInt32");
	boost::algorithm::replace_all(content, "WriteLong",             "WriteInt32");
	boost::algorithm::replace_all(content, "WriteLONG",             "WriteInt32");
	boost::algorithm::replace_all(content, "Writeulong",            "WriteInt32");
	boost::algorithm::replace_all(content, "WriteULong",            "WriteInt32");
	boost::algorithm::replace_all(content, "WriteULONG",            "WriteInt32");
	boost::algorithm::replace_all(content, "Writewchar_t",          "WriteInt32");
													           
	boost::algorithm::replace_all(content, "Writeint64",            "WriteInt64");
	boost::algorithm::replace_all(content, "WriteInt64",            "WriteInt64");
	boost::algorithm::replace_all(content, "WriteINT64",            "WriteInt64");
	boost::algorithm::replace_all(content, "Writeuint64",           "WriteInt64");
	boost::algorithm::replace_all(content, "WriteUInt64",           "WriteInt64");
	boost::algorithm::replace_all(content, "WriteUINT64",           "WriteInt64");
														            
	boost::algorithm::replace_all(content, "Writefloat",            "WriteFloat");
	boost::algorithm::replace_all(content, "WriteFloat",            "WriteFloat");
	boost::algorithm::replace_all(content, "WriteFLOAT",            "WriteFloat");
	boost::algorithm::replace_all(content, "WriteFloat32",          "WriteFloat");
	boost::algorithm::replace_all(content, "Writefloat32",          "WriteFloat");
	boost::algorithm::replace_all(content, "Writefloat64",          "WriteFloat");
	boost::algorithm::replace_all(content, "WriteFloat64",          "WriteFloat");
														            
	boost::algorithm::replace_all(content, "Writedouble",           "WriteDouble");
	boost::algorithm::replace_all(content, "WriteDouble",           "WriteDouble");
	boost::algorithm::replace_all(content, "WriteDOUBLE",           "WriteDouble");
														            
	boost::algorithm::replace_all(content, "Writestring",           "WriteString");
	boost::algorithm::replace_all(content, "WriteString",           "WriteString");
	boost::algorithm::replace_all(content, "WriteSTRING",           "WriteString");
																    
	boost::algorithm::replace_all(content, "WriteTSBasicSdo",       "WriteTSObjectHandle");
	boost::algorithm::replace_all(content, "WriteBooleanForArray",  "WriteBoolForArray");

	//替换简单类型的读接口
    boost::algorithm::replace_all(content, "Readbool",              "ReadBool");
	boost::algorithm::replace_all(content, "ReadBoolean",           "ReadBool");
	boost::algorithm::replace_all(content, "ReadBOOL",              "ReadBool");
														           
	boost::algorithm::replace_all(content, "Readsbyte",				"ReadInt8");
	boost::algorithm::replace_all(content, "ReadSBYTE",				"ReadInt8");
	boost::algorithm::replace_all(content, "Readbyte",				"ReadInt8");
	boost::algorithm::replace_all(content, "ReadBYTE",				"ReadInt8");
	boost::algorithm::replace_all(content, "Readchar",				"ReadInt8");
	boost::algorithm::replace_all(content, "ReadChar",				"ReadInt8");
	boost::algorithm::replace_all(content, "Readuchar",				"ReadInt8");
	boost::algorithm::replace_all(content, "ReadUChar",				"ReadInt8");
	boost::algorithm::replace_all(content, "Readint8",				"ReadInt8");
	boost::algorithm::replace_all(content, "ReadInt8",				"ReadInt8");
	boost::algorithm::replace_all(content, "ReadINT8",				"ReadInt8");
	boost::algorithm::replace_all(content, "Readuint8",				"ReadInt8");
	boost::algorithm::replace_all(content, "ReadUInt8",				"ReadInt8");
	boost::algorithm::replace_all(content, "ReadUINT8",				"ReadInt8");
														           
    boost::algorithm::replace_all(content, "Readshort",				"ReadInt16");
    boost::algorithm::replace_all(content, "ReadShort",				"ReadInt16");
    boost::algorithm::replace_all(content, "ReadSHORT",				"ReadInt16");
    boost::algorithm::replace_all(content, "Readushort",			"ReadInt16");
    boost::algorithm::replace_all(content, "ReadUShort",			"ReadInt16");
    boost::algorithm::replace_all(content, "ReadUSHORT",			"ReadInt16");
	boost::algorithm::replace_all(content, "Readint16",				"ReadInt16");
	boost::algorithm::replace_all(content, "ReadInt16",				"ReadInt16");
	boost::algorithm::replace_all(content, "ReadINT16",				"ReadInt16");
	boost::algorithm::replace_all(content, "Readuint16",			"ReadInt16");
	boost::algorithm::replace_all(content, "ReadUInt16",			"ReadInt16");
	boost::algorithm::replace_all(content, "ReadUINT16",			"ReadInt16");
														           
	boost::algorithm::replace_all(content, "Readint(",				"ReadInt32(");
	boost::algorithm::replace_all(content, "ReadInt(",				"ReadInt32(");
	boost::algorithm::replace_all(content, "ReadINT(",				"ReadInt32(");
	boost::algorithm::replace_all(content, "Readuint(",				"ReadInt32(");
	boost::algorithm::replace_all(content, "ReadUInt(",				"ReadInt32(");
	boost::algorithm::replace_all(content, "ReadUINT(",				"ReadInt32(");
	boost::algorithm::replace_all(content, "Readint32",				"ReadInt32");
	boost::algorithm::replace_all(content, "ReadInt32",				"ReadInt32");
	boost::algorithm::replace_all(content, "ReadINT32",				"ReadInt32");
	boost::algorithm::replace_all(content, "Readuint32",			"ReadInt32");
	boost::algorithm::replace_all(content, "ReadUInt32",			"ReadInt32");
	boost::algorithm::replace_all(content, "ReadUINT32",			"ReadInt32");
	boost::algorithm::replace_all(content, "Readlong",				"ReadInt32");
	boost::algorithm::replace_all(content, "ReadLong",				"ReadInt32");
	boost::algorithm::replace_all(content, "ReadLONG",				"ReadInt32");
	boost::algorithm::replace_all(content, "Readulong",				"ReadInt32");
	boost::algorithm::replace_all(content, "ReadULong",				"ReadInt32");
	boost::algorithm::replace_all(content, "ReadULONG",				"ReadInt32");
	boost::algorithm::replace_all(content, "Readwchar_t",			"ReadInt32");
														           
    boost::algorithm::replace_all(content, "Readint64",            "ReadInt64");
	boost::algorithm::replace_all(content, "ReadInt64",            "ReadInt64");
	boost::algorithm::replace_all(content, "ReadINT64",            "ReadInt64");
	boost::algorithm::replace_all(content, "Readuint64",           "ReadInt64");
	boost::algorithm::replace_all(content, "ReadUInt64",           "ReadInt64");
	boost::algorithm::replace_all(content, "ReadUINT64",           "ReadInt64");
    													           
	boost::algorithm::replace_all(content, "Readfloat",            "ReadFloat");
	boost::algorithm::replace_all(content, "ReadFloat",            "ReadFloat");
	boost::algorithm::replace_all(content, "ReadFLOAT",            "ReadFloat");
	boost::algorithm::replace_all(content, "Readfloat32", "ReadFloat");
	boost::algorithm::replace_all(content, "ReadFloat32",          "ReadFloat");
    boost::algorithm::replace_all(content, "Readfloat64",          "ReadFloat");
    boost::algorithm::replace_all(content, "ReadFloat64",          "ReadFloat");
														           
    boost::algorithm::replace_all(content, "Readdouble",           "ReadDouble");
    boost::algorithm::replace_all(content, "ReadDouble",           "ReadDouble");
	boost::algorithm::replace_all(content, "ReadDOUBLE",           "ReadDouble");
														           
	boost::algorithm::replace_all(content, "Readstring",           "ReadString");
	boost::algorithm::replace_all(content, "ReadString",           "ReadString");
	boost::algorithm::replace_all(content, "ReadSTRING",           "ReadString");
	
    boost::algorithm::replace_all(content, "ReadTSBasicSdo",       "ReadTSObjectHandle");
    boost::algorithm::replace_all(content, "ReadBooleanForArray",  "ReadBoolForArray");
    
    //特殊替换
	boost::algorithm::replace_all(content, "ReadintFor",			"ReadInt32For");
	boost::algorithm::replace_all(content, "ReadIntFor",			"ReadInt32For");
	boost::algorithm::replace_all(content, "ReadINTFor",			"ReadInt32For");
	boost::algorithm::replace_all(content, "ReaduintFor",			"ReadInt32For");
	boost::algorithm::replace_all(content, "ReadUIntFor",			"ReadInt32For");
	boost::algorithm::replace_all(content, "ReadUINTFor",			"ReadInt32For");

    boost::algorithm::replace_all(content,"Vector<int>",           "Vector<Integer>");
    boost::algorithm::replace_all(content,"Vector<byte>",          "Vector<Byte>");
    boost::algorithm::replace_all(content,"Vector<long>",          "Vector<Long>");
    boost::algorithm::replace_all(content,"Vector<short>",         "Vector<Short>");
    boost::algorithm::replace_all(content,"Vector<string>",        "Vector<String>");
    boost::algorithm::replace_all(content,"Vector<double>",        "Vector<Double>");
    boost::algorithm::replace_all(content,"Vector<boolean>",       "Vector<Boolean>");
    boost::algorithm::replace_all(content,"Vector<float>",         "Vector<Float>");
    boost::algorithm::replace_all(content,"(boolean)0",            "false");

	boost::algorithm::replace_all(content, "::",                   "_");
    
	ProjFile->AppendLast(TSString2Utf8(content));
 }

 TSString TSJavaPlugin::GetJAVAFunctioinType( TSTypeDefPtr tmpDef)
 {
     TSString paramsInit;
     if (tmpDef->Type == TSTypeDef::Array)//array类型
     {
         TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(tmpDef,TSVectorBuiltinTypeDefPtr);

         if( vectorField->Value->Type == TSTypeDef::Enum)
         {
             paramsInit = "Vector<" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)+ "> ";
         }
         else if (vectorField->Value->Type != TSTypeDef::Builtin)
         {
             paramsInit = "Vector<" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "> ";
         }
         else//基础数组数据类型
         {
             paramsInit = TSIBuiltinPlugin::GetSuitFullTypeName(tmpDef,true);                   
         }
     }
     else if (tmpDef->Type == TSTypeDef::Map)//Map
     {

         TSMapBuiltinTypeDefPtr mapField = TS_CAST(tmpDef,TSMapBuiltinTypeDefPtr);

         TSString mapkey = "Number";
         TSString mapvalue = "Number";
		 if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) ==  "String")
		 {
			 mapkey = "String";
		 }
		 else if (mapField->Key->Type == TSTypeDef::Enum )
		 {
			 mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key); 
		 }
         else if(mapField->Key->Type == TSTypeDef::Struct)
         {
            mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key,true); 
         }
		 else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) == "boolean")
		 {
			 mapkey = "Boolean";
		 }

		 if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "boolean")
		 {
			 mapvalue = "Boolean";
		 }

		 if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "String")
		 {
			 mapvalue = "String";
		 }
		 else if(mapField->Value->Type == TSTypeDef::Enum )
		 {
			 mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value);
		 }
         else if(mapField->Value->Type == TSTypeDef::Struct)
         {
             mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value,true);
         }

         paramsInit =" Map<" + mapkey +"," 
             + mapvalue + ">";                   
     }
     else//其他类型
     {

         if(TSIBuiltinPlugin::GetSuitFullTypeName(tmpDef,true).empty())
         {
			 if( tmpDef->Type == TSTypeDef::Enum)
			 {
				 paramsInit += tmpDef->TypeName;
			 }
			 else
			 {
				paramsInit = " int ";
			 }
             
         }
         else
         {
             if( tmpDef->Type == TSTypeDef::Enum)
             {
                 paramsInit += TSIBuiltinPlugin::GetSuitFullTypeName(tmpDef);                                     
             }
             else
             {
                  paramsInit = TSIBuiltinPlugin::GetSuitFullTypeName(tmpDef,true);
             }
            
         }
     }
     boost::algorithm::replace_all(paramsInit,"::","_");
     return paramsInit;
 }

 /*
 * TopicSupport.java 
 */
 void TSJavaPlugin::GenerateTOPICSupportFile(TSProjFilePtr ProjFile,TSTypeDefPtr TypeDef )
 {
     TSString content = _JAVA_TOPIC_SUPPORT_FILE_;

	 int filedKey = 0;
	 TSTypeDefWithFiledPtr TypeField = TS_CAST(TypeDef, TSTypeDefWithFiledPtr);
	 BOOST_FOREACH(TSFieldDefPtr field, TypeField->Fileds)
	 {
		 if (field->FindSet != 0)
		 {
			 filedKey = field->FindSet;
			 break;
		 }
	 }
	 TSString HasFindSetTContent = "if (0 == Flag) {\n" + TSIDL_2TABLE + TSIDL_TABLE + "return " + ((filedKey == 0) ? "true" : "false") + ";\n" + TSIDL_2TABLE + "}\n";
	 HasFindSetTContent += TSIDL_2TABLE + "if (2 == Flag) {\n" + TSIDL_2TABLE + TSIDL_TABLE + "return " + ((filedKey == 2) ? "true" : "false") + ";\n" + TSIDL_2TABLE + "}\n";
	 HasFindSetTContent += TSIDL_2TABLE + "if (4 == Flag) {\n" + TSIDL_2TABLE + TSIDL_TABLE + "return " + ((filedKey == 4) ? "true" : "false") + ";\n" + TSIDL_2TABLE + "}\n";
	 HasFindSetTContent += TSIDL_2TABLE + "if (8 == Flag) {\n" + TSIDL_2TABLE + TSIDL_TABLE + "return " + ((filedKey == 8) ? "true" : "false") + ";\n" + TSIDL_2TABLE + "}\n";
	 HasFindSetTContent += TSIDL_2TABLE + "if (16 == Flag) {\n" + TSIDL_2TABLE + TSIDL_TABLE + "return " + ((filedKey == 16) ? "true" : "false") + ";\n" + TSIDL_2TABLE + "}\n";

     TSString ParseFileBaseName = 
         GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
	 TSTypeDefWithFiledPtr TypeFile = TS_CAST(TypeDef,TSTypeDefWithFiledPtr);
	 TSString HasCodeVersion = TSIDLUtils::GenerateTypeVersionNum(TypeFile);
     boost::algorithm::replace_all(content,"_JAVA_PACKAGE_NAME_",ParseFileBaseName);
	 boost::algorithm::replace_all(content,"_HASHVERSION_CODE_",HasCodeVersion);
     boost::algorithm::replace_all(content,"_JAVA_STRUCT_NAME_",TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true));
	 boost::algorithm::replace_all(content, "_HASFINDSETTCONTENT_", HasFindSetTContent);
     boost::algorithm::replace_all(content,"::","_");
     ProjFile->AppendLast(TSString2Utf8(content));
 }


 
 void TSJavaPlugin::GenerateTOPICEnumFile(TSProjFilePtr ProjFile,TSEnumDefPtr TypeDef ,TSString filename)
 {
     TSString content = _JAVA_TOPIC_ENUM_FILE_;
     TSString ParseFileBaseName = 
        GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

     boost::algorithm::replace_all(content,"_JAVA_PACKAGE_NAME_",ParseFileBaseName);    
     boost::algorithm::replace_all(content,"_JAVA_STRUCT_NAME_",filename);  
     TSString paramsInit, paramsTopicInit, paramsSerializeInit, paramsDeserializeInit;

     paramsInit += TSIDL_TABLE;
	 TSString FristEnumValue;
     for(int i = 0; i < TypeDef->Fileds.size(); i++)
     {
         if(TSEnumFiledDefPtr enumVal = TypeDef->Fileds[i])
         {
			paramsInit += enumVal->ValName;
			if (i == 0)
			{
				FristEnumValue = enumVal->ValName;
				std::vector<TSString> vecs = TSStringUtil::Split(FristEnumValue, "=", false);
				if (vecs.size())
				{
					FristEnumValue = vecs[0];
				}
			}
			if(enumVal->ValName.find("=") != TSString::npos)
			{
                if( i != TypeDef->Fileds.size() - 1)
                {
                    paramsInit += "),";
                }
                else
                {
                paramsInit += ");";
                }
			}
			else
			{
                if( i != TypeDef->Fileds.size() - 1)
                {
                paramsInit += ",";
                }
                else
                {
                    paramsInit += ";";
                }
			}
         }
     }
     boost::algorithm::replace_all(paramsInit,"=", "(");
     boost::algorithm::replace_all(content,"_JAVA_CLASS_FUNC_", paramsInit);
     boost::algorithm::replace_all(content,"_JAVA_PUBILC_FUNC_", paramsTopicInit);
     boost::algorithm::replace_all(content,"_JAVA_SERIALIZE_FUNC_", paramsSerializeInit);
     boost::algorithm::replace_all(content,"_JAVA_DERIALIZE_FUNC_", paramsDeserializeInit);
     boost::algorithm::replace_all(content,"::", "_");
	 if (FristEnumValue.empty())
	 {
		 boost::algorithm::replace_all(content, "_DEFAULT_ENUMVALUE_", "null");
	 }
	 else
	 {
		 boost::algorithm::replace_all(content, "_DEFAULT_ENUMVALUE_", FristEnumValue);
	 }

     ProjFile->AppendLast(TSString2Utf8(content));
 }


 void TSJavaPlugin::GenerateSERVICEFile(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString className)
 {
     TSString content = _JAVA_SERVICE_FILE_;
     TSString ParseFileBaseName = 
         GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

     boost::algorithm::replace_all(content,"_JAVA_STRUCT_NAME_",className);
     boost::algorithm::replace_all(content,"_JAVA_TOPIC_NAME_",className);
     className += "Topic";
     boost::algorithm::replace_all(content,"_JAVA_PACKAGE_NAME_",ParseFileBaseName);

     boost::algorithm::replace_all(content,"_JAVA_CLASS_NAME_",className);
     boost::algorithm::replace_all(content,"::","_");
     
     ProjFile->SetContent(TSString2Utf8(content));
 }

 void TSJavaPlugin::GenerateSERVICEStructFile(TSProjFilePtr ProjFile, TSFunctionDefPtr Function,TSString className,TSString ParseFileBaseName,bool IsinOrOut)
 {
     TSString content = _JAVA_SERVICE_STRUCT_FILE_;
     boost::algorithm::replace_all(content,"_JAVA_PACKAGE_NAME_",ParseFileBaseName);
     std::vector<TSStructDefPtr> Defs =  _p->_BuiltinPlugin->GetAllStructDefs();
      
     TSString paramsInit, paramsTopicInit, paramsSerializeInit, paramsDeserializeInit,importFile,importFileStruct;
     importFile = "import "+ className + "4Java.*" +  ";\n";
     importFileStruct = GetImportJarNameFunc(Function,ParseFileBaseName);
 
     if(Function != NULL)
     {   
        for (unsigned int i = 0; i < Function->Params.size(); i++)
        {
          TSParamDefPtr field = Function->Params[i];  
          if (IsinOrOut)
          {
            if(field->Type != TSParamDef::Out)
            {   
                if(field->Type == TSParamDef::Inout)
                {   
                    paramsInit += TSIDL_TABLE + "public " + className + "_" + Function->ValName + "_" +  field->ValName + " " + field->ValName + ";\n";
                   // if( field->Def->Type != TSTypeDef::Enum)
                    //{
                    paramsTopicInit += TSIDL_TABLE + field->ValName + " = new " + className + "_" +Function->ValName + "_" +  field->ValName + "();\n";  
                    //}
                  
                    if (field->Def->Type == TSTypeDef::Array)//array类型
                    {
                        TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(field->Def,TSVectorBuiltinTypeDefPtr);
                        if (vectorField->Value->Type != TSTypeDef::Builtin)
                        {
                            if(vectorField->Value->Type == TSTypeDef::Enum)
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName + "." + field->ValName +".size());" + TSIDL_NEWLINE +
                                    TSIDL_2TABLE + "for(" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)  + " " + field->Def->TypeName  + "topic : " +  field->ValName + "." + field->ValName +")" + TSIDL_NEWLINE
                                    +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_TABLE + TSIDL_2TABLE + "buffer.WriteInt32((int)" + field->Def->TypeName + "topic.getValue());" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + "}" +  TSIDL_NEWLINE;

                                paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                                    +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "capacity; i ++)"  + TSIDL_NEWLINE + TSIDL_2TABLE +"{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE                   
                                    + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) +" topic = " + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ".getEnumByValue(buffer.ReadInt32());"
                                    + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + field->ValName + "." + field->ValName + ".add(topic);" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + "}" +  TSIDL_NEWLINE;

                            }
                            else
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName +"." + field->ValName +".size());" + TSIDL_NEWLINE +
                                    TSIDL_2TABLE + "for(" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + " topic : " +  field->ValName + "." + field->ValName +")" + TSIDL_NEWLINE
                                    +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE  + TSIDL_TABLE + "topic.Serialize(buffer,2);" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + "}" +  TSIDL_NEWLINE;

                                paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                                    +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "capacity; i ++)"  + TSIDL_NEWLINE + TSIDL_2TABLE +"{" + TSIDL_NEWLINE
                                    +  TSIDL_2TABLE +TSIDL_TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + " topic = new " + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "();"  + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE
                                    + "topic.Deserialize(buffer);" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + field->ValName + "." + field->ValName +".add(topic);" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + "}" +  TSIDL_NEWLINE;

                            }

                        }
                        else//基础数组数据类型
                        {

                            if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "String")
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "if(this." +  field->ValName + "." + field->ValName +".size() == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")\" \");" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n"
                                    + TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[]" + field->ValName + "Devan = new "
                                    + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[this." + field->ValName + "." + field->ValName +".size()];\n" + TSIDL_2TABLE + "for(int i = 0; i < this." + field->ValName + "." + field->ValName + ".size(); i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                    + TSIDL_TABLE + field->ValName + "Devan[i] = this." + field->ValName + "." + field->ValName +".get(i);\n" + TSIDL_2TABLE + "}\n";
                            }
                            else
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "if(this." +  field->ValName + "." + field->ValName +".size() == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")0);" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n"
                                    + TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[]" + field->ValName + "Devan = new "
                                    + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[this." + field->ValName + "." + field->ValName +".size()];\n" + TSIDL_2TABLE + "for(int i = 0; i < this." + field->ValName + "." + field->ValName +".size(); i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                    + TSIDL_TABLE + field->ValName + "Devan[i] = this." + field->ValName + "." + field->ValName + ".get(i);\n" + TSIDL_2TABLE + "}\n";
                            }             

                            if(vectorField->ValueTypeName == "uint8" || vectorField->ValueTypeName == "UINT8" ||vectorField->ValueTypeName == "uint16" || vectorField->ValueTypeName == "UINT16" ||
                                vectorField->ValueTypeName == "uint32" || vectorField->ValueTypeName == "UINT32" ||vectorField->ValueTypeName == "uint64" || vectorField->ValueTypeName == "UINT64" ||
                                vectorField->ValueTypeName == "uint32" || vectorField->ValueTypeName == "UINT32" ||vectorField->ValueTypeName == "uint64" || vectorField->ValueTypeName == "UINT64" ||
                                vectorField->ValueTypeName == "ushort" || vectorField->ValueTypeName == "ulong" ||vectorField->ValueTypeName == "uint" ||
                                vectorField->ValueTypeName == "udouble")
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteArray" +  "(" + field->ValName +"Devan,false);" + TSIDL_NEWLINE;
                            }
                            else
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteArray" +  "(" + field->ValName +"Devan,true);" + TSIDL_NEWLINE;
                            } 

                            paramsDeserializeInit += TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[] " + field->ValName + "Bin = " + "buffer.Read" + vectorField->Value->TypeName + "ForArray();" + TSIDL_NEWLINE; 

                            if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "String")
                            {
                                paramsDeserializeInit += TSIDL_2TABLE + "if(" +  field->ValName + "Bin.length == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")\" \");" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n" + TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "Bin.length; i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                    + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add(" + field->ValName + "Bin[i]);\n" + TSIDL_2TABLE + "}\n";
                            }
                            else
                            {
                                paramsDeserializeInit += TSIDL_2TABLE + "if(" +  field->ValName + "Bin.length == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")0);" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n" + TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "Bin.length; i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                    + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add(" + field->ValName + "Bin[i]);\n" + TSIDL_2TABLE + "}\n";
                            }           
                        }

                    }
                    else if (field->Def->Type == TSTypeDef::Map)//Map
                    {

                        TSMapBuiltinTypeDefPtr mapField = TS_CAST(field->Def,TSMapBuiltinTypeDefPtr);

                        TSString mapkey = "Number";
                        TSString mapvalue = "Number";
                        if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) ==  "String")
                        {
                            mapkey = "String";
                        }
                        else if (mapField->Key->Type == TSTypeDef::Enum )
                        {
                            mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key); 
                        }
                        else if ( mapField->Key->Type == TSTypeDef::Struct)
                        {
                            mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key,true);
                        }
                        else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) == "boolean")
                        {
                            mapkey = "Boolean";
                        }

                        if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "String")
                        {
                            mapvalue = "String";
                        }
                        else if(mapField->Value->Type == TSTypeDef::Enum)
                        {
                            mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value);
                        }
                        else if(mapField->Value->Type == TSTypeDef::Struct)
                        {
                            mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value,true);
                        }
                        else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "boolean")
                        {
                            mapvalue = "Boolean";
                        }

                        paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName + "." + field->ValName +".size());" + TSIDL_NEWLINE +
                            TSIDL_2TABLE + "for(" + mapkey + " obj : " +  field->ValName + "." + field->ValName + ".keySet())" + TSIDL_NEWLINE
                            +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE+ "buffer.Write";
                        if (mapField->Key->Type == TSTypeDef::Enum)
                        {
                            paramsSerializeInit += "Int32";
                            paramsSerializeInit += "((int)((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + ")obj).getValue());" + TSIDL_NEWLINE;
                        }
                        else
                        {
                            paramsSerializeInit += mapField->Key->TypeName;
                            paramsSerializeInit += "((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + ")obj);" + TSIDL_NEWLINE; 
                        }

                        if (mapField->Value->Type == TSTypeDef::Enum)
                        {
                            paramsSerializeInit += TSIDL_2TABLE + TSIDL_TABLE + "buffer.Write";
                            paramsSerializeInit += "Int32";
                            paramsSerializeInit += "(((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + ")" + field->ValName +".get(obj)).getValue());" + TSIDL_NEWLINE 
                                +  TSIDL_2TABLE + "}" +  TSIDL_NEWLINE;
                        }
                        else if(mapField->Value->Type == TSTypeDef::Struct)
                        {
                            paramsSerializeInit += TSIDL_2TABLE + TSIDL_TABLE + field->ValName + "." + field->ValName+ ".get(obj).Serialize(buffer,2);" + TSIDL_NEWLINE 
                                +  TSIDL_2TABLE + "}" +  TSIDL_NEWLINE;
                        }
                        else
                        {
                            paramsSerializeInit += TSIDL_2TABLE + TSIDL_TABLE + "buffer.Write";
                            paramsSerializeInit += mapField->Value->TypeName;
                            paramsSerializeInit += "((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + ")" + field->ValName +".get(obj));" + TSIDL_NEWLINE 
                                +  TSIDL_2TABLE + "}" +  TSIDL_NEWLINE;
                        }

                        paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                            +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName +"capacity; i ++)"+ TSIDL_NEWLINE  + TSIDL_2TABLE  +"{" + TSIDL_NEWLINE;

                        if(mapField->Key->Type == TSTypeDef::Enum)
                        {
                            paramsDeserializeInit +=  TSIDL_2TABLE  + TSIDL_TABLE + field->ValName +".put(" + mapField->Key->TypeName + ".getEnumByValue(buffer.ReadInt32()),";
                            if (mapField->Value->Type == TSTypeDef::Enum)
                            {
                                paramsDeserializeInit += "buffer.ReadInt32());" + TSIDL_NEWLINE
                                    + TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
                            }
                            else
                            {
                                paramsDeserializeInit += " buffer.Read" + mapField->Value->TypeName + "());" + TSIDL_NEWLINE
                                    + TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
                            }
                        }
                        else
                        {
                            if ( mapField->Value->Type == TSTypeDef::Struct )
                            {
                                paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + mapkey + " tmpKey = buffer.Read" + mapField->KeyTypeName + "();\n";
                                paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + mapvalue + " tmpValue = new " + mapvalue + "();\n";
                                paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + "tmpValue.Deserialize(buffer);\n";
                                paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + field->ValName + "." + field->ValName+ ".put(tmpKey,tmpValue);\n        }\n";
                            }
                            else
                            {
                                paramsDeserializeInit +=  TSIDL_2TABLE  + TSIDL_TABLE + field->ValName + ".put(buffer.Read" + mapField->Key->TypeName + "(),";
                                if (mapField->Value->Type == TSTypeDef::Enum)
                                {
                                    paramsDeserializeInit += "buffer.ReadInt32());" + TSIDL_NEWLINE
                                        + TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
                                }
                                else
                                {
                                    paramsDeserializeInit += "buffer.Read" + mapField->Value->TypeName + "());" + TSIDL_NEWLINE
                                        + TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
                                }
                            }
                        }   

                    }
                    else//其他类型
                    {
                        if( field->Def->Type == TSTypeDef::Struct)//里面含有其他Struct
                        {
                            //   paramsTopicInit += TSIDL_TABLE + "this." + field->ValName + " = new " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true)  + "();" + TSIDL_NEWLINE;
                            paramsSerializeInit += TSIDL_TABLE + "this." + field->ValName + "." + field->ValName + ".Serialize(buffer,2)"  + ";" + TSIDL_NEWLINE;
                            paramsDeserializeInit += TSIDL_TABLE +"this." + field->ValName + "." + field->ValName + ".Deserialize(buffer)"  + ";" + TSIDL_NEWLINE;
                        }
                        else if( field->Def->Type == TSTypeDef::Enum)
                        {
                            paramsSerializeInit += TSIDL_TABLE + "buffer.WriteInt32((int)this." +  field->ValName + "."+field->ValName+ ".getValue());" + TSIDL_NEWLINE;
                            paramsDeserializeInit += TSIDL_TABLE +" this." + field->ValName  + "."+field->ValName + " = " + GetJAVAFunctioinType(field->Def) + ".getEnumByValue(buffer.ReadInt32());\n";
                        }
                        else 
                        {
                            if(TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true) == "String[]")
                            {
                                //  paramsTopicInit += TSIDL_TABLE + "this." + field->ValName + " = new " + "String[1];" + TSIDL_NEWLINE;
                                paramsSerializeInit += TSIDL_TABLE + "buffer.Write" + "Array" + "(this." + field->ValName + "." +  field->ValName +");" + TSIDL_NEWLINE;
                            }
                            else //基础数据类型
                            {
                                paramsSerializeInit += TSIDL_TABLE + "buffer.Write" +  field->Def->TypeName + "(this." + field->ValName + "." +  field->ValName + ");" + TSIDL_NEWLINE;
                                paramsDeserializeInit += TSIDL_TABLE +" this." + field->ValName + "." + field->ValName + " = " + "buffer.Read" + field->Def->TypeName + "();" + TSIDL_NEWLINE; 
                            }

                        }
                    }
                }
                else//In类型
                {

                if (field->Def->Type == TSTypeDef::Array)//array类型
                {
                    TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(field->Def,TSVectorBuiltinTypeDefPtr);
                    if (vectorField->Value->Type != TSTypeDef::Builtin)
                    {
                        if(vectorField->Value->Type == TSTypeDef::Enum)
                        {
                            paramsInit += TSIDL_TABLE + "public " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def)+ field->ValName + ";"+TSIDL_NEWLINE;
                            paramsTopicInit += TSIDL_2TABLE + "this." + field->ValName +  " = new "+ TSIBuiltinPlugin::GetSuitFullTypeName(field->Def) + "();\n ";
                            paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName +".size());" + TSIDL_NEWLINE +
                                TSIDL_2TABLE + "for(" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)  + " "  + "topic : " +  field->ValName + ")" + TSIDL_NEWLINE
                                +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_TABLE + TSIDL_2TABLE + "buffer.WriteInt32((int)" + field->Def->TypeName + "topic.getValue());" + TSIDL_NEWLINE + TSIDL_2TABLE
                                + "}" +  TSIDL_NEWLINE;

                            paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                                +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "capacity; i ++)"  + TSIDL_NEWLINE + TSIDL_2TABLE +"{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE                   
                                + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)  +" topic = " + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ".getEnumByValue(buffer.ReadInt32());"
                                + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + field->ValName + ".add(topic);" + TSIDL_NEWLINE + TSIDL_2TABLE
                                + "}" +  TSIDL_NEWLINE;

                        }
                        else
                        {
                            paramsInit += TSIDL_TABLE + "public"  + " Vector<" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "> " + field->ValName + ";"+TSIDL_NEWLINE;
                            paramsTopicInit += TSIDL_2TABLE + "this." + field->ValName +  " = new " + " Vector<" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ">();\n ";
                            paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName +".size());" + TSIDL_NEWLINE +
                                TSIDL_2TABLE + "for(" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + " topic : " +  field->ValName + ")" + TSIDL_NEWLINE
                                +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE  + TSIDL_TABLE + "topic.Serialize(buffer,2);" + TSIDL_NEWLINE + TSIDL_2TABLE
                                + "}" +  TSIDL_NEWLINE;

                            paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                                +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "capacity; i ++)"  + TSIDL_NEWLINE + TSIDL_2TABLE +"{" + TSIDL_NEWLINE
                                +  TSIDL_2TABLE +TSIDL_TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + " topic = new " + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "();"  + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE
                                + "topic.Deserialize(buffer);" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + field->ValName + ".add(topic);" + TSIDL_NEWLINE + TSIDL_2TABLE
                                + "}" +  TSIDL_NEWLINE;

                        }

                    }
                    else//基础数组数据类型
                    {
                        paramsInit += TSIDL_TABLE + "public"  + " " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true)+ " " + field->ValName + ";"+TSIDL_NEWLINE;
                        paramsTopicInit += TSIDL_2TABLE + "this." + field->ValName +  " = new " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true) + "();\n";

                        if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "String")
                        {
                            paramsSerializeInit += TSIDL_2TABLE + "if(this." +  field->ValName + ".size() == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + "this." + field->ValName + ".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")\" \");" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n"
                                + TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[]" + field->ValName + "Devan = new "
                                + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[this." + field->ValName + ".size()];\n" + TSIDL_2TABLE + "for(int i = 0; i < this." + field->ValName + ".size(); i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                + TSIDL_TABLE + field->ValName + "Devan[i] = this." + field->ValName + ".get(i);\n" + TSIDL_2TABLE + "}\n";
                        }
                        else
                        {
                            paramsSerializeInit += TSIDL_2TABLE + "if(this." +  field->ValName + ".size() == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + "this." + field->ValName + ".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")0);" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n"
                                + TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[]" + field->ValName + "Devan = new "
                                + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[this." + field->ValName + ".size()];\n" + TSIDL_2TABLE + "for(int i = 0; i < this." + field->ValName + ".size(); i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                + TSIDL_TABLE + field->ValName + "Devan[i] = this." + field->ValName + ".get(i);\n" + TSIDL_2TABLE + "}\n";
                        }             

                        if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint8" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT8" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint16" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT16" ||
                            TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint32" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT32" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint64" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT64" ||
                            TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint32" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT32" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint64" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT64" ||
                            TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "ushort" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "ulong" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint" ||
                            TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "udouble")
                        {
                            paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteArray" +  "(" + field->ValName +"Devan,false);" + TSIDL_NEWLINE;
                        }
                        else
                        {
                            paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteArray" +  "(" + field->ValName +"Devan,true);" + TSIDL_NEWLINE;
                        } 

                        paramsDeserializeInit += TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[] " + field->ValName + "Bin = " + "buffer.Read" + vectorField->Value->TypeName + "ForArray();" + TSIDL_NEWLINE; 

                        if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "String")
                        {
                            paramsDeserializeInit += TSIDL_2TABLE + "if(" +  field->ValName + "Bin.length == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE
                                + TSIDL_TABLE + "this." + field->ValName + ".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")\" \");" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n" + TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "Bin.length; i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                + TSIDL_TABLE + "this." + field->ValName + ".add(" + field->ValName + "Bin[i]);\n" + TSIDL_2TABLE + "}\n";
                        }
                        else
                        {
                            paramsDeserializeInit += TSIDL_2TABLE + "if(" +  field->ValName + "Bin.length == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE
                                + TSIDL_TABLE + "this." + field->ValName + ".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")0);" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n" + TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "Bin.length; i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                + TSIDL_TABLE + "this." + field->ValName + ".add(" + field->ValName + "Bin[i]);\n" + TSIDL_2TABLE + "}\n";
                        }           
                    }
                }
                else if (field->Def->Type == TSTypeDef::Map)//Map
                {

                    TSMapBuiltinTypeDefPtr mapField = TS_CAST(field->Def,TSMapBuiltinTypeDefPtr);

                    TSString mapkey = "Number";
                    TSString mapvalue = "Number";
					if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) ==  "String")
					{
						mapkey = "String";
					}
					else if (mapField->Key->Type == TSTypeDef::Enum)
					{
						mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key); 
					}
                    else if( mapField->Key->Type == TSTypeDef::Struct)
                    {
                        mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key,true); 
                    }
					else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) == "boolean")
					{
						mapkey = "Boolean";
					}

					if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "String")
					{
						mapvalue = "String";
					}
					else if(mapField->Value->Type == TSTypeDef::Enum)
					{
						mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value);
					}
                    else if (mapField->Value->Type == TSTypeDef::Struct)
                    {
                        mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value,true);
                    }
					else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "boolean")
					{
						mapvalue = "Boolean";
					}

                    if(field->Type != TSParamDef::In)//Inout
                    {
                        paramsInit += TSIDL_TABLE + "public " + className + "_" + Function->ValName + "_" +  field->ValName + " " + field->ValName + ";\n";
                    }
                    else
                    {
                        paramsInit += TSIDL_TABLE + "public"  + " Map<" + mapkey +"," 
                            + mapvalue + ">" + field->ValName + ";"+TSIDL_NEWLINE;
                    }
                                   
                    paramsTopicInit += TSIDL_TABLE + field->ValName +  " = new " +"HashMap<" +mapkey +"," 
                        + mapvalue + ">"  + "();\n";

                    if (mapField->Key->Type == TSTypeDef::Struct)
                    {
                        paramsSerializeInit += TSIDL_TABLE + "buffer.WriteInt32(" + field->ValName +".size());" + TSIDL_NEWLINE +
                            TSIDL_TABLE + "for(" + mapkey + " obj : " +  field->ValName + ".keySet())" + TSIDL_NEWLINE
                            +  TSIDL_TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + " obj.Serialize(buffer,2);\n";
                        if(mapField->Value->Type == TSTypeDef::Struct)
                        {
                            paramsSerializeInit += TSIDL_2TABLE + field->ValName + ".get(obj).Serialize(buffer,2);\n    }\n";
                        }
                    }
                    else
                    {
                       paramsSerializeInit += TSIDL_TABLE + "buffer.WriteInt32(" + field->ValName +".size());" + TSIDL_NEWLINE +
                            TSIDL_TABLE + "for(" + mapkey + " obj : " +  field->ValName + ".keySet())" + TSIDL_NEWLINE
                            +  TSIDL_TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + "buffer.Write";
                    }
			
					if (mapField->Key->Type == TSTypeDef::Enum)
					{
						paramsSerializeInit += "Int32";
						paramsSerializeInit += "((int)((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + ")obj).getValue());" + TSIDL_NEWLINE;
					}
					else if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) == "boolean")
					{
						paramsSerializeInit += "Bool";
						paramsSerializeInit += "((Boolean)obj.booleanValue());" + TSIDL_NEWLINE;
					}
					else if(mapField->Key->Type != TSTypeDef::Struct )
					{
						paramsSerializeInit += mapField->Key->TypeName;
						paramsSerializeInit += "((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + ")obj);" + TSIDL_NEWLINE; 
					}
					
					if (mapField->Value->Type == TSTypeDef::Enum)
					{
                        paramsSerializeInit += TSIDL_2TABLE + "buffer.Write";
						paramsSerializeInit += "Int32";
                        paramsSerializeInit += "((int)(" + field->ValName +".get(obj)).getValue());\n"+  TSIDL_NEWLINE + TSIDL_TABLE + "}" +  TSIDL_NEWLINE;
					}
					else if(mapField->Value->Type != TSTypeDef::Struct )
					{
                        paramsSerializeInit += TSIDL_2TABLE + "buffer.Write";
						paramsSerializeInit += mapField->Value->TypeName;
						paramsSerializeInit += "((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + ")" + field->ValName +".get(obj));" + TSIDL_NEWLINE 
							+  TSIDL_TABLE + "}" +  TSIDL_NEWLINE;
					}
                    else
                    {
                        paramsSerializeInit += TSIDL_2TABLE + field->ValName + ".get(obj).Serialize(buffer,2);" + TSIDL_NEWLINE 
                            +  TSIDL_2TABLE + "}" +  TSIDL_NEWLINE;
                    }

                    if(mapField->Value->Type == TSTypeDef::Struct)
                    {
                        paramsDeserializeInit += TSIDL_TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                            +  TSIDL_TABLE + "for(int i = 0; i < " + field->ValName +"capacity; i ++)"+ TSIDL_NEWLINE  + TSIDL_TABLE  +"{" + TSIDL_NEWLINE;
                        if(mapField->Key->Type == TSTypeDef::Struct)
                        {
                            paramsDeserializeInit += TSIDL_2TABLE + mapkey + " topic1 = new " + mapkey + "();\n    	topic1.Deserialize(buffer);\n"
                                + TSIDL_2TABLE + mapvalue + " topic2 = new " + mapvalue + "();\n     	topic2.Deserialize(buffer);\n" + TSIDL_2TABLE
                                + field->ValName + ".put(topic1,topic2);\n    }";
                        }
                        else
                        {
                            paramsDeserializeInit += TSIDL_2TABLE + mapkey + " topic1 = ""buffer.Read" + mapField->Key->TypeName + "();\n"
                                + TSIDL_2TABLE + mapvalue + " topic2 = new " + mapvalue + "();\n    	topic2.Deserialize(buffer);\n" + TSIDL_2TABLE
                                + field->ValName + ".put(topic1,topic2);\n    }";              
                        }
                    }
                    else
                    {
                        paramsDeserializeInit += TSIDL_TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                            +  TSIDL_TABLE + "for(int i = 0; i < " + field->ValName +"capacity; i ++)"+ TSIDL_NEWLINE  + TSIDL_TABLE  +"{" + TSIDL_NEWLINE
                            +  TSIDL_2TABLE + field->ValName;
                    }
					
					if(mapField->Key->Type == TSTypeDef::Enum)
					{
						paramsDeserializeInit +=  ".put(" + mapField->Key->TypeName + ".getEnumByValue(buffer.ReadInt32()),";
						if (mapField->Value->Type == TSTypeDef::Enum)
						{
							paramsDeserializeInit +=  mapField->Key->TypeName + ".getEnumByValue(buffer.ReadInt32()));" + TSIDL_NEWLINE
								+ TSIDL_TABLE +"}" + TSIDL_NEWLINE;
						}
						else
						{
							paramsDeserializeInit += " buffer.Read" + mapField->Value->TypeName + "());" + TSIDL_NEWLINE
								+ TSIDL_TABLE +"}" + TSIDL_NEWLINE;
						}
					}
					else if(mapField->Value->Type != TSTypeDef::Struct)
					{
						paramsDeserializeInit +=  ".put(buffer.Read" + mapField->Key->TypeName + "(),";
						if (mapField->Value->Type == TSTypeDef::Enum)
						{
							paramsDeserializeInit += "buffer.ReadInt32());" + TSIDL_NEWLINE
								+ TSIDL_TABLE +"}" + TSIDL_NEWLINE;
						}
						else
						{
							paramsDeserializeInit += " buffer.Read" + mapField->Value->TypeName + "());" + TSIDL_NEWLINE
								+ TSIDL_TABLE +"}" + TSIDL_NEWLINE;
						}
					}   

                }
                else//其他类型
                {
                    if( TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true).empty())
                    {
                         paramsInit += TSIDL_TABLE + "public"  + " " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true)+ field->DefTypeName + " " + field->ValName + ";"+TSIDL_NEWLINE;                                      
                    }
                    else
                    {
                        if( field->Def->Type == TSTypeDef::Enum)
                        {
                          paramsInit += TSIDL_TABLE + "public"  + " " + GetJAVAFunctioinType(field->Def) + " " + field->ValName + ";"+TSIDL_NEWLINE;                                      
                        }
                        else
                        {
                         paramsInit += TSIDL_TABLE + "public"  + " " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true)+ " " + field->ValName + ";"+TSIDL_NEWLINE;                                      
                        }
                       
                    }
                    if( field->Def->Type == TSTypeDef::Struct)//里面含有其他Struct
                    {
                        paramsTopicInit += TSIDL_TABLE + "this." + field->ValName + " = new " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true)  + "();" + TSIDL_NEWLINE;
                        paramsSerializeInit += TSIDL_TABLE + "this." + field->ValName + ".Serialize(buffer,2)"  + ";" + TSIDL_NEWLINE;
                        paramsDeserializeInit += TSIDL_TABLE + "this." + field->ValName + ".Deserialize(buffer)"  + ";" + TSIDL_NEWLINE;
                    }
                    else if( field->Def->Type == TSTypeDef::Enum)
                    {
                        paramsSerializeInit += TSIDL_TABLE + "buffer.WriteInt32((int)this." +  field->ValName+ ".getValue());" + TSIDL_NEWLINE;
                        paramsDeserializeInit += TSIDL_TABLE +" this." + field->ValName + " = " +   GetJAVAFunctioinType(field->Def) + ".getEnumByValue(buffer.ReadInt32());\n";
                    }
                    else 
                    {
                        if(TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true) == "String[]")
                        {
                            paramsTopicInit += TSIDL_TABLE + "this." + field->ValName + " = new " + "String[1];" + TSIDL_NEWLINE;
                            paramsSerializeInit += TSIDL_TABLE + "buffer.Write" + "Array" + "(this." + field->ValName +");" + TSIDL_NEWLINE;
                        }
                        else //基础数据类型
                        {
                            paramsSerializeInit += TSIDL_TABLE + "buffer.Write" +  field->Def->TypeName + "(this." + field->ValName + ");" + TSIDL_NEWLINE;

                            paramsDeserializeInit += TSIDL_TABLE +"this." + field->ValName + " = " + "buffer.Read" + field->Def->TypeName + "();" + TSIDL_NEWLINE; 
                        }
                        if(TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true) == "String")
                        {
                            paramsTopicInit += TSIDL_TABLE + "this." + field->ValName + " =  " + "\"\";" + TSIDL_NEWLINE;
                         }
                      }
                   }     
                  }
                } 

                }
               else if(field->Type != TSParamDef::In)//out类型
                  {   
                    paramsInit += TSIDL_TABLE + "public " + className + "_" + Function->ValName + "_" +  field->ValName + " " + field->ValName + ";\n";
                   // if( field->Def->Type != TSTypeDef::Enum)
                   // {
                        paramsTopicInit += TSIDL_TABLE + field->ValName + " = new " + className + "_" +Function->ValName + "_" +  field->ValName + "();\n";  
                  //  }
                    //paramsTopicInit += TSIDL_TABLE + field->ValName + " = new " + className + "_" +Function->ValName + "_" +  field->ValName + "();\n";  
                    if (field->Def->Type == TSTypeDef::Array)//array类型
                    {
                        TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(field->Def,TSVectorBuiltinTypeDefPtr);
                        if (vectorField->Value->Type != TSTypeDef::Builtin)
                        {
                            if(vectorField->Value->Type == TSTypeDef::Enum)
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName + "." + field->ValName +".size());" + TSIDL_NEWLINE +
                                    TSIDL_2TABLE + "for(" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)  + " " + field->Def->TypeName  + "topic : " +  field->ValName + "." + field->ValName +")" + TSIDL_NEWLINE
                                    +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_TABLE + TSIDL_2TABLE + "buffer.WriteInt32((int)" + field->Def->TypeName + "topic.getValue());" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + "}" +  TSIDL_NEWLINE;

                                paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                                    +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "capacity; i ++)"  + TSIDL_NEWLINE + TSIDL_2TABLE +"{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE                   
                                    + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)  +" topic = " + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value)  +  ".getEnumByValue(buffer.ReadInt32());"
                                    + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + field->ValName + "." + field->ValName + ".add(topic);" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + "}" +  TSIDL_NEWLINE;

                            }
                            else
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName +"." + field->ValName +".size());" + TSIDL_NEWLINE +
                                    TSIDL_2TABLE + "for(" + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + " topic : " +  field->ValName + "." + field->ValName +")" + TSIDL_NEWLINE
                                    +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE  + TSIDL_TABLE + "topic.Serialize(buffer,2);" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + "}" +  TSIDL_NEWLINE;

                                paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                                    +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "capacity; i ++)"  + TSIDL_NEWLINE + TSIDL_2TABLE +"{" + TSIDL_NEWLINE
                                    +  TSIDL_2TABLE +TSIDL_TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + " topic = new " + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "();"  + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE
                                    + "topic.Deserialize(buffer);" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + field->ValName + "." + field->ValName +".add(topic);" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + "}" +  TSIDL_NEWLINE;

                            }

                        }
                        else//基础数组数据类型
                        {

                            if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "String")
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "if(this." +  field->ValName + "." + field->ValName +".size() == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")\" \");" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n"
                                    + TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[]" + field->ValName + "Devan = new "
                                    + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[this." + field->ValName + "." + field->ValName +".size()];\n" + TSIDL_2TABLE + "for(int i = 0; i < this." + field->ValName + "." + field->ValName +".size(); i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                    + TSIDL_TABLE + field->ValName + "Devan[i] = this." + field->ValName + "." + field->ValName +".get(i);\n" + TSIDL_2TABLE + "}\n";
                            }
                            else
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "if(this." +  field->ValName + "." + field->ValName +".size() == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")0);" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n"
                                    + TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[]" + field->ValName + "Devan = new "
                                    + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[this." + field->ValName + "." + field->ValName +".size()];\n" + TSIDL_2TABLE + "for(int i = 0; i < this." + field->ValName + "." + field->ValName +".size(); i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                    + TSIDL_TABLE + field->ValName + "Devan[i] = this." + field->ValName + "." + field->ValName + ".get(i);\n" + TSIDL_2TABLE + "}\n";
                            }             

                            if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint8" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT8" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint16" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT16" ||
                                TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint32" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT32" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint64" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT64" ||
                                TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint32" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT32" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint64" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT64" ||
                                TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "ushort" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "ulong" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint" ||
                                TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "udouble")
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteArray" +  "(" + field->ValName +"Devan,false);" + TSIDL_NEWLINE;
                            }
                            else
                            {
                                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteArray" +  "(" + field->ValName +"Devan,true);" + TSIDL_NEWLINE;
                            } 

                            paramsDeserializeInit += TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[] " + field->ValName + "Bin = " + "buffer.Read" + vectorField->Value->TypeName + "ForArray();" + TSIDL_NEWLINE; 

                            if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "String")
                            {
                                paramsDeserializeInit += TSIDL_2TABLE + "if(" +  field->ValName + "Bin.length == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")\" \");" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n" + TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "Bin.length; i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                    + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add(" + field->ValName + "Bin[i]);\n" + TSIDL_2TABLE + "}\n";
                            }
                            else
                            {
                                paramsDeserializeInit += TSIDL_2TABLE + "if(" +  field->ValName + "Bin.length == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE
                                    + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")0);" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n" + TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName + "Bin.length; i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                                    + TSIDL_TABLE + "this." + field->ValName + "." + field->ValName +".add(" + field->ValName + "Bin[i]);\n" + TSIDL_2TABLE + "}\n";
                            }           
                        }

                      }

                      else if (field->Def->Type == TSTypeDef::Map)//Map
                      {

                          TSMapBuiltinTypeDefPtr mapField = TS_CAST(field->Def,TSMapBuiltinTypeDefPtr);

                          TSString mapkey = "Number";
                          TSString mapvalue = "Number";
                          if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) ==  "String")
                          {
                              mapkey = "String";
                          }
                          else if (mapField->Key->Type == TSTypeDef::Enum )
                          {
                              mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key); 
                          }
                          else if ( mapField->Key->Type == TSTypeDef::Struct)
                          {
                              mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key,true);
                          }
                          else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) == "boolean")
                          {
                              mapkey = "Boolean";
                          }

                          if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "String")
                          {
                              mapvalue = "String";
                          }
                          else if(mapField->Value->Type == TSTypeDef::Enum)
                          {
                              mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value);
                          }
                          else if(mapField->Value->Type == TSTypeDef::Struct)
                          {
                              mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value,true);
                          }
                          else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "boolean")
                          {
                              mapvalue = "Boolean";
                          }


                          paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(" + field->ValName + "." + field->ValName +".size());" + TSIDL_NEWLINE +
                              TSIDL_2TABLE + "for(" + mapkey + " obj : " +  field->ValName + "." + field->ValName + ".keySet())" + TSIDL_NEWLINE
                              +  TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE+ "buffer.Write";
                          if (mapField->Key->Type == TSTypeDef::Enum)
                          {
                              paramsSerializeInit += "Int32";
                              paramsSerializeInit += "((int)((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + ")obj).getValue());" + TSIDL_NEWLINE;
                          }
                          else
                          {
                              paramsSerializeInit += mapField->Key->TypeName;
                              paramsSerializeInit += "((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + ")obj);" + TSIDL_NEWLINE; 
                          }

                          if (mapField->Value->Type == TSTypeDef::Enum)
                          {
                              paramsSerializeInit += TSIDL_2TABLE + TSIDL_TABLE + "buffer.Write";
                              paramsSerializeInit += "Int32";
                              paramsSerializeInit += "(((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + ")" + field->ValName +"." + field->ValName + ".get(obj)).getValue());" + TSIDL_NEWLINE 
                                  +  TSIDL_2TABLE + "}" +  TSIDL_NEWLINE;
                          }
                          else if(mapField->Value->Type == TSTypeDef::Struct)
                          {
                              paramsSerializeInit += TSIDL_2TABLE + TSIDL_TABLE + field->ValName +"." + field->ValName + ".get(obj).Serialize(buffer,2);" + TSIDL_NEWLINE 
                                  +  TSIDL_2TABLE + "}" +  TSIDL_NEWLINE;
                          }
                          else
                          {
                              paramsSerializeInit += TSIDL_2TABLE + TSIDL_TABLE + "buffer.Write";
                              paramsSerializeInit += mapField->Value->TypeName;
                              paramsSerializeInit += "((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + ")" + field->ValName +".get(obj));" + TSIDL_NEWLINE 
                                  +  TSIDL_2TABLE + "}" +  TSIDL_NEWLINE;
                          }

                          paramsDeserializeInit += TSIDL_2TABLE + "int " + field->ValName + "capacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                              +  TSIDL_2TABLE + "for(int i = 0; i < " + field->ValName +"capacity; i ++)"+ TSIDL_NEWLINE  + TSIDL_2TABLE  +"{" + TSIDL_NEWLINE;

                          if(mapField->Key->Type == TSTypeDef::Enum)
                          {
                              paramsDeserializeInit +=  TSIDL_2TABLE  + TSIDL_TABLE + field->ValName +".put(" + mapField->Key->TypeName + ".getEnumByValue(buffer.ReadInt32()),";
                              if (mapField->Value->Type == TSTypeDef::Enum)
                              {
                                  paramsDeserializeInit += "buffer.ReadInt32());" + TSIDL_NEWLINE
                                      + TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
                              }
                              else
                              {
                                  paramsDeserializeInit += " buffer.Read" + mapField->Value->TypeName + "());" + TSIDL_NEWLINE
                                      + TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
                              }
                          }
                          else
                          {
                              if ( mapField->Value->Type == TSTypeDef::Struct )
                              {
                                  paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + mapkey + " tmpKey = buffer.Read" + mapField->KeyTypeName + "();\n";
                                  paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + mapvalue + " tmpValue = new " + mapvalue + "();\n";
                                  paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + "tmpValue.Deserialize(buffer);\n";
                                  paramsDeserializeInit += TSIDL_2TABLE  + TSIDL_TABLE + field->ValName + "." + field->ValName  +".put(tmpKey,tmpValue);\n        }\n";
                              }
                              else
                              {
                                  paramsDeserializeInit +=  TSIDL_2TABLE  + TSIDL_TABLE + field->ValName + ".put(buffer.Read" + mapField->Key->TypeName + "(),";
                                  if (mapField->Value->Type == TSTypeDef::Enum)
                                  {
                                      paramsDeserializeInit += "buffer.ReadInt32());" + TSIDL_NEWLINE
                                          + TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
                                  }
                                  else
                                  {
                                      paramsDeserializeInit += "buffer.Read" + mapField->Value->TypeName + "());" + TSIDL_NEWLINE
                                          + TSIDL_2TABLE +"}" + TSIDL_NEWLINE;
                                  }
                              }
                          }   

                        }
                        else//其他类型
                        {
                            if( field->Def->Type == TSTypeDef::Struct)//里面含有其他Struct
                            {
                                //   paramsTopicInit += TSIDL_TABLE + "this." + field->ValName + " = new " + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true)  + "();" + TSIDL_NEWLINE;
                                paramsSerializeInit += TSIDL_TABLE + "this." + field->ValName + "." + field->ValName + ".Serialize(buffer,2)"  + ";" + TSIDL_NEWLINE;
                                paramsDeserializeInit += TSIDL_TABLE +"this." + field->ValName + "." + field->ValName + ".Deserialize(buffer)"  + ";" + TSIDL_NEWLINE;
                            }
                            else if( field->Def->Type == TSTypeDef::Enum)
                            {
                              paramsSerializeInit += TSIDL_TABLE + "buffer.WriteInt32((int)this." + field->ValName +"." +  field->ValName+ ".getValue());" + TSIDL_NEWLINE;
                              paramsDeserializeInit += TSIDL_TABLE +" this." + field->ValName +"."+ field->ValName + " = " +   GetJAVAFunctioinType(field->Def)+ ".getEnumByValue(buffer.ReadInt32());\n";
                            }
                            else 
                            {
                                if(TSIBuiltinPlugin::GetSuitFullTypeName(field->Def,true) == "String[]")
                                {
                                    //  paramsTopicInit += TSIDL_TABLE + "this." + field->ValName + " = new " + "String[1];" + TSIDL_NEWLINE;
                                    paramsSerializeInit += TSIDL_TABLE + "buffer.Write" + "Array" + "(this." + field->ValName + "." +  field->ValName +");" + TSIDL_NEWLINE;
                                }
                                else //基础数据类型
                                {
                                    paramsSerializeInit += TSIDL_TABLE + "buffer.Write" +  field->Def->TypeName + "(this." + field->ValName + "." +  field->ValName + ");" + TSIDL_NEWLINE;

                                    paramsDeserializeInit += TSIDL_TABLE +" this." + field->ValName + "." + field->ValName + " = " + "buffer.Read" + field->Def->TypeName + "();" + TSIDL_NEWLINE; 
                                }

                            }
                        }
                }
       } 
       //对函数返回值进行处理
       if (!IsinOrOut)
       {    
           if(Function->DefTypeName != "void")
           {
               if(Function->Def->Type == TSTypeDef::Array)
               {
                     TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(Function->Def,TSVectorBuiltinTypeDefPtr);
                     if (vectorField->Value->Type != TSTypeDef::Builtin)
                     {
                         if( vectorField->Value->Type == TSTypeDef::Enum)
                         {
                             paramsInit += TSIDL_TABLE +"public Vector<" + GetJAVAFunctioinType(vectorField->Value) +"> rnt" +  ";\n";
                         }
                         else
                         {
                             paramsInit += TSIDL_TABLE +"public " + GetJAVAFunctioinType(Function->Def) +" rnt" +  ";\n";
                         }
                     }
                     else
                     {
                        paramsInit += TSIDL_TABLE +"public " + GetJAVAFunctioinType(Function->Def) +" rnt" +  ";\n";
                     }
               }
               else
               {
                    paramsInit += TSIDL_TABLE +"public " + GetJAVAFunctioinType(Function->Def) +" rnt" +  ";\n";
               }
               
           }

            if (Function->Def->Type == TSTypeDef::Array)//array类型
            {
                TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(Function->Def,TSVectorBuiltinTypeDefPtr);

                if (vectorField->Value->Type != TSTypeDef::Builtin)
                {
                   if( vectorField->Value->Type == TSTypeDef::Enum)
                   {
                      
                       paramsTopicInit += TSIDL_TABLE + "this.rnt" +  " = new Vector<" +  GetJAVAFunctioinType(vectorField->Value) + ">();\n ";

                       paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32(rnt.size());" + TSIDL_NEWLINE +
                           TSIDL_TABLE + "for(" + GetJAVAFunctioinType(vectorField->Value) + " topic : rnt )" + TSIDL_NEWLINE
                           +  TSIDL_TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + "buffer.WriteInt32((int)topic.getValue());" + TSIDL_NEWLINE + TSIDL_TABLE
                           + "}" +  TSIDL_NEWLINE;

                       paramsDeserializeInit += TSIDL_2TABLE + "int rntcapacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                           +  TSIDL_TABLE + "for(int i = 0; i < rntcapacity; i ++)"  + TSIDL_NEWLINE + TSIDL_TABLE +"{" + TSIDL_NEWLINE
                           +  TSIDL_2TABLE + GetJAVAFunctioinType(vectorField->Value) + " topic = " + GetJAVAFunctioinType(vectorField->Value) + ".getEnumByValue(buffer.ReadInt32());"  + TSIDL_NEWLINE + TSIDL_2TABLE 
                           + TSIDL_NEWLINE + TSIDL_2TABLE + "rnt.add(topic);" + TSIDL_NEWLINE + TSIDL_TABLE
                           + "}" +  TSIDL_NEWLINE;

                   }
                   else
                   {
                       paramsTopicInit += TSIDL_TABLE + "this.rnt" +  " = new " +  GetJAVAFunctioinType(Function->Def) + "();\n ";
                       paramsSerializeInit += TSIDL_TABLE + "buffer.WriteInt32(rnt.size());" + TSIDL_NEWLINE +
                           TSIDL_TABLE + "for(" + GetJAVAFunctioinType(vectorField->Value) + " topic : rnt )" + TSIDL_NEWLINE
                           +  TSIDL_TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + "topic.Serialize(buffer,2);" + TSIDL_NEWLINE + TSIDL_TABLE
                           + "}" +  TSIDL_NEWLINE;

                       paramsDeserializeInit += TSIDL_TABLE + "int rntcapacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                           +  TSIDL_TABLE + "for(int i = 0; i < rntcapacity; i ++)"  + TSIDL_NEWLINE + TSIDL_TABLE +"{" + TSIDL_NEWLINE
                           +  TSIDL_2TABLE + GetJAVAFunctioinType(vectorField->Value) + " topic = new " + GetJAVAFunctioinType(vectorField->Value) + "();"  + TSIDL_NEWLINE + TSIDL_2TABLE 
                           + "topic.Deserialize(buffer);" + TSIDL_NEWLINE + TSIDL_2TABLE + "rnt.add(topic);" + TSIDL_NEWLINE + TSIDL_TABLE
                           + "}" +  TSIDL_NEWLINE;

                   }
                  
                }
                else//基础数组数据类型
                {
                    paramsTopicInit += TSIDL_TABLE + "this.rnt" +  " = new " + GetJAVAFunctioinType(Function->Def) + "();\n";

                    if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "String")
                    {
                        paramsSerializeInit += TSIDL_2TABLE + "if(this.rnt.size() == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + "this.rnt.add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")\" \");" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n"
                            + TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[] rntDevan = new "
                            + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[this.rnt.size()];\n" + TSIDL_2TABLE + "for(int i = 0; i < this.rnt.size(); i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                            + TSIDL_TABLE + "rntDevan[i] = this.rnt.get(i);\n" + TSIDL_2TABLE + "}\n";
                    }
                    else
                    {
                        paramsSerializeInit += TSIDL_2TABLE + "if(this.rnt.size() == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE + TSIDL_TABLE + "this.rnt.add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")0);" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n"
                            + TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[] rntDevan = new "
                            + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[this.rnt.size()];\n" + TSIDL_2TABLE + "for(int i = 0; i < this.rnt.size(); i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                            + TSIDL_TABLE + "rntDevan[i] = this.rnt.get(i);\n" + TSIDL_2TABLE + "}\n";
                    } 

                    if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint8" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT8" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint16" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT16" ||
                        TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint32" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT32" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint64" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT64" ||
                        TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint32" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT32" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint64" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "UINT64" ||
                        TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "ushort" || TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "ulong" ||TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "uint" ||
                        TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "udouble")
                    {
                        paramsSerializeInit += TSIDL_TABLE + "buffer.WriteArray" +  "(rntDevan,false);" + TSIDL_NEWLINE;
                    }
                    else
                    {
                        paramsSerializeInit += TSIDL_TABLE + "buffer.WriteArray" +  "(rntDevan,true);" + TSIDL_NEWLINE;
                    } 
                    paramsDeserializeInit += TSIDL_2TABLE + TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + "[] rntBin = " + "buffer.Read" + vectorField->Value->TypeName + "ForArray();" + TSIDL_NEWLINE; 

                    if(TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) == "String")
                    {
                        paramsDeserializeInit += TSIDL_2TABLE + "if(rntBin.length == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE
                            + TSIDL_TABLE + "this.rnt.add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")\" \");" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n" + TSIDL_2TABLE + "for(int i = 0; i < rntBin.length; i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                            + TSIDL_TABLE + "this.rnt.add(rntBin[i]);\n" + TSIDL_2TABLE + "}\n";
                    }
                    else
                    {
                        paramsDeserializeInit += TSIDL_2TABLE + "if(rntBin.length == 0)" + TSIDL_NEWLINE + TSIDL_2TABLE + "{" + TSIDL_NEWLINE + TSIDL_2TABLE
                            + TSIDL_TABLE + "this.rnt.add((" +  TSIBuiltinPlugin::GetSuitFullTypeName(vectorField->Value,true) + ")0);" + TSIDL_NEWLINE + TSIDL_2TABLE + "}\n" + TSIDL_2TABLE + "for(int i = 0; i < rntBin.length; i++)\n" + TSIDL_2TABLE +"{\n" + TSIDL_2TABLE
                            + TSIDL_TABLE + "this.rnt.add(rntBin[i]);\n" + TSIDL_2TABLE + "}\n";
                    }
                }

        }
        else if (Function->Def->Type == TSTypeDef::Map)//Map
        {
            TSMapBuiltinTypeDefPtr mapField = TS_CAST(Function->Def,TSMapBuiltinTypeDefPtr);

            TSString mapkey = "Number";
            TSString mapvalue = "Number";
			if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) ==  "String")
			{
				mapkey = "String";
			}
            else if (mapField->Key->Type == TSTypeDef::Enum)
            {
                mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key); 
            }
            else if( mapField->Key->Type == TSTypeDef::Struct)
            {
                mapkey = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key,true); 
            }
            else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) == "boolean")
            {
                mapkey = "Boolean";
            }

            if(TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "String")
            {
                mapvalue = "String";
            }
            else if(mapField->Value->Type == TSTypeDef::Enum)
            {
                mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value);
            }
            else if (mapField->Value->Type == TSTypeDef::Struct)
            {
                mapvalue = TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value,true);
            }
			else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) ==  "boolean")
			{
				mapvalue = "Boolean";
			}

            paramsTopicInit += TSIDL_TABLE + "this.rnt" +  " = new " +"HashMap<" +mapkey +"," 
                + mapvalue + ">"  + "();\n";
            paramsSerializeInit += TSIDL_TABLE + "buffer.WriteInt32(rnt.size());" + TSIDL_NEWLINE +
                TSIDL_TABLE + "for(" + mapkey + " obj : rnt.keySet())" + TSIDL_NEWLINE+  TSIDL_TABLE + "{";
            if(mapField->Key->Type == TSTypeDef::Enum)
            {
                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32((int)((" + mapField->Key->TypeName + ")obj).getValue());\n" ;
                paramsDeserializeInit += TSIDL_TABLE + "int rntcapacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                       +  TSIDL_TABLE + "for(int i = 0; i < rntcapacity; i ++)"+ TSIDL_NEWLINE + TSIDL_TABLE  +"{" + TSIDL_NEWLINE
                       +  TSIDL_2TABLE + "rnt.put(" + mapField->Key->TypeName + ".getEnumByValue(buffer.ReadInt32()),";
            }
            else if(mapField->Key->Type == TSTypeDef::Struct)
            {
                paramsSerializeInit += TSIDL_NEWLINE + TSIDL_2TABLE + "obj.Serialize(buffer,2);\n" ;

                if(mapField->Value->Type == TSTypeDef::Struct)
                {
                    paramsDeserializeInit += TSIDL_TABLE + "int rntcapacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                        +  TSIDL_TABLE + "for(int i = 0; i < rntcapacity; i ++)"+ TSIDL_NEWLINE + TSIDL_TABLE  +"{" + TSIDL_NEWLINE
                        + TSIDL_2TABLE + mapkey + " topic1 = new " + mapkey +"();\n    	topic1.Deserialize(buffer);\n" + TSIDL_2TABLE +  mapkey + " topic2 = new " + mapkey +"();\n    	topic2.Deserialize(buffer);\n"
                        +  TSIDL_2TABLE + "rnt.put(topic1,";                
                }
                else
                {
                    paramsDeserializeInit += TSIDL_TABLE + "int rntcapacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                        +  TSIDL_TABLE + "for(int i = 0; i < rntcapacity; i ++)"+ TSIDL_NEWLINE + TSIDL_TABLE  +"{" + TSIDL_NEWLINE
                        + TSIDL_2TABLE + mapkey + " topic1 = new " + mapkey +"();\n    	+ topic1.Deserialize(buffer);\n"
                        +  TSIDL_2TABLE + "rnt.put(topic1,";               
                }      
            }
            else
            {
                paramsSerializeInit += TSIDL_2TABLE + "buffer.Write" + mapField->Key->TypeName
                    +"((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + ")obj);" + TSIDL_NEWLINE;
                paramsDeserializeInit += TSIDL_TABLE + "int rntcapacity = buffer.ReadInt32();" + TSIDL_NEWLINE
                           +  TSIDL_TABLE + "for(int i = 0; i < rntcapacity; i ++)"+ TSIDL_NEWLINE + TSIDL_TABLE  +"{" + TSIDL_NEWLINE
                           +  TSIDL_2TABLE + "rnt.put(buffer.Read" +  mapField->Key->TypeName + "(),";
            }

            if( mapField->Value->Type == TSTypeDef::Enum)
            {
                paramsSerializeInit += TSIDL_2TABLE + "buffer.WriteInt32((int)(rnt.get(obj)).getValue());\n" + TSIDL_NEWLINE 
                    +  TSIDL_TABLE + "}" +  TSIDL_NEWLINE;
                paramsDeserializeInit += mapField->Value->TypeName + ".getEnumByValue(buffer.ReadInt32()));" + TSIDL_NEWLINE+ TSIDL_TABLE +"}" + TSIDL_NEWLINE; 
            }
            else if( mapField->Value->Type == TSTypeDef::Struct)
            {
                paramsSerializeInit += TSIDL_2TABLE + "rnt.get(obj).Serialize(buffer,2);\n" + TSIDL_NEWLINE 
                    +  TSIDL_TABLE + "}" +  TSIDL_NEWLINE;
                paramsDeserializeInit += "topic2);" + TSIDL_NEWLINE+ TSIDL_TABLE +"}" + TSIDL_NEWLINE; 
            }
            else
            {
                paramsSerializeInit += TSIDL_2TABLE + "buffer.Write" + mapField->Value->TypeName
                    +"((" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + ")rnt.get(obj));" + TSIDL_NEWLINE 
                    +  TSIDL_TABLE + "}" +  TSIDL_NEWLINE;
                paramsDeserializeInit += " buffer.Read" + mapField->Value->TypeName + "());" + TSIDL_NEWLINE+ TSIDL_TABLE +"}" + TSIDL_NEWLINE; 
            }

        }
        else//其他类型
        {

            if( Function->Def->Type == TSTypeDef::Struct)//里面含有其他Struct
            {
                paramsTopicInit += TSIDL_TABLE + "this.rnt" + " = new " + TSIBuiltinPlugin::GetSuitFullTypeName(Function->Def,true)  + "();" + TSIDL_NEWLINE;
                paramsSerializeInit += TSIDL_TABLE + "this.rnt.Serialize(buffer,2)"  + ";" + TSIDL_NEWLINE;
                paramsDeserializeInit += TSIDL_TABLE + "this.rnt.Deserialize(buffer)"  + ";" + TSIDL_NEWLINE;
            }
            else if( Function->Def->Type == TSTypeDef::Enum)
            {
                paramsSerializeInit += TSIDL_TABLE + "buffer.WriteInt32((int)this.rnt.getValue());" + TSIDL_NEWLINE;
                paramsDeserializeInit += TSIDL_TABLE +" this.rnt = " +   GetJAVAFunctioinType(Function->Def) + ".getEnumByValue(buffer.ReadInt32());\n";
            }
            else 
            {
                if(TSIBuiltinPlugin::GetSuitFullTypeName(Function->Def,true) == "String[]")
                {
                    paramsTopicInit += TSIDL_TABLE + "this.rnt" + " = new " + "String[1];" + TSIDL_NEWLINE;
                    paramsSerializeInit += TSIDL_TABLE + "buffer.Write" + "Array" + "(this.rnt);" + TSIDL_NEWLINE;
                }
                else //基础数据类型
                {
                     if(Function->Def->TypeName != "void")
                     {
                         paramsSerializeInit += TSIDL_TABLE + "buffer.Write" +  Function->Def->TypeName + "(this.rnt);" + TSIDL_NEWLINE;

                         paramsDeserializeInit += TSIDL_TABLE +"this.rnt = " + "buffer.Read" + Function->Def->TypeName + "();" + TSIDL_NEWLINE; 
                     
                     }
                   
                }
                if(TSIBuiltinPlugin::GetSuitFullTypeName(Function->Def,true) == "String")
                {
                    paramsTopicInit += TSIDL_TABLE + "this.rnt" + " =  " + "\"\";" + TSIDL_NEWLINE;
                }
            }
        }     
       }
       if(IsinOrOut)
       {
          boost::algorithm::replace_all(content,"_JAVA_STRUCT_EXTEND_NAME_", className + "_" + Function->ValName + "_Request");  
       }
       else
       {
          boost::algorithm::replace_all(content,"_JAVA_STRUCT_EXTEND_NAME_", className + "_" + Function->ValName + "_Response");  
       }

     }
     else
     {
        boost::algorithm::replace_all(content,"_JAVA_STRUCT_EXTEND_NAME_", className);  
     }

     boost::algorithm::replace_all(content,"_JAVA_IMPORTFILE_", importFile + importFileStruct);
     boost::algorithm::replace_all(content,"_JAVA_CLASS_FUNC_", paramsInit);
     boost::algorithm::replace_all(content,"_JAVA_PUBILC_FUNC_", paramsTopicInit);
     boost::algorithm::replace_all(content,"_JAVA_SERIALIZE_FUNC_", paramsSerializeInit);
     boost::algorithm::replace_all(content,"_JAVA_DERIALIZE_FUNC_", paramsDeserializeInit);

	 //替换简单类型的写接口
	 boost::algorithm::replace_all(content, "Writebool", "WriteBool");
	 boost::algorithm::replace_all(content, "WriteBoolean", "WriteBool");
	 boost::algorithm::replace_all(content, "WriteBOOL", "WriteBool");

	 boost::algorithm::replace_all(content, "Writesbyte", "WriteInt8");
	 boost::algorithm::replace_all(content, "WriteSBYTE", "WriteInt8");
	 boost::algorithm::replace_all(content, "Writebyte", "WriteInt8");
	 boost::algorithm::replace_all(content, "WriteBYTE", "WriteInt8");
	 boost::algorithm::replace_all(content, "Writechar", "WriteInt8");
	 boost::algorithm::replace_all(content, "WriteChar", "WriteInt8");
	 boost::algorithm::replace_all(content, "Writeuchar", "WriteInt8");
	 boost::algorithm::replace_all(content, "WriteUChar", "WriteInt8");
	 boost::algorithm::replace_all(content, "Writeint8", "WriteInt8");
	 boost::algorithm::replace_all(content, "WriteInt8", "WriteInt8");
	 boost::algorithm::replace_all(content, "WriteINT8", "WriteInt8");
	 boost::algorithm::replace_all(content, "Writeuint8", "WriteInt8");
	 boost::algorithm::replace_all(content, "WriteUInt8", "WriteInt8");
	 boost::algorithm::replace_all(content, "WriteUINT8", "WriteInt8");

	 boost::algorithm::replace_all(content, "Writeshort", "WriteInt16");
	 boost::algorithm::replace_all(content, "WriteShort", "WriteInt16");
	 boost::algorithm::replace_all(content, "WriteSHORT", "WriteInt16");
	 boost::algorithm::replace_all(content, "Writeushort", "WriteInt16");
	 boost::algorithm::replace_all(content, "WriteUShort", "WriteInt16");
	 boost::algorithm::replace_all(content, "WriteUSHORT", "WriteInt16");
	 boost::algorithm::replace_all(content, "Writeint16", "WriteInt16");
	 boost::algorithm::replace_all(content, "WriteInt16", "WriteInt16");
	 boost::algorithm::replace_all(content, "WriteINT16", "WriteInt16");
	 boost::algorithm::replace_all(content, "Writeuint16", "WriteInt16");
	 boost::algorithm::replace_all(content, "WriteUInt16", "WriteInt16");
	 boost::algorithm::replace_all(content, "WriteUINT16", "WriteInt16");

	 boost::algorithm::replace_all(content, "Writeint(", "WriteInt32(");
	 boost::algorithm::replace_all(content, "WriteInt(", "WriteInt32(");
	 boost::algorithm::replace_all(content, "WriteINT(", "WriteInt32(");
	 boost::algorithm::replace_all(content, "Writeuint(", "WriteInt32(");
	 boost::algorithm::replace_all(content, "WriteUInt(", "WriteInt32(");
	 boost::algorithm::replace_all(content, "WriteUINT(", "WriteInt32(");
	 boost::algorithm::replace_all(content, "Writeint32", "WriteInt32");
	 boost::algorithm::replace_all(content, "WriteInt32", "WriteInt32");
	 boost::algorithm::replace_all(content, "WriteINT32", "WriteInt32");
	 boost::algorithm::replace_all(content, "Writeuint32", "WriteInt32");
	 boost::algorithm::replace_all(content, "WriteUInt32", "WriteInt32");
	 boost::algorithm::replace_all(content, "WriteUINT32", "WriteInt32");
	 boost::algorithm::replace_all(content, "Writelong", "WriteInt32");
	 boost::algorithm::replace_all(content, "WriteLong", "WriteInt32");
	 boost::algorithm::replace_all(content, "WriteLONG", "WriteInt32");
	 boost::algorithm::replace_all(content, "Writeulong", "WriteInt32");
	 boost::algorithm::replace_all(content, "WriteULong", "WriteInt32");
	 boost::algorithm::replace_all(content, "WriteULONG", "WriteInt32");
	 boost::algorithm::replace_all(content, "Writewchar_t", "WriteInt32");

	 boost::algorithm::replace_all(content, "Writeint64", "WriteInt64");
	 boost::algorithm::replace_all(content, "WriteInt64", "WriteInt64");
	 boost::algorithm::replace_all(content, "WriteINT64", "WriteInt64");
	 boost::algorithm::replace_all(content, "Writeuint64", "WriteInt64");
	 boost::algorithm::replace_all(content, "WriteUInt64", "WriteInt64");
	 boost::algorithm::replace_all(content, "WriteUINT64", "WriteInt64");

	 boost::algorithm::replace_all(content, "Writefloat", "WriteFloat");
	 boost::algorithm::replace_all(content, "WriteFloat", "WriteFloat");
	 boost::algorithm::replace_all(content, "WriteFLOAT", "WriteFloat");
	 boost::algorithm::replace_all(content, "WriteFloat32", "WriteFloat");
	 boost::algorithm::replace_all(content, "Writefloat32", "WriteFloat");
	 boost::algorithm::replace_all(content, "Writefloat64", "WriteFloat");
	 boost::algorithm::replace_all(content, "WriteFloat64", "WriteFloat");

	 boost::algorithm::replace_all(content, "Writedouble", "WriteDouble");
	 boost::algorithm::replace_all(content, "WriteDouble", "WriteDouble");
	 boost::algorithm::replace_all(content, "WriteDOUBLE", "WriteDouble");

	 boost::algorithm::replace_all(content, "Writestring", "WriteString");
	 boost::algorithm::replace_all(content, "WriteString", "WriteString");
	 boost::algorithm::replace_all(content, "WriteSTRING", "WriteString");

	 boost::algorithm::replace_all(content, "WriteTSBasicSdo", "WriteTSObjectHandle");
	 boost::algorithm::replace_all(content, "WriteBooleanForArray", "WriteBoolForArray");

	 //替换简单类型的读接口
	 boost::algorithm::replace_all(content, "Readbool", "ReadBool");
	 boost::algorithm::replace_all(content, "ReadBoolean", "ReadBool");
	 boost::algorithm::replace_all(content, "ReadBOOL", "ReadBool");

	 boost::algorithm::replace_all(content, "Readsbyte", "ReadInt8");
	 boost::algorithm::replace_all(content, "ReadSBYTE", "ReadInt8");
	 boost::algorithm::replace_all(content, "Readbyte", "ReadInt8");
	 boost::algorithm::replace_all(content, "ReadBYTE", "ReadInt8");
	 boost::algorithm::replace_all(content, "Readchar", "ReadInt8");
	 boost::algorithm::replace_all(content, "ReadChar", "ReadInt8");
	 boost::algorithm::replace_all(content, "Readuchar", "ReadInt8");
	 boost::algorithm::replace_all(content, "ReadUChar", "ReadInt8");
	 boost::algorithm::replace_all(content, "Readint8", "ReadInt8");
	 boost::algorithm::replace_all(content, "ReadInt8", "ReadInt8");
	 boost::algorithm::replace_all(content, "ReadINT8", "ReadInt8");
	 boost::algorithm::replace_all(content, "Readuint8", "ReadInt8");
	 boost::algorithm::replace_all(content, "ReadUInt8", "ReadInt8");
	 boost::algorithm::replace_all(content, "ReadUINT8", "ReadInt8");

	 boost::algorithm::replace_all(content, "Readshort", "ReadInt16");
	 boost::algorithm::replace_all(content, "ReadShort", "ReadInt16");
	 boost::algorithm::replace_all(content, "ReadSHORT", "ReadInt16");
	 boost::algorithm::replace_all(content, "Readushort", "ReadInt16");
	 boost::algorithm::replace_all(content, "ReadUShort", "ReadInt16");
	 boost::algorithm::replace_all(content, "ReadUSHORT", "ReadInt16");
	 boost::algorithm::replace_all(content, "Readint16", "ReadInt16");
	 boost::algorithm::replace_all(content, "ReadInt16", "ReadInt16");
	 boost::algorithm::replace_all(content, "ReadINT16", "ReadInt16");
	 boost::algorithm::replace_all(content, "Readuint16", "ReadInt16");
	 boost::algorithm::replace_all(content, "ReadUInt16", "ReadInt16");
	 boost::algorithm::replace_all(content, "ReadUINT16", "ReadInt16");

	 boost::algorithm::replace_all(content, "Readint(", "ReadInt32(");
	 boost::algorithm::replace_all(content, "ReadInt(", "ReadInt32(");
	 boost::algorithm::replace_all(content, "ReadINT(", "ReadInt32(");
	 boost::algorithm::replace_all(content, "Readuint(", "ReadInt32(");
	 boost::algorithm::replace_all(content, "ReadUInt(", "ReadInt32(");
	 boost::algorithm::replace_all(content, "ReadUINT(", "ReadInt32(");
	 boost::algorithm::replace_all(content, "Readint32", "ReadInt32");
	 boost::algorithm::replace_all(content, "ReadInt32", "ReadInt32");
	 boost::algorithm::replace_all(content, "ReadINT32", "ReadInt32");
	 boost::algorithm::replace_all(content, "Readuint32", "ReadInt32");
	 boost::algorithm::replace_all(content, "ReadUInt32", "ReadInt32");
	 boost::algorithm::replace_all(content, "ReadUINT32", "ReadInt32");
	 boost::algorithm::replace_all(content, "Readlong", "ReadInt32");
	 boost::algorithm::replace_all(content, "ReadLong", "ReadInt32");
	 boost::algorithm::replace_all(content, "ReadLONG", "ReadInt32");
	 boost::algorithm::replace_all(content, "Readulong", "ReadInt32");
	 boost::algorithm::replace_all(content, "ReadULong", "ReadInt32");
	 boost::algorithm::replace_all(content, "ReadULONG", "ReadInt32");
	 boost::algorithm::replace_all(content, "Readwchar_t", "ReadInt32");

	 boost::algorithm::replace_all(content, "Readint64", "ReadInt64");
	 boost::algorithm::replace_all(content, "ReadInt64", "ReadInt64");
	 boost::algorithm::replace_all(content, "ReadINT64", "ReadInt64");
	 boost::algorithm::replace_all(content, "Readuint64", "ReadInt64");
	 boost::algorithm::replace_all(content, "ReadUInt64", "ReadInt64");
	 boost::algorithm::replace_all(content, "ReadUINT64", "ReadInt64");

	 boost::algorithm::replace_all(content, "Readfloat", "ReadFloat");
	 boost::algorithm::replace_all(content, "ReadFloat", "ReadFloat");
	 boost::algorithm::replace_all(content, "ReadFLOAT", "ReadFloat");
	 boost::algorithm::replace_all(content, "Readfloat32", "ReadFloat");
	 boost::algorithm::replace_all(content, "ReadFloat32", "ReadFloat");
	 boost::algorithm::replace_all(content, "Readfloat64", "ReadFloat");
	 boost::algorithm::replace_all(content, "ReadFloat64", "ReadFloat");

	 boost::algorithm::replace_all(content, "Readdouble", "ReadDouble");
	 boost::algorithm::replace_all(content, "ReadDouble", "ReadDouble");
	 boost::algorithm::replace_all(content, "ReadDOUBLE", "ReadDouble");

	 boost::algorithm::replace_all(content, "Readstring", "ReadString");
	 boost::algorithm::replace_all(content, "ReadString", "ReadString");
	 boost::algorithm::replace_all(content, "ReadSTRING", "ReadString");

	 boost::algorithm::replace_all(content, "ReadTSBasicSdo", "ReadTSObjectHandle");
	 boost::algorithm::replace_all(content, "ReadBooleanForArray", "ReadBoolForArray");

	 //特殊替换
	 boost::algorithm::replace_all(content, "ReadintFor", "ReadInt32For");
	 boost::algorithm::replace_all(content, "ReadIntFor", "ReadInt32For");
	 boost::algorithm::replace_all(content, "ReadINTFor", "ReadInt32For");
	 boost::algorithm::replace_all(content, "ReaduintFor", "ReadInt32For");
	 boost::algorithm::replace_all(content, "ReadUIntFor", "ReadInt32For");
	 boost::algorithm::replace_all(content, "ReadUINTFor", "ReadInt32For");

	 boost::algorithm::replace_all(content, "Vector<int>", "Vector<Integer>");
	 boost::algorithm::replace_all(content, "Vector<byte>", "Vector<Byte>");
	 boost::algorithm::replace_all(content, "Vector<long>", "Vector<Long>");
	 boost::algorithm::replace_all(content, "Vector<short>", "Vector<Short>");
	 boost::algorithm::replace_all(content, "Vector<string>", "Vector<String>");
	 boost::algorithm::replace_all(content, "Vector<double>", "Vector<Double>");
	 boost::algorithm::replace_all(content, "Vector<boolean>", "Vector<Boolean>");
	 boost::algorithm::replace_all(content, "Vector<float>", "Vector<Float>");
	 boost::algorithm::replace_all(content, "(boolean)0", "false");

	 boost::algorithm::replace_all(content, "::", "_");

     ProjFile->SetContent(TSString2Utf8(content));
 }

 void TSJavaPlugin::GenerateSERVICESupportFile(TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString className,TSString HashVersionCode)
 {
     TSString content = _JAVA_SERVEICE_SUPPORT_FILE_;

	 int filedKey = 0;
	 TSTypeDefWithFiledPtr TypeField = TS_CAST(TypeDef, TSTypeDefWithFiledPtr);
	 BOOST_FOREACH(TSFieldDefPtr field, TypeField->Fileds)
	 {
		 if (field->FindSet != 0)
		 {
			 filedKey = field->FindSet;
			 break;
		 }
	 }
	 TSString HasFindSetTContent = "if (0 == Flag) {\n" + TSIDL_2TABLE + TSIDL_TABLE + "return " + ((filedKey == 0) ? "true" : "false") + ";\n" + TSIDL_2TABLE + "}\n";
	 HasFindSetTContent += TSIDL_2TABLE + "if (2 == Flag) {\n" + TSIDL_2TABLE + TSIDL_TABLE + "return " + ((filedKey == 2) ? "true" : "false") + ";\n" + TSIDL_2TABLE + "}\n";
	 HasFindSetTContent += TSIDL_2TABLE + "if (4 == Flag) {\n" + TSIDL_2TABLE + TSIDL_TABLE + "return " + ((filedKey == 4) ? "true" : "false") + ";\n" + TSIDL_2TABLE + "}\n";
	 HasFindSetTContent += TSIDL_2TABLE + "if (8 == Flag) {\n" + TSIDL_2TABLE + TSIDL_TABLE + "return " + ((filedKey == 8) ? "true" : "false") + ";\n" + TSIDL_2TABLE + "}\n";
	 HasFindSetTContent += TSIDL_2TABLE + "if (16 == Flag) {\n" + TSIDL_2TABLE + TSIDL_TABLE + "return " + ((filedKey == 16) ? "true" : "false") + ";\n" + TSIDL_2TABLE + "}\n";

     TSString ParseFileBaseName = 
         GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

	 boost::algorithm::replace_all(content,"_HASHVERSION_CODE_",HashVersionCode);
     boost::algorithm::replace_all(content,"_JAVA_PACKAGE_NAME_",ParseFileBaseName);
     boost::algorithm::replace_all(content,"_JAVA_STRUCT_NAME_",className);
	 boost::algorithm::replace_all(content,"_HASFINDSETTCONTENT_", HasFindSetTContent);
     boost::algorithm::replace_all(content,"::","_");
     boost::algorithm::replace_all(content,"Vector<int>","Vector<Integer>");
     boost::algorithm::replace_all(content,"Vector<byte>","Vector<Byte>");
     boost::algorithm::replace_all(content,"Vector<long>","Vector<Long>");
     boost::algorithm::replace_all(content,"Vector<short>","Vector<Short>");
     boost::algorithm::replace_all(content,"Vector<string>","Vector<String>");
     boost::algorithm::replace_all(content,"Vector<double>","Vector<Double>");
     boost::algorithm::replace_all(content,"Vector<boolean>","Vector<Boolean>");
     boost::algorithm::replace_all(content,"Vector<float>","Vector<Float>");
     ProjFile->SetContent(TSString2Utf8(content));
 
 }

 void TSJavaPlugin::GenerateMethodFile( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString FilePackageName,TSString ParseFileBaseName, std::vector<TSString>& serviceName)
 {    
     TSString content = _JAVA_SERVICE_METHOD_;
     TSString funcs;
     bool haveRetrurn = false;
     bool Init = true;
     int count = 0;
     TSString importFile;
     BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
     { 
		 /*TSString funName = Function->ValName;
		 std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		 if (iterTopic == serviceName.end())
		 {
			 continue;
		 }*/
         TSString filenameReponse = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "_" + Function->ValName + "_";
         boost::algorithm::replace_all(filenameReponse,"::","_");
         funcs += Table(2) + "public abstract " + GetJavaFunctions(Function,filenameReponse) + ";\n";
         importFile += GetImportJarNameFunc(Function,ParseFileBaseName);
     }
     boost::algorithm::replace_all(content,"_IMPORT_NAME_",importFile);
     boost::algorithm::replace_all(content, "_FUNC_", funcs);
     boost::algorithm::replace_all(content,"_PACKAGE_NAME_",FilePackageName);
     boost::algorithm::replace_all(content,"::","_");
     boost::algorithm::replace_all(content,"Vector<int>","Vector<Integer>");
     boost::algorithm::replace_all(content,"Vector<byte>","Vector<Byte>");
     boost::algorithm::replace_all(content,"Vector<long>","Vector<Long>");
     boost::algorithm::replace_all(content,"Vector<short>","Vector<Short>");
     boost::algorithm::replace_all(content,"Vector<string>","Vector<String>");
     boost::algorithm::replace_all(content,"Vector<double>","Vector<Double>");
     boost::algorithm::replace_all(content,"Vector<boolean>","Vector<Boolean>");
     boost::algorithm::replace_all(content,"Vector<float>","Vector<Float>");
     ProjFile->AppendLast(TSString2Utf8(content));
 }

 TSString TSJavaPlugin::GetJavaFunctions(TSFunctionDefPtr Func,TSString PackageName,bool IsAsync)
 {
     TSString content;
     TSString TypeName;
     if(IsAsync)
     {
        content += "void" + TSIDL_BLANK + Func->ValName + "Async(";
     }
     else
     {
         content += GetJAVAFunctioinType(Func->Def)+ TSIDL_BLANK + Func->ValName + "(";
     }

     for (unsigned int i = 0; i < Func->Params.size(); i++)
     {
         TSParamDefPtr field = Func->Params[i];
         if (field->Def->Type != TSTypeDef::Builtin ||
             field->Def->Type != TSTypeDef::Enum)
         {
             //if( field->Def->Type == TSTypeDef::Array)
             //{
             //
             //}
             TypeName = GetJAVAFunctioinType(field->Def);

         }
         else
         {
             TypeName = Link2Type(field->DefTypeName);
         }
   
        TSString TypeName = GetJAVAFunctioinType(field->Def);;

        if (field->Type == TSParamDef::In)
        {  
            content += TypeName + TSIDL_BLANK + field->ValName ;
        }
        else 
        {    
            TSString filenameReponse = PackageName + field->ValName;
            content += filenameReponse + " " + field->ValName;
        }
        if (Func->Params.size() != i + 1)
        {
            content += ",";
        }
     }

     if(IsAsync)
     {
		 if (Func->Params.size())
		 {
			 content += ", proxyInter func) throws Exception";
		 }
		 else
		 {
			content += "proxyInter func) throws Exception";
		 
		 }
         
     }
     else
     {
        content += ")";
     }
     return content;

 }
 
 TSString TSJavaPlugin::GetJavaFunctionsWithOutType(TSFunctionDefPtr Func,TSString PackageName,bool IsAsync)
 {
     TSString content;
     TSString TypeName;
     if(IsAsync)
     {
         content += Func->ValName + "Async(";
     }
     else
     {
         content += Func->ValName + "(";
     }

     for (unsigned int i = 0; i < Func->Params.size(); i++)
     {
         TSParamDefPtr field = Func->Params[i];

         if (field->Type == TSParamDef::In)
         {  
             content += field->ValName;
         }
         else 
         {    
             content += Func->ValName + "_" + field->ValName;
         }
         if (Func->Params.size() != i + 1)
         {
             content += ",";
         }
     }
     content += ")";
     return content;
 }
 void TSJavaPlugin::GenerateProxyFile( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString FilePackageName,TSString ParseFileBaseName, std::vector<TSString>& serviceName)
 {
     TSString content = PROXY_CODE_SERVICE_JAVA;
     TSString suit_full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);

     TSString funcstr,funcOnDataStr,regietermethodstr;
     TSString importFile = ParseFileBaseName + "_xidl.*;\n";
     TSString importFileStruct = "";
     BOOST_FOREACH(TSFunctionDefPtr func,TypeDef->Functions)
     {
		 /*TSString funName = func->ValName;
		 std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		 if (iterTopic == serviceName.end())
		 {
			 continue;
		 }*/
         TSString suit_Request = suit_full_type_name;
         TSString suit_Response = suit_full_type_name;

         suit_Request += "::" + func->ValName + "::Request";
         suit_Response += "::" + func->ValName + "::Response";

         TSString suit_Request_ID = suit_Request;
         boost::algorithm::replace_all(suit_Request_ID, "::","_");
         boost::algorithm::replace_all(suit_Request, "::",".");

         TSString suit_Response_ID = suit_Response;
         boost::algorithm::replace_all(suit_Response_ID, "::","_");
         boost::algorithm::replace_all(suit_Response, "::",".");

         importFileStruct += GetImportJarNameFunc(func,ParseFileBaseName);

         funcOnDataStr += Table(1) + "public static void " + func->ValName + "AsyncResponseInvoker(Object UserData, long ResponseTopic, Object ResponseData,Object Invoker,boolean IsTimeout){\n";
         funcOnDataStr += Table(2) + "if(( ResponseTopic == " + suit_Response_ID + "Topic.ID.m_value)&&(!IsTimeout))\n" + Table(3) + "{\n";
         funcOnDataStr += Table(3) + suit_Response_ID + " Res = (" + suit_Response_ID + ")ResponseData;\n";

         BOOST_FOREACH(TSParamDefPtr paramDef,func->Params)
         {
             if(paramDef->Type != TSParamDef::In)
             {
                 funcOnDataStr += Table(3) + GetJAVAFunctioinType(paramDef->Def) + " "+ paramDef->ValName + " = Res." + paramDef->ValName + "." + paramDef->ValName+ ";\n";
             }
         }
         if(func->DefTypeName != "void")
         {
             funcOnDataStr += Table(3) +  GetJAVAFunctioinType(func->Def) + " rnt = Res.rnt;\n";
         }
          funcOnDataStr += Table(3) + "proxyInter sysc = (proxyInter)(" + "TSServerLocator.EntityMAPProxyMethod.get(" + suit_Response_ID + "Topic.ID));\n";
         funcOnDataStr += Table(3) + "sysc.Inter" + func->ValName +"(";
         int countInnum = 0;
         BOOST_FOREACH(TSParamDefPtr paramDef,func->Params)//遍历里面有多少个非In类型
         {
             if(paramDef->Type != TSParamDef::In)
             {
                 countInnum++;
             }
         }

         if(func->DefTypeName != "void")
         {
             countInnum++;
         }  
         for (unsigned int i = 0; i < func->Params.size(); i++)
         {
             TSParamDefPtr field = func->Params[i];

             if (field->Type == TSParamDef::In)
             {  
                 countInnum++;
             }
             else
             {            
                 funcOnDataStr += "Res." + field->ValName;
                 if (i != countInnum - 1)
                 {
                     funcOnDataStr += ",";
                 }
             }
         }

         if(func->DefTypeName != "void")
         {
             funcOnDataStr += "rnt);\n" + Table(2) + "}\n" + Table(1)+ "}\n";
         }
         else
         {
             funcOnDataStr +=");\n" + Table(2) + "}\n" + Table(1)+ "}\n";
         }

         regietermethodstr += Table(3) + "RegisterMethod(\"" + func->ValName + "\"," + suit_Request_ID + "Topic.ID," + suit_Response_ID + "Topic.ID,null,\"" +suit_full_type_name
                + "4Java/ResponseHandler/" + func->ValName + "AsyncResponseInvoker\");\n";

         TSString filenameReponse = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "_" + func->ValName + "_";
         funcstr += TSIDL_TABLE + "public " + GetJavaFunctions(func,filenameReponse) + " throws Exception {\n";
         funcstr += Table(2) + TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true)+ "_" + func->ValName + "_Request Request = new " + TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true)+ "_" + func->ValName + "_Request();\n";
         funcstr += Table(2) + "TSTOPICHANDLE ErrorCode = new TSTOPICHANDLE();\n";
         BOOST_FOREACH(TSParamDefPtr paramDef,func->Params)
         {
             if(paramDef->Type != TSParamDef::Out)
             {
                 funcstr += Table(2) + "Request." + paramDef->ValName + " = " + paramDef->ValName + ";\n";
             }
         }
         funcstr += Table(2) +TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true)+ "_" + func->ValName + "_Response Response = (" + TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true)+ "_" + func->ValName + "_Response)SendRequestSync(\"" + func->ValName
                    + "\",Request,ErrorCode);\n";
         funcstr += Table(2) + "if (ErrorCode.m_value != 0) {\n" + Table(3) + "System.out.println(\"代理调用服务失败\");\n" + Table(3) + "throw new Exception(\"代理调用服务失败\");\n" + Table(2) +"}\n";
         funcstr += Table(2) + "if (Response == null) {\n" + Table(3) + "System.out.println(\"代理调用服务超时\");\n" + Table(3) + "throw new Exception(\"代理调用服务超时\");\n" + Table(2) + "}\n";
		 BOOST_FOREACH(TSParamDefPtr paramDef,func->Params)
         {
             if(paramDef->Type != TSParamDef::In)
             {
                 funcstr += Table(2) + paramDef->ValName +  "." + paramDef->ValName + " = Response." + paramDef->ValName + "." + paramDef->ValName + ";\n";
             }
         }
         if(func->DefTypeName != "void")
         {
             funcstr += Table(2) + "return Response.rnt;\n" + Table() + "}\n";
         }
         else
         {
             funcstr += Table(1) + "}\n";
         }
         //JAVA代理的异步有问题，暂时不作处理
     /*    funcstr += TSIDL_TABLE + "public " +  GetJavaFunctions(func,filenameReponse,true) + "{\n";
         funcstr += Table(2) + "super.SetTopicID(" + suit_Response_ID + "Topic.ID,func);\n";
         funcstr += Table(2) + "TSTOPICHANDLE ErrorCode = null;\n";
         funcstr += Table(2) + TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true)+ "_" + func->ValName + "_Request Request = new " + TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true)+ "_" + func->ValName + "_Request();\n";
   
         BOOST_FOREACH(TSParamDefPtr paramDef,func->Params)
         {
             if(paramDef->Type != TSParamDef::Out)
             {
                 funcstr += Table(2) + "Request." + paramDef->ValName + " = " + paramDef->ValName + ";\n";
             }
         }

         funcstr += Table(2) + "SendRequestAsync(\"" + func->ValName + "\",Request,func,ErrorCode);\n" + Table(1) + "}\n";*/
     }

     boost::algorithm::replace_all(content, "_IMPORT_NAME_",importFile + importFileStruct);
     boost::algorithm::replace_all(content,"_REGISTERMETHOD_",regietermethodstr);
     boost::algorithm::replace_all(content,"_ONDATA_FAUNC_",funcOnDataStr);
     boost::algorithm::replace_all(content, "_IMPORT_NAME_",ParseFileBaseName + "_xidl.*;\n");
     boost::algorithm::replace_all(content, "_PACKAGE_NAME_",FilePackageName);
     boost::algorithm::replace_all(content, "_FUNS_",funcstr);
     boost::algorithm::replace_all(content,"::","_");
     boost::algorithm::replace_all(content,"Vector<int>","Vector<Integer>");
     boost::algorithm::replace_all(content,"Vector<byte>","Vector<Byte>");
     boost::algorithm::replace_all(content,"Vector<long>","Vector<Long>");
     boost::algorithm::replace_all(content,"Vector<short>","Vector<Short>");
     boost::algorithm::replace_all(content,"Vector<string>","Vector<String>");
     boost::algorithm::replace_all(content,"Vector<double>","Vector<Double>");
     boost::algorithm::replace_all(content,"Vector<boolean>","Vector<Boolean>");
     boost::algorithm::replace_all(content,"Vector<float>","Vector<Float>");

     ProjFile->AppendLast(content);
 }

 void TSJavaPlugin::GenerateTOPICFile(TSProjFilePtr ProjFile,TSTypeDefPtr TypeDef, TSTypeDefWithFiledPtr  structPtr )
 {
     TSString content = _JAVA_TOPIC_FILE_;
     TSString ParseFileBaseName = 
         GetBaseName(structPtr->Parse.lock()->GetFullFileName());
	 TSString ParseFileBaseNameLocl =
		 GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
     TSTypeDefWithFiledPtr TypeField = TS_CAST(TypeDef,TSTypeDefWithFiledPtr);
     boost::algorithm::replace_all(content,"_JAVA_STRUCT_NAME_", TSIBuiltinPlugin::GetSuitFullTypeName(structPtr,true));

     boost::algorithm::replace_all(content,"_JAVA_PACKAGE_NAME_",ParseFileBaseName);
	 boost::algorithm::replace_all(content, "_JAVA_PACKAGELOCL_NAME_", ParseFileBaseNameLocl);
     boost::algorithm::replace_all(content,"_JAVA_TOPIC_NAME_", TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true));
     TSString topicClassName = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "Topic";
     boost::algorithm::replace_all(content,"_JAVA_TOPIC_", topicClassName);
     boost::algorithm::replace_all(content,"::","_");
     boost::algorithm::replace_all(content,"Vector<int>","Vector<Integer>");
     boost::algorithm::replace_all(content,"Vector<byte>","Vector<Byte>");
     boost::algorithm::replace_all(content,"Vector<long>","Vector<Long>");
     boost::algorithm::replace_all(content,"Vector<short>","Vector<Short>");
     boost::algorithm::replace_all(content,"Vector<string>","Vector<String>");
     boost::algorithm::replace_all(content,"Vector<double>","Vector<Double>");
     boost::algorithm::replace_all(content,"Vector<boolean>","Vector<Boolean>");
     boost::algorithm::replace_all(content,"Vector<float>","Vector<Float>");
     ProjFile->AppendLast(TSString2Utf8(content));
 }

 void TSJavaPlugin::GenerateProxyInterFile( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString FilePackageName,TSString ParseFileBaseName, std::vector<TSString>& serviceName)
 {
     TSString content = _PROXYINTERFILE_;
     TSString funcs;
     bool haveRetrurn = false;
     int count = 0;
     TSString importFileStruct = "";
     BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
     {
		 /*TSString funName = Function->ValName;
		 std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		 if (iterTopic == serviceName.end())
		 {
			 continue;
		 }*/
         importFileStruct += GetImportJarNameFunc(Function,ParseFileBaseName);

         TSString filenameReponse = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "_" + Function->ValName + "_";
         boost::algorithm::replace_all(filenameReponse,"::","_");
         std::vector<TSParamDefPtr> vecParams; 
         haveRetrurn = false;

         
         if (Function->DefTypeName != "void" && 
             Function->DefTypeName != "Void")
         {
             haveRetrurn = true;
         }
         int countInnum = 0;
         funcs += Table(2) + "void " + "Inter" + Function->ValName +"(";
         BOOST_FOREACH(TSParamDefPtr paramDef,Function->Params)//遍历里面有多少个非In类型
         {
             if(paramDef->Type != TSParamDef::In)
             {
                 countInnum++;
             }
         }

         if(Function->DefTypeName != "void")
         {
              countInnum++;
         }  
         for (unsigned int i = 0; i < Function->Params.size(); i++)
         {
             TSParamDefPtr field = Function->Params[i];

             if (field->Type == TSParamDef::In)
             {  
                 countInnum++;
             }
             else
             {            
                 funcs += filenameReponse + field->ValName + " " + field->ValName;
                 if (i != countInnum - 1)
                 {
                     funcs += ",";
                 }
             }
         }

        if(Function->DefTypeName != "void")
        {
            funcs += GetJAVAFunctioinType(Function->Def) + " rnt);"+ TSIDL_NEWLINE;
        }  
        else
        {
            funcs += ");"+ TSIDL_NEWLINE;
        }
     }
     TSString MethodFun = funcs;
     boost::algorithm::replace_all(content,"_IMPORT_NAME_",importFileStruct);
     boost::algorithm::replace_all(content, "_FUNC_", MethodFun);
     boost::algorithm::replace_all(content,"_PACKAGENAME_",FilePackageName);
     boost::algorithm::replace_all(content,"::","_");
     boost::algorithm::replace_all(content,"Vector<int>","Vector<Integer>");
     boost::algorithm::replace_all(content,"Vector<byte>","Vector<Byte>");
     boost::algorithm::replace_all(content,"Vector<long>","Vector<Long>");
     boost::algorithm::replace_all(content,"Vector<short>","Vector<Short>");
     boost::algorithm::replace_all(content,"Vector<string>","Vector<String>");
     boost::algorithm::replace_all(content,"Vector<double>","Vector<Double>");
     boost::algorithm::replace_all(content,"Vector<boolean>","Vector<Boolean>");
     boost::algorithm::replace_all(content,"Vector<float>","Vector<Float>");

     ProjFile->AppendLast(TSString2Utf8(content));
 }

 void TSJavaPlugin::GenerateServerSkelsFile( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString FilePackageName,TSString ParseFileBaseName, std::vector<TSString>& serviceName)
 {
     TSString content = SERVICE_SERVICEKEL_CODER_4JAVA;
     
     TSString suit_full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
     TSString funcstr,registerstr;
     TSString ServerName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "::ServerSkel";

     TSString importFile = ParseFileBaseName + "_xidl.*;\n";
     TSString importFileStruct = "";

     BOOST_FOREACH(TSFunctionDefPtr func,TypeDef->Functions)
     {
		 /*TSString funName = func->ValName;
		 std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		 if (iterTopic == serviceName.end())
		 {
			 continue;
		 }*/
         importFileStruct += GetImportJarNameFunc(func,ParseFileBaseName);     
         TSString filenameReponse = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "_" + func->ValName + "_";
         TSString suit_Request = suit_full_type_name;
         TSString suit_Response = suit_full_type_name;
 
         suit_Request += "::" + func->ValName + "::Request";
         suit_Response += "::" + func->ValName + "::Response";

         TSString suit_Request_ID = suit_Request;
         boost::algorithm::replace_all(suit_Request_ID, "::","_");
         boost::algorithm::replace_all(suit_Request, "::",".");

         TSString suit_Response_ID = suit_Response;
         boost::algorithm::replace_all(suit_Response_ID, "::","_");
         boost::algorithm::replace_all(suit_Response, "::",".");
        
         funcstr += Table(2) + "public  static void " + func->ValName + "MethodRequestHandler(String UserData, TSTOPICHANDLE RequestTopic, Object RequestDataIn,TSTOPICHANDLE ResponseTopic, Object ResponseDataOut) {\n";
         funcstr += Table(3) + "ServerSkels Skel = (ServerSkels)TSServer.EntityMAP.get(" + suit_Request_ID + "Topic.ID.m_value + UserData);\n";
         funcstr += Table(3) + suit_Request_ID + " Req = (" + suit_Request_ID + ")RequestDataIn;\n";

         for(int i = 0; i < func->Params.size(); i++)
         {
             if (TSParamDefPtr paramDef = func->Params[i])
             {
                 if(paramDef->Type == TSParamDef::In)
                 {
                     funcstr +=  Table(3) +  GetJAVAFunctioinType(paramDef->Def) + " " + paramDef->ValName + " = " + "Req." + paramDef->ValName + ";\n";
                 }

                 if(paramDef->Type != TSParamDef::In)
                 {
                     funcstr += Table(3) + TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true)+ "_" + func->ValName + "_" + paramDef->ValName
                                + " " + func->ValName + "_" + paramDef->ValName + " =  new " +  TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true)+ "_" + func->ValName + "_" + paramDef->ValName + "();\n";
                 }

                 if(paramDef->Type == TSParamDef::Inout)
                 {
                     funcstr += Table(3) +  func->ValName + "_" + paramDef->ValName +"." + paramDef->ValName  + " = " + "Req." + paramDef->ValName +"." + paramDef->ValName+ ";\n";
                 }
      
             }
         }
         if(func->DefTypeName != "void")
         {
              funcstr += Table(3) +  GetJAVAFunctioinType(func->Def) + " rnt = ";
         }      
         funcstr += "((Method) Skel.GetMethod())." + GetJavaFunctionsWithOutType(func,filenameReponse) + ";\n";
         funcstr += Table(3) + suit_Response_ID + " Response = (" + suit_Response_ID + ") ResponseDataOut;\n";
         for(int i = 0; i < func->Params.size(); i++)
         {
             if (TSParamDefPtr paramDef = func->Params[i])
             {
                 if(paramDef->Type != TSParamDef::In)
                 {
                     funcstr += Table(3) + "Response." + paramDef->ValName + "." + paramDef->ValName + " = " + func->ValName +  "_" + paramDef->ValName + "." + paramDef->ValName + ";\n";
                 }
             }
         }

         if (func->DefTypeName != "void" && 
             func->DefTypeName != "Void")
         {

             funcstr += Table(3) + "Response.rnt = rnt;\n";
         }
         funcstr += Table(2) + "}\n";

         registerstr += Table(3) + "if(super.EntityMAP.containsKey(" + suit_Request_ID + "Topic.ID.m_value + DomainId+ super.GetServerName()) != true){\n";
         registerstr += Table(4) + "super.SetTopicID(" + suit_Request_ID + "Topic.ID.m_value + DomainId+ super.GetServerName(), this);\n";
         registerstr += Table(4) + "RegisterMethod(\"" + func->ValName + "\"," + suit_Request_ID + "Topic.ID," + suit_Response_ID + "Topic.ID,"
             + "\"" + suit_full_type_name + "4Java/RequestHandler/" + func->ValName + "MethodRequestHandler/\" + DomainId + super.GetServerName() , null);\n" + Table(4) + "}\n";
     }
     
     boost::algorithm::replace_all(content, "_IMPORTNAMESKEL_",importFile + importFileStruct);
     boost::algorithm::replace_all(content, "_IMPORTNAMESKEL_",ParseFileBaseName + "_xidl.*;\n");
     boost::algorithm::replace_all(funcstr,"::","_");

     boost::algorithm::replace_all(content, "_PACKAGENAME_",FilePackageName);
     boost::algorithm::replace_all(content,"_RESISTERMETHOD_",registerstr);
     boost::algorithm::replace_all(content, "_FUNC_HANDLER_",funcstr);

     boost::algorithm::replace_all(content,"::","_");
     boost::algorithm::replace_all(content, "_SERVERSKEL_TYPENAME_",ServerName);
     boost::algorithm::replace_all(content,"Vector<int>","Vector<Integer>");
     boost::algorithm::replace_all(content,"Vector<byte>","Vector<Byte>");
     boost::algorithm::replace_all(content,"Vector<long>","Vector<Long>");
     boost::algorithm::replace_all(content,"Vector<short>","Vector<Short>");
     boost::algorithm::replace_all(content,"Vector<string>","Vector<String>");
     boost::algorithm::replace_all(content,"Vector<double>","Vector<Double>");
     boost::algorithm::replace_all(content,"Vector<boolean>","Vector<Boolean>");
     boost::algorithm::replace_all(content,"Vector<float>","Vector<Float>");

     ProjFile->AppendLast(content);
 }

 void TSJavaPlugin::GenerateOutClassFile( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSString className,TSString ParseFileBaseName )
 {

 }

 TSString TSJavaPlugin::GetImportJarNameFunc( TSFunctionDefPtr Function,TSString ParseFileBaseName )
 {
     TSString importFileStruct = "";
     std::vector<TSString> tmp;
     if(Function != NULL)
     {   
         if(Function->Def->Type == TSTypeDef::Struct||Function->Def->Type == TSTypeDef::Enum)
         {
             if( GetBaseName(Function->Def->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
             {
                 importFileStruct = "import " + ParseFileBaseName + "4Java.*;\n";
             }
             else
             {
                 importFileStruct = "import " + GetBaseName(Function->Def->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
             }
         }

         if( Function->Def->Type == TSTypeDef::Array)
         {
             TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(Function->Def,TSVectorBuiltinTypeDefPtr);
             if (vectorField->Value->Type != TSTypeDef::Builtin||vectorField->Value->Type == TSTypeDef::Enum)
             {
                 if( GetBaseName(vectorField->Value->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                 {
                     importFileStruct = "import " + ParseFileBaseName + "4Java.*;\n";
                 }
                 else
                 {
                     importFileStruct = "import " + GetBaseName(vectorField->Value->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                 }
             }
         }

         if( Function->Def->Type == TSTypeDef::Map)
         {
             TSMapBuiltinTypeDefPtr mapField = TS_CAST(Function->Def,TSMapBuiltinTypeDefPtr);

             if (mapField->Key->Type != TSTypeDef::Builtin||mapField->Key->Type == TSTypeDef::Enum)
             {
                 if( GetBaseName(mapField->Key->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                 {
                     importFileStruct = "import "+ ParseFileBaseName + "4Java.*" +  ";\n";
                 }
                 else
                 {
                     importFileStruct = "import " + GetBaseName(mapField->Key->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                 }
             }

             if (mapField->Value->Type != TSTypeDef::Builtin||mapField->Value->Type == TSTypeDef::Enum)
             {
                 if( GetBaseName(mapField->Value->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                 {
                     importFileStruct = "import "+ ParseFileBaseName + "4Java.*" +  ";\n";
                 }
                 else
                 {
                     importFileStruct = "import " + GetBaseName(mapField->Value->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                 }
             }
         }

         for (unsigned int i = 0; i < Function->Params.size(); i++)
         {
             TSParamDefPtr field = Function->Params[i];  

             if(field->Def->Type == TSTypeDef::Struct||field->Def->Type == TSTypeDef::Enum)
             {
                 if( GetBaseName(field->Def->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                 {
                     importFileStruct += "import "+ ParseFileBaseName + "4Java.*" +  ";\n";
                 }  
                 else
                 {
                     importFileStruct = "import " + GetBaseName(field->Def->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                 }
             }

             if( field->Def->Type == TSTypeDef::Array)
             {
                 TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(field->Def,TSVectorBuiltinTypeDefPtr);
                 if (vectorField->Value->Type != TSTypeDef::Builtin||vectorField->Value->Type == TSTypeDef::Enum)
                 {
                     if( GetBaseName(vectorField->Value->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                     {
                         importFileStruct += "import "+ ParseFileBaseName + "4Java.*" +  ";\n";
                     }
                     else
                     {
                         importFileStruct = "import " + GetBaseName(vectorField->Value->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                     }
                 }
             }

             if( field->Def->Type == TSTypeDef::Map)
             {
                 TSMapBuiltinTypeDefPtr mapField = TS_CAST( field->Def,TSMapBuiltinTypeDefPtr);

                 if (mapField->Key->Type != TSTypeDef::Builtin||mapField->Key->Type == TSTypeDef::Enum)
                 {
                     if( GetBaseName(mapField->Key->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                     {
                         importFileStruct += "import "+ ParseFileBaseName + "4Java.*" +  ";\n";
                     }
                     else
                     {
                         importFileStruct = "import " + GetBaseName(mapField->Key->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                     }
                 }

                 if (mapField->Value->Type != TSTypeDef::Builtin||mapField->Value->Type == TSTypeDef::Enum)
                 {
                     if( GetBaseName(mapField->Value->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                     {
                         importFileStruct += "import "+ ParseFileBaseName + "4Java.*" +  ";\n";
                     }
                     else
                     {
                         importFileStruct = "import " + GetBaseName(mapField->Value->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                     }
                 }
             }
         }
    }
   return importFileStruct;
}

TSString TSJavaPlugin::GetImportJarNameTopic( TSTypeDefPtr Function,TSString ParseFileBaseName )
{
    TSString importFileStruct = "";
    TSTypeDefWithFiledPtr TypeField = TS_CAST(Function,TSTypeDefWithFiledPtr);

    if(Function != NULL)
    { 
         /*BOOST_FOREACH(TSFieldDefPtr field,TypeField->Fileds)
         {
             if(field->Def->Type == TSTypeDef::Struct||field->Def->Type == TSTypeDef::Enum)
             {

                 if( GetBaseName(field->Def->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                 {
                     importFileStruct = "import " + ParseFileBaseName + "4Java.*;\n";
                 }
                 else
                 {
                     importFileStruct = "import " + GetBaseName(field->Def->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                 }
             }

             if( field->Def->Type == TSTypeDef::Array)
             {
                 TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(field->Def,TSVectorBuiltinTypeDefPtr);
                 if (vectorField->Value->Type != TSTypeDef::Builtin||vectorField->Value->Type == TSTypeDef::Enum)
                 {
                     if( GetBaseName(vectorField->Value->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                     {
                         importFileStruct = "import " + ParseFileBaseName + "4Java.*;\n";
                     }
                     else
                     {
                         importFileStruct = "import " + GetBaseName(vectorField->Value->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                     }
                 }
             }

             if( field->Def->Type == TSTypeDef::Map)
             {
                 TSMapBuiltinTypeDefPtr mapField = TS_CAST(field->Def,TSMapBuiltinTypeDefPtr);

                 if (mapField->Key->Type != TSTypeDef::Builtin||mapField->Key->Type == TSTypeDef::Enum)
                 {
                     if( GetBaseName(mapField->Key->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                     {
                         importFileStruct = "import "+ ParseFileBaseName + "4Java.*" +  ";\n";
                     }
                     else
                     {
                         importFileStruct = "import " + GetBaseName(mapField->Key->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                     }
                 }

                 if (mapField->Value->Type != TSTypeDef::Builtin||mapField->Value->Type == TSTypeDef::Enum)
                 {
                     if( GetBaseName(mapField->Value->Parse.lock()->GetFullFileName()) == ParseFileBaseName)
                     {
                         importFileStruct = "import "+ ParseFileBaseName + "4Java.*" +  ";\n";
                     }
                     else
                     {
                         importFileStruct = "import " + GetBaseName(mapField->Value->Parse.lock()->GetFullFileName()) + "4Java.*;\n";
                     }
                 }
             }
                
         }*/

		 TSStringArray StringAarry = Function->Parse.lock()->ImportFileNames;
		 for (size_t i = 0; i < StringAarry.size(); i++)
		 {
			 importFileStruct += "import " + GetBaseName(StringAarry[i]) + "4Java.*;\n";
		 }

    }
    return importFileStruct;
}

//获得要生成的服务，如果追加的服务为空则返回true
bool TSJavaPlugin::GetGenerateService(std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec, std::vector<TSString>& serviceVecStruct, bool serviceFlag)
{
	bool tempServiceFlag = false;
	if (!vecSelectService.empty())
	{
		if (serviceFlag)
		{
			tempServiceVec = GetVectorDifference(vecSelectService, serviceVecStruct);

			if (tempServiceVec.empty() || tempServiceVec[0] == "N")
			{
				tempServiceFlag = true;
			}
		}
		else
		{
			serviceVecStruct = vecSelectService;
			tempServiceVec = vecSelectService;
		}
	}
	return tempServiceFlag;
}

std::vector<TSString> TSJavaPlugin::GetVectorDifference(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& topicVec)
{
	std::vector<TSString> vecAppend;
	sort(vecSelectTopic.begin(), vecSelectTopic.end());
	sort(topicVec.begin(), topicVec.end());
	//创建差集
	std::set_difference(vecSelectTopic.begin(), vecSelectTopic.end(), topicVec.begin(), topicVec.end(), inserter(vecAppend, vecAppend.begin()));
	//当当前选中的主题和上一次选中的主题存在差集时记录当前选中的主题
	if (!vecAppend.empty())
	{
		topicVec = vecSelectTopic;
	}

	return vecAppend;
}

//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
bool TSJavaPlugin::GetGenerateTopic(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& tempTopicVec, std::vector<TSString>& topicVecStruct, bool topicFlag)
{
	bool emptyTopicFlag = false;
	//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
	if (!vecSelectTopic.empty())
	{
		if (topicFlag)
		{
			//获得当前主题和上次选中主题的差集
			tempTopicVec = GetVectorDifference(vecSelectTopic, topicVecStruct);
			if (tempTopicVec.empty() || tempTopicVec[0] == "N")
			{
				emptyTopicFlag = true;
				tempTopicVec.push_back("N");
			}
		}
		else
		{
			//记录第一次选中的主题
			topicVecStruct = vecSelectTopic;
			tempTopicVec = vecSelectTopic;
		}
	}
	return emptyTopicFlag;
}

//将和服务相关的结构体存入vector中
//serviceMap 服务和依赖结构体组成的Map, structVec服务依赖的结构体容器, indexServiceVec 记录服务的容器, vecSelectService 工具选中的服务, tempServiceVec 实际用来操作的服务容器
void TSJavaPlugin::StoreServiceAboutStruct(std::map<TSString, TSString>& serviceMap, std::vector<TSString>& serviceVec, std::vector<TSString>& indexServiceVec, std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec)
{
	if (!vecSelectService.empty() && !tempServiceVec.empty())
	{
		//将选中的服务存储起来
		for (auto it = serviceMap.begin(); it != serviceMap.end(); ++it)
		{
			std::vector<TSString>::iterator iterService = std::find(tempServiceVec.begin(), tempServiceVec.end(), it->first);
			if (iterService != tempServiceVec.end())
			{
				indexServiceVec.push_back(it->first);
				//将服务依赖的结构体存起来
				serviceVec.push_back(it->second);
			}
		}
	}
}

//过滤掉没有选中的节点以及可能重复生成的节点
bool TSJavaPlugin::FilterNotSelectNode(TSTypeDefPtr TypeDef, std::vector<TSString>& indexVec, std::vector<TSString>& structVec, std::vector<TSString>& storeStructRec)
{
	//过滤掉没有被选中的主题
	if (!indexVec.empty())
	{
		std::vector<TSString>::iterator iterTopic = std::find(indexVec.begin(), indexVec.end(), TypeDef->TypeName);
		bool flag = false;
		//如果服务有依赖的结构体要生成
		if (!structVec.empty())
		{
			//判断当前结构体是否被服务依赖
			std::vector<TSString>::iterator iterService = std::find(structVec.begin(), structVec.end(), TypeDef->TypeName);
			if (iterService != structVec.end())
			{
				flag = true;
			}
			//如果当前结构体生成过，则不再重复生成
			std::vector<TSString>::iterator iterCurrentStruct = std::find(storeStructRec.begin(), storeStructRec.end(), TypeDef->TypeName);
			if (iterCurrentStruct != storeStructRec.end())
			{
				return true;
			}
			//如果当前结构体没有被选中并且不被服务依赖则跳过当前循环
			if (iterTopic != indexVec.end() && !flag)
			{
				return true;
			}
		}
		else
		{
			//当前结构体没有被选中则跳过当前循环
			if (iterTopic != indexVec.end())
			{
				return true;
			}
			else
			{
				//如果当前结构体生成过，则不再重复生成
				std::vector<TSString>::iterator iterCurrentStruct = std::find(storeStructRec.begin(), storeStructRec.end(), TypeDef->TypeName);
				if (iterCurrentStruct != storeStructRec.end())
				{
					return true;
				}
			}
		}
	}
	return false;
}

//获得服务函数和依赖的结构体的Map
void TSJavaPlugin::GetServiceAndStructMap(std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs, std::map<TSString, TSString>& serviceMap)
{
	TSString tempName;
	BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr typeDef, TypeDefs)
	{
		if (typeDef->Type == TSTypeDef::Service)
		{
			BOOST_FOREACH(TSFunctionDefPtr Func, typeDef->Functions)
			{
				BOOST_FOREACH(TSParamDefPtr param, Func->Params)
				{
					if (tempName != param->Def->TypeName)
					{
						std::pair<TSString, TSString> servicePair(Func->ValName, param->Def->TypeName);
						serviceMap.insert(servicePair);
					}
					tempName = param->Def->TypeName;
				}
			}
		}
	}
}

std::vector<TSString> TSJavaPlugin::GetTopicInVec(std::vector<TSString>& tempTopicVec, std::vector<TSTypeDefPtr>& TypeDefs)
{
	std::vector<TSString> indexVec;
	if (!tempTopicVec.empty())
	{
		for (int i = 0; i < TypeDefs.size(); ++i)
		{
			bool flag = false;

			if (TypeDefs[i]->Type == TSTypeDef::Struct)
			{
				for (int j = 0; j < tempTopicVec.size(); ++j)
				{
					int pos = tempTopicVec[j].rfind(":");
					TSString str = tempTopicVec[j].substr(pos + 1, tempTopicVec[j].size());

					if (TypeDefs[i]->TypeName == str)
					{
						flag = true;
						break;
					}
				}
				if (!flag)
				{
					indexVec.push_back(TypeDefs[i]->TypeName);
				}
			}
		}
	}
	return indexVec;
}