#include "stdafx.h"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimUtil/pugixml.hpp>
#include <TopSimUtil/TSDynamicLib.h>
#else
#include <TopSimRuntime/pugixml.hpp>
#include <TopSimRuntime/TSDynamicLib.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TSSubscriberPlugin.h"
#include "TSCSharpPlugin.h"
#include "TSJavaPlugin.h"
#include "TSGenSupportPlugin.h"
#include "TSIDLUtils.h"
#include "temp/SubTopic.temp.h"



using namespace TSIDLUtils;

BEGIN_METADATA(TSSubscriberPlugin)
    REG_BASE(TSIIDLPlugin);
END_METADATA()

struct TSSubscriberPluginPrivate
{
    TSIBuiltinPluginPtr _BuiltinPlugin;
};
TSSubscriberPlugin::TSSubscriberPlugin( void ):_p(new TSSubscriberPluginPrivate)
{

}

TSSubscriberPlugin::~TSSubscriberPlugin( void )
{
    delete _p;
}

TSString TSSubscriberPlugin::GetName()
{
    return GB18030ToTSString("Subscriber文件生成插件");
}

void TSSubscriberPlugin::OnPluginLoaded()
{
    _p->_BuiltinPlugin = GetPluginT<TSIBuiltinPlugin>();
    ASSERT(_p->_BuiltinPlugin && "TSSubscriberPlugin::OnPluginLoaded");
}

bool TSSubscriberPlugin::DoParse()
{
    return true;
}

bool TSSubscriberPlugin::DoBuild()
{
    if (HasOption(NodeFileKey)) 
    {
        TSString nodeFile = Option<TSString>(NodeFileKey);
        TSString FileFullName = boost::filesystem::system_complete(boost::filesystem::path(nodeFile)).string();
        if (!boost::filesystem::exists(FileFullName.c_str()))
        {
            if(boost::filesystem::exists(boost::filesystem::current_path() / nodeFile.c_str()))
            {
                FileFullName = (boost::filesystem::current_path() / nodeFile.c_str()).string();
            }
            else
            {
                const std::vector<TSString> & IncludePaths = TSIDLPluginMgr::Instance()->GetIncludePaths();
                BOOST_FOREACH(const TSString & Path,IncludePaths)
                {
                    if(boost::filesystem::exists((Path + TS_TEXT("/") + nodeFile).c_str()))
                    {
                        FileFullName = Path + TS_TEXT("/") + nodeFile;
                        break;
                    }
                }
            }
        }
        
        pugi::xml_document doc;
        if(LoadNodeFile(doc, FileFullName))
        {
            ParserNodeFile(doc,_nodeFilePtr);
        }
        
        TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();

        if (Cp == "c#")
        {
            GenerateCsharp(_nodeFilePtr);
        }
        else if (Cp == "c++")
        {
            GenerateCpp(_nodeFilePtr);
        }
        else if (Cp == "java")
        {
            GenerateJava(_nodeFilePtr);
        }
        
    }
   return true;
}

void TSSubscriberPlugin::Clean()
{

}

void TSSubscriberPlugin::GenerateCpp( std::vector<TSNodeFilePtr> nodeFilePtr )
{
    TSString error;
    TSGenSupportPluginPtr CallBackAndSubImplCoder = boost::make_shared<TSGenSupportPlugin>();
    std::vector<TSTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTypes();
    for (size_t j = 0 ; j < nodeFilePtr.size() ; j++)
    {   
        for (size_t k = 0 ; k < nodeFilePtr[j]->NodeData.size() ; k++)
        {
            TSString code = SUBSCRIBERCODE;
            TSIDLUtils::ReplaceLinkerVersion(code,"_LINKER_VERSION_");
            TSString include , topicCallback , funcStr , callbackStr , pubStr;
            for (size_t i = 0 ; i < nodeFilePtr[j]->NodeData[k]->SubFullName.size() ; i++)
            {   

                TSString::size_type pos = nodeFilePtr[j]->NodeData[k]->SubFullName[i].find_last_of(":");   
                TSString typeName = nodeFilePtr[j]->NodeData[k]->SubFullName[i];

                if (pos != TSString::npos)
                {
                    typeName =  nodeFilePtr[j]->NodeData[k]->SubFullName[i].substr(pos + 1);
                }

                if (nodeFilePtr[j]->NodeData[k]->SubType[i] == "Topic")
                {                   
                    TSString topicStr;
                    std::vector<TSTopicTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTopicTypeDefs();
                    TSStructDefPtr structDef;
                    for(size_t m = 0; m < TypeDefs.size() ;++m)
                    {
                        if(TSTopicTypeDefPtr decl = TypeDefs[m])
                        {
                            if(decl->TypeName == typeName)
                            {   
                                std::vector<TSStructDefPtr> structDefs = _p->_BuiltinPlugin->GetAllStructDefs();
                                for (size_t k = 0; k < structDefs.size(); ++k)
                                {
                                    if(TSStructDefPtr structDef = structDefs[k])
                                    {
                                        TSString subFullName = TSBuiltinPlugin::GetSuitFullTypeName(structDef,true);
                                        if (subFullName == decl->DefTypeName)
                                        {
                                            boost::algorithm::replace_all(subFullName , "::" , ".");
                                            topicStr = TSIDLUtils::GetTopicName(decl);
                                            topicCallback += TOPICCALLBACK;
                                            TSString structName = TSBuiltinPlugin::GetSuitFullTypeName(structDef,true);
                                            callbackStr += TOPICFUNC;
                                            boost::algorithm::replace_all(callbackStr, "_TOPIC_NAME_", topicStr);
                                            boost::algorithm::replace_all(topicCallback, "_TOPIC_NAME_", topicStr);
                                            boost::algorithm::replace_all(topicCallback, "_STRUCT_NAME_", structName);
                                            break;
                                        }
                                    }
                                } 
                                break;
                            }
                            
                        }
                    }
                    include += "#include \"../../Defined.h\"\n";                    
                    boost::algorithm::replace_all(callbackStr , "_TOPICPLUSE_" , topicStr);
                }
                else if (nodeFilePtr[j]->NodeData[k]->SubType[i] == "Service")
                {
                    code = SERVICECOMMONCODE;
                    include += SERVICE_INCLUDESUB;
                    TSString subFullName = nodeFilePtr[j]->NodeData[k]->SubFullName[i];
                    boost::algorithm::replace_all(subFullName , "::" , ".");
                    boost::algorithm::replace_all(include , "_PACKAGE_" , subFullName);
                    for(size_t m = 0; m < TypeDefs.size() ;++m)
                    {
                        if(TSServiceDefPtr Services = TS_CAST(TypeDefs[m],TSServiceDefPtr))
                        {
                            if (TSBuiltinPlugin::GetSuitFullTypeName(Services) == subFullName)
                            {
                                TSString subPackage = TSBuiltinPlugin::GetSuitFullTypeName(Services,true);
                                boost::algorithm::replace_all(subPackage,"::",".");
                                boost::algorithm::replace_all(include , "_FILE_NAME_" , nodeFilePtr[j]->NodeData[k]->SubFileName[i]);

                                funcStr += SERVICE_SUBFUNC;            
                                boost::algorithm::replace_all(funcStr , "_TYPES_" , nodeFilePtr[j]->NodeData[k]->SubFullName[i]);                  
                                boost::algorithm::replace_all(funcStr , "_TYPE_" , typeName);
                                boost::algorithm::replace_all(funcStr , "_PACKAGENAME_" ,nodeFilePtr[j]->NodeData[k]->SubFullName[i]);
                                BOOST_FOREACH(TSFunctionDefPtr func, Services->Functions)
                                {
                                    boost::algorithm::replace_all(funcStr , "_REGISTER_NAME_" , TSIDLUtils::GetFuncCallStr(func));
                                    TSString paramsInit;
                                    TSString funcAsync = func->ValName + "Async(";
                                    TSString callbackfunc = "void" + TSIDL_BLANK + "On" + func->ValName + "(" + "bool rnt";
                                    boost::algorithm::replace_all(funcStr, "_PARAMS_INITIALIZE_", paramsInit);

                                    funcAsync += ",";
                                    funcAsync += "boost::bind(&On" + func->ValName+",";
                                    size_t out_count = 0;
                                    for(size_t i = 0; i < func->Params.size() ;++i)
                                    {
                                         TSParamDefPtr param = func->Params[i];
                                         if (param->Type != TSParamDef::In)
                                         {
                                            out_count++;
                                         }
                                    }

                                    for(size_t i = 0; i <= out_count; i++)                                
                                    {
                                        funcAsync += "_" + boost::lexical_cast<TSString>(i+1);
                                        if( i != out_count)
                                        {
                                            funcAsync += ",";
                                        }
                                    } 

                                    if(func->Params.size())
                                    {
                                       funcAsync += "));";
                                       boost::algorithm::replace_all(funcStr , "_REGISTERASY_NAME_" , funcAsync);

                                     }    
                                }
                            }
                        }
                    }             
                }
            }

            TSString pubFuncStr,attrDefine;
            for (size_t i = 0 ; i < nodeFilePtr[j]->NodeData[k]->PubFileName.size() ; i++) 
            {
                TSString::size_type pos = nodeFilePtr[j]->NodeData[k]->PubFullName[i].find_last_of(":");   
                TSString typeName = nodeFilePtr[j]->NodeData[k]->PubFullName[i];
                if (pos != TSString::npos)
                {
                    typeName =  nodeFilePtr[j]->NodeData[k]->PubFullName[i].substr(pos + 1);
                }


                if (nodeFilePtr[j]->NodeData[k]->PubType[i] == "Service")
                {
                    TSString callbackfunc = "";
                    code = SERVICECOMMONCODE;
                    include += SERVICE_INCLUDEPUB;

                    TSString pubFullName = nodeFilePtr[j]->NodeData[k]->PubFullName[i]; 
                    callbackfunc = "class " + nodeFilePtr[j]->NodeData[k]->PubFileName[i] + "Method : public " + pubFullName + "::Method\n" + "{\n";
                    
                    for(size_t m = 0; m < TypeDefs.size() ;++m)
                    {
                        if(TSServiceDefPtr Services = TS_CAST(TypeDefs[m],TSServiceDefPtr) )
                        {     
                              BOOST_FOREACH(TSFunctionDefPtr func, Services->Functions)
                              {
                                  if(typeName == Services->TypeName)
                                  {
                                     callbackfunc += Table() + "virtual " + TSBuiltinPlugin::GetSuitFullTypeName(func->Def) + " " + func->ValName + "(";
                                     for(size_t i = 0; i < func->Params.size(); ++i)
                                        {
                                            TSParamDefPtr Param = func->Params[i];
                                            TSString TypeName = TSBuiltinPlugin::GetSuitFullTypeName(Param->Def);
                                            if (Param->Type == TSParamDef::Inout || Param->Type == TSParamDef::Out)
                                            {
                                                callbackfunc += TypeName + TSIDL_BLANK + "& " + Param->ValName;
                                            }
                                            else
                                            {
                                                 callbackfunc += "const " + TypeName + TSIDL_BLANK + "& " + Param->ValName;
                                            }

                                            if (func->Params.size() != i + 1)
                                            {
                                                callbackfunc += ",";
                                            }
                                        }
                                     }
                                 }
                           }
                    }
                    callbackfunc += ")\n" + Table(2) + "{\n";
                    callbackfunc +=  Table(4) + "//user code" + TSIDL_NEWLINE + Table(4) + "return NULL;\n"  + Table(2) + "}\n};";                   
					//boost::algorithm::replace_all(include , "_ONREGISTERFUNC_" , callbackfunc);
                    boost::algorithm::replace_all(include , "_FILE_NAME_" , nodeFilePtr[j]->NodeData[k]->PubFileName[i]);
                    boost::algorithm::replace_all(pubFullName , "::" , ".");
                    boost::algorithm::replace_all(include , "_PACKAGE_" , pubFullName);
                    pubFuncStr += SERVICE_PUBFUNC;
                    boost::algorithm::replace_all(pubFuncStr , "_SERVICENAME_" , nodeFilePtr[j]->NodeData[k]->PubFileName[i]);
                    boost::algorithm::replace_all(pubFuncStr , "_TYPE_" , typeName);
                    boost::algorithm::replace_all(pubFuncStr , "_TYPE_NAME_" , typeName);
                    boost::algorithm::replace_all(pubFuncStr , "_PUB_FULLNAME_" , nodeFilePtr[j]->NodeData[k]->PubFullName[i]);
                    boost::algorithm::replace_all(pubFuncStr , "_PACKAGENAME_" , nodeFilePtr[j]->NodeData[k]->PubFullName[i]);
                }
                else if (nodeFilePtr[j]->NodeData[k]->PubType[i] == "Topic")
                {
                    callbackStr += TOPICPUBFUNC;
                    TSString topicStr , topicInit;
                    std::vector<TSTopicTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTopicTypeDefs();
                    for(size_t m = 0; m < TypeDefs.size() ;++m)
                    {
                        if(TSTopicTypeDefPtr decl = TypeDefs[m])
                        {
                     
                                if(decl->TypeName == typeName)
                                {
                                    std::vector<TSStructDefPtr> structDefs = _p->_BuiltinPlugin->GetAllStructDefs();
                                    for (size_t k = 0; k < structDefs.size(); ++k)
                                    {
                                        if (TSStructDefPtr structdef = structDefs[k])
                                        {
                                            TSString subFullName = TSBuiltinPlugin::GetSuitFullTypeName(structdef,true);
                                            if (subFullName == decl->DefTypeName)
                                            {
                                                std::vector<TSFieldDefPtr> vectorFeid = structdef->Fileds;
                                                TSString structName = TSBuiltinPlugin::GetSuitFullTypeName(structdef);
                                                topicInit += structName + " st;\n";                                                          
                                                TSString pubFullName = TSBuiltinPlugin::GetSuitFullTypeName(structdef,true);
                                                boost::algorithm::replace_all(pubFullName , "::" , ".");
                                                boost::algorithm::replace_all(pubFuncStr , "_TYPE_NAME_" , structdef->TypeName);
                                                boost::algorithm::replace_all(pubFuncStr , "_PUB_FULLNAME_" , 
                                                TSBuiltinPlugin::GetSuitFullTypeName(structdef,true));
                                                topicStr = TSIDLUtils::GetTopicName(decl);
                                                break; 
                                            }
                                        }
                                    }
                                }
                        }
                    }
                    include += "#include \"../../Defined.h\"\n";
                    boost::algorithm::replace_all(callbackStr, "_NA_ME_", topicStr);
                    boost::algorithm::replace_all(callbackStr, "_INIT_", topicInit);
                    
                }     

            }
            boost::algorithm::replace_all(code, "_PUBFUNC_", pubFuncStr);
            boost::algorithm::replace_all(code , "_APPNAME_" , nodeFilePtr[j]->ApplicationName[k]);
            boost::algorithm::replace_all(code, "_CALL_BACK_", callbackStr);
            funcStr += pubFuncStr;
            boost::algorithm::replace_all(code, "_CALL_CLASS_BACK_", funcStr);
            boost::algorithm::replace_all(code, "_INCLUDE_", include);

            boost::algorithm::replace_all(code, "_TOPICCALLBACK_", topicCallback);
            boost::algorithm::replace_all(code, "_NODE_", nodeFilePtr[j]->LorisDefaultDomain);
            boost::algorithm::replace_all(code, "_TIMES_", "1000");
            GeneratePubSubFile(nodeFilePtr[j]->Nodes[k],code);

        }
    } 
}

void TSSubscriberPlugin::GenerateCsharp( std::vector<TSNodeFilePtr> nodeFilePtr )
{
#ifdef _MSC_VER
    TSString error;
    std::vector<TSTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTypes();
    TSCSharpPluginPtr CallBackImplCoder = boost::make_shared<TSCSharpPlugin>();
    for (size_t j = 0 ; j < nodeFilePtr.size() ; j++)
    {   
        for (size_t k = 0 ; k < nodeFilePtr[j]->NodeData.size() ; k++)
        {
            TSString code = CSSUBSCRIBERCODE;
            boost::algorithm::replace_all(code, "_APPNAME_", nodeFilePtr[j]->ApplicationName[k]);
            TSString  funcStr;
            for (size_t i = 0 ; i < nodeFilePtr[j]->NodeData[k]->SubFullName.size() ; i++)
            {   
                TSString::size_type pos = nodeFilePtr[j]->NodeData[k]->SubFullName[i].find_last_of(":");   
                TSString typeName = nodeFilePtr[j]->NodeData[k]->SubFullName[i];

                if (pos != TSString::npos)
                {
                    typeName =  nodeFilePtr[j]->NodeData[k]->SubFullName[i].substr(pos + 1);
                }

                if (nodeFilePtr[j]->NodeData[k]->SubType[i] == "Topic")
                {
                    TSString topicStr;
                    funcStr = CSTOPICSUB;
                    std::vector<TSTopicTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTopicTypeDefs();
                    TSStructDefPtr structDef;
                    for(size_t m = 0; m < TypeDefs.size() ;++m)
                    {
                        if(TSTopicTypeDefPtr decl = TypeDefs[m])
                        {
                            if(decl->TypeName == typeName)
                            {           
                                std::vector<TSStructDefPtr> structDefs = _p->_BuiltinPlugin->GetAllStructDefs();
                                for (size_t k = 0; k < structDefs.size(); ++k)
                                {
                                    if (TSStructDefPtr structDef = structDefs[k])
                                    {
                                        TSString struTyname =  TSBuiltinPlugin::GetSuitFullTypeName(structDef,true);
                                        if (struTyname == decl->DefTypeName)
                                        {
                                            TSString subFullName = TSBuiltinPlugin::GetSuitFullTypeName(structDef,true) + TSIDL_2COLON + "DataType";
                                            boost::algorithm::replace_all(subFullName , "::" , ".");
                                           
                                            topicStr = TSIDLUtils::GetTopicName(decl); 
                                            boost::algorithm::replace_all(funcStr , "_TOPIC_NAME_" , 
                                                topicStr);
                                            boost::algorithm::replace_all(code , "_TOPIC_NAME_" , topicStr);
                                            boost::algorithm::replace_all(code , "_STRUCT_NAME_" , subFullName);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }               
                }
                else if (nodeFilePtr[j]->NodeData[k]->SubType[i] == "Service")
                {
                    TSString subFullName = nodeFilePtr[j]->NodeData[k]->SubFullName[i];
                    boost::algorithm::replace_all(subFullName , "::" , ".");
                    funcStr += CSSERVICESUB;                
                    boost::algorithm::replace_all(funcStr , "_SUB_FULLNAME_" , subFullName);                  
                    boost::algorithm::replace_all(funcStr , "_TYPE_NAME_" , typeName);
                    std::vector<TSServiceDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllServiceDefs();
                    for(size_t i = 0; i < TypeDefs.size() ;++i)
                    {
                        if(TSServiceDefPtr Services = TypeDefs[i])
                        {
                            TSString subPackage = TSBuiltinPlugin::GetSuitFullTypeName(Services,true);
                            boost::algorithm::replace_all(subPackage,"::",".");
                            if (subPackage == subFullName)
                            {
                                TSString folderPath = Services->TypeName + ".CallbackImpl";
                            }
                        }
                    }
                }   
            }
            if(!funcStr.empty())
            {
                boost::algorithm::replace_all(code ,"_CALLBACK_FUNS_" , funcStr);
            }
            TSString pubFuncStr;

            for (size_t i = 0 ; i < nodeFilePtr[j]->NodeData[k]->PubFullName.size() ; i++)
            {
                code = CSPUBSCRIBERCODE;
                TSString::size_type pos = nodeFilePtr[j]->NodeData[k]->PubFullName[i].find_last_of(":");   
                TSString typeName = nodeFilePtr[j]->NodeData[k]->PubFullName[i];

                if (pos != TSString::npos)
                {
                    typeName =  nodeFilePtr[j]->NodeData[k]->PubFullName[i].substr(pos + 1);
                }

                if (nodeFilePtr[j]->NodeData[k]->PubType[i] == "Topic")
                {
                    TSString topicStr;
                    for(size_t m = 0; m < TypeDefs.size() ;++m)
                    {
                        if(TSTopicTypeDefPtr decl = TS_CAST(TypeDefs[m],TSTopicTypeDefPtr))
                        {
                            if(decl->TypeName == typeName)
                            {     
                                std::vector<TSStructDefPtr> structDefs = _p->_BuiltinPlugin->GetAllStructDefs();
                                for (size_t k = 0; k < structDefs.size(); ++k)
                                {
                                    if (TSStructDefPtr structDef = structDefs[k])
                                    {
                                        TSString struTyname =  TSBuiltinPlugin::GetSuitFullTypeName(structDef,true);                                    
                                        if (struTyname == decl->DefTypeName)
                                        {
                                            pubFuncStr += CSTOPICPUB;
                                            TSString pubFullName = TSBuiltinPlugin::GetSuitFullTypeName(structDef,true) + TSIDL_2COLON + "DataType";
                                            boost::algorithm::replace_all(pubFullName , "::", ".");
                                            topicStr = TSIDLUtils::GetTopicName(decl);
                                            boost::algorithm::replace_all(pubFuncStr, "_TOPIC_NAME_", topicStr);
                                            boost::algorithm::replace_all(pubFuncStr, "_STRUCT_NAME_", pubFullName);
                                            break;
                                        }
                                    }
                                }
                                                                 
                            }
                            
                        }
                    }               

                }
                else if (nodeFilePtr[j]->NodeData[k]->PubType[i] == "Service")
                {
                    TSString pubFullName = nodeFilePtr[j]->NodeData[k]->PubFullName[i];
                    boost::algorithm::replace_all(pubFullName , "::" , ".");
                    pubFuncStr += CSSERVICEPUB;
                    boost::algorithm::replace_all(pubFuncStr , "_TYPE_NAME_" , typeName);
                    boost::algorithm::replace_all(pubFuncStr , "_PUB_FULLNAME_" , pubFullName);
                }
                else if (nodeFilePtr[j]->NodeData[k]->PubType[i] == "Message")
                {

                    pubFuncStr += CSMESSAGEPUB;
                    TSString pubFullName = nodeFilePtr[j]->NodeData[k]->PubFullName[i];
                    boost::algorithm::replace_all(pubFullName , "::" , ".");
                    boost::algorithm::replace_all(pubFuncStr , "_TYPE_NAME_" , typeName);
                    boost::algorithm::replace_all(pubFuncStr , "_PUB_FULLNAME_" , pubFullName);
                }
                else if (nodeFilePtr[j]->NodeData[k]->PubType[i] == "SDO" || nodeFilePtr[j]->NodeData[k]->PubType[i] == "LocalClass")
                {
                    pubFuncStr += CSSDOPUB;
                    TSString pubFullName = nodeFilePtr[j]->NodeData[k]->PubFullName[i];
                    boost::algorithm::replace_all(pubFullName , "::" , ".");
                    boost::algorithm::replace_all(pubFuncStr , "_TYPE_NAME_" , typeName);
                    boost::algorithm::replace_all(pubFuncStr , "_PUB_FULLNAME_" , pubFullName);
                }


            }
            if (!pubFuncStr.empty())
            {
                boost::algorithm::replace_all(code, "_CALL_CLASS_BACK_", pubFuncStr);
            }
            boost::algorithm::replace_all(code , "_NODE_" , nodeFilePtr[j]->LorisDefaultDomain);
            GeneratePubSubFile(nodeFilePtr[j]->Nodes[k], code);
        }

    }
#endif    
}

void TSSubscriberPlugin::GenerateJava( std::vector<TSNodeFilePtr> nodeFilePtr )
{
    TSString error;
   // TSJavaPluginPtr CallBackAndSubImplCoder = boost::make_shared<TSJavaPlugin>();
    std::vector<TSTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTypes();
    for (size_t j = 0 ; j < nodeFilePtr.size() ; j++)
    {   
        for (size_t k = 0 ; k < nodeFilePtr[j]->NodeData.size() ; k++)
        {
            TSString code = JAVASUBSCRIBERCODE;//code
            GeneratePubSubFile(nodeFilePtr[j]->Nodes[k], code);
        }
    }

}

void TSSubscriberPlugin::GeneratePubSubFile( const TSString & FileName,const TSString & Coder)
{
    TSProjFilePtr TopicFile;
 
    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();

    TSString BaseFile = GetBaseName(TSIDLPluginMgr::Instance()->GetCurParseFileFullPath());
    if(!BaseFile.empty())
    {
        if(TSSolutionPtr Solution = SolutionMgr()->GetOrCreateSolution(BaseFile))
        {
            TSProjectPtr Dir  = Solution->GetOrCreateProj(FileName);
            Dir->SetProFileType(Impl);
            if (Cp == "c#")
            {
                TopicFile = Dir->GetOrCreateProjFile(FileName,".cs");
                TopicFile->SetFileType(Impl);
            }
            else if (Cp == "c++")
            {
                TopicFile = Dir->GetOrCreateProjFile(FileName,".cpp");
                TopicFile->SetFileType(Impl);
            }
            else if (Cp == "java")
            {
                TopicFile = Dir->GetOrCreateProjFile(FileName,".java");
                TopicFile->SetFileType(Impl);
            }
            Dir->SetNodeFilePro(true);
            TopicFile->AppendLast(Coder);
        }
    }
}

bool TSSubscriberPlugin::LoadNodeFile( pugi::xml_document& doc , TSString NodeFile )
{
    bool isOpen = doc.load_file(NodeFile.c_str());
    if (!isOpen)
    {
        std::cout<<  NodeFile + GB18030ToTSString("加载失败!") << std::endl;
        
        return false;
    }
    return true;
}

TSString TSSubscriberPlugin::ParserNodeFile( pugi::xml_document& doc , std::vector<TSNodeFilePtr>& NodeFilePtr )
{
    pugi::xpath_node_set nodePubSub = doc.select_nodes("//NodePublishSubscribe");
    pugi::xpath_node_set::const_iterator it = nodePubSub.begin();
    TSString error;

    for(size_t i = 0 ; it != nodePubSub.end() ; it++ , i++)
    {
        NodeFilePtr.push_back(boost::make_shared<TSNodeFile>());
        NodeFilePtr[i]->LorisDefaultDomain = it->node().first_attribute().value();

        pugi::xml_object_range<pugi::xml_node_iterator> nodes = it->node().children();
        error += ParserPubSub(nodes, NodeFilePtr[i]);

        if (!error.empty())
        {
            return error;
        }
    }
    return "";
}

TSString TSSubscriberPlugin::ParserPubSub( pugi::xml_object_range<pugi::xml_node_iterator> nodes, 
    TSNodeFilePtr& NodeFilePtr )
{
    pugi::xml_node_iterator itPublish = nodes.begin();
    TSString error;

    for(size_t k = 0; itPublish != nodes.end(); itPublish++, k++)
    {
        TSString nodeName = itPublish->first_attribute().value();
        TSString applicationName = itPublish->attribute("applicationname").value();
        if (applicationName.empty() || nodeName.empty())
        {
            return "nodeName or appName is empty!";
        }

        TSNodePubSubDataPtr PubSubData = boost::make_shared<TSNodePubSubData>();

        pugi::xpath_node a = itPublish->child("Publish");
        pugi::xml_node interfaceObjectModel = a.node().first_child();
        error += ParserNode(interfaceObjectModel, PubSubData, "Publish");
        a = itPublish->child("Subscribe");
        interfaceObjectModel = a.node().first_child();
        error += ParserNode(interfaceObjectModel, PubSubData, "Subscribe");

        if (!error.empty())
        {
            return error;
        }

        if (!PubSubData->PubFileName.empty() || !PubSubData->SubFileName.empty())
        {
            NodeFilePtr->NodeData.push_back(PubSubData);
            NodeFilePtr->Nodes.push_back(nodeName);
            NodeFilePtr->ApplicationName.push_back(applicationName);
        }

    }
    return "";
}

TSString TSSubscriberPlugin::ParserNode( pugi::xml_node& interfaceObjectModel , TSNodePubSubDataPtr& PubSubData ,TSString Str )
{
    std::vector<TSTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTypes();
    for(interfaceObjectModel; interfaceObjectModel; interfaceObjectModel = interfaceObjectModel.next_sibling())
    {
        TSString fullName = interfaceObjectModel.first_attribute().value();
        TSString type = interfaceObjectModel.first_attribute().next_attribute().value();
        TSDataTypePtr datatype = boost::make_shared<TSDataType>();
        if (fullName.empty() || type.empty())
        {
            return "nodefile Publush fullName or type empty\n";
        }
        if (fullName.size() > 2)
        {
            if (fullName[0] == ':' && fullName[1] == ':')
            {
                fullName = fullName.substr(2);
            }
        }

        TSString::size_type pos = fullName.find_last_of(":");
        TSString typeName = fullName;
        if (pos != TSString::npos)
        {
            typeName = fullName.substr(pos+1);
        }

        bool isFind = false;

        if(type == "SDO" || type == "Message" || type == "LocalClass")
        {
            for(size_t i = 0; i < TypeDefs.size(); i++)
            {
                if (TSTypeDefPtr typePtr = TypeDefs[i])
                {
                    if (typePtr->TypeName == typeName)
                    {
                        if (Str == "Publish")
                        {
                            if(PubSubData->PubFullName.end() != find(PubSubData->PubFullName.begin(),PubSubData->PubFullName.end(),fullName))
                            {
                                std::cout << "nodefile " + Str + " " + fullName + " already exists\n" << std::endl;
                                continue;
                            }

                            TSString fileName = GetBaseName(typePtr->Parse.lock()->GetFullFileName());

                            PubSubData->PubFullName.push_back(fullName);
                            PubSubData->PubType.push_back(type);
                            PubSubData->PubFileName.push_back(fileName);

                            isFind = true;
                            break;
                        }
                        else
                        {	

                            if(PubSubData->SubFullName.end() != find(PubSubData->SubFullName.begin(),PubSubData->SubFullName.end(),fullName))
                            {
                               std::cout << "nodefile " + Str + " " + fullName + " already exists\n" << std::endl;
                               continue;
                            }
                            TSString fileName = GetBaseName(typePtr->Parse.lock()->GetFullFileName());

                            PubSubData->SubFullName.push_back(fullName);
                            PubSubData->SubType.push_back(type);
                            PubSubData->SubFileName.push_back(fileName);

                            isFind = true;
                            break;
                        }
                    }
                }	
            }
        }
        else if (type == "Topic")
        {
            for (size_t i = 0; i < TypeDefs.size() ; i++)
            {
                if (TSTypeDefPtr typePtr = TypeDefs[i])
                {
                    if (typePtr->Type != TSTypeDef::Topic)
                    {
                        continue;
                    }
                    if (typePtr->TypeName == typeName)
                    {
                        if (Str == "Publish")
                        {

                            if(PubSubData->PubFullName.end() != find(PubSubData->PubFullName.begin(),PubSubData->PubFullName.end(),fullName))
                            {
                                std::cout << "nodefile Publish " + fullName + " already exists\n" << std::endl;
                                continue;
                            }
                            TSString fileName = GetBaseName(typePtr->Parse.lock()->GetFullFileName());							
                            PubSubData->PubFullName.push_back(fullName);
                            PubSubData->PubType.push_back(type);
                            PubSubData->PubFileName.push_back(fileName);

                            isFind = true;
                            break;
                        }
                        else
                        {

                            if(PubSubData->SubFullName.end() != find(PubSubData->SubFullName.begin(),PubSubData->SubFullName.end(),fullName))
                            {
                               std::cout << "nodefile Sublish " + fullName + " already exists\n" << std::endl;
                               continue;
                            }
                            TSString fileName = GetBaseName(typePtr->Parse.lock()->GetFullFileName());	

                            PubSubData->SubFullName.push_back(fullName);
                            PubSubData->SubType.push_back(type);
                            PubSubData->SubFileName.push_back(fileName);

                            isFind = true;
                            break;
                        }
                    }
                }
            }
        }
        else if (type == "Service")
        {
            for(size_t i = 0 ; i < TypeDefs.size() ; i++)
            {
                if (TSTypeDefPtr typePtr = TypeDefs[i])
                {
                    if (typePtr->Type != TSTypeDef::Service)
                    {
                        continue;
                    }
                    if (typePtr->TypeName == typeName)
                    {
                        if (Str == "Publish")
                        {

                            if(PubSubData->PubFullName.end() != find(PubSubData->PubFullName.begin(),PubSubData->PubFullName.end(),fullName))
                            {
                                std::cout << "nodefile Publish " + fullName + " already exists\n" << std::endl;
                                continue;
                            }
                            TSString fileName = GetBaseName(typePtr->Parse.lock()->GetFullFileName());

                            PubSubData->PubFullName.push_back(fullName);
                            PubSubData->PubType.push_back(type);
                            PubSubData->PubFileName.push_back(fileName);

                            isFind = true;
                            break;
                        }
                        else
                        {
                            if(PubSubData->SubFullName.end() != find(PubSubData->SubFullName.begin(),PubSubData->SubFullName.end(),fullName))
                            {
                                std::cout << "nodefile Publish " + fullName + " already exists\n" << std::endl;
                                continue;
                            }
                            TSString fileName = GetBaseName(typePtr->Parse.lock()->GetFullFileName());

                            PubSubData->SubFullName.push_back(fullName);
                            PubSubData->SubType.push_back(type);
                            PubSubData->SubFileName.push_back(fileName);
                            isFind = true;
                            break;
                        }
                    }
                }	
            }
        }
        if (!isFind)
        {
            return "nodefile " + Str +" fullName or type error\n";
        }	
        pugi::xml_node dataNode = interfaceObjectModel.child("DataType");
        datatype->Times = dataNode.first_attribute().value();      
        pugi::xml_node memberNode = dataNode.child("Member");
        for (memberNode; memberNode; memberNode = memberNode.next_sibling())
        {
            TSDataMemberPtr meber = boost::make_shared<TSDataMember>();
            TSString dataName = memberNode.first_attribute().value();
            for(size_t i = 0 ; i < TypeDefs.size() ; i++)
            {
                if (TSTypeDefPtr typePtr = TypeDefs[i])
                {
                    TSString Typename = TSBuiltinPlugin::GetSuitFullTypeName(typePtr,true);
                    TSTypeDefPtr TypePtr = typePtr;
                    if (typePtr->Type == TSTypeDef::Topic)
                    {
                        Typename = TSIDLUtils::GetTopicName(typePtr);
                        TSTopicTypeDefPtr topicPtr = TS_CAST(typePtr,TSTopicTypeDefPtr);
                        TypePtr = topicPtr->Def.lock();
                    }
                    if (Typename == fullName)
                    {
                        if (TSTypeDefWithFiledPtr fieldType = TS_CAST(TypePtr,TSTypeDefWithFiledPtr))
                        {
                            BOOST_FOREACH(TSFieldDefPtr field,fieldType->Fileds)
                            {
                                if (field->ValName == dataName)
                                {
                                    TSString valueData = memberNode.first_attribute().next_attribute().value();
                                    meber->Name = dataName;
                                    meber->Value = valueData;
                                    datatype->Members.push_back(meber);
                                    break;
                                }
                                else
                                {
                                    if (fieldType->Fileds.back() == field)
                                    {
                                        TSString exp = GB18030ToTSString("未定义的属性名") + dataName;
                                        throw TSException(exp);
                                    }
                                }
                            }
                  
                        }
                    }
                }
            }
        } 
        PubSubData->DataMap[fullName] = datatype;
    }
    return "";
}




