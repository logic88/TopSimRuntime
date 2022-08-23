#include "stdafx.h"

#include <ctype.h>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <TSIDLXmlCoder.h>
#include "TSGenSupportPlugin.h"
#include "temp/topic.temp.h"
#include "temp/support.temp.h"
#include "temp/define.temp.h"
#include "temp/ArchiveFactory.temp.h"
#include "temp/CallBack.temp.h"
#include "temp/Convert.temp.h"
#include "temp/Interceptor.temp.h"
#include "temp/Interface.temp.h"
#include "temp/Method.temp.h"
#include "temp/Proxy.temp.h"
#include "temp/RemoteMehod.temp.h"
#include "temp/Service.temp.h"
#include "temp/Servant.temp.h"
#include "temp/State.temp.h"
#include "temp/SubscriptionFactory.temp.h"
#include "temp/Type.temp.h"
#include "temp/Updater.temp.h"
#include "temp/CallBack.temp.h"
#include "temp/python.temp.h"
#include "TSSolutionMgr.h"

#if defined(XSIM3_2) || defined(XSIM3_3)
#include <TopSimTypes/TSOrderTypeMgr.h>
#endif

using namespace TSIDLUtils;

BEGIN_METADATA(TSGenSupportPlugin)
    REG_BASE(TSIIDLPlugin);
END_METADATA()

struct TSGenSupportPluginPrivate
{
    TSIBuiltinPluginPtr _BuiltinPlugin;
};

TSGenSupportPlugin::TSGenSupportPlugin(void)
    :_p(new TSGenSupportPluginPrivate)
{
	_topicFlagStruct = new TSCPTopicFlag;
	_topicFlagStruct->buildTopic = false;
	_topicFlagStruct->buildSupport = false;
	_topicFlagStruct->buildDefine = false;
	_topicFlagStruct->buildPython = false;

	_serviceFlagStruct = new TSCPServiceFlag;
	_serviceFlagStruct->buildService = false;
	_serviceFlagStruct->buildSupport = false;
	_serviceFlagStruct->buildTopic = false;
	_serviceFlagStruct->buildDefine = false;
	_serviceFlagStruct->buildProxy = false;
	_serviceFlagStruct->buildMethod = false;
	_serviceFlagStruct->buildMethodImpl = false;
}

TSGenSupportPlugin::~TSGenSupportPlugin(void)
{
    delete _p;
	delete _topicFlagStruct;
	delete _serviceFlagStruct;
}

TSString TSGenSupportPlugin::GetName()
{
    return GB18030ToTSString("Support文件生成插件");

}

void TSGenSupportPlugin::OnPluginLoaded()
{
    _p->_BuiltinPlugin = GetPluginT<TSIBuiltinPlugin>();
    ASSERT(_p->_BuiltinPlugin && "TSGenSupportPlugin");
}

bool TSGenSupportPlugin::DoParse()
{
    return true;
}

bool TSGenSupportPlugin::DoBuild()
{

    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
    if (HasOption("GXml"))
    {
        TSIDLXmlCoder xml;
        xml.CreateXml();
        return true;
    }
    if(Cp != "c++" && Cp != "python" && Cp != "C++" && Cp != "PYTHON")
    {
        return true;
    }
    if(!(TSIDLPluginMgr::Instance()->GetTopicOrService()))
    {
        return true;
    }

    const TSIDLPluginMgr::ParseMap_T & parsemap = PluginMgr()->GetParserUtils();
    TSIDLPluginMgr::ParseMap_T::const_iterator iterU = parsemap.begin();
    for (; iterU != parsemap.end(); iterU++)
    {
        if(!TSIDLPluginMgr::Instance()->IsImportParse(iterU->second->Parse) || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            TSString base = GetBaseName(iterU->second->Parse->GetFullFileName());
            BuildExport(base);
        }

    }

	//对于自定义类型 typedef 填写在Topic.h文件头部语法错误，将其移动到文件末尾
    //BuildTypeDefine();

    BuildTopic();

	BuildSupport();

    BuildDefine();

    BuildService();

    BuildRemoteMethod(); 

    BuildProxy();

    BuildMethod();

    BuildMethodImpl();

	if (HasOption("Python"))
	{
		BuildPython();

		BuildPythonService();
	}
    return true;
}

void TSGenSupportPlugin::Clean()
{

}

void TSGenSupportPlugin::DeleteStringOfFile(TSProjFilePtr TopicHeaderFile, TSString& str)
{
	TSString tempFileContent = TopicHeaderFile->GetContent();
	int nCount = str.size();

	while (nCount)
	{
		int endifPos = tempFileContent.find_last_of(str);
		tempFileContent.erase(endifPos, 1);
		--nCount;
	}

	TopicHeaderFile->SetContent(tempFileContent);
}

std::vector<TSString> TSGenSupportPlugin::GetTopicInVec(std::vector<TSString>& tempTopicVec, std::vector<TSTopicTypeDefPtr>& TypeDefs)
{
	std::vector<TSString> indexVec;
	if (!tempTopicVec.empty())
	{
		//根据选中的主题查找TypeDefs中不被使用的主题
		for (int i = 0; i < TypeDefs.size(); ++i)
		{
			std::vector<TSString>::iterator iterTopic = std::find(tempTopicVec.begin(), tempTopicVec.end(), TypeDefs[i]->DefTypeName);
			if (iterTopic == tempTopicVec.end())
			{
				indexVec.push_back(TypeDefs[i]->DefTypeName);
			}
		}
	}
	return indexVec;
}

std::vector<TSString> TSGenSupportPlugin::GetTopicInVec(std::vector<TSString>& tempTopicVec, std::vector<TSTypeDefWithFuncAndFiledPtr>& TypeDefs)
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

//获得服务函数和依赖的结构体的Map
void TSGenSupportPlugin::GetServiceAndStructMap(std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs, std::map<TSString, TSString>& serviceMap)
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

//将和服务相关的结构体存入vector中
//serviceMap 服务和依赖结构体组成的Map, structVec服务依赖的结构体容器, indexServiceVec 记录服务的容器, vecSelectService 工具选中的服务, tempServiceVec 实际用来操作的服务容器
void TSGenSupportPlugin::StoreServiceAboutStruct(std::map<TSString, TSString>& serviceMap, std::vector<TSString>& serviceVec, std::vector<TSString>& indexServiceVec, std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec)
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

//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
bool TSGenSupportPlugin::GetGenerateTopic(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& tempTopicVec, std::vector<TSString>& topicVecStruct, bool topicFlag)
{
	bool emptyTopicFlag = false;
	
	if (!vecSelectTopic.empty())
	{
		//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
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

//过滤掉没有选中的节点以及可能重复生成的节点
bool TSGenSupportPlugin::FilterNotSelectNode(TSTypeDefPtr TypeDef, std::vector<TSString>& indexVec, std::vector<TSString>& structVec, std::vector<TSString>& storeStructRec)
{
	//过滤掉没有被选中的主题
	if (!indexVec.empty())
	{
		if (TypeDef->Type == TSTypeDef::Struct)
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
	}
	return false;
}

//获得要生成的服务，如果追加的服务为空则返回true
bool TSGenSupportPlugin::GetGenerateService(std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec, std::vector<TSString>& serviceVecStruct, bool serviceFlag)
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

std::vector<TSString> TSGenSupportPlugin::GetVectorDifference(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& topicVec)
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

void TSGenSupportPlugin::GenerateSupportHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName)
{
    TSString content = __DATA_TYPE_SUPPORT__;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSIDLUtils::ReplaceFullNameSpaceString("__NAMESPACE_BEGINS__",
        "__NAMESPACE_ENDS__",
        content,
        TypeDef,
        "",
        true);

    boost::algorithm::replace_all(content,"__SUPPROTTYPE__","TypeSupport");
    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);

    TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);  

    TSString supername, supperVersionCode, unSupperVersionCode;
    if (TypeDef->Extend.lock())
    {
        supername = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
		if (TSTypeDefWithFiledPtr verPtr = TS_CAST(TypeDef->Extend.lock(),TSTypeDefWithFiledPtr))
		{
			supperVersionCode += "#define PARENTTYPEVERSIONMACRO " + GenerateTypeVersionNum(verPtr) + "\n";
			supperVersionCode += "#define SUPERVERSIONMACRO PARENTTYPEVERSIONMACRO";
			unSupperVersionCode += "#undef SUPERVERSIONMACRO\n";
			unSupperVersionCode += "#undef PARENTTYPEVERSIONMACRO";
		}
    }
    else
    {
        if (TypeDef->Type == TSTypeDef::Service)
        {
            supername = "TSBasicService";
        }
        else if (TSTypeDef::Struct == TypeDef->Type)
        {
            supername = "";
        }
        else
        {
            supername = "TSBasicSdo";
        }

    }
	TSString VerSionCode = TSIDLUtils::GenerateTypeVersionNum(TypeDef);
	boost::algorithm::replace_all(content,"_SUPPER_VERSIONMACRO",supperVersionCode);
    boost::algorithm::replace_all(content,"_ATTRABUTE_NUMBER_",VerSionCode);
	boost::algorithm::replace_all(content,"_UNDEF_SUPPERVERSION_",unSupperVersionCode);
    boost::algorithm::replace_all(content,"_SUPER_NS",supername);
    boost::algorithm::replace_all(content,"_SELF_NS",selfns);
    boost::algorithm::replace_all(content,"_TOPIC_TYPE_","DataType");
    boost::algorithm::replace_all(content,"__DATATYPE__","DataType");
    boost::algorithm::replace_all(content,"__SUPPROTTYPE__","TypeSupport");

    ReplaceSerializeStatic(content,TypeDef,supername);

    if (TypeDef->Type == TSTypeDef::Service)
    {
        content = "";
        content += GenSupportWithName(TypeDef,"Request", serviceName);
        content += GenSupportWithName(TypeDef,"Response", serviceName);
    }

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateSupportCPP( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName)
{
    TSString content = __DATA_DEFINE_SUPPORT__ + TSIDL_NEWLINE;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSIDLUtils::ReplaceFullNameSpaceString("__NAMESPACE_BEGINS__",
        "__NAMESPACE_ENDS__",
        content,
        TypeDef,
        "",
        true);
    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);

    TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);  

    TSString supername = "TSBasicSdo";
	 TSString supperVersionCode,unSupperVersionCode;
    if (TypeDef->Type == TSTypeDef::Struct ||
        TypeDef->Type == TSTypeDef::Topic)
    {
        supername = "";
    }
    else if (TSTypeDef::Service == TypeDef->Type)
    {
        supername = "TSBasicService";
    }

    if (TypeDef->Extend.lock())
    {
        supername = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
		if (TSTypeDefWithFiledPtr verPtr = TS_CAST(TypeDef->Extend.lock(),TSTypeDefWithFiledPtr))
		{
			supperVersionCode += "#define PARENTTYPEVERSIONMACRO " + GenerateTypeVersionNum(verPtr) + "\n";
			supperVersionCode += "#define SUPERVERSIONMACRO PARENTTYPEVERSIONMACRO";
			unSupperVersionCode += "#undef SUPERVERSIONMACRO\n";
			unSupperVersionCode += "#undef PARENTTYPEVERSIONMACRO";
		}
    }

    boost::algorithm::replace_all(content,"_SUPER_NS",supername);
    boost::algorithm::replace_all(content,"_SELF_NS",selfns);

    TSString regfield;
    if (TypeDef->Type == TSTypeDef::Struct ||
        TypeDef->Type == TSTypeDef::Topic)
    {
        if(TypeDef->Extend.lock())
        {
            regfield = Table() + "REG_BASE(SUP_NAMESPACE::DataType);" + TSIDL_NEWLINE;
        }
		else
		{
			regfield = Table() + "REG_BASE(TSInterObject);" + TSIDL_NEWLINE;
		}
    }

    if (TypeDef->Type == TSTypeDef::Service)
    {
        regfield = Table() + "REG_BASE(SUP_NAMESPACE::DataType);" + TSIDL_NEWLINE;
    }

    for(size_t fieldId = 0; fieldId < TypeDef->Fileds.size(); ++fieldId)
    {
        if(TSFieldDefPtr field = TypeDef->Fileds[fieldId])
        {
            if (0 != fieldId)
            {
                regfield += TSIDL_NEWLINE;
            }

            if( field->Flag.empty())
            {
                if (!field->Desc.empty())
                {
                    regfield += TSIDL_REG_FIELD(field->ValName,field->Desc);
                }
                else
                {
                    regfield += TSIDL_REG_FIELD(field->ValName);
                }
            }
            else
            {
                TSString metatype = field->ValName;
                boost::algorithm::replace_all(metatype,"|","&TSMetaProperty::");
                boost::algorithm::replace_all(metatype,"&","|");
                if (!field->Desc.empty())
                {
                    regfield += TSIDL_REG_FIELD(field->ValName,field->Desc,metatype);
                }
                else
                {
                    regfield += TSIDL_REG_FIELD(field->ValName,field->ValName,metatype);
                }
            }
        }
    }
	TSString VerSionCode = TSIDLUtils::GenerateTypeVersionNum(TypeDef);
	boost::algorithm::replace_all(content,"_SUPPER_VERSIONMACRO",supperVersionCode);
	boost::algorithm::replace_all(content,"_ATTRABUTE_NUMBER_",VerSionCode);
	boost::algorithm::replace_all(content,"_UNDEF_SUPPERVERSION_",unSupperVersionCode);
    boost::algorithm::replace_all(content,"__DATATYPE__","DataType");
    boost::algorithm::replace_all(content,"__SUPPROTTYPE__","TypeSupport");
    boost::algorithm::replace_all(content,"_REG_FIELD",regfield);

    if (TypeDef->Type == TSTypeDef::Service)
    {
        content = "";
        content += GenSupportCPPWithName(TypeDef,"Request", serviceName);
        content += GenSupportCPPWithName(TypeDef,"Response", serviceName);
    }

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateDefineHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, TSTopicTypeDefPtr topic)
{
	bool FirstOrNot = false;
    TSString content = __DATA_DESC_DEFINE__;
    if (TypeDef)
    {
        if (TSTypeDef::Service == TypeDef->Type)
        {
                content = "";   
        }
    }

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSIDLUtils::ReplaceFullNameSpaceString("__NAMESPACE_BEGINS__",
        "__NAMESPACE_ENDS__",
        content,
        TypeDef,
        "",
        true);
    if (topic)
    {
        TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,topic);
    }
    else
    {
        TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);
    }

    TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);

    TSString selfname; 
    TSString descname;
    if (topic)
    {
        selfname = GetTopicName(topic);
        descname = topic->Desc;
    }
    else
    {
        selfname = GetTopicName(TypeDef);
        descname = selfname;
    }

    boost::replace_all(descname,"\"","");

    TSString superns;
    TSString supername = TSIDL_SDO_BASIC;
    if (TypeDef)
    {
        if (TypeDef->Type == TSTypeDef::Struct || 
            TypeDef->Type == TSTypeDef::Service)
        {
            supername = TSIDL_TOPIC_BASIC;
        }
    }

    if (topic)
    {
        if (topic->Extend.lock())
        {
            supername = GetTopicName(topic->Extend.lock());
            superns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);

        }
    }
    boost::algorithm::replace_all(content,"_SUPER_NAME",supername);
    boost::algorithm::replace_all(content,"_SUPER_NS",superns);
    boost::algorithm::replace_all(content,"_SELF_NS",selfns);
    boost::algorithm::replace_all(content,"_SELF_NAME",selfname);
    boost::algorithm::replace_all(content,"_SELF_DESCRIPTION",descname);
    boost::algorithm::replace_all(content,"__DATATYPE__","DataType");
    boost::algorithm::replace_all(content,"__SUPPROTTYPE__","TypeSupport");
    if (TypeDef)
    {
        if (TSTypeDef::Service == TypeDef->Type)
        {
            content += GenDefineWithName(TypeDef,"Request", serviceName, true);
            content += GenDefineWithName(TypeDef,"Response", serviceName, true);
        }
    }

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateDefineCPP( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, TSTopicTypeDefPtr topic)
{
	bool FistOrNot = false;
    TSString content = __DATA_DEFINE_DEFINE__;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSIDLUtils::ReplaceFullNameSpaceString("__NAMESPACE_BEGINS__",
        "__NAMESPACE_ENDS__",
        content,
        TypeDef,
        "",
        true);
    if (topic)
    {
        TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,topic);
    }
    else
    {
        TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);
    }

    TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);  
    TSString selfname;
    TSString descname;
    if(topic)
    {
        selfname = GetTopicName(topic);
        descname = topic->Desc;
    } 
    else
    {
        selfname = GetTopicName(TypeDef);
        descname = selfname;
    }

    boost::algorithm::replace_all(descname,"\"","");

    TSString superns;
    TSString supername = TSIDL_SDO_BASIC;
    if (TypeDef)
    {
        if (TypeDef->Type == TSTypeDef::Struct || 
            TypeDef->Type == TSTypeDef::Service)
        {
            supername = TSIDL_TOPIC_BASIC;

        }
    }

    if (topic)
    {
        if (topic->Extend.lock())
        {
            supername = GetTopicName(topic->Extend.lock());
            superns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
        }
    }


    boost::algorithm::replace_all(content,"_SUPER_NAME",supername);
    boost::algorithm::replace_all(content,"_SUPER_NS",superns);
    boost::algorithm::replace_all(content,"_SELF_NS",selfns);
    boost::algorithm::replace_all(content,"_SELF_NAME",selfname);
    boost::algorithm::replace_all(content,"_SELF_DESCRIPTION",descname);
    boost::algorithm::replace_all(content,"__DATATYPE__","DataType");
    boost::algorithm::replace_all(content,"__SUPPROTTYPE__","TypeSupport");

    if (TypeDef)
    {
        if (TSTypeDef::Service == TypeDef->Type)
        {
            content = "";
            content += GenDefineWithName(TypeDef,"Request", serviceName);
            content += GenDefineWithName(TypeDef,"Response", serviceName);
        }
    }
    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateTypeDefineHeader( TSProjFilePtr ProjFile,TSTypeDefineLinkPtr TypeDef )
{
	bool FirstOrNot = false;
   TSString content = __HEADER_TYPEDEFINE__;
   ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
       __NAMESPACE_END__,
       content,
       TypeDef);
   TSString typeDefStr;
   typeDefStr += Table() + "typedef " + TSBuiltinPlugin::GetSuitFullTypeName(TypeDef->Field->Def) + " " + TypeDef->TypeName + ";";
   boost::algorithm::replace_all(content, "_TYPE_DEFINE_", typeDefStr);
   ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::BuildTypeDefine()
{
    TSProjFilePtr TopicHeaderFile;
    TSString currentFile = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();

    TSString currentBaseFile = GetBaseName(currentFile);

    std::vector<TSTypeDefineLinkPtr> TypeDefineLinks = _p->_BuiltinPlugin->GetAllTypeDefineDefs();
    BOOST_FOREACH(TSTypeDefineLinkPtr TypeDef, TypeDefineLinks)
    {
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()) || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                Dir->SetProFileType(None);
                TopicHeaderFile = Dir->GetOrCreateProjFile("Topic",".h");
                GenerateTypeDefineHeader(TopicHeaderFile,TypeDef);
                TopicHeaderFile->SetFileType(None);
            }
        }
    }
}

void TSGenSupportPlugin::BuildTopic()
{
    TSProjFilePtr TopicHeaderFile;
    TSString currentFile = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    TSString currentBaseFile = GetBaseName(currentFile);
    std::vector<TSEnumDefPtr> EnumDefs = _p->_BuiltinPlugin->GetAllEnumDefs();
    BOOST_FOREACH(TSEnumDefPtr EnumDef, EnumDefs)
    {
        TSString ParseFileBaseName = 
            GetBaseName(EnumDef->Parse.lock()->GetFullFileName());
        if(!TSIDLPluginMgr::Instance()->IsImportParse(EnumDef->Parse.lock()) || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                Dir->SetProFileType(None);
                TopicHeaderFile = Dir->GetOrCreateProjFile("Topic",".h");
                TSIDLUtils::GenerateEnum(TopicHeaderFile,EnumDef);
                TopicHeaderFile->SetFileType(None);
            }
        }
    }

	std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	std::vector<TSString> tempServiceVec;
	//获得要生成的服务
	bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildTopicVec, _serviceFlagStruct->buildTopic);

	std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);
	
	//获取主题选择工具中选中的主题
	std::vector<TSString> vecSelectTopic = TSIDLPluginMgr::Instance()->GetSelectTopicVector();
	std::vector<TSString> tempTopicVec;
	//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
	bool emptyTopicFlag = GetGenerateTopic(vecSelectTopic, tempTopicVec, _topicVecStruct.buildTopicVec, _topicFlagStruct->buildTopic);
	
	std::vector<TSString> indexVec;
	if (!tempTopicVec.empty() && !vecSelectTopic.empty())
	{
		//用来存储没有被选中的主题
		indexVec = GetTopicInVec(tempTopicVec, TypeDefs);
	}
	
	bool deleteEndIfFlag = false;
	bool IsService = false;

	//先创建当前xidl文件的解决方案（解决当前xidl文件没有结构体和枚举等结构并且定义了依赖xidl文件中结构体的主题时不能正确生成的情况）
	TSString ParseFileBaseName = GetBaseName(currentBaseFile);
	if (TSSolutionPtr Solution = SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
	{
		TSProjectPtr Dir = Solution->GetOrCreateProj(ParseFileBaseName);
		Dir->SetProFileType(None);
		TopicHeaderFile = Dir->GetOrCreateProjFile("Topic", ".h");
		TopicHeaderFile->SetFileType(None);
	}

	BOOST_FOREACH(TSTypeDefPtr TypeDef, TypeDefs)
	{
		//过滤掉没有选中的节点以及可能重复生成的节点
		/*if (FilterNotSelectNode(TypeDef, indexVec, structVec, _storeStructRec.buildTopicRec))
		{
			continue;
		}*/
		//只生成一次该文件
		if (_topicFlagStruct->buildTopic)
		{
			break;
		}

		//存储生成的结构体
		_storeStructRec.buildTopicRec.push_back(TypeDef->TypeName);

		TSString ParseFileBaseName =
			GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
		if (TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
		{
			if (TSSolutionPtr Solution =
				SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
			{
				TSProjectPtr Dir = Solution->GetOrCreateProj(ParseFileBaseName);
				Dir->SetProFileType(None);
				TopicHeaderFile = Dir->GetOrCreateProjFile("Topic", ".h");
				if (TypeDef->Type == TSTypeDef::Struct ||
					TypeDef->Type == TSTypeDef::Service)
				{
					if (TypeDef->Type == TSTypeDef::Service)
					{
						IsService = true;
					}
					//从第二次追加主题开始，每次先去除原文件中 #endif // 字符串
					if (_topicFlagStruct->buildTopic && !deleteEndIfFlag)
					{
						TSString str = "#endif //";
						DeleteStringOfFile(TopicHeaderFile, str);
						deleteEndIfFlag = true;
					}
					
					TSTypeDefWithFuncAndFiledPtr typeDef = TS_CAST(TypeDef, TSTypeDefWithFuncAndFiledPtr);
					GenerateTopicHeader(TopicHeaderFile, typeDef, indexServiceVec);
				}
				TopicHeaderFile->SetFileType(None);
			}
		}
	}

    if (!TSIDLPluginMgr::Instance()->GetIsPlugin())
    {    
        std::vector<TSSolutionPtr> solutions = SolutionMgr()->GetSolutions(); 
        std::vector<TSSolutionPtr>::const_iterator its = solutions.begin();
        BOOST_FOREACH(TSSolutionPtr its,solutions)
        {
            TSString  solutionDir = its->GetSolutionName();
            const ProjMap_T &Projs = its->GetProjects();
            for (ProjMap_T::const_iterator iter = Projs.begin(); iter != Projs.end(); iter++)
            {
                const ProjFileMap_T files = iter->second->GetProjFiles();
                TSString baseDir = iter->second->GetShortName();
                for (ProjFileMap_T::const_iterator iterfile = files.begin(); iterfile != files.end(); iterfile++)
                {
                    TSString FileName = iterfile->first;
                    if (FileName == "Topic.h")
                    {
                        TSString content = __HEADER_INCLUDE_TOPIC__;
                        
                        TSIDLUtils::ReplaceExportHeader(__EXPORT_HEADER__,content,solutionDir);
                        TSIDLUtils::ReplaceLinkerVersion(content,"_LINKER_VERSION_");

                        const TSIDLPluginMgr::ParseMap_T & parsemap = PluginMgr()->GetParserUtils();
                        TSIDLPluginMgr::ParseMap_T::const_iterator iterU = parsemap.begin();
                    
                        for (; iterU != parsemap.end(); iterU++)
                        {
                            TSString baseFile = GetBaseName(iterU->second->Parse->GetFullFileName());
                             if (baseFile == baseDir)
                             {                        
                                 BOOST_FOREACH(TSString fielname,iterU->second->Parse->ImportFileNames)
                                 {
                                    if (HasOption(IncludePathKey) && HasOption(GImportKey))
                                    {
										/*TSString relativePath;
									   if(!iterU->first.empty())
										{
											TSString::size_type separatorPos1 = iterU->first.find_last_of("/");
											TSString::size_type separatorPos2 = iterU->first.find_last_of("\\");
											TSString::size_type separatorPos = 0;
											if(separatorPos1 != TSString::npos)
											{
												separatorPos = separatorPos1;
											}
											else if(separatorPos2 != TSString::npos)
											{
												if(separatorPos < separatorPos2)
												{
													separatorPos = separatorPos2;
												}
											}
											relativePath = iterU->first.substr(0,separatorPos);
										}*/
										bool     IsXidl   = TSIDLPluginMgr::Instance()->GetFileNameWithXidl();
										if(IsXidl)//是true则加后缀_xidl
										{
											if (TSIDLPluginMgr::Instance()->IsDefualtXidlFile(fielname))
											{
												content += "#include<" + fielname + "_xidl/Topic.h>\n";
											}
											else
											{
												content += "#include<" + fielname + "_xidl/cpp/Topic.h>\n";
											}
										}
										else
										{
											if (TSIDLPluginMgr::Instance()->IsDefualtXidlFile(fielname))
											{
												content += "#include<" + fielname + "/Topic.h>\n";
											}
											else
											{
												content += "#include<" + fielname + "/cpp/Topic.h>\n";
											}
										}
                                    }
                                    else
                                    {
                                        bool     IsXidl   = TSIDLPluginMgr::Instance()->GetFileNameWithXidl();

                                        if(IsXidl)//是true则加后缀_xidl
                                        {
                                            content += "#include<" + fielname + "_xidl/cpp/Topic.h>\n";
                                        }
                                        else
                                        {
                                           content += "#include<" + fielname + "/cpp/Topic.h>\n";
                                        }
                                       
                                    }
                                }
                             }
                        }
                        
						if (IsService)
						{
							TSString serviceHeader = "#include <TopSimRPC/TSBasicService/Support.h>";
							boost::algorithm::replace_all(content, "_SERVER_INFO_", serviceHeader);
						}
						else
						{
							boost::algorithm::replace_all(content, "_SERVER_INFO_", "");
						}
						//只定义一次头文件
						if (!_topicFlagStruct->buildTopic)
						{
							iterfile->second->AppendFirst(content);
							iterfile->second->AppendFirst(TSIDLUtils::GenHeadUUID("Topic.h"));
						}

						//typedef 添加到 Topic.h 文件末尾
						BuildTypeDefine();

                        iterfile->second->AppendLast("#endif //");
                    }
                }
            }
        }    
    }
	_topicFlagStruct->buildTopic = true;
	_serviceFlagStruct->buildTopic = true;
}

void TSGenSupportPlugin::BuildPython()
{
    TSProjFilePtr PythonCppFile;
	TSProjFilePtr PythonCppHFile;

    TSString currentFile = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();

    TSString currentBaseFile = GetBaseName(currentFile);
    
    std::vector<TSTopicTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTopicTypeDefs();

	//获取主题选择工具中选中的主题
	std::vector<TSString> vecSelectTopic = TSIDLPluginMgr::Instance()->GetSelectTopicVector();
	std::vector<TSString> tempTopicVec;
	//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
	bool emptyTopicFlag = GetGenerateTopic(vecSelectTopic, tempTopicVec, _topicVecStruct.buildPythonVec, _topicFlagStruct->buildPython);

	std::vector<TSString> indexVec;
	if (!tempTopicVec.empty() && !vecSelectTopic.empty())
	{
		//用来存储没有被选中的主题
		indexVec = GetTopicInVec(tempTopicVec, TypeDefs);
	}

	//用来判断ParseFileBaseName变量是否更替了xidl
	TSString PreParseFileBaseName = "N";
    TSString PythonCon;
    TSString RegisterCon;
    TSString pythondodule;

	//构建python module 代码
    pythondodule = __PYTHON_MODULE__;
    BOOST_FOREACH(TSTopicTypeDefPtr TypeDef,TypeDefs)
    {
		//过滤掉没有选中的主题
		if (!indexVec.empty())
		{
			if (TypeDef->Type == TSTypeDef::Topic)
			{
				//判断当前主题是否是过滤掉的主题，如果是则跳过当前循环
				std::vector<TSString>::iterator iterTopic = std::find(indexVec.begin(), indexVec.end(), TypeDef->DefTypeName);
				if (iterTopic != indexVec.end())
				{
					continue;
				}
			}
		}
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                Dir->SetProFileType(None);
                PythonCppFile = Dir->GetOrCreateProjFile("PythonModule",".cpp");
				PythonCppHFile = Dir->GetOrCreateProjFile("PythonModule", ".h");

				TSString currentBaseFile = GetBaseName(currentFile);

                if (TypeDef->Type == TSTypeDef::Topic)
                {
                    TSTypeDefWithFuncAndFiledPtr typeDef = TS_CAST(TypeDef->Def.lock(),TSTypeDefWithFuncAndFiledPtr);
                    if (typeDef)
                    {
                        RegisterCon += GeneratePython(PythonCppFile, PythonCppHFile,typeDef,TypeDef);
						//只向PythonModule.h中写入一次需要包含的头文件
						if (PreParseFileBaseName != ParseFileBaseName)
						{
							//PythonModule.h 引入的头文件
							TSString pyinclude = __PYTHON_INCLUDE__;
							if (PythonCppHFile != NULL && !_topicFlagStruct->buildPython)
							{
								PythonCppHFile->AppendFirst(pyinclude);
							}

							PreParseFileBaseName = ParseFileBaseName;
						}
                    }
                    else
                    {
                        TSString Exp = TypeDef->DefTypeName + GB18030ToTSString("未找到对应的结构体类型");
                        std::cout << Exp << std::endl;
                        throw(Exp);
                    }
                }
               
                PythonCppFile->SetFileType(None);
				PythonCppHFile->SetFileType(None);
            }   
        } 

		//使用currentBaseFile替换ParseFileBaseName
		boost::algorithm::replace_all(pythondodule,"_XIDL_NAME", currentBaseFile);
    }

    TSString FileName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    TSString currentFileName = GetBaseName(FileName);

    TSString EnumCon;
    std::vector<TSEnumDefPtr> EnumDefs = _p->_BuiltinPlugin->GetAllEnumDefs();

    BOOST_FOREACH(TSEnumDefPtr EnumDef, EnumDefs)
    {
        if (EnumDef->Type != TSTypeDef::Enum)
        {
            continue;
        }

        TSString ParseFileBaseName = 
            GetBaseName(EnumDef->Parse.lock()->GetFullFileName());

        if(!TSIDLPluginMgr::Instance()->IsImportParse(EnumDef->Parse.lock()) || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                
                TSString full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(EnumDef,true);
                std::vector<TSString> Names = TSMetaType4Link::Split(full_type_name,PACKAGE_SPLIT,false);
                TSString PackName;
                TSString PackNameAS;
                for (unsigned int i = 0; i < Names.size(); i++)
                {
                    TSString item = Names[i];
                    PackName += item;
                    PackNameAS += item;
					if (i < Names.size())
                    {
                        PackNameAS += '_';
                    }

                    PackName += "::";
                }
                
                EnumCon += "enum_<" + PackName + EnumDef->TypeName + ">(\"" + PackNameAS + EnumDef->TypeName + "\")\n";
                for(size_t i = 0; i < EnumDef->Fileds.size(); ++i)
                {
                    if(TSEnumFiledDefPtr enumVal = EnumDef->Fileds[i])
                    {
                        EnumCon += TSIDL_2TABLE + ".value(\"_VALUEPY_NAME\",_VALUE_NAME)\n";

						//对主题XIDL中枚举类型中最后的变量进行字符处理
						if (i == EnumDef->Fileds.size() - 1)
						{
							//检查最后一个枚举变量中是否存在换行符
							while(isspace(enumVal->Name[(enumVal->Name).length() - 1]))
							{
								//如果主题枚举字符串最后的字符是换行符则删除
								enumVal->Name.pop_back();
							}
						}

						boost::algorithm::replace_all(EnumCon, "_VALUEPY_NAME", PackNameAS + enumVal->Name);
						boost::algorithm::replace_all(EnumCon, "_VALUE_NAME", PackName + enumVal->Name);
                    }
                }
				
                EnumCon += TSIDL_2TABLE + ".export_values();\n";
				EnumCon += "\n";
            }
        }
    }

    boost::algorithm::replace_all(pythondodule,"_ENUM_PROPERTY",EnumCon);
    boost::algorithm::replace_all(pythondodule,"_REGISTER_PROPERTY",RegisterCon);

    if( PythonCppFile != NULL)
    {
        PythonCppFile->AppendLast(pythondodule);
    }
    
	std::vector<TSString> structVec;
	for (int i = 0; i < indexVec.size(); ++i)
	{
		TSString tempStr = indexVec[i];
		int pos = tempStr.rfind(":");
		tempStr = tempStr.substr(pos + 1);
		structVec.push_back(tempStr);
	}

    TSString pythonclass;
    std::vector<TSTypeDefWithFuncAndFiledPtr>  StructDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();
    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef, StructDefs)
    {
		std::cout << "BuildPython: " << TypeDef->TypeName << " 类型：" << TypeDef->Type << std::endl;
        if (TypeDef->Type != TSTypeDef::Struct)
        {
            continue;
        }

		//过滤掉没有选中的主题结构体
		if (!structVec.empty())
		{
			//判断当前主题是否是过滤掉的主题，如果是则跳过当前循环
			std::vector<TSString>::iterator iterTopic = std::find(structVec.begin(), structVec.end(), TypeDef->TypeName);
			if (iterTopic != structVec.end())
			{
				continue;
			}
		}

        pythonclass += __PYTHON_CLASS__;

        boost::algorithm::replace_all(pythonclass,"_SELF_NAME",TypeDef->TypeName);
        
        TSString propty;

        BOOST_FOREACH(TSFieldDefPtr field,TypeDef->Fileds)
        {
            if(field!=NULL)
            {
                propty += __PYTHON_BASIC_DATA__;
                
                boost::algorithm::replace_all(propty,"_SELF_NAME",TypeDef->TypeName);
                boost::algorithm::replace_all(propty,"_SELF_ATTR",field->ValName);
            }    
        }

        TSString supername = "";
        if (TypeDef->Extend.lock())
        {
            supername += ",bases<";
            supername += TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
            supername += "::DataType> ";
        }
        TSString full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
        std::vector<TSString> Names = TSMetaType4Link::Split(full_type_name,PACKAGE_SPLIT,false);
        TSString PackName;
        TSString PackNameAS;

        for (unsigned int i = 0; i < Names.size(); i++)
        {
            TSString item = Names[i];
            PackName += item;
            PackNameAS += item;
            if (i < Names.size()-1)
            {
                PackNameAS += '_';
            }

            PackName += "::";
        }

        boost::algorithm::replace_all(pythonclass,"_SUPER_NS",supername);
        boost::algorithm::replace_all(pythonclass,"_PROPERTY_NAME",propty);
        boost::algorithm::replace_all(pythonclass,"_SELF_NS",PackName);
        boost::algorithm::replace_all(pythonclass,"_SELF_CLASS",PackNameAS);
        
    }

    if( PythonCppFile != NULL)
    {
        PythonCppFile->AppendLast(pythonclass);
        PythonCppFile->AppendLast("}");
    }


    if (!TSIDLPluginMgr::Instance()->GetIsPlugin())
    {    
        std::vector<TSSolutionPtr> solutions = SolutionMgr()->GetSolutions();

        std::vector<TSSolutionPtr>::const_iterator its = solutions.begin();
        BOOST_FOREACH(TSSolutionPtr its,solutions)
        {
            TSString  solutionDir = its->GetSolutionName();

            const ProjMap_T &Projs = its->GetProjects();
            for (ProjMap_T::const_iterator iter = Projs.begin(); iter != Projs.end(); iter++)
            {
                const ProjFileMap_T files = iter->second->GetProjFiles();

                for (ProjFileMap_T::const_iterator iterfile = files.begin(); iterfile != files.end(); iterfile++)
                {
                    TSString FileName = iterfile->first;

                    if (FileName == "PythonModule.cpp")
                    {
						TSString FileFullPath;

						//获取LINKSDK根目录
						FileFullPath = GetAbsoluteDir(currentFile);
						//拼接成根目录下的*.xidl文件
						FileFullPath += "/" + solutionDir + ".xidl";
						//以文件流的形式读取文件
						std::ifstream ifs(FileFullPath.c_str(), std::ifstream::in | std::ifstream::binary);

						std::ostringstream oss;
						//将文件流内容输出到输出流对象中
						oss << ifs.rdbuf();

						//xidl文件内容
						TSString xidlcontent = Utf8ToAscii(oss.str()).c_str();
						//xidl文件中 import关键字开始的位置，posendimport的结束位置即"\n"位置
						TSString::size_type posstart, posend;

						int pos = 0;

						//存储当前工程依赖的其他xidl
						std::vector<TSString> vecxidl;

						//对xidl文件中依赖的其它xidl进行提取
						while ((posstart = xidlcontent.find("import", pos)) != TSString::npos)
						{
							posend = xidlcontent.find("\n", pos + 1);
							TSString::size_type arrowstart, pospoint;
							arrowstart = xidlcontent.find("<", pos);
							pospoint = xidlcontent.find(".", pos);
							TSString strxidl = xidlcontent.substr(arrowstart + 1, pospoint - arrowstart - 1);
							vecxidl.push_back(strxidl);
							pos = posend;
						}
						//存储xidl的头文件的字符串
						TSString strInclude;
						TSString content;
						//将当前工程依赖的xidl文件拼接成头文件
						for (auto it = vecxidl.begin(); it != vecxidl.end(); ++it)
						{
							TSString temp = "#include <" + *it + "_xidl/cpp/PythonModule.h>\n\n";
							strInclude += temp;
						}

						if (!_topicFlagStruct->buildPython)
						{
							content = __PYTHON_INCLUDE__;
							content += "\n" + strInclude;
						}

                        TSIDLUtils::ReplaceExportHeader(__EXPORT_HEADER__,content,solutionDir);

                        content += PythonCon;
                        iterfile->second->AppendFirst(content);
                    }
                }
            }
        }
    }

    if( PythonCppFile != NULL)
    {
		if (!_topicFlagStruct->buildPython)
		{
			PythonCppFile->AppendFirst("#ifdef _PYTHON \n");
		}
        PythonCppFile->AppendLast("\n#endif //_PYTHON");
    }
	_topicFlagStruct->buildPython = true;
}

//生成Support文件
void TSGenSupportPlugin::BuildSupport()
{
    TSProjFilePtr SupportHeaderFile;
    TSProjFilePtr SupportCppFile;
    TSString currentFile = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    TSString currentBaseFile = GetBaseName(currentFile);
	 bool    IsXidl   = TSIDLPluginMgr::Instance()->GetFileNameWithXidl();
    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildSupportVec, _serviceFlagStruct->buildSupport);

	std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

	//主题选择工具中选中的主题
	std::vector<TSString> vecSelectTopic = TSIDLPluginMgr::Instance()->GetSelectTopicVector();
	std::vector<TSString> tempTopicVec;
	//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
	bool emptyTopicFlag = GetGenerateTopic(vecSelectTopic, tempTopicVec, _topicVecStruct.buildSupportVec, _topicFlagStruct->buildSupport);;
	
	std::vector<TSString> indexVec;
	if (!tempTopicVec.empty() && !vecSelectTopic.empty())
	{
		//用来存储没有被选中的主题
		indexVec = GetTopicInVec(tempTopicVec, TypeDefs);
	}
	
	bool deleteEndIfFlag = false;
    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
		//只生成一次该文件
		/*if (_topicFlagStruct->buildSupport)
		{
			break;
		}*/
		//过滤掉没有选中的节点以及可能重复生成的节点
		/*if (FilterNotSelectNode(TypeDef, indexVec, structVec, _storeStructRec.buildSupportRec))
		{
			continue;
		}*/
		//存储生成的结构体
		//_storeStructRec.buildSupportRec.push_back(TypeDef->TypeName);

        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir = Solution->GetOrCreateProj(ParseFileBaseName);
                SupportHeaderFile = Dir->GetOrCreateProjFile("Support",".h");

				//从第二次追加主题开始，每次先去除原文件中 #endif // 字符串
				/*if (_topicFlagStruct->buildSupport && !deleteEndIfFlag)
				{
					TSString str = "#endif //";
					DeleteStringOfFile(SupportHeaderFile, str);
					deleteEndIfFlag = true;
				}*/

                SupportCppFile = Dir->GetOrCreateProjFile("Support",".cpp");
                if (TypeDef->Type == TSTypeDef::Struct || TypeDef->Type == TSTypeDef::Service)
                {   

                    TSTypeDefWithFuncAndFiledPtr typeDef = TS_CAST(TypeDef,TSTypeDefWithFuncAndFiledPtr);
                    GenerateSupportHeader(SupportHeaderFile,typeDef, indexServiceVec);
                    GenerateSupportCPP(SupportCppFile, typeDef, indexServiceVec);
                }
                SupportHeaderFile->SetFileType(None);
                SupportCppFile->SetFileType(None);
            }        
        }               
    }

    if (!TSIDLPluginMgr::Instance()->GetIsPlugin())
    {
        std::vector<TSSolutionPtr> solutions = SolutionMgr()->GetSolutions(); 
        std::vector<TSSolutionPtr>::const_iterator its = solutions.begin();
        BOOST_FOREACH(TSSolutionPtr its,solutions)
        {
            TSString  solutionDir = its->GetSolutionName();
            const ProjMap_T &Projs = its->GetProjects();
            for (ProjMap_T::const_iterator iter = Projs.begin(); iter != Projs.end(); iter++)
            {
                TSString baseDir = iter->second->GetShortName();
                const ProjFileMap_T files = iter->second->GetProjFiles();
                for (ProjFileMap_T::const_iterator iterfile = files.begin(); iterfile != files.end(); iterfile++)
                {
                    TSString FileName = iterfile->first;
                    if (FileName == "Support.h")
                    {
                        TSString content = __HEADER_INCLUDE_SUPPORT__;
						TSString topicPath;
						if (IsXidl)
						{
							topicPath = solutionDir + "_xidl/cpp/Topic.h";
						}
						else
						{
							topicPath = solutionDir + "/cpp/Topic.h";
						}
                        boost::algorithm::replace_all(content,"_TOPIC_HEAD_",topicPath);
                        TSIDLUtils::ReplaceExportHeader(__EXPORT_HEADER__,content,solutionDir);
                        const TSIDLPluginMgr::ParseMap_T & parsemap = PluginMgr()->GetParserUtils();
                        TSIDLPluginMgr::ParseMap_T::const_iterator iterU = parsemap.begin();
                        for (; iterU != parsemap.end(); iterU++)
                        {
                            TSString baseFile = GetBaseName(iterU->second->Parse->GetFullFileName());
                            if (baseFile == baseDir)
                            {                        
                                BOOST_FOREACH(TSString fielname,iterU->second->Parse->ImportFileNames)
                                {
									if (HasOption(IncludePathKey) && HasOption(GImportKey))
									{
										 /*TSString relativePath;
										 if(!iterU->first.empty())
										 {
											 TSString::size_type separatorPos1 = iterU->first.find_last_of("/");
											 TSString::size_type separatorPos2 = iterU->first.find_last_of("\\");
											 TSString::size_type separatorPos = 0;
											 if(separatorPos1 != TSString::npos)
											 {
												 separatorPos = separatorPos1;
											 }
											 else if(separatorPos2 != TSString::npos)
											 {
												 if(separatorPos < separatorPos2)
												 {
													 separatorPos = separatorPos2;
												 }
											 }
											 relativePath = iterU->first.substr(0,separatorPos);
										 }*/
										bool     IsXidl   = TSIDLPluginMgr::Instance()->GetFileNameWithXidl();
										if(IsXidl)//是true则加后缀_xidl
										{
											if (TSIDLPluginMgr::Instance()->IsDefualtXidlFile(fielname))
											{
												content += "#include <" + fielname + "_xidl/Support.h>\n";
											}
											else
											{
												content += "#include <" + fielname + "_xidl/cpp/Support.h>\n";
											}
										}
										else
										{
											if (TSIDLPluginMgr::Instance()->IsDefualtXidlFile(fielname))
											{
												content += "#include <" + fielname + "/Support.h>\n";
											}
											else
											{
												content += "#include <" + fielname + "/cpp/Support.h>\n";
											}
										}
                                        
                                    }
                                    else
                                    {
                                        bool     IsXidl   = TSIDLPluginMgr::Instance()->GetFileNameWithXidl();

                                        if(IsXidl)//是true则加后缀_xidl
                                        {
                                            content += "#include<" + fielname + "_xidl/cpp/Support.h" + ">\n";
                                        }
                                        else
                                        {
                                            content += "#include<" + fielname + "/cpp/Support.h" + ">\n";
                                        }
                                       
                                    }

                                }
                            }
                        }
						//只定义一次头文件
						if (!_topicFlagStruct->buildSupport)
						{
							iterfile->second->AppendFirst(content);
							iterfile->second->AppendFirst(TSIDLUtils::GenHeadUUID("Support.h"));
						}
                        
                        iterfile->second->AppendLast("#endif //");
                    }
                    if (FileName == "Support.cpp")
                    {
                        iterfile->second->AppendFirst(CPP_INCLUDE_SUPPORT__);
                    }
                }
            }
        }    
    }
	/*_topicFlagStruct->buildSupport = true;
	_serviceFlagStruct->buildSupport = true;*/
}

//生成Define文件
void TSGenSupportPlugin::BuildDefine()
{
    TSProjFilePtr DefinedHeaderFile;
    TSProjFilePtr DefinedCppFile;
    TSString currentFile = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    TSString currentBaseFile = GetBaseName(currentFile);

    std::vector<TSTopicTypeDefPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTopicTypeDefs();
	std::vector<TSServiceDefPtr> TypeDefServices = _p->_BuiltinPlugin->GetAllServiceDefs();

	//服务依赖的结构体
	//std::map<TSString, TSString> serviceMap;
	/*TSString tempName;
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
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildDefineVec, _serviceFlagStruct->buildDefine);

	std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

	//接收选中的主题
	std::vector<TSString> vecSelectTopic = TSIDLPluginMgr::Instance()->GetSelectTopicVector();
	std::vector<TSString> tempTopicVec;
	//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
	//bool emptyTopicFlag = GetGenerateTopic(vecSelectTopic, tempTopicVec, _topicVecStruct.buildDefineVec, _topicFlagStruct->buildDefine);
	
	std::vector<TSString> indexVec;
	if (!tempTopicVec.empty() && !vecSelectTopic.empty())
	{
		//用来存储没有被选中的主题
		indexVec = GetTopicInVec(tempTopicVec, TypeDefs);
	}

    std::vector<TSStructDefPtr> StructTypeDefs = _p->_BuiltinPlugin->GetAllStructDefs();
    std::vector<TSTypeDefWithFunctionPtr> FuncTypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFunctions();
    std::vector<TSString> supportFileName;
    bool IsService = false;

    BOOST_FOREACH(TSStructDefPtr TypeDef,StructTypeDefs)
    {
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

                DefinedHeaderFile = Dir->GetOrCreateProjFile("Defined",".h");

                DefinedHeaderFile->SetFileType(None);                                     
            }     
        }    
    }

    BOOST_FOREACH(TSTypeDefWithFunctionPtr TypeDef,FuncTypeDefs)
    {
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                supportFileName.push_back(ParseFileBaseName);
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                DefinedHeaderFile = Dir->GetOrCreateProjFile("Defined",".h");
                DefinedHeaderFile->SetFileType(None);                                     
            }     
        }    
    }

	bool deleteEndIfFlag = false;
    BOOST_FOREACH(TSTopicTypeDefPtr TypeDef,TypeDefs)
    {
		//过滤掉没有选中的主题
		//if (!indexVec.empty())
		//{
		//	if (TypeDef->Type == TSTypeDef::Topic)
		//	{
		//		//如果当前结构体生成过，则不再重复生成
		//		std::vector<TSString>::iterator iterCurrentStruct = std::find(_storeStructRec.buildDefineRec.begin(), _storeStructRec.buildDefineRec.end(), TypeDef->TypeName);
		//		if (iterCurrentStruct != _storeStructRec.buildDefineRec.end())
		//		{
		//			continue;
		//		}
		//		//判断当前主题是否是过滤掉的主题，如果是则跳过当前循环
		//		std::vector<TSString>::iterator iterTopic = std::find(indexVec.begin(), indexVec.end(), TypeDef->DefTypeName);
		//		if (iterTopic != indexVec.end())
		//		{
		//			if (!structVec.empty())
		//			{
		//				TSString tempStr = TypeDef->DefTypeName;
		//				int pos = tempStr.rfind(":");
		//				tempStr = tempStr.substr(pos+1);
		//				std::vector<TSString>::iterator iterService = std::find(structVec.begin(), structVec.end(), tempStr);
		//				if (iterService == structVec.end())
		//				{
		//					continue;
		//				}
		//			}
		//			else
		//			{
		//				continue;
		//			}
		//		}
		//	}
		//}

		//存储生成的结构体
		_storeStructRec.buildDefineRec.push_back(TypeDef->TypeName);

        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                DefinedHeaderFile = Dir->GetOrCreateProjFile("Defined",".h");
				//从第二次追加主题开始，每次先去除原文件中 #endif // 字符串
				/*if (_topicFlagStruct->buildDefine && !deleteEndIfFlag)
				{
					TSString str = "#endif //";
					DeleteStringOfFile(DefinedHeaderFile, str);
					deleteEndIfFlag = true;
				}*/

                DefinedCppFile = Dir->GetOrCreateProjFile("Defined",".cpp");
                if (TypeDef->Type == TSTypeDef::Topic)
                {
                    TSTopicTypeDefPtr TypeDeftopic = TS_CAST(TypeDef,TSTopicTypeDefPtr);
                    TSTypeDefWithFuncAndFiledPtr typeDef = TS_CAST(TypeDeftopic->Def.lock(),TSTypeDefWithFuncAndFiledPtr);
                    if (typeDef)
                    {
                        GenerateDefineHeader(DefinedHeaderFile, typeDef, indexServiceVec,TypeDeftopic);
                        GenerateDefineCPP(DefinedCppFile, typeDef, indexServiceVec,TypeDeftopic);
                    }
                    else
                    {
                        TSString Exp = TypeDeftopic->DefTypeName + GB18030ToTSString("未找到对应的结构体类型");
                        std::cout << Exp << std::endl;
                        throw(Exp);
                    }
                }

                DefinedHeaderFile->SetFileType(None);                                     
                DefinedCppFile->SetFileType(None);
            }     
        }    
    }

    BOOST_FOREACH(TSServiceDefPtr TypeDef,TypeDefServices)
    {
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

                DefinedHeaderFile = Dir->GetOrCreateProjFile("Defined",".h");
                DefinedCppFile = Dir->GetOrCreateProjFile("Defined",".cpp");

				//从第二次追加服务开始，每次先去除原文件中 #endif // 字符串
				/*if (_topicFlagStruct->buildDefine && !deleteEndIfFlag)
				{
				TSString str = "#endif //";
				DeleteStringOfFile(DefinedHeaderFile, str);
				deleteEndIfFlag = true;
				}*/

                if (TypeDef->Type == TSTypeDef::Service)
                {
                    TSTypeDefWithFuncAndFiledPtr typeDef = TS_CAST(TypeDef,TSTypeDefWithFuncAndFiledPtr);
                    GenerateDefineHeader(DefinedHeaderFile, typeDef, indexServiceVec);
                    GenerateDefineCPP(DefinedCppFile, typeDef, indexServiceVec);
                    IsService = true;
                }

                DefinedHeaderFile->SetFileType(None);                                     
                DefinedCppFile->SetFileType(None);
            }     
        }    
    }
  
    if (!TSIDLPluginMgr::Instance()->GetIsPlugin())
    {
        std::vector<TSSolutionPtr> solutions = SolutionMgr()->GetSolutions(); 
        std::vector<TSSolutionPtr>::const_iterator its = solutions.begin();
        BOOST_FOREACH(TSSolutionPtr its,solutions)
        {
            TSString  solutionDir = its->GetSolutionName();
            const ProjMap_T &Projs = its->GetProjects();
            for (ProjMap_T::const_iterator iter = Projs.begin(); iter != Projs.end(); iter++)
            {
                TSString baseDir = iter->second->GetShortName();
                const ProjFileMap_T files = iter->second->GetProjFiles();
                for (ProjFileMap_T::const_iterator iterfile = files.begin(); iterfile != files.end(); iterfile++)
                {
                    TSString FileName = iterfile->first;
                    if (FileName == "Defined.h")
                    {
                        bool    IsXidl   = TSIDLPluginMgr::Instance()->GetFileNameWithXidl();
                        TSString content = __HEADER_INCLUDE_DEFINE__;
                        TSString SupportPath = "";
                        bool IsHaveSupportFile = true;
                        for(int i = 0; i < supportFileName.size(); i++)
                        {
                            if(solutionDir == supportFileName[i])
                            {
                                IsHaveSupportFile = false;
                            }
                        }

                        if( IsXidl )
                        {
                            if( IsHaveSupportFile)
                            {
                                SupportPath = solutionDir + "_xidl/cpp/Export.h";
                            }
                            else
                            {
                                SupportPath = solutionDir + "_xidl/cpp/Support.h";
                            }
                        }
                        else
                        {
                            if( IsHaveSupportFile)
                            {
                                SupportPath = solutionDir + "/cpp/Export.h";
                            }
                            else
                            {
                                SupportPath = solutionDir + "/cpp/Support.h";
                            }
                        }

                        boost::algorithm::replace_all(content,"_SUPPORT_HEAD_",SupportPath);
                        TSIDLUtils::ReplaceExportHeader(__EXPORT_HEADER__,content,solutionDir);
                        const TSIDLPluginMgr::ParseMap_T & parsemap = PluginMgr()->GetParserUtils();
                        TSIDLPluginMgr::ParseMap_T::const_iterator iterU = parsemap.begin();
                        for (; iterU != parsemap.end(); iterU++)
                        {
                            TSString baseFile = GetBaseName(iterU->second->Parse->GetFullFileName());
                            if (baseFile == baseDir)
                            {                        
                                BOOST_FOREACH(TSString fielname,iterU->second->Parse->ImportFileNames)
                                {
                                    if (HasOption(IncludePathKey) && HasOption(GImportKey))
                                    {
										/*TSString relativePath;
										if(!iterU->first.empty())
										{
											TSString::size_type separatorPos1 = iterU->first.find_last_of("/");
											TSString::size_type separatorPos2 = iterU->first.find_last_of("\\");
											TSString::size_type separatorPos = 0;
											if(separatorPos1 != TSString::npos)
											{
												separatorPos = separatorPos1;
											}
											else if(separatorPos2 != TSString::npos)
											{
												if(separatorPos < separatorPos2)
												{
													separatorPos = separatorPos2;
												}
											}
											relativePath = iterU->first.substr(0,separatorPos);
										}*/

										if(IsXidl)//是true则加后缀_xidl
										{
											if (TSIDLPluginMgr::Instance()->IsDefualtXidlFile(fielname))
											{
												content += "#include<" + fielname + "_xidl/Defined.h>\n";
											}
											else
											{
												content += "#include<" + fielname + "_xidl/cpp/Defined.h>\n";
											}
										}
										else
										{
											if (TSIDLPluginMgr::Instance()->IsDefualtXidlFile(fielname))
											{
												content += "#include<" + fielname + "/Defined.h>\n";
											}
											else
											{
												content += "#include<" + fielname + "/cpp/Defined.h>\n";
											}
										}
                                    }
                                    else
                                    {
                                         if(IsXidl)//是true则加后缀_xidl
                                         {
                                              content += "#include<" + fielname + "_xidl/cpp/Defined.h" + ">\n";
                                         }
                                         else
                                         {
                                              content += "#include<" + fielname + "/cpp/Defined.h" + ">\n";
                                         }
                                       
                                    }

                                }
                            }
                        }
                        if (IsService)
                        {
                            TSString serviceHeader = "#include <TopSimRPC/TSBasicService/Support.h>";
                            boost::algorithm::replace_all(content,"_SERVER_INFO_",serviceHeader);
                        }
                        else
                        {
                            boost::algorithm::replace_all(content,"_SERVER_INFO_","");
                        }
						//只定义一次头文件
						if (!_topicFlagStruct->buildDefine)
						{
							iterfile->second->AppendFirst(content);
							iterfile->second->AppendFirst(TSIDLUtils::GenHeadUUID("Defined.h"));
						}
                       
                        iterfile->second->AppendLast("#endif //");
                    }
                    if (FileName == "Defined.cpp")
                    {
                        iterfile->second->AppendFirst(CPP_INCLUDE_DEFINE__);
                    }
                }
            }
        }    
    }
	//_topicFlagStruct->buildDefine = true;
	//_serviceFlagStruct->buildDefine = true;
}

void TSGenSupportPlugin::BuildExport(TSString FileName)
{
    TSString code = EXPORT_HEADER;
    TSString codeExport = EXPORT_HEADER;
    if(HasOption(GExe))
    {
        code = EXPORT_HEADER_EXE;
        codeExport = EXPORT_HEADER_EXE;
    }

    TSIDLUtils::ReplaceLinkerVersion(code,"_LINKER_VERSION_");
    TSIDLUtils::ReplaceLinkerVersion(codeExport,"_LINKER_VERSION_");

    TSString FileNameUpper = FileName;
    boost::algorithm::to_upper(FileNameUpper);
    TSString exportStr = FileNameUpper + "_EXPORTS";
    TSString declStr = FileNameUpper + "_DECL";
    TSString maro;
    if (HasOption("ExportMacro"))
    {
        maro = Option<TSString>("ExportMacro");
    }
    if (!maro.empty())
    {
        declStr = maro;
    }
    TSString staticStr = "USE_" + FileNameUpper + "_STATIC";

    boost::algorithm::replace_all(code,"_EXPORT_STR_",exportStr);
    boost::algorithm::replace_all(code,"_DECL_STR_",declStr);
    boost::algorithm::replace_all(code,"STATIC_STR",staticStr);
    boost::algorithm::replace_all(code,"_FILE_NAME_",FileName);
    TSString FileContent = "";
    TSString exportFile = TSIDLUtils::ReplaceExportFileName(FileContent);
    if(TSSolutionPtr Solution = 
        SolutionMgr()->GetOrCreateSolution(FileName))
    {
        TSProjectPtr Dir  = Solution->GetOrCreateProj(FileName);
        Dir->SetProFileType(None);
        TSProjFilePtr TopicHeaderFile = Dir->GetOrCreateProjFile(exportFile,"");
        if(!FileContent.empty())
        {
            TopicHeaderFile->SetContent(FileContent);
        }
        else
        {
            TopicHeaderFile->SetContent(code);
        }
        
        TopicHeaderFile->SetFileType(None);

        TSString stdHeadCode = "#ifdef XSIM3_2\n"\
            "#include <TopSimTypes/TopSimTypes.h> \n"\
            "#include <TopSimDataInterface/TopSimDataInterface4XSim.h>\n"\
            "#elif defined XSIM3_2\n"\
            "#include <TopSimTypes/TopSimTypes.h>\n"
            "#include <TopSimDataInterface/TopSimDataInterface4XSim3.2.h>\n"\
            "#else \n"\
            "#include <TopSimRuntime/TopSimRuntime.h>\n"\
            "#endif // XSIM3_2 \n"\
            "#ifdef _PYTHON \n"\
            "#define BOOST_PYTHON_STATIC_LIB\n"\
            "#include <boost/python.hpp>\n"\
			"#ifdef _WIN32\n"\
			"#ifdef _DEBUG\n"\
			"#pragma comment(lib,\"libboost_python36-vc141-mt-gd-x64-1_67.lib\")\n"\
			"#else\n"\
			"#pragma comment(lib,\"libboost_python36-vc141-mt-x64-1_67.lib\")\n"\
			"#endif\n"\
			"#endif\n"\
            "#endif\n";

        TSString stdCppCode = "#include \"stdafx.h\"";
        TSIDLUtils::ReplaceLinkerVersion(stdHeadCode,"_LINKER_VERSION_");
        if (!HasOption(Gstdafx))
        {
            TSProjFilePtr stdHeaderFile = Dir->GetOrCreateProjFile("stdafx",".h");
            stdHeaderFile->SetContent(stdHeadCode);
            stdHeaderFile->SetFileType(None);
            TSProjFilePtr stdCppFile = Dir->GetOrCreateProjFile("stdafx",".cpp");
            stdCppFile->SetContent(stdCppCode);
			stdCppFile->SetFileType(None);
        }
    }
    TSProjFilePtr ExportHeadFile;
    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();
    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
        if (TypeDef->Type == TSTypeDef::Struct || TypeDef->Type == TSTypeDef::Topic)
        {
            continue;
        }

        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(TSSolutionPtr Solution = 
            SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
        {
            TSString Path = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
            boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
            TSProjectPtr Dir  = Solution->GetOrCreateProj(Path);
            Dir->SetProFileType(Impl);
            ExportHeadFile = Dir->GetOrCreateProjFile("Export",".h");
            TSString Code = codeExport;
            TSString filename = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
            TSString upFileName = filename;
            boost::algorithm::replace_all(filename,"::",".");
            boost::algorithm::replace_all(upFileName,"::","_");
            boost::algorithm::to_upper(upFileName);
            TSString exStr = upFileName + "_EXPORTS";
            TSString deStr = upFileName + "_DECL";
            TSString staStr = "USE_" + upFileName + "_STATIC";

            boost::algorithm::replace_all(Code,"_EXPORT_STR_",exStr);
            boost::algorithm::replace_all(Code,"_DECL_STR_",deStr);
            boost::algorithm::replace_all(Code,"STATIC_STR",staStr);
            boost::algorithm::replace_all(Code,"_FILE_NAME_",filename);

            ExportHeadFile->SetContent(Code);
            ExportHeadFile->SetFileType(Impl);
            ExportHeadFile->AppendFirst(TSIDLUtils::GenHeadUUID("Export.h"));
            ExportHeadFile->AppendLast("#endif //");

        }

    }

}

void TSGenSupportPlugin::ReplaceSerializeStatic( TSString &Code,TSTypeDefWithFiledPtr TypeDef,TSString SuperName)
{
    TSString serCode,deserCode,regfield;

    if (TypeDef->Type == TSTypeDef::Struct ||
        TypeDef->Type == TSTypeDef::Topic ||
        TypeDef->Type == TSTypeDef::Service)
    {
        if (!SuperName.empty())
        {
            serCode += "Marshal_Supper;\n";
            deserCode += "Demarshal_Supper;\n";
            regfield = Table() + "REG_BASE(SUP_NAMESPACE::DataType);" + TSIDL_NEWLINE;
        }
    }

    TSString findsetname = "TopSimDataInterface::kNone";
    bool isparent = false;
    TSString attribut = "";
    for(size_t fieldId = 0; fieldId < TypeDef->Fileds.size(); ++fieldId)
    {
        if(TSFieldDefPtr field = TypeDef->Fileds[fieldId])
        {
            TSString findsettype;
            TSString keyname;

            if (TypeDef->Fileds.size() == fieldId + 1 )
            {
                serCode += "Marshal_Filed(" + field->ValName + "," + boost::lexical_cast<TSString>(field->FindSetEnumType) + ");";
                deserCode += "Demarshal_Filed(" + field->ValName + "," + boost::lexical_cast<TSString>(field->FindSetEnumType) + ");";
            }
            else
            {
                serCode += "Marshal_Filed(" + field->ValName + "," + boost::lexical_cast<TSString>(field->FindSetEnumType) + ");" + TSIDL_NEWLINE;
                deserCode += "Demarshal_Filed(" + field->ValName + "," + boost::lexical_cast<TSString>(field->FindSetEnumType) + ");" + TSIDL_NEWLINE;
            }
            if (field->Def->Type == TSTypeDef::Struct && field->FindSet != 0)
            {
                attribut += Table(1) + "if(HasFindSetT<" + TSIBuiltinPlugin::GetSuitFullTypeName(field->Def) + 
                    ">(Flag))\n";
                attribut += Table(1) + "{\n";
                attribut += Table(2) + "return true;\n";
                attribut += Table(1) + "}\n";
            }

            if (field->FindSet == TSFieldDef::Handle)
            {
                findsetname = "TopSimDataInterface::kHandleType";
            }
            else if (field->FindSet == TSFieldDef::ModelOrTmplId)
            {
                findsetname = "TopSimDataInterface::kModelOrTmplIdType";
            }
            else if (field->FindSet == TSFieldDef::SpacialRange)
            {
                findsetname = "TopSimDataInterface::kSpacialRangeType";
            }
			else if (field->FindSet == TSFieldDef::Key)
			{
				findsetname = "TopSimDataInterface::kKey";
			}
        }
    }

    boost::algorithm::replace_all(Code,"_FINDSET_FLAG",findsetname);
    if (TypeDef->Type == TSTypeDef::Struct)
    {
        boost::algorithm::replace_all(Code,"_HASFINDSET_STRUCT_",attribut);
    }
    else
    {
        boost::algorithm::replace_all(Code,"_HASFINDSET_STRUCT_","");
    }

    TSString parentfindset;
    if (!SuperName.empty())
    {
        parentfindset = 
            Table(1) + "if (HasFindSetT<" + SuperName + "::DataType>(Flag))" + TSIDL_NEWLINE +
            Table(1) + "{" + TSIDL_NEWLINE +
            Table(2) + "return true;" + TSIDL_NEWLINE +
            Table(1) + "}" + TSIDL_NEWLINE;
    }

    boost::algorithm::replace_all(Code,"_PARENT_FINDSET",parentfindset);

    boost::algorithm::replace_all(Code,"_MARSHAL_FILED",serCode);
    boost::algorithm::replace_all(Code,"_DEMARSHAL_FILED",deserCode);

    boost::algorithm::replace_all(Code,"_TOPIC_FLAG",GetTopicFlag(TypeDef->Type));

}

TSString TSGenSupportPlugin::GetTopicFlag( TSTypeDef::EnumTypeDef DataType )
{
    //topic flag
    TSString topicFlag;
    switch(DataType)
    {
    case TSTypeDef::Struct:
        {
            topicFlag = "TF_DataFlag";
            break;
        }
    case TSTypeDef::Service:
        {
            topicFlag = "TF_Service";
            break;
        }
    default:
        {
            topicFlag = "TF_InvalidFlag";
            break;
        }
    }
    return topicFlag;
}

TSString TSGenSupportPlugin::GetAttributes( TSTypeDefWithFiledPtr TypeDef, const TSString & NameSpace)
{
    TSString FiledDeclare;
    BOOST_FOREACH(TSFieldDefPtr Filed,TypeDef->Fileds)
    {
        FiledDeclare += Table();
        if (NameSpace == "Interface")
        {
            FiledDeclare += "virtual ";
        }
        TSString Typename = TSBuiltinPlugin::GetSuitFullTypeName(Filed->Def);
        FiledDeclare += Typename + TSIDL_BLANK + "Get" + Filed->ValName + "()";
        if (NameSpace == "Interface")
        {
            FiledDeclare += " = 0";
        }
        FiledDeclare += ";" + TSIDL_NEWLINE;
        if (NameSpace != "Interface")
        {
            FiledDeclare += Table() + "void Set" + Filed->ValName + 
                "(" + Typename + TSIDL_BLANK + Filed->ValName + ");" + TSIDL_NEWLINE;
        }

    }

    return FiledDeclare;
}

TSString TSGenSupportPlugin::GetFunctions( TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, const TSString & Namespace)
{
    TSString result;

    BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
    {
		/*TSString funName = Function->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
        result += Table();
        if (Namespace == "Method" || 
            Namespace == "Interface" || 
            //Namespace == "Proxy" ||
            Namespace == "Servant")
        {
            result += "virtual ";
        }
        result += TSBuiltinPlugin::GetSuitFullTypeName(Function->Def) + TSIDL_BLANK + Function->ValName + "(";

        for (unsigned int i = 0; i < Function->Params.size(); i++)
        {
            TSParamDefPtr Param = Function->Params[i];
            TSString TypeName = TSBuiltinPlugin::GetSuitFullTypeName(Param->Def);
            if (Param->Type == TSParamDef::Inout || Param->Type == TSParamDef::Out)
            {
                result += TypeName + TSIDL_BLANK + "& " + Param->ValName;
            }
            else
            {
                result += "const " + TypeName + TSIDL_BLANK + "& " + Param->ValName;
            }

            if (Function->Params.size() != i + 1)
            {
                result += ",";
            }
        }
        result += ")";
        if (Namespace == "Method" || Namespace == "Interface")
        {
            result += " = 0";
        }
        result += ";" + TSIDL_NEWLINE;
    }

    return result;
}

TSString TSGenSupportPlugin::GetAttributesImpl( TSTypeDefWithFiledPtr TypeDef,TSString NameSpace)
{
    TSString FiledDeclare;
    BOOST_FOREACH(TSFieldDefPtr Filed,TypeDef->Fileds)
    {
        TSString Typename = TSBuiltinPlugin::GetSuitFullTypeName(Filed->Def);
        FiledDeclare += Typename + TSIDL_BLANK + NameSpace + "::Get" + Filed->ValName + "()" + TSIDL_NEWLINE + 
            "{" + TSIDL_NEWLINE;
        if (NameSpace != "State")
        {
            FiledDeclare += Table() + "return _State->Get" + Filed->ValName + "();" + TSIDL_NEWLINE;
        }
        else
        {

            FiledDeclare += Table() + "return getState()." + Filed->ValName +";" +  TSIDL_NEWLINE;

        }

        FiledDeclare += "}" + TSIDL_NEWLINE;

        FiledDeclare += "void " + NameSpace + "::Set" + Filed->ValName + 
            "(" + Typename + TSIDL_BLANK + Filed->ValName + ")" + TSIDL_NEWLINE + 
            "{" + TSIDL_NEWLINE ; 
        if (NameSpace != "State")
        {
            FiledDeclare += Table() + "_State->Set" + Filed->ValName + "(" + Filed->ValName + ");" + TSIDL_NEWLINE;
        }
        else
        {
            FiledDeclare += Table() + "getState()." + 
                Filed->ValName + " = " + Filed->ValName + ";" + TSIDL_NEWLINE;

        }
        FiledDeclare += "}" + TSIDL_NEWLINE;
    }
    if (NameSpace == "Interceptor")
    {
        boost::algorithm::replace_all(FiledDeclare,"_State","_pState");
    }
    return FiledDeclare;
}

TSString TSGenSupportPlugin::GetFunctionsAddHandler( TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, TSString NameSpace)
{
    TSString result;
    BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
    {
		TSString funName = Function->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}
        if(NameSpace == "Proxy")
        {
            result += Table() + "TSServerLocator::AddHandler(" +  GetTopicName(TypeDef) +"_" +  Function->ValName + "_" + "Request,"
                +  GetTopicName(TypeDef) +"_" +  Function->ValName + "_" + "Response," + "new ResponseHandlerType(boost::bind(&RemoteMethod::"
                +  Function->ValName +"AsyncHandler,Mhd,_1,_2)));\n";
        }
        else if (NameSpace == "ServerSkel")
        {
            result += Table() + "AddHandler(" +  GetTopicName(TypeDef) +"_" +  Function->ValName + "_" + "Response,"
                +  GetTopicName(TypeDef) +"_" +  Function->ValName + "_" + "Request," + "new RequestHandlerType(boost::bind(&ServerSkel::"
                +  Function->ValName +"Handler,this,_1)));\n";
        }  
    }
    return result;
}
TSString TSGenSupportPlugin::GetFunctionsImpl( TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, TSString NameSpace)
{

    TSString result;

    BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
    {
		TSString funName = Function->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}
        result += TSBuiltinPlugin::GetSuitFullTypeName(Function->Def) + TSIDL_BLANK +  NameSpace + "::" + Function->ValName + "(";
        TSString funParams;
        for (unsigned int i = 0; i< Function->Params.size(); i++)
        {
            TSParamDefPtr Param = Function->Params[i];
            funParams += Param->ValName;
            if (Param->Type == TSParamDef::Inout || Param->Type == TSParamDef::Out)
            {
                result += TSBuiltinPlugin::GetSuitFullTypeName(Param->Def) + TSIDL_BLANK + "& " + Param->ValName;
            }
            else
            {
                result += "const " + TSBuiltinPlugin::GetSuitFullTypeName(Param->Def) + TSIDL_BLANK + "& " + Param->ValName;
            }
            if (Function->Params.size() != i + 1)
            {
                result += ",";
                funParams += ",";
            }
        }
        result += ")" + TSIDL_NEWLINE;

        result += "{" + TSIDL_NEWLINE;
        if (NameSpace == "Interceptor")
        {
            result += Table();
            if(Function->DefTypeName != "void")
            {
                result += "return ";
            }
            result += "_pMethod->" + TSIDLUtils::GetFuncCallStr(Function) + ";\n";
        }
        else if (NameSpace == "Servant")
        {
            if (Function->DefTypeName != "void")
            {
                result += Table() + "return ";
            }
            else
            {
                result += Table();
            }
            result += "_Method->" + Function->ValName + "(" + funParams + ");" + TSIDL_NEWLINE;
        }
        else if (NameSpace == "Proxy")
        {
            result += Table();
            if (Function->DefTypeName != "void")
            {
                result += "return ";
            }
           
            if(TypeDef->Type == TSTypeDef::Service)
            {
                result += "GetMethodT<RemoteMethod>()->" + Function->ValName + "(" + funParams;
            }
            else
            {
                 result += "_RemoteMethod->" + Function->ValName + "(" + funParams;
            }
            result += ");" + TSIDL_NEWLINE;
        }
        else
        {
            result += Table() + "if(ServantPtr aServant = _Servant.lock())" + TSIDL_NEWLINE;
            result += Table() + "{" + TSIDL_NEWLINE;
            if (Function->DefTypeName != "void")
            {
                result += Table(2) + "return aServant->" + Function->ValName + "(" + funParams + ");" + TSIDL_NEWLINE;
            }
            else
            {
                result += Table(2) + "aServant->" + Function->ValName + "(" + funParams + ");" + TSIDL_NEWLINE;
            }
            result += Table() + "}" + TSIDL_NEWLINE;
            result += Table() + "else" + TSIDL_NEWLINE;
            result += Table() + "{" + TSIDL_NEWLINE;
            result += Table(2) + "throw TSException(\"Updater::" + Function->ValName + " Servant invalid!\");\n";
            result += Table() + "}" + TSIDL_NEWLINE;

        }

        result += "}" + TSIDL_NEWLINE;
    }

    return result;
}

void TSGenSupportPlugin::BuildService()
{
    TSProjFilePtr ServiceSkelHeaderFile,ServiceSkelCppFile;

    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildServiceVec, _serviceFlagStruct->buildService);

	std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
		if (TypeDef->Type != TSTypeDef::Service)
		{
			continue;
		}
		else if (TypeDef->Type == TSTypeDef::Service)
		{
			if (!vecSelectService.empty())
			{
				if (vecSelectService[0] == "N")
				{
					continue;
				}
			}
		}

        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
            || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            TSString ParseFileBaseName = 
                GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
            
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
				TSString fullPath = Solution->GetFullPath();
				boost::algorithm::replace_all(fullPath, "\\", "\/");

                TSString Path = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
                boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
                ServiceSkelHeaderFile = Dir->GetOrCreateProjFile(Path + "/ServerSkel",".h");
				ServiceSkelCppFile = Dir->GetOrCreateProjFile(Path + "/ServerSkel", ".cpp");
				TSString genericPath = Path;
				boost::algorithm::replace_all(genericPath, "::", ".");
				genericPath = fullPath + "/generic/" + genericPath;
				/*TSString SavePath = (boost::filesystem::current_path() / TS_TEXT("Projects")).string();
				boost::algorithm::replace_all(SavePath, "\\", "/");*/
				
				TSIDLPluginMgr::Instance()->SetGenericFullPath(genericPath);
				//if (!_serviceFlagStruct->buildService)
				//{
					GenerateServerSkelHeader(ServiceSkelHeaderFile, TypeDef);
					ServiceSkelHeaderFile->AppendFirst(TSIDLUtils::GenHeadUUID("ServerSkel.h"));
					ServiceSkelHeaderFile->AppendLast("#endif //");
					ServiceSkelHeaderFile->SetFileType(Generic);
					GenerateServerSkelCpp(ServiceSkelCppFile, TypeDef, indexServiceVec, _serviceFlagStruct->buildService);
					ServiceSkelCppFile->SetFileType(Generic);
				//}
				//else if (!tempServiceFlag)
				//{
					TSString functions;
					if (TypeDef->Type == TSTypeDef::Service)
					{
						functions = InvokerAndInitializeFunc(TypeDef, indexServiceVec, true, _serviceFlagStruct->buildService);
					}
					TSString funcString;
					BOOST_FOREACH(TSFunctionDefPtr func, TypeDef->Functions)
					{
						/*TSString funName = func->ValName;
						std::vector<TSString>::iterator iterTopic = std::find(indexServiceVec.begin(), indexServiceVec.end(), funName);
						if (iterTopic == indexServiceVec.end())
						{
							continue;
						}*/
						funcString = funcString += Table(2) + "RegisterMethod(\"" + func->ValName + "\"," + GetTopicName(TypeDef) + "_" + func->ValName + "_" + "Request,"
							+ GetTopicName(TypeDef) + "_" + func->ValName + "_" + "Response,&" + func->ValName + "RequestHandler," + "NULL);\n";
						TSString str = ServiceSkelCppFile->GetContent();
						/*int pos = str.find("return true;");
						str.insert(pos, funcString);
						pos = str.find("bool");
						str.insert(pos, functions);*/
						ServiceSkelCppFile->SetContent(str);
					}
				//}
            }
        }
		//_serviceFlagStruct->buildService = true;
    }
}

void TSGenSupportPlugin::GenerateServiceHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName )
{
	bool FirstOrNot = false;
    TSString content = SERVICE_HEADER_CODE;
    
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);

    TSString funcs = GetFunctions(TypeDef, serviceName);
    TSString serverPath = GetBaseName(TypeDef->Parse.lock()->GetFullFileName()) + "_xidl/cpp/generic/";
    serverPath += TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(serverPath,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
    boost::algorithm::replace_all(content,"_SERVICE_PATH_",serverPath);
    boost::algorithm::replace_all(content,"_FUNCS_",funcs);

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateServiceCPP( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName )
{
    TSString content = SERVICE_SERVICE_CPP_CODE;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);

    TSString funcs = GetFunctionsImpl(TypeDef, serviceName,"Service");
    boost::algorithm::replace_all(content,"_FUNCS_",funcs);

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::BuildRemoteMethod()
{
    /*TSProjFilePtr RemoteMethodHeaderFile;
    TSProjFilePtr RemoteMethodCppFile;

    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
        if (TypeDef->Type == TSTypeDef::Struct)
        {
            continue;
        }
        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
            || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            TSString ParseFileBaseName = 
                GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

                TSString Path = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
                boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

                RemoteMethodHeaderFile = Dir->GetOrCreateProjFile(Path + "/RemoteMethod",".h");
                GenerateRemoteMethodHeader(RemoteMethodHeaderFile, TypeDef);
                RemoteMethodHeaderFile->AppendFirst(TSIDLUtils::GenHeadUUID("RemoteMethod.h"));
                RemoteMethodHeaderFile->AppendLast("#endif //");
                RemoteMethodHeaderFile->SetFileType(Generic);

                RemoteMethodCppFile = Dir->GetOrCreateProjFile(Path + "/RemoteMethod",".cpp");
                GenerateRemoteMethodCPP(RemoteMethodCppFile, TypeDef);
                RemoteMethodCppFile->SetFileType(Generic);
            }
        }
    }*/
}

void TSGenSupportPlugin::GenerateRemoteMethodHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef)
{
    TSString content = REMOTEMETHOD_HEADER_CODE;
    if (TypeDef->Type == TSTypeDef::Service)
    {
        content = REMOTEMETHOD_SERVICE_HEADER_CODE;
    }

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);
	std::vector<TSString> serviceName;
    TSString functions = GetFunctions(TypeDef, serviceName, "RemoteMethod");
    functions +=  GetFunctionsAsync(TypeDef, serviceName);
    TSString funchandle = GetFunctionsAsyncHandle(TypeDef, serviceName);
    if (TypeDef->Extend.lock())
    {  
        TSTypeDefWithFuncAndFiledPtr extfun = TS_CAST(TypeDef->Extend.lock(),TSTypeDefWithFuncAndFiledPtr);
        functions += GetFunctions(extfun, serviceName, "RemoteMethod");
        functions += GetFunctionsAsync(extfun, serviceName,"RemoteMethod");
        funchandle += GetFunctionsAsyncHandle(extfun, serviceName);
    }

    TSString serverPath = GetBaseName(TypeDef->Parse.lock()->GetFullFileName()) + "_xidl/cpp/generic/";
    serverPath += TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(serverPath,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
    boost::algorithm::replace_all(content,"_SERVICE_PATH_",serverPath);
    boost::algorithm::replace_all(content,"_FUNCS_",functions);  
    boost::algorithm::replace_all(content,"_FUNC_HANDLERS_",funchandle);

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateRemoteMethodCPP( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef)
{
	std::vector<TSString> serviceName;
    TSString content = REMOTEMETHOD_CPP_CODE;
    if (TypeDef->Type == TSTypeDef::Service)
    {
        content = REMOTEMETHOD_SERVICE_CODE;
    }
    TSString constFuncs;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);

    TSString TopicPath = GetBaseName(TypeDef->Parse.lock()->GetFullFileName()) + "_xidl/cpp";
    boost::algorithm::replace_all(content,"_TOPIC_PATH_",TopicPath);

    TSString funchandle,pubTopic,subTopic,unPubTopic,unSubTopic;
    TSTypeDefWithFuncAndFiledPtr extfun;
    if (TypeDef->Extend.lock())
    {  
        extfun = TS_CAST(TypeDef->Extend.lock(),TSTypeDefWithFuncAndFiledPtr);
        BOOST_FOREACH(TSFunctionDefPtr funcPtr ,extfun->Functions)
        {
            if(TypeDef->Type == TSTypeDef::Service)
            {
                constFuncs += Table() + "GetOperation()->SetHandler(_TOPIC_NAME__" + funcPtr->ValName + "_Response,\n";
                constFuncs += Table() + "new ResponseHandlerType(boost::bind(&RemoteMethod::" + funcPtr->ValName 
                    + "AsyncHandler,this,_1,_2)));\n";
            }
            else
            {
                constFuncs += Table() + "_pProxy->GetOperation()->SetHandler(TS_TEXT(\"" + funcPtr->ValName + "\"),\n";
                constFuncs += Table() + "new ResponseHandlerType(boost::bind(&RemoteMethod::" + funcPtr->ValName 
                    + "AsyncHandler,this,_1,_2,_3)));\n";
            }
            pubTopic += Table() + "PublishTopic(" + TSIDLUtils::GetTopicName(TypeDef) + "_" + funcPtr->ValName + "_Request);\n";
            subTopic += Table() + "SubscribeTopic(" + TSIDLUtils::GetTopicName(TypeDef) + "_" + funcPtr->ValName + "_Response);\n";
            unPubTopic += Table() + "UnpublishTopic(" + TSIDLUtils::GetTopicName(TypeDef) + "_" + funcPtr->ValName + "_Request);\n";
            unSubTopic += Table() + "UnsubscribeTopic(" + TSIDLUtils::GetTopicName(TypeDef) + "_" + funcPtr->ValName + "_Response);\n";
            funchandle += GetFunCode(funcPtr,"RemoteMethod");

            if (TypeDef->Type == TSTypeDef::Service)
            {
                funchandle = GetServiceFunction(TypeDef, serviceName);
            }
        }

    }
    BOOST_FOREACH(TSFunctionDefPtr funcPtr ,TypeDef->Functions)
    {
        if(TypeDef->Type == TSTypeDef::Service)
        {
            constFuncs += Table() + "GetOperation()->SetHandler(_TOPIC_NAME__" + funcPtr->ValName + "_Response,\n";
            constFuncs += Table() + "new ResponseHandlerType(boost::bind(&RemoteMethod::" + funcPtr->ValName 
                + "AsyncHandler,this,_1,_2)));\n";
        }
        else
        {
            constFuncs += Table() + "_pProxy->GetOperation()->SetHandler(TS_TEXT(\"" + funcPtr->ValName + "\"),\n";
            constFuncs += Table() + "new ResponseHandlerType(boost::bind(&RemoteMethod::" + funcPtr->ValName 
                + "AsyncHandler,this,_1,_2,_3)));\n";
        }
        pubTopic += Table() + "PublishTopic(" + TSIDLUtils::GetTopicName(TypeDef) + "_" + funcPtr->ValName + "_Request);\n";
        subTopic += Table() + "SubscribeTopic(" + TSIDLUtils::GetTopicName(TypeDef) + "_" + funcPtr->ValName + "_Response);\n";
        unPubTopic += Table() + "UnpublishTopic(" + TSIDLUtils::GetTopicName(TypeDef) + "_" + funcPtr->ValName + "_Request);\n";
        unSubTopic += Table() + "UnsubscribeTopic(" + TSIDLUtils::GetTopicName(TypeDef) + "_" + funcPtr->ValName + "_Response);\n";
        funchandle += GetFunCode(funcPtr,"RemoteMethod");

        if (TypeDef->Type == TSTypeDef::Service)
        {
            funchandle = GetServiceFunction(TypeDef, serviceName);  
        }
    }
    TSString Pinclude = GetIncludeHeadFile(TypeDef,"Type");
    boost::algorithm::replace_all(content,"_INCLUDES_",Pinclude);
    TSString functions = GetFunctions(TypeDef, serviceName);

    TSString topicName = TSIDLUtils::GetTopicName(TypeDef); 
    boost::algorithm::replace_all(content,"_FUNCS_",functions);
    boost::algorithm::replace_all(content,"_FUNC_AND_HANDLERS_",funchandle);
    boost::algorithm::replace_all(content,"_CONSTRUCTOR_",constFuncs);
    boost::algorithm::replace_all(content,"_PUB_TOPIC_",pubTopic);
    boost::algorithm::replace_all(content,"_SUB_TOPIC_",subTopic);
    boost::algorithm::replace_all(content,"_UNPUB_TOPIC_",unPubTopic);
    boost::algorithm::replace_all(content,"_UNSUB_TOPIC_",unSubTopic);
    boost::algorithm::replace_all(content,"_TOPIC_NAME_",topicName);
    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::BuildProxy()
{
    TSProjFilePtr ProxyHeaderFile;
    TSProjFilePtr ProxyCppFile;
    TSProjFilePtr ProxyFactoryHeaderFile;

    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildProxyVec, _serviceFlagStruct->buildProxy);

	std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
		if (TypeDef->Type == TSTypeDef::Struct)
		{
			continue;
		}
		else if (TypeDef->Type == TSTypeDef::Service)
		{
			if (!vecSelectService.empty())
			{
				if (vecSelectService[0] == "N")
				{
					continue;
				}
			}
		}

        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
            || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            TSString ParseFileBaseName = 
                GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

                TSString Path = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
                boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

                ProxyHeaderFile = Dir->GetOrCreateProjFile(Path + "/Proxy",".h");
				ProxyCppFile = Dir->GetOrCreateProjFile(Path + "/Proxy", ".cpp");
				//if (!_serviceFlagStruct->buildProxy)
				//{
					GenerateProxyHeader(ProxyHeaderFile, TypeDef, indexServiceVec);
					ProxyHeaderFile->AppendFirst(TSIDLUtils::GenHeadUUID("Proxy.h"));
					ProxyHeaderFile->AppendLast("#endif //");
					ProxyHeaderFile->SetFileType(Generic);
					GenerateProxyCPP(ProxyCppFile, TypeDef, indexServiceVec, _serviceFlagStruct->buildProxy);
					ProxyCppFile->SetFileType(Generic);
				//}
				//else if (!tempServiceFlag)
				//{
					//获取定义的函数
					TSString functions = GetFunctions(TypeDef, indexServiceVec, "Proxy");
					functions += GetFunctionsAsync(TypeDef, indexServiceVec, "Proxy");
					//TSString str = ProxyHeaderFile->GetContent();
					//int pos = str.find("protected");
					////将获取的函数插入到当前文件的特定位置
					//str.insert(pos, functions);
					//ProxyHeaderFile->SetContent(str);

					if (TypeDef->Type == TSTypeDef::Service)
					{
						functions = InvokerAndInitializeFunc(TypeDef, indexServiceVec, false, _serviceFlagStruct->buildProxy) + GetServiceFunction(TypeDef, indexServiceVec);
					}
					
					/*TSString funcString;
					BOOST_FOREACH(TSFunctionDefPtr Function, TypeDef->Functions)
					{
						TSString funName = Function->ValName;
						std::vector<TSString>::iterator iterTopic = std::find(indexServiceVec.begin(), indexServiceVec.end(), funName);
						if (iterTopic == indexServiceVec.end())
						{
							continue;
						}

						if (!tempServiceFlag)
						{
							funcString = Table(2) + "RegisterMethod(\"" + Function->ValName + "\"," + GetTopicName(TypeDef) + "_" + Function->ValName + "_" + "Request,"
								+ GetTopicName(TypeDef) + "_" + Function->ValName + "_" + "Response," + "NULL,&" + Function->ValName + "AsyncResponseInvoker);\n";
						}
					}*/
					/*str = ProxyCppFile->GetContent();
					pos = str.find("return true;");
					str.insert(pos, funcString);
					pos = str.find("void");
					str.insert(pos, functions);
					ProxyCppFile->SetContent(str);*/
				//}
                
                if (TypeDef->Type != TSTypeDef::Service &&
                    TypeDef->Type != TSTypeDef::Struct)
                {
                    ProxyFactoryHeaderFile = Dir->GetOrCreateProjFile(Path + "/ProxyFactory", ".h");
                    GenerateProxyFactoryHeader(ProxyFactoryHeaderFile, TypeDef);
                    ProxyFactoryHeaderFile->AppendFirst(TSIDLUtils::GenHeadUUID("ProxyFactory.h"));
                    ProxyFactoryHeaderFile->AppendLast("#endif //");
                    ProxyFactoryHeaderFile->SetFileType(Generic);

                    TSProjFilePtr ProxyFactoryCppFile = Dir->GetOrCreateProjFile(Path + "/ProxyFactory",".cpp");
                    GenerateProxyFactoryCPP(ProxyFactoryCppFile, TypeDef);
                    ProxyFactoryCppFile->SetFileType(Generic);
                }
            }
        }
		//_serviceFlagStruct->buildProxy = true;
    }
}

void TSGenSupportPlugin::GenerateProxyHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName)
{
    TSString content = PROXY_HEADER_CODE;
    if(TypeDef->Type == TSTypeDef::Service)
    {
        content = PROXY_SERVICE_HEADER_CODE;
    }

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);

    TSString parentState("TSISDOObject");
    TSString Pinclude;
    if (TypeDef->Extend.lock())
    {
        parentState = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true) + "::Proxy";
        Pinclude = "#include <generic/" + GetFileRelativePath(TypeDef->Extend.lock()) + "/Proxy.h>\n";
    }
    boost::algorithm::replace_all(content,"_PARENT_NAME_",parentState);
    boost::algorithm::replace_all(content,"_PINCLUDE_",Pinclude);
    TSString functions = GetFunctions(TypeDef, serviceName, "Proxy");
    functions += GetFunctionsAsync(TypeDef, serviceName, "Proxy");

    TSString serverPath = GetBaseName(TypeDef->Parse.lock()->GetFullFileName()) + "_xidl/cpp/generic/";
    serverPath += TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(serverPath,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
    boost::algorithm::replace_all(content,"_SERVICE_PATH_",serverPath);
    boost::algorithm::replace_all(content,"_FUNCS_",functions);

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateProxyCPP( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, bool buildProxyFlag )
{
    TSString content = PROXY_CPP_CODE;
    if (TypeDef->Type == TSTypeDef::Service)
    {
        content = PROXY_SERVICE_CPP_CODE;
        content += PROXY_FOR_CREATE;
    }

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);
    TSString parentState("TSISDOObject");
    if (TypeDef->Extend.lock())
    {
        parentState = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true) + "::Proxy";
    }
    boost::algorithm::replace_all(content,"_PARENT_NAME_",parentState);
    TSString packageName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(content,"_PACKAGES_",packageName);

    TSString functions ;
    if (TypeDef->Type == TSTypeDef::Service)
    {
        functions  = InvokerAndInitializeFunc(TypeDef, serviceName,false, buildProxyFlag) + GetServiceFunction(TypeDef, serviceName);
    }

//    functions += GetFunctionsImpl(TypeDef,"Proxy");
   // functions += GetFunctionsAsyncImpl(TypeDef,"Proxy");
    boost::algorithm::replace_all(content,"_FUNCS_",functions);
    TSString serverName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(content, "_SERVICE_NAME_", serverName);
    TSString serverNameLL = serverName;
    boost::algorithm::replace_all(serverNameLL, "::", "");
    boost::algorithm::replace_all(content,"_SERVICE_NAMELL_",serverNameLL);
    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateProxyFactoryHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = PROXY_FACTORY_HEADER_CODE;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateProxyFactoryCPP( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = PROXY_FACTORY_CPP_CODE;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);

    TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true); 
    boost::algorithm::replace_all(content,"_PACKAGES_",selfns);
    TSString selfname = GetTopicName(TypeDef);
    boost::algorithm::replace_all(content,"_TOPIC_NAME_",selfname);

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::BuildMethod()
{
    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	//std::map<TSString, TSString> serviceMap;
	////获得服务和其依赖的结构体存入serviceMap容器中
	//GetServiceAndStructMap(TypeDefs, serviceMap);

	////获取主题选择工具中选中的服务
	//std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	//std::vector<TSString> tempServiceVec;
	////获得要生成的服务，如果追加的服务为空则返回true
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildMethodVec, _serviceFlagStruct->buildMethod);

	//std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	////将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
		if (TypeDef->Type == TSTypeDef::Struct)
		{
			continue;
		}
		/*else if (TypeDef->Type == TSTypeDef::Service)
		{
			if (!vecSelectService.empty())
			{
				if (vecSelectService[0] == "N")
				{
					continue;
				}
			}
		}*/

        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
            || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            TSString ParseFileBaseName = 
                GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

                TSString Path = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
                boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
				TSProjFilePtr MethodHeaderFile = Dir->GetOrCreateProjFile(Path + "/Method", ".h");
               
				//if (!_serviceFlagStruct->buildMethod) //第一次生成method.h文件
				//{
					GenerateMethod(MethodHeaderFile, TypeDef, indexServiceVec);
					MethodHeaderFile->AppendFirst(TSIDLUtils::GenHeadUUID("Method.h"));
					MethodHeaderFile->AppendLast("#endif //");
					MethodHeaderFile->SetFileType(Generic);
				//}
				//else if (!tempServiceFlag) //第二次以后生成method.h文件
				//{
					//获取定义的函数
					//TSString functions = GetFunctions(TypeDef, indexServiceVec, "Method");
					//TSString str = MethodHeaderFile->GetContent();
					//int pos = str.find("};");
					//将获取的函数插入到当前文件的特定位置
					//str.insert(pos, functions);
					//MethodHeaderFile->SetContent(str);
				//}
                if (TypeDef->Type != TSTypeDef::Service)
                {
                    TSProjFilePtr MethodFactoryFile = Dir->GetOrCreateProjFile(Path + "/MethodFactory",".h");
                    GenerateMethodFactory(MethodFactoryFile, TypeDef);
                    MethodFactoryFile->AppendFirst(TSIDLUtils::GenHeadUUID("MethodFactory.h"));
                    MethodFactoryFile->AppendLast("#endif //");
                    MethodFactoryFile->SetFileType(Generic);

                    TSProjFilePtr MethodArchiveHeaderFile = Dir->GetOrCreateProjFile(Path + "/MethodArchive", ".h");
                    GenerateMethodArchiveHeader(MethodArchiveHeaderFile, TypeDef);
                    MethodArchiveHeaderFile->AppendFirst(TSIDLUtils::GenHeadUUID("MethodArchive.h"));
                    MethodArchiveHeaderFile->AppendLast("#endif //");
                    MethodArchiveHeaderFile->SetFileType(Generic);

                    TSProjFilePtr MethodArchiveCppFile = Dir->GetOrCreateProjFile(Path + "/MethodArchive",".cpp");
                    GenerateMethodArchiveCPP(MethodArchiveCppFile, TypeDef);
                    MethodArchiveCppFile->SetFileType(Generic);
                }
            }
        }
		//_serviceFlagStruct->buildMethod = true;
    }
}

void TSGenSupportPlugin::GenerateMethod( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName)
{
    TSString content = METHOD_HEADER_CODE;
    if(TypeDef->Type == TSTypeDef::Service)
    {
        content = METHOD_SERVICE_HEADER_CODE;
    }
	
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
	
    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);
    TSString filename = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    boost::algorithm::replace_all(content,"_FILE_NAME_",FILEDIRCPP);

    TSString superns;
    TSString Pinclude;
    TSString functions;
    if (TypeDef->Extend.lock())
    {
        superns = " : virtual public " + TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true) + "::Method";
        Pinclude = "#include <generic/" + GetFileRelativePath(TypeDef->Extend.lock()) + "/Method.h>\n";
        TSTypeDefWithFuncAndFiledPtr extDef = TS_CAST(TypeDef->Extend.lock(),TSTypeDefWithFuncAndFiledPtr);
        functions += GetFunctions(extDef, serviceName,"Method");
    }
    Pinclude += GetIncludeHeadFile(TypeDef,"Interface");
    boost::algorithm::replace_all(content,"_INHERIT_",superns);
    boost::algorithm::replace_all(content,"_PINCLUDE_",Pinclude);

    functions += GetFunctions(TypeDef, serviceName,"Method");
	std::cout << "functions2: " << functions << std::endl;
    boost::algorithm::replace_all(content,"_FUNCS_",functions);
    TSString TopicPath = GetBaseName(TypeDef->Parse.lock()->GetFullFileName()) + "_xidl/cpp";
    boost::algorithm::replace_all(content,"_TOPIC_HEAD_",TopicPath);

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateMethodFactory( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = FACTORY_HEADER_CODE;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);

    if (TSTypeDef::Service == TypeDef->Type)
    {
        boost::algorithm::replace_all(content, "__PARAMETER__", FACTORY_CREATE_SERVICE);
    }
    else
    {
        boost::algorithm::replace_all(content,"__PARAMETER__",FACTORY_CREATE_MESSAGE);
    }

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateMethodArchiveHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = ARCHIVE_HEADER_CODE;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);
    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateMethodArchiveCPP( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = ARCHIVE_CPP_CODE;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSString Pinclude = GetIncludeHeadFile(TypeDef,"Type");
    Pinclude += "#include \"../../Support.h\"";
    boost::algorithm::replace_all(content,"_PINCLUDE_",Pinclude);
    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);
    TSString Archive = GetArchiveCode(TypeDef);
    boost::algorithm::replace_all(content,"_ARCHIVE_",Archive);

    ProjFile->AppendLast(content);
}

TSString TSGenSupportPlugin::GetArchiveCode( TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString result;

    //result += Table() + "if(TSIArgArchive::Archive(Ser,Item))";
    BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
    {
        result += Table(2) + "if(Item.MethodName == TS_TEXT(\"" + Function->ValName + "\"))" + TSIDL_NEWLINE;
        result += Table(2) + "{" + TSIDL_NEWLINE;
        result += Table(3) + "switch(ArchiveType)" + TSIDL_NEWLINE;
        result += Table(3) + "{" + TSIDL_NEWLINE;
        //SaveInput
        result += Table(4) + "case TSIArgArchive::SaveInputArchive:" + TSIDL_NEWLINE;
        result += Table(4) + "{" + TSIDL_NEWLINE;
        result += Table(5) + "switch(GetArgIndex())" + TSIDL_NEWLINE;
        result += Table(5) + "{" + TSIDL_NEWLINE;
        for (unsigned int i = 0; i< Function->Params.size(); i++)
        {
            TSParamDefPtr Param = Function->Params[i];
            result += Table(6) + "case " + TSValue_Cast<TSString>(i + 1) + ":" + TSIDL_NEWLINE;
            result += Table(6) + "{" + TSIDL_NEWLINE;
            result += Table(7) + "Marshal(Ser,Item.Arg.Value<" + TSBuiltinPlugin::GetSuitFullTypeName(Param->Def,false,ClassIdentity)
                + ">());\n" + TSIDL_NEWLINE;
            result += Table(7) + "return true;" + TSIDL_NEWLINE;
            result += Table(6) + "}" + TSIDL_NEWLINE;
        }
        result += Table(5) + "}" + TSIDL_NEWLINE;
        result += Table(4) + "}" + TSIDL_NEWLINE;
        result += Table(4) + "break;" + TSIDL_NEWLINE;
        //loadInput
        result += Table(4) + "case TSIArgArchive::LoadInputArchive:" + TSIDL_NEWLINE;
        result += Table(4) + "{" + TSIDL_NEWLINE;
        result += Table(5) + "switch(GetArgIndex())" + TSIDL_NEWLINE;
        result += Table(5) + "{" + TSIDL_NEWLINE;
        for (unsigned int i = 0; i< Function->Params.size(); i++)
        {
            TSParamDefPtr Param = Function->Params[i];
            result += Table(6) + "case " + TSValue_Cast<TSString>(i + 1) + ":" + TSIDL_NEWLINE;
            result += Table(6) + "{" + TSIDL_NEWLINE;
            result += Table(7) + TSBuiltinPlugin::GetSuitFullTypeName(Param->Def,false,ClassIdentity) + " Arg;" + TSIDL_NEWLINE;
            result += Table(7) + "Demarshal(Ser,Arg);" + TSIDL_NEWLINE;
            result += Table(7) + "Item.Arg = TSVariant::FromValue(Arg);" + TSIDL_NEWLINE;
            result += Table(7) + "return true;" + TSIDL_NEWLINE;
            result += Table(6) + "}" + TSIDL_NEWLINE;
        }
        result += Table(5) + "}" + TSIDL_NEWLINE;
        result += Table(4) + "}" + TSIDL_NEWLINE;
        result += Table(4) + "break;" + TSIDL_NEWLINE;
        //SaveOutput
        result += Table(4) + "case TSIArgArchive::SaveOutputArchive:" + TSIDL_NEWLINE;
        result += Table(4) + "{" + TSIDL_NEWLINE;
        result += Table(5) + "switch(GetArgIndex())" + TSIDL_NEWLINE;
        result += Table(5) + "{" + TSIDL_NEWLINE;
        if (Function->DefTypeName == "void")
        {
            for (unsigned int i = 0; i< Function->Params.size(); i++)
            {
                TSParamDefPtr Param = Function->Params[i];
                if (Param->Type != TSParamDef::In)
                {
                    result += Table(6) + "case " + TSValue_Cast<TSString>(i + 1) + ":" + TSIDL_NEWLINE;
                    result += Table(6) + "{" + TSIDL_NEWLINE;
                    result += Table(7) + "Marshal(Ser,Item.Arg.Value<" + TSBuiltinPlugin::GetSuitFullTypeName(Param->Def,false,ClassIdentity) + 
                        ">());\n" + TSIDL_NEWLINE;
                    result += Table(7) + "return true;" + TSIDL_NEWLINE;
                    result += Table(6) + "}" + TSIDL_NEWLINE;
                }

            }
        }
        else
        {
            result += Table(6) + "case " + TSValue_Cast<TSString>(1) + ":" + TSIDL_NEWLINE;
            result += Table(6) + "{" + TSIDL_NEWLINE;
            result += Table(7) + "Marshal(Ser,Item.Arg.Value<" + TSBuiltinPlugin::GetSuitFullTypeName(Function->Def,false,ClassIdentity) + 
                ">());\n" + TSIDL_NEWLINE;
            result += Table(7) + "return true;" + TSIDL_NEWLINE;
            result += Table(6) + "}" + TSIDL_NEWLINE;
            for (unsigned int i = 1; i< Function->Params.size(); i++)
            {
                TSParamDefPtr Param = Function->Params[i];
                if (Param->Type != TSParamDef::In)
                {
                    result += Table(6) + "case " + TSValue_Cast<TSString>(i + 1) + ":" + TSIDL_NEWLINE;
                    result += Table(6) + "{" + TSIDL_NEWLINE;
                    result += Table(7) + "Marshal(Ser,Item.Arg.Value<" + TSBuiltinPlugin::GetSuitFullTypeName(Param->Def,false,ClassIdentity) + 
                        ">());\n" + TSIDL_NEWLINE;
                    result += Table(7) + "return true;" + TSIDL_NEWLINE;
                    result += Table(6) + "}" + TSIDL_NEWLINE;
                }

            }
        }

        result += Table(5) + "}" + TSIDL_NEWLINE;
        result += Table(4) + "}" + TSIDL_NEWLINE;
        result += Table(4) + "break;" + TSIDL_NEWLINE;
        //LoadOutput
        result += Table(4) + "case TSIArgArchive::LoadOutputArchive:" + TSIDL_NEWLINE;
        result += Table(4) + "{" + TSIDL_NEWLINE;
        result += Table(5) + "switch(GetArgIndex())" + TSIDL_NEWLINE;
        result += Table(5) + "{" + TSIDL_NEWLINE;
        for (unsigned int i = 0; i< Function->Params.size(); i++)
        {
            TSParamDefPtr Param = Function->Params[i];
            result += Table(6) + "case " + TSValue_Cast<TSString>(i + 1) + ":" + TSIDL_NEWLINE;
            result += Table(6) + "{" + TSIDL_NEWLINE;
            result += Table(7) + "Marshal(Ser,Item.Arg.Value<" + TSBuiltinPlugin::GetSuitFullTypeName(Param->Def,false,ClassIdentity) + 
                ">());\n" + TSIDL_NEWLINE;
            result += Table(7) + "return true;" + TSIDL_NEWLINE;
            result += Table(6) + "}" + TSIDL_NEWLINE;
        }
        result += Table(5) + "}" + TSIDL_NEWLINE;
        result += Table(4) + "}" + TSIDL_NEWLINE;
        result += Table(4) + "break;" + TSIDL_NEWLINE;

        //result += Table(4) + "}" + TSIDL_NEWLINE;
        result += Table(3) + "}" + TSIDL_NEWLINE;
        result += Table(2) + "}" + TSIDL_NEWLINE;
    }

    return result;
}

TSString TSGenSupportPlugin::GetFunctionsAsync( TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, const TSString & Namespace)
{
    TSString result;
    bool IsService = false;

    BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
    {
		/*TSString funName = Function->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
        result += Table();
       // if (Namespace == "Proxy")
       // {
           // result += "virtual ";

       // }
        if (TypeDef->Type == TSTypeDef::Service )
        {
            IsService = true;
        }

        if (IsService && Namespace == "Proxy")
        {
            result += TSIDLUtils::GetFuncAsyncStr(Function,Namespace,IsService,true) + ";\n";
        }
        else
        {
            result += TSIDLUtils::GetFuncAsyncStr(Function,Namespace,IsService) + ";\n";
        }
    }  
    return result;
}

TSString TSGenSupportPlugin::GetFunctionsAsyncHandle( TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName )
{
    TSString result;

    BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
    {
		/*TSString funName = Function->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
        if (TypeDef->Type == TSTypeDef::Service)
        {
            result += Table() + 
                "void" + TSIDL_BLANK + 
                Function->ValName + 
                "AsyncHandler(TSBasicService::DataTypePtr Response,void * Invoker);" + TSIDL_BLANK;
        }
        else
        {
            result += Table() + 
                "void" + TSIDL_BLANK + 
                Function->ValName + 
                "AsyncHandler(TSRpcResponse::DataTypePtr Response,void *Invoker,const TSAsyncCall::TSAsyncError& Error);" +
                TSIDL_NEWLINE;
        }
    }

    return result;
}

TSString TSGenSupportPlugin::GetFunctionsAsyncImpl( TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, TSString NameSpace)
{
    TSString result;
    bool IsService = false;
    BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
    {
		/*TSString funName = Function->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
        if (TypeDef->Type == TSTypeDef::Service)
        {
            IsService = true;
        }
        result += TSIDLUtils::GetFuncAsyncStr(Function,NameSpace,IsService) + "\n";
        result += "{\n";
        TSString params = "";
        for(size_t i = 0; i < Function->Params.size() ;++i)
        {
            if(TSParamDefPtr param = Function->Params[i])
            {
                params += param->ValName;
                if (i != Function->Params.size() - 1)
                {
                    params += ",";
                }
            }
        }
        if (NameSpace == "Interceptor")
        {
            result += Table() + "if(ProxyPtr aProxy = _pProxy.lock())\n";
            result += Table() + "{\n";
            result += Table(2) + "return aProxy->" + Function->ValName + "Async(";
            if (!params.empty())
            {
                result += params + ",";
            }
            result += "func,Timeout);\n"; 
            result += Table() + "}\n";
            result += Table() + "else\n";
            result += Table() + "{\n";
            result += Table() + "throw TSException(\"" + TSBuiltinPlugin::GetSuitFullTypeName(TypeDef) + "::" +
                Function->ValName + "Async Proxy invalid!\");\n";
            result += Table() + "}\n";
        }
        else if (NameSpace == "Proxy")
        {
             if (TypeDef->Type == TSTypeDef::Service)
             {
                 result += Table() + "GetMethodT<RemoteMethod>()->" + Function->ValName + "Async(";
                 if (!params.empty())
                 {
                     result += params + ",";
                 }
                 result += "func);\n";
             }
             else
             {
                 result += Table() + "_RemoteMethod->" + Function->ValName + "Async("; 
                 if (!params.empty())
                 {
                     result += params + ",";
                 }
                 result += "func,Timeout);\n";
             }
         }
           
        result += "}" + TSIDL_NEWLINE;
    }

    return result;
}

TSString TSGenSupportPlugin::GetFunctionsAsyncHandleImpl( TSTypeDefWithFuncAndFiledPtr TypeDef,TSString NameSpace )
{
    TSString result;


    return result;
}

TSString TSGenSupportPlugin::RecourseParam( TSParamDefPtr Param )
{
    TSString result;

    TSTypeDef::EnumTypeDef paramType = Param->Def->Type;

    if (TSTypeDef::Builtin == paramType)
    {
        result += Table() + "Item.Arg = TSVariant::FromValue(" + Param->ValName + ");\n";
        result += Table() + "InputArchive.Archive(Ser,Item);\n";
    }

    return result;
}

TSString TSGenSupportPlugin::GetFunCode( TSFunctionDefPtr Func,const TSString & Namespace ,bool IsService)
{
    TSString result,AsyncRequestCode;
    result += TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + TSIDL_BLANK +
        Namespace + "::" + 
        TSIDLUtils::GetFuncWithoutResultStr(Func) + TSIDL_NEWLINE;
    result += "{" + TSIDL_NEWLINE;
    AsyncRequestCode += Table() + "TSArgArchiveItem Item;\n";
    AsyncRequestCode += Table() + "Item.MethodName = TS_TEXT(\"" + Func->ValName + "\");\n\n";
    AsyncRequestCode += Table() + "TSRpcRequest::DataTypePtr Request = boost::make_shared<TSRpcRequest::DataType>();\n";
    AsyncRequestCode += Table() + "Request->Handle = _pProxy->GetOperation()->GetHandle();\n";
    AsyncRequestCode += Table() + "Request->Directive = TS_TEXT(\"" + Func->ValName + "\");\n\n";
    AsyncRequestCode += Table() + "TSSerializer Ser;\n\n";

    AsyncRequestCode += Table() + "MethodArchive InputArchive(TSIArgArchive::SaveInputArchive);\n";
    result += AsyncRequestCode;
    std::vector<TSParamDefPtr> outParams;
    for(size_t i = 0; i < Func->Params.size() ;++i)
    {
        TSParamDefPtr param = Func->Params[i];

        if(TSParamDef::Out == param->Type || TSParamDef::Inout == param->Type)
        {
            outParams.push_back(param);
        }

        if (TSParamDef::Out != param->Type || TSParamDef::Inout == param->Type)
        {
            result += RecourseParam(param);
        }
        else
        {
            result += " InputArchive.Archive(Ser,Item);\n";
        }
        AsyncRequestCode += RecourseParam(param);
    }
    result += Table() + "BufferToU8(Request->Context,Ser.GetBuffer()->Data(),Ser.GetBuffer()->Length());\n";
    AsyncRequestCode += Table() + "BufferToU8(Request->Context,Ser.GetBuffer()->Data(),Ser.GetBuffer()->Length());\n";
    TSString responceCode,handlerCode;
    if(Func->DefTypeName != "void" || outParams.size())
    {
        responceCode += "	TSSerializer Deser;\n";
        responceCode += "	U8ToBuffer(Response->Context,Deser.GetBuffer());\n\n";
        responceCode += "	MethodArchive OutputArchive(TSIArgArchive::LoadOutputArchive);\n";
    }
    bool isSimple = false;
    TSString paramStr;
    if (Func->DefTypeName != "void")
    {
        responceCode += Table() + "OutputArchive.Archive(Deser,Item);\n";
        responceCode += Table() + TSBuiltinPlugin::GetSuitFullTypeName(Func->Def,false,ClassIdentity) + " rets = Item.Arg.Value<" +
            TSBuiltinPlugin::GetSuitFullTypeName(Func->Def,false,ClassIdentity) + ">();\n\n";
        TSString retStr = GetRecourseCode(Func->Def,"ret",isSimple);
        if (!isSimple)
        {
            responceCode += "	" + TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + " ret;\n";
            paramStr += ",ret";
        }
        else
        {
            paramStr += ",rets";
        }
        boost::algorithm::replace_all(retStr , "aTypes" , "rets");
        responceCode += retStr;
    }
    handlerCode = responceCode;
    for(size_t i = 0; i < outParams.size(); ++i)
    {
        responceCode += Table() + "{\n";
        responceCode += Table(2) + "OutputArchive.Archive(Deser,Item);\n";
        responceCode += Table(2) +  TSBuiltinPlugin::GetSuitFullTypeName(outParams[i]->Def,false,ClassIdentity) + " aTypes = \n\
                                                                                                                  Item.Arg.Value<" + TSBuiltinPlugin::GetSuitFullTypeName(outParams[i]->Def,false,ClassIdentity) + ">();\n";
        bool istrue = true;
        responceCode += GetRecourseCode(outParams[i]->Def , outParams[i]->ValName , istrue);
        responceCode += Table() + "}\n";
        handlerCode += Table() + TSBuiltinPlugin::GetSuitFullTypeName(outParams[i]->Def,false,ClassIdentity) + " " + outParams[i]->ValName + ";\n";
        handlerCode += Table() + "{\n";
        handlerCode += Table(2) + "OutputArchive.Archive(Deser,Item);\n";
        handlerCode += Table(2) + TSBuiltinPlugin::GetSuitFullTypeName(outParams[i]->Def,false,ClassIdentity) + " aTypes = \n\
                                                                                                                Item.Arg.Value<" + TSBuiltinPlugin::GetSuitFullTypeName(outParams[i]->Def,false,ClassIdentity) + ">();\n";
        istrue = true;
        handlerCode += GetRecourseCode(outParams[i]->Def , outParams[i]->ValName , istrue);
        handlerCode += "	}\n";
        paramStr += "," + outParams[i]->ValName;
    }
    result += "\n	TSRpcResponse::DataTypePtr Response = _pProxy->GetOperation()->InvokeOperation(Request);\n\n";
    result += responceCode;
    if (Func->DefTypeName != "void")
    {
        isSimple ? result += Table() + "return rets;\n" : result += " return ret;\n"; 
    }
    result += "}" + TSIDL_NEWLINE + TSIDL_NEWLINE;
    result += TSIDLUtils::GetFuncAsyncStr(Func,"RemoteMethod") + "\n";
    result += "{" + TSIDL_NEWLINE;
    result += AsyncRequestCode;
    result += "	_pProxy->GetOperation()->InvokeOperationAsync(Request,new " + 
        TSIDLUtils::GetFuncAsyncCallBackStr(Func) + "(func),Timeout);\n";
    result += "}\n\n";
    result += TSIDLUtils::GetFuncAsyncHandlerStr(Func,"RemoteMethod") + "\n";
    result += "{\n";
    if (Func->DefTypeName != "void" || outParams.size())
    {
        result += Table() + "TSArgArchiveItem Item;\n";
        result += Table() + "Item.MethodName = TS_TEXT(\"" + Func->ValName + "\");\n\n";
    }
    result += handlerCode;
    result += "	(*(" + TSIDLUtils::GetFuncAsyncCallBackStr(Func) + "*)Invoker)(Error" + paramStr + ");\n";
    result += "	delete Invoker;\n";
    result += "}\n\n";

    return result;
}

TSString TSGenSupportPlugin::GetRecourseCode( TSTypeDefPtr TypeDef,TSString Name, bool & IsSimple )
{
    TSString code;
    if (TypeDef->Type == TSTypeDef::Array)
    {
        TSVectorBuiltinTypeDefPtr vectorResouce = TS_CAST(TypeDef,TSVectorBuiltinTypeDefPtr);
        code += GetRecourseCode(vectorResouce->Value, Name , IsSimple);
    }
    else if (TypeDef->Type == TSTypeDef::Builtin)
    {
        IsSimple ? code += Table(2) + Name + " = aTypes;\n" : code;
        IsSimple = true;
    }

    return code;
}

TSString TSGenSupportPlugin::GetFunAndHandleCode( TSFunctionDefPtr Func,const TSString & Namespace )
{
    bool isHaveOut = false;
    if (Func->DefTypeName != "void")
    {
        isHaveOut = true;
    }
    for (size_t i = 0; i < Func->Params.size(); ++i)
    {
        if (Func->Params[i]->Inout || Func->Params[i]->Out)
        {
            isHaveOut = true;
            break;
        }
    }
    //func
    TSString result;
    result += TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + TSIDL_BLANK + Namespace + 
        "::" + TSIDLUtils::GetFuncWithoutResultStr(Func) + "\n";
    result += "{\n";
    result += Table();
    if (Func->DefTypeName != "void")
    {
        result += "return ";
    }
    result += "_Method->" + TSIDLUtils::GetFuncCallStr(Func) + ";\n";
    result += "}\n\n";

    //func handler
    result += TSIDLUtils::GetFuncHandlerStr(Func,"Servant") + "\n";
    result += "{\n";
    result += Table() + "TSArgArchiveItem Item;\n";
    result += Table() + "Item.MethodName = TS_TEXT(\"" + Func->ValName + "\");\n";
    result += Table() + "TSSerializer Deser;\n";
    result += Table() + "U8ToBuffer(Request->Context,Deser.GetBuffer());\n";
    result += "\n";
    result += Table() + "MethodArchive InputArchive(TSIArgArchive::LoadInputArchive);\n";
    std::vector<TSParamDefPtr> outParams;
    for(size_t i = 0; i < Func->Params.size() ;++i)
    {
        if(TSParamDefPtr param = Func->Params[i])
        {
            if(param->Type == TSParamDef::Out||param->Type == TSParamDef::Inout)
            {
                outParams.push_back(param);
            }

            result += Table() + "InputArchive.Archive(Deser,Item);\n";

            if (param->Type == TSParamDef::Out)
            {
                result += Table() + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + TSIDL_BLANK + 
                    param->ValName + ";\n";
            }
            else
            {
                result += RecourseGetCode(param); 
            }
        }
    }
    if (Func->DefTypeName != "void")
    {
        result += Table() + TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + " ret = ";
    }
    result += TSIDLUtils::GetFuncCallStr(Func) + ";\n";
    result += "   TSRpcResponse::DataTypePtr Response = boost::make_shared<TSRpcResponse::DataType>();\n";
    result += "   Response->Handle = Request->Handle;\n";
    result += "   Response->Directive = Request->Directive;\n";
    result += "   Response->Seq = Request->Seq;\n\n";
    if (isHaveOut)
    {
        result += "   TSSerializer Ser;\n";
        result += "   MethodArchive OutputArchive(TSIArgArchive::SaveOutputArchive);\n";
        if (Func->DefTypeName != "void")
        {
            result += TSIDLUtils::GetOutParamCode(Func->Def , "ret" , Func->Def);
        }
        for(size_t i = 0; i < outParams.size(); ++i)
        {
            result += GetOutParamCode(outParams[i]->Def , outParams[i]->ValName , outParams[i]->Def);
        }
        result += Table() + "BufferToU8(Response->Context,Ser.GetBuffer()->Data(),Ser.GetBuffer()->Length());\n";
    }
    result += Table() + "return Response;\n";
    result += "}\n\n";
    return result;
}

TSString TSGenSupportPlugin::RecourseGetCode( TSParamDefPtr Param )
{
    TSString code;
    if (Param->Def->Type == TSTypeDef::Builtin)
    {
        code += Table() + TSBuiltinPlugin::GetSuitFullTypeName(Param->Def) + TSIDL_BLANK + Param->ValName + " = " +
            "Item.Arg.Value<" + TSBuiltinPlugin::GetSuitFullTypeName(Param->Def) + ">();\n";
    }
    else if (Param->Def->Type == TSTypeDef::Array)
    {
        TSVectorBuiltinTypeDefPtr vecParam = TS_CAST(Param->Def,TSVectorBuiltinTypeDefPtr);
        if (vecParam)
        {
            code += Table() + "std::vector<" + TSBuiltinPlugin::GetSuitFullTypeName(vecParam->Value) + "> " +
                Param->ValName + " = Item.Arg.Value<std::vector<" + TSBuiltinPlugin::GetSuitFullTypeName(vecParam->Value) + ">>();\n";
        }

    }
    return code;
}

void TSGenSupportPlugin::BuildMethodImpl()
{
    TSProjFilePtr MethodImplHeaderFile;
    TSProjFilePtr MethodImplCppFile;
    TSProjFilePtr MethodImplFactoryHeaderFile;
    TSProjFilePtr MethodFactoryImplFile;

    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	//std::map<TSString, TSString> serviceMap;
	////获得服务和其依赖的结构体存入serviceMap容器中
	//GetServiceAndStructMap(TypeDefs, serviceMap);

	////获取主题选择工具中选中的服务
	//std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	//std::vector<TSString> tempServiceVec;
	////获得要生成的服务，如果追加的服务为空则返回true
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildMethodImplVec, _serviceFlagStruct->buildMethodImpl);

	//std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	////将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

	//bool deleteEndIfFlag = false;
    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
		if (TypeDef->Type == TSTypeDef::Struct)
		{
			continue;
		}
		/*else if (TypeDef->Type == TSTypeDef::Service)
		{
			if (tempServiceFlag)
			{
				continue;
			}
			if (!vecSelectService.empty())
			{
				if (vecSelectService[0] == "N")
				{
					continue;
				}
			}
		}*/

        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
            || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            TSString ParseFileBaseName = 
                GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSString Path = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
				TSString fullPath = Solution->GetFullPath();
				boost::algorithm::replace_all(fullPath, "\\", "\/");

                boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
                if (Path == ParseFileBaseName)
                {
                    Path = ParseFileBaseName + "." + Path;
                }
                TSProjectPtr Dir  = Solution->GetOrCreateProj(Path);
                Dir->SetProFileType(Impl);
                MethodImplHeaderFile = Dir->GetOrCreateProjFile("MethodImpl",".h");
				MethodImplCppFile = Dir->GetOrCreateProjFile("MethodImpl", ".cpp");
				
				TSString implPath = Path;
				boost::algorithm::replace_all(implPath, "::", ".");
				implPath = fullPath + "/impl/" + implPath;
				//设置impl目录路径
				TSIDLPluginMgr::Instance()->SetImplFullPath(implPath);
				//if (!_serviceFlagStruct->buildMethodImpl) //第一次生成MethodImpl.h
				//{
					GenerateMethodImplHeader(MethodImplHeaderFile, TypeDef, indexServiceVec);
					MethodImplHeaderFile->AppendFirst(TSIDLUtils::GenHeadUUID("MethodImpl.h"));
					MethodImplHeaderFile->AppendLast("#endif //");
					MethodImplHeaderFile->SetFileType(Impl);
					GenerateMethodImplCPP(MethodImplCppFile, TypeDef, indexServiceVec);
					MethodImplCppFile->SetFileType(Impl);
				//}
				//else if (!tempServiceFlag) //第二次以后生成MethodImpl.h
				//{
					//获取定义的函数
					TSString functions = GetFunctions(TypeDef, indexServiceVec);
					//TSString str = MethodImplHeaderFile->GetContent();
					//int pos = str.find("private:");
					////将获取的函数插入到当前文件的特定位置
					//str.insert(pos, functions);
					//MethodImplHeaderFile->SetContent(str);

					BOOST_FOREACH(TSFunctionDefPtr Func, TypeDef->Functions)
					{
						/*TSString funName = Func->ValName;
						std::vector<TSString>::iterator iterTopic = std::find(indexServiceVec.begin(), indexServiceVec.end(), funName);
						if (iterTopic == indexServiceVec.end())
						{
							continue;
						}*/
						functions = TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + " MethodImpl::" +
							TSIDLUtils::GetFuncWithoutResultStr(Func) + "\n";
						functions += "{\n";
						functions += Table() + "//user    code\n";
						if (Func->DefTypeName != "void")
						{
							functions += Table() + "return " + TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + "();\n";
						}
						functions += "}\n\n";

						/*str = MethodImplCppFile->GetContent();
						std::cout << "functions: " << functions << std::endl;
						pos = str.find("}\n}\n}");
						str.insert(pos, functions);
						std::cout << "str1: " << str << std::endl;
						MethodImplCppFile->SetContent(str);*/
					}
				//}

                if (TypeDef->Type != TSTypeDef::Service)
                {
                    MethodImplFactoryHeaderFile = Dir->GetOrCreateProjFile("MethodFactoryImpl",".h");
                    GenerateMethodFactoryImplHeader(MethodImplFactoryHeaderFile,TypeDef);
                    MethodImplFactoryHeaderFile->SetFileType(Impl);
                    MethodImplFactoryHeaderFile->AppendFirst(TSIDLUtils::GenHeadUUID("MethodFactoryImpl.h"));
                    MethodImplFactoryHeaderFile->AppendLast("#endif //");

                    MethodFactoryImplFile = Dir->GetOrCreateProjFile("MethodFactoryImpl",".cpp");
                    MethodFactoryImplFile->SetFileType(Impl);
                    GenerateMethodFactoryImplCPP(MethodFactoryImplFile,TypeDef);
                }
            }
        }
		//_serviceFlagStruct->buildMethodImpl = true;
    }
}

void TSGenSupportPlugin::GenerateMethodImplHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName )
{
    TSString content = IMPL_HEADER_CODE;
    if(TypeDef->Type == TSTypeDef::Service)
    {
        content = METHODIMPL_SERVICE_HEADER_CODE;
    }
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSString fileName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    //TSString base = GetBaseName(fileName);
    TSString typeName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(typeName,"::",".");
    boost::algorithm::replace_all(content,"_TYPES_NAME_",typeName);

    TSString classinlude = "#include <generic/" + typeName + "/Servant.h>" + TSIDL_NEWLINE;
    TSString exporth = "#include<impl/" + typeName + "/Export.h>\n";
    boost::algorithm::replace_all(content , "_EXPORT_" , exporth);
    TSString FileNameUpper = typeName;
    boost::algorithm::to_upper(FileNameUpper);
    boost::algorithm::replace_all(FileNameUpper,".","_");
    boost::algorithm::replace_all(content , "_TYPE_PLUSE_NAME_" , FileNameUpper + "_DECL");

    TSString function = GetFunctions(TypeDef, serviceName);
    if (TypeDef->Extend.lock())
    {
        TSTypeDefWithFuncAndFiledPtr extenfun = TS_CAST(TypeDef->Extend.lock(),TSTypeDefWithFuncAndFiledPtr);
        function += GetFunctions(extenfun, serviceName);
    }
    boost::algorithm::replace_all(content , "_FUNCS_" , function);
    if (TypeDef->Type == TSTypeDef::Service)
    {
    }
    else
    {
        boost::algorithm::replace_all(content, "__PARAMETER__", FACTORY_CREATE_MESSAGE);
        boost::algorithm::replace_all(content, "__FUNCSPRIVATE__", FUNC_PRIVATE_MESSAGE);
        boost::algorithm::replace_all(content, "__CLASS_INCLUDE__", "");
    }
    ProjFile->AppendLast(content);

}

void TSGenSupportPlugin::GenerateMethodImplCPP( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName )
{
    TSString content = IMPL_CPP_CODE;
    if (TypeDef->Type == TSTypeDef::Service)
    {
        content = SERVICE_IMPL_CPP_CODE;
        content+= SERVICE_FOR_CREATE;
    }
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSString serverName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(content, "_SERVICE_NAME_", serverName);
    TSString serverNamell = serverName;
    boost::algorithm::replace_all(serverNamell, "::", "");
    boost::algorithm::replace_all(content,"_SERVICE_NAMELL_",serverNamell);
    TSString Include = "#include <generic/" + TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "/ServerSkel.h>\n";
    boost::algorithm::replace_all(Include, "::", ".");
    boost::algorithm::replace_all(content, "__INCLUDE__", Include);
    TSString FileNameUpper = serverName;
    boost::algorithm::to_upper(FileNameUpper);
    boost::algorithm::replace_all(FileNameUpper,"::","_");
    boost::algorithm::replace_all(content , "_TYPE_PLUSE_NAME_" , FileNameUpper + "_DECL");
    TSString funcCode;
    if (TypeDef->Extend.lock())
    {
        TSTypeDefWithFuncAndFiledPtr extenfun = TS_CAST(TypeDef->Extend.lock(),TSTypeDefWithFuncAndFiledPtr);
        BOOST_FOREACH(TSFunctionDefPtr Func,extenfun->Functions)
        {
			/*TSString funName = Func->ValName;
			std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
			if (iterTopic == serviceName.end())
			{
				continue;
			}*/
            funcCode += TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + " MethodImpl::" + 
                TSIDLUtils::GetFuncWithoutResultStr(Func) + "\n";
            funcCode += "{\n";
            funcCode += Table() + "";
            if (Func->DefTypeName != "void")
            {
                funcCode += Table() + "return " + TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + "();\n"; 
            }
            funcCode += "}\n\n";  
        }

    }
    BOOST_FOREACH(TSFunctionDefPtr Func,TypeDef->Functions)
    {
		/*TSString funName = Func->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
        funcCode += TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + " MethodImpl::" + 
            TSIDLUtils::GetFuncWithoutResultStr(Func) + "\n";
        funcCode += "{\n";
        funcCode += Table() +"//user    code\n";
        if (Func->DefTypeName != "void")
        {
            funcCode += Table() + "return " + TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + "();\n"; 
        }
        funcCode += "}\n\n";  
    }
    boost::algorithm::replace_all(content, "_FUNCS_", funcCode);

    if(TypeDef->Type == TSTypeDef::Service)
    {
        boost::algorithm::replace_all(content, "__PARAMETER__", FACTORY_CREATE_SERVICE);
    }
    else
    {
        boost::algorithm::replace_all(content, "__PARAMETER__", FACTORY_CREATE_MESSAGE);
        boost::algorithm::replace_all(content, "__FUNCSPRIVATE_CPP", "");
    }
    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateMethodFactoryImplHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = FACTORYIMPL_HEADER_CODE;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    boost::algorithm::replace_all(content , "_EXPORT_" , "#include \"Export.h\"\n");

    if(TypeDef->Type == TSTypeDef::Service)
    {

    }
    else
    {
        boost::algorithm::replace_all(content, "__PARAMETER__", FACTORY_CREATE_MESSAGE);
    }
    TSString typeName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(typeName,"::",".");
    boost::algorithm::replace_all(content,"_TYPES_NAME_",typeName);
    TSString fileName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateMethodFactoryImplCPP( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = FACTORYIMPL_CPP_CODE;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSString typeName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    TSString packages = typeName;
    boost::algorithm::replace_all(typeName,"::",".");
    boost::algorithm::replace_all(content,"_TYPES_NAME_",typeName);
    TSString topicName = TSIDLUtils::GetTopicName(TypeDef);
    boost::algorithm::replace_all(content,"_TOPIC_NAME_",topicName);
    TSString fileName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    boost::algorithm::replace_all(content,"_PACKAGES_",packages);
    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::BuildCallBackImpl( TSTypeDefPtr TypeDef,const TSString & FileName )
{
    if (TypeDef->Type == TSTypeDef::Struct)
    {
        return;
    }
    if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
        || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
    {
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
        if(TSSolutionPtr Solution = 
            SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
        {
            TSProjectPtr Dir  = Solution->GetOrCreateProj(FileName);

            TSString Path = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
            boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
            TSProjFilePtr CallBackImplHeaderFile = Dir->GetOrCreateProjFile(Path + "/CallBackImpl",".h");
            CallBackImplHeaderFile->SetFileType(Impl);
            TSProjFilePtr CallBackImplFile = Dir->GetOrCreateProjFile(Path + "/CallBackImpl",".cpp");
            CallBackImplFile->SetFileType(Impl);
            TSProjFilePtr SubscribeFactoryImplHeaderFile = Dir->GetOrCreateProjFile(Path + "/SubscriptionFactoryImpl",".h");
            TSProjFilePtr SubscribeFactoryImplFile = Dir->GetOrCreateProjFile(Path + "/SubscriptionFactoryImpl",".cpp");
            SubscribeFactoryImplHeaderFile->SetFileType(Impl);
            SubscribeFactoryImplFile->SetFileType(Impl);
            if(TSTypeDefWithFuncAndFiledPtr typeDef = TS_CAST(TypeDef,TSTypeDefWithFuncAndFiledPtr))
            {
                GenerateCallBackHeaderImpl(CallBackImplHeaderFile, typeDef);
                CallBackImplHeaderFile->AppendFirst(TSIDLUtils::GenHeadUUID("CallBackImpl.h"));
                CallBackImplHeaderFile->AppendLast("#endif //");

                GenerateCallBackImpl(CallBackImplFile,typeDef);

                GenerateSubscribeFactoryHeaderImpl(SubscribeFactoryImplHeaderFile,typeDef);
                SubscribeFactoryImplHeaderFile->AppendFirst(TSIDLUtils::GenHeadUUID("SubscriptionFactoryImpl.h"));
                SubscribeFactoryImplHeaderFile->AppendLast("#endif //");

                GenerateSubscribeFactoryImpl(SubscribeFactoryImplFile, typeDef);
            }

        }

    }
}

void TSGenSupportPlugin::GenerateCallBackImpl( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = CALLBACKIMPL_CPP_CODE;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateCallBackHeaderImpl( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = CALLBACKIMPL_HEADER_CODE;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSString typeName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(typeName,"::",".");
    boost::algorithm::replace_all(content, "_TYPES_NAME_", typeName);
    TSString fileName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    TSString relativepath = GetFileRelativePath(TypeDef);
    boost::algorithm::replace_all(content,"_RELATIVE_PATH",relativepath);
    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateSubscribeFactoryImpl( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = SUBSCRIPTION_IMPLCPP_CODE;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    boost::algorithm::replace_all(content , TSString("_ARGC_") , TSString(""));
    boost::algorithm::replace_all(content , TSString("ARGCS") , TSString(""));

    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateSubscribeFactoryHeaderImpl( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = SUBSCRIPTION_IMPLHEADER_CODE;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSString typeName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(typeName,"::",".");
    boost::algorithm::replace_all(content, "_TYPES_NAME_", typeName);
    TSString fileName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    boost::algorithm::replace_all(content, "ARGC,", "");
    ProjFile->AppendLast(content);
}

TSString TSGenSupportPlugin::GeneratePython(TSProjFilePtr ProjFile, TSProjFilePtr ProjHFile,TSTypeDefWithFuncAndFiledPtr TypeDef,TSTopicTypeDefPtr topic)
{

    TSString content = __PYTHON_WRAP__;
	TSString rnt = __PYTHON_REGISTER__;

	//PythonModule.cpp的.h头文件
	TSString pycppincludeh = __PYTHON_H_WRAP__;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSIDLUtils::ReplaceFullNameSpaceString("__NAMESPACE_BEGINS__",
        "__NAMESPACE_ENDS__",
        content,
        TypeDef,
        "",
        true);
	//std::cout << "pythonMoudle内容：" << std::endl << content << std::endl;
    if (topic)
    {
        TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,topic);
    }
    else
    {
        TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);
    }
	//std::cout << "pythonMoudle内容：" << std::endl << content << std::endl;

    TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);

    TSString selfname; 
    TSString descname;
    if (topic)
    {
        selfname = GetTopicName(topic);
        descname = topic->Desc;
    }
    else
    {
        selfname = GetTopicName(TypeDef);
        descname = selfname;
    }
    TSString full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
	//std::cout << "pythonMoudle内容：" << std::endl << content << std::endl;
    std::vector<TSString> Names = TSMetaType4Link::Split(full_type_name,PACKAGE_SPLIT,false);
    TSString PackName;
    BOOST_FOREACH(const TSString&item,Names)
    {
        PackName += item;
        PackName += "::";
    }
    boost::algorithm::replace_all(content,"_SELF_NS",PackName);
	//std::cout << "pythonMoudle内容：" << std::endl << content << std::endl;
    boost::algorithm::replace_all(content,"_SELF_NAME",selfname);
	//std::cout << "pythonMoudle内容：" << std::endl << content << std::endl;
	boost::algorithm::replace_all(pycppincludeh, "_SELF_NAME", selfname);

    if(ProjFile != NULL)
    {
        ProjFile->AppendLast(content);
    }

	//向PythonModule.h写入函数声明
	if (ProjHFile != NULL)
	{
		ProjHFile->AppendLast(pycppincludeh);
	}

    boost::algorithm::replace_all(rnt,"_SELF_NS",PackName);
    boost::algorithm::replace_all(rnt,"_SELF_NAME",selfname);

    return rnt;
}

void TSGenSupportPlugin::BuildPythonService()
{
    TSProjFilePtr ServiceFile,ServiceFileProxy,ServiceFileServer;

    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildPythonServiceVec, _serviceFlagStruct->buildPythonService);

	std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefService;
    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)//先遍历一遍把服务的Type取出来
    {
        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
            || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            if(TypeDef->Type == TSTypeDef::Service)//服务才进来，不是服务则不进来
            {
                TypeDefService.push_back( TypeDef);
            }
        }
    }

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefService)//全是服务
    {
		if (!vecSelectService.empty())
		{
			if (vecSelectService[0] == "N")
			{
				continue;
			}
		}
        if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
            || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
              TSString ParseFileBaseName = 
                GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

                TSString Path = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
                boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
                if (Path == ParseFileBaseName)
                {
                    Path = ParseFileBaseName + "." + Path;
                }

                ServiceFile = Dir->GetOrCreateProjFile("PythonModuleService",".cpp");
                GeneratePythonService(ServiceFile, TypeDefService, indexServiceVec);
                ServiceFile->SetFileType(None);

                bool IsPython  = TSIDLPluginMgr::Instance()->GetGeneratePy();
                if( IsPython )
                {
                    Dir  = Solution->GetOrCreateProj(Path);
                    Dir->SetProFileType(Impl);
                    TSString tmp = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
                    boost::algorithm::replace_all(tmp,"::","");
                    ServiceFileProxy = Dir->GetOrCreateProjFile(tmp + "Server" ,".py");
                    GeneratePythonServerPy(ServiceFileProxy, TypeDef);
                    ServiceFileProxy->SetFileType(Impl);

                    ServiceFileServer = Dir->GetOrCreateProjFile(tmp + "Proxy",".py");
                    GeneratePythonProxyPy(ServiceFileServer, TypeDef);
                    ServiceFileServer->SetFileType(Impl);
                }
                    
                break;
            }
        }
    }//BOOST_FOREACH
}

void TSGenSupportPlugin::GeneratePythonService( TSProjFilePtr ProjFile, std::vector<TSTypeDefWithFuncAndFiledPtr> TypeDefs, std::vector<TSString>& serviceName)
{
    TSString content = __PYTHON_SERVICE_MODULE__;
    TSString includeString;
    TSString funcString;
    TSString pythonFunction ;
    TSString contentPythonModule = __PYTHON_SERVICE_MODULE_END__;
    TSString funcLast;
    TSString pythonParamWithStruct;//_TYPEFUNC_
    TSString pythonclass;
    std::vector<TSTypeDefWithFuncAndFiledPtr>  StructDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();
    pythonclass = GetPythonStructDef(StructDefs);
    boost::algorithm::replace_all(contentPythonModule,"_TYPEFUNC_", pythonclass);
    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef, TypeDefs)
    {
       pythonFunction += __PYTHON_SERVICE_WRAP__;
       funcLast += _FUNC_PYTHON_SERVICE_MODULE_PYPROXY_;
       TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
           __NAMESPACE_END__,
           pythonFunction,
           TypeDef);

       TSString includeStringtmp = __PYTHON_SERVICE_MODULE__HEADER__;

       boost::algorithm::replace_all(includeStringtmp,"PACKAGENAMEALL", TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true));
       boost::algorithm::replace_all(includeStringtmp,"::",".");

       includeString += includeStringtmp;

       TSString funcStringtmp = __PYTHON_SERVICE_MODULE__PROXY__SERVER__SETDOMAIN__WRAP__;

       TSString tmp = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);

       boost::algorithm::replace_all(funcStringtmp,"PACKAGENAME::SERVICENAME", tmp);
       boost::algorithm::replace_all(tmp,"::","_");
       boost::algorithm::replace_all(funcStringtmp,"PACKAGENAME_SERVICENAME", tmp);

       TSString funcGet;
       TSString funcVirtual;
       TSString funcDef,funcDefPy;
       BOOST_FOREACH(TSFunctionDefPtr funcPtr ,TypeDef->Functions)
       {
		   /*TSString funName = funcPtr->ValName;
		   std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		   if (iterTopic == serviceName.end())
		   {
			   continue;
		   }*/
           //_PYTHON_FUNC_
            if (funcPtr->Def->Type ==  TSTypeDef::Array)
            {
              funcGet +="             boost::python::list " + funcPtr->ValName + "Py";            
            }
            else
            {
              funcGet +="            " + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Def) + " " + funcPtr->ValName + "Py";
            }
            funcVirtual += "    virtual " + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Def) + " " + funcPtr->ValName;
            funcDef += "        .def(\"" + funcPtr->ValName + "\" ,&PACKAGENAME::SERVICENAME::Proxy::" + funcPtr->ValName + ")\n";
            funcDefPy +=  "        .def(\"" + funcPtr->ValName + "Py\", &PACKAGENAME::SERVICENAME::PyProxy::" + funcPtr->ValName + "Py)\n";
            TSString funcParams,funcOutParams,funcOutParamsEnd,funcParamsWithoutType,funcParamsVirtual,funcOutParamsVirtual,funcOutParamsVirtualEnd;
            //遍历拿到函数参数
            funcParams = "(";
            funcParamsWithoutType = "(";
            funcParamsVirtual = "(";
            TSString funcCheckError;
            for(int i = 0; i < funcPtr->Params.size(); i++)
            {     
                if(funcPtr->Params[i]->Type == TSParamDef::Inout && funcPtr->Def->Type !=  TSTypeDef::Array)
                {
                    funcCheckError += "              if(boost::python::len(py" + funcPtr->Params[i]->ValName + ") > 1)\n";
                        funcCheckError += "              {\n                 std::cout<<\"WARNING: " + funcPtr->Params[i]->ValName +  "赋值越界,默认传递第一个元素\"<<std::endl;\n              }\n";
                }
                if(funcPtr->Params[i]->Type == TSParamDef::In)
                {
                      if (funcPtr->Params[i]->Def->Type ==  TSTypeDef::Array)
                      {
                          funcParams += "boost::python::list " + funcPtr->Params[i]->ValName;
                          funcParamsVirtual += "const boost::python::list &" + funcPtr->Params[i]->ValName;

                          TSVectorBuiltinTypeDefPtr tmpArrayType = TS_CAST(funcPtr->Params[i]->Def,TSVectorBuiltinTypeDefPtr);
                          if( tmpArrayType != NULL)
                          {
                              funcOutParams += "              " + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Params[i]->Def) + " py" + funcPtr->Params[i]->ValName + " = ConvertPList<" + TSBuiltinPlugin::GetSuitFullTypeName(tmpArrayType->Value) + ">(" + funcPtr->Params[i]->ValName + ");\n";                 
                              funcParamsWithoutType += "py" + funcPtr->Params[i]->ValName;
                          }
                      }
                      else
                      {
                          funcParamsWithoutType += funcPtr->Params[i]->ValName;
                          funcParams += TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Params[i]->Def) + " " + funcPtr->Params[i]->ValName;
                          funcParamsVirtual += "const " + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Params[i]->Def) + " &" + funcPtr->Params[i]->ValName;
                      }
                   
                }
                else
                {
                    funcParamsWithoutType += funcPtr->Params[i]->ValName;
                    funcParams += "boost::python::list &  py" + funcPtr->Params[i]->ValName;
                    funcParamsVirtual +=  TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Params[i]->Def) + " &" + funcPtr->Params[i]->ValName;

                    if(funcPtr->Params[i]->Def->Type ==  TSTypeDef::Array)
                    {
                        TSVectorBuiltinTypeDefPtr tmpArrayType = TS_CAST(funcPtr->Params[i]->Def,TSVectorBuiltinTypeDefPtr);
                        if( tmpArrayType != NULL)
                        {
                            funcOutParams += "              " + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Params[i]->Def) + " " + funcPtr->Params[i]->ValName + " = ConvertPList<" + TSBuiltinPlugin::GetSuitFullTypeName(tmpArrayType->Value) + ">(py" + funcPtr->Params[i]->ValName + ");\n";  
                            funcOutParamsEnd += "              py" + funcPtr->Params[i]->ValName + " *= 0;\n";
                            funcOutParamsEnd += "              py" + funcPtr->Params[i]->ValName + ".extend(ConvertVector(" + funcPtr->Params[i]->ValName + "));\n";
                        }
                       
                    }
                    else
                    {
                        funcOutParams += "              " + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Params[i]->Def) + " " + funcPtr->Params[i]->ValName + " = extract<" + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Params[i]->Def) + ">(py" + funcPtr->Params[i]->ValName + "[0]);\n"; 
                        funcOutParamsEnd += "              py" + funcPtr->Params[i]->ValName + "[0] = " + funcPtr->Params[i]->ValName + ";\n";
                    }

                    funcOutParamsVirtual += "                    boost::python::list py" + funcPtr->Params[i]->ValName + ";\n";
                    funcOutParamsVirtual += "                    py" + funcPtr->Params[i]->ValName + ".append(" + funcPtr->Params[i]->ValName + ");\n";

                    funcOutParamsVirtualEnd += "                    " + funcPtr->Params[i]->ValName + " = extract<" + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Params[i]->Def) + ">(py" + funcPtr->Params[i]->ValName + "[0]);\n";
                }

                
                if( i != (funcPtr->Params.size()-1) )
                {
                    funcParams += ", ";
                    funcParamsWithoutType += ", ";
                    funcParamsVirtual += ", ";
                }
            }//for循环遍历Param

            funcParams += ")\n";
            funcParamsWithoutType += ")";
            funcParamsVirtual += ")\n";
            if (funcPtr->Def->Type ==  TSTypeDef::Array )
            {
               funcOutParams += "              " + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Def) + " cpprnt = " + funcPtr->ValName + funcParamsWithoutType + ";\n\n" + funcOutParamsEnd + "              return ConvertVector(cpprnt);\n";
            }
            else if(TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Def) != "void" )
            {
               funcOutParams += "              " + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Def) + " cpprnt = " + funcPtr->ValName + funcParamsWithoutType + ";\n\n" + funcOutParamsEnd + "              return cpprnt;\n";
            }
            else
            {
               funcOutParams += "              " + funcPtr->ValName + funcParamsWithoutType + ";\n\n" + funcOutParamsEnd + "              \n";
            }

           

            funcGet += funcParams + "            {\n" + funcCheckError + funcOutParams + "            }\n";

            funcVirtual += funcParamsVirtual + "            {\n                 if(boost::python::override f = this->get_override(\"" + funcPtr->ValName + "\"))\n                 {\n" + funcOutParamsVirtual +
                            "                    boost::python::detail::method_result rnt = f" + funcParamsWithoutType + ";\n" + funcOutParamsVirtualEnd
                            + "                    " + TSBuiltinPlugin::GetSuitFullTypeName(funcPtr->Def) + " test = rnt;\n                    return test;\n                 }\n                  //return  null;\n          }\n";

       }//boost，遍历服务里面的方法

       boost::algorithm::replace_all(pythonFunction,"_FUNC_GET_",funcGet);
       boost::algorithm::replace_all(funcStringtmp,"_FUNC_VIRTUAL_",funcVirtual);
       //funcDef,funcDefPy;
       funcString += funcStringtmp;
       boost::algorithm::replace_all(contentPythonModule,"SOLUTIONNAME", GetBaseName(TypeDef->Parse.lock()->GetFullFileName()));

       boost::algorithm::replace_all(funcLast,"_DEFFUNC_",funcDef);
       boost::algorithm::replace_all(funcLast,"_DEFFUNCPY_",funcDefPy);

       tmp = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
       boost::algorithm::replace_all(funcLast,"PACKAGENAME::SERVICENAME", tmp);
       boost::algorithm::replace_all(tmp,"::","_");
       boost::algorithm::replace_all(funcLast,"PACKAGENAME_SERVICENAME", tmp);
       
    }//遍历所有的服务
    
    boost::algorithm::replace_all(contentPythonModule,"_FUNC_PYPROXY_", funcLast);

    TSString EnumCon;
    std::vector<TSEnumDefPtr> EnumDefs = _p->_BuiltinPlugin->GetAllEnumDefs();
    BOOST_FOREACH(TSEnumDefPtr EnumDef, EnumDefs)
    {
        if (EnumDef->Type != TSTypeDef::Enum)
        {
            continue;
        }

        TSString ParseFileBaseName = 
            GetBaseName(EnumDef->Parse.lock()->GetFullFileName());
        if(!TSIDLPluginMgr::Instance()->IsImportParse(EnumDef->Parse.lock()) || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {

                TSString full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(EnumDef,true);
                std::vector<TSString> Names = TSMetaType4Link::Split(full_type_name,PACKAGE_SPLIT,false);
                TSString PackName;
                TSString PackNameAS;
                for (unsigned int i = 0; i < Names.size(); i++)
                {
                    TSString item = Names[i];
                    PackName += item;
                    PackNameAS += item;
                    if (i < Names.size())
                    {
                        PackNameAS += '_';
                    }

                    PackName += "::";
                }

                EnumCon += "enum_<" + PackName + EnumDef->TypeName + ">(\"" + PackNameAS + EnumDef->TypeName + "\") \n";
                for(size_t i = 0; i < EnumDef->Fileds.size(); ++i)
                {
                    if(TSEnumFiledDefPtr enumVal = EnumDef->Fileds[i])
                    {
                        EnumCon += TSIDL_2TABLE + ".value(\"_VALUE_AS\",_VALUE_NAME)\n";
                        boost::algorithm::replace_all(EnumCon,"_VALUE_NAME",PackName + enumVal->ValName);
                        boost::algorithm::replace_all(EnumCon,"_VALUE_AS",PackNameAS + enumVal->ValName);
                    }
                }
                EnumCon += TSIDL_2TABLE + ".export_values();\n";

            }
        }
    }

    boost::algorithm::replace_all(contentPythonModule,"_ENUM_PROPERTY",EnumCon);
    if(ProjFile != NULL)
    {
        content += includeString;
        content += pythonFunction;
        content += funcString;
        content += contentPythonModule;
        ProjFile->AppendLast(content);
    }
}

TSString TSGenSupportPlugin::GetPythonStructDef( std::vector<TSTypeDefWithFuncAndFiledPtr> StructDefs )
{
    TSString pythonclass,pythonclassExtend;
    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef, StructDefs)
    {

        if (TypeDef->Type != TSTypeDef::Struct)
        {
            continue;
        }
        pythonclass += __PYTHON_CLASS__;
        boost::algorithm::replace_all(pythonclass,"_SELF_NAME",TypeDef->TypeName);

        TSString propty;

        BOOST_FOREACH(TSFieldDefPtr field,TypeDef->Fileds)
        {
            if(field!=NULL)
            {
                propty += __PYTHON_BASIC_DATA__;

                boost::algorithm::replace_all(propty,"_SELF_NAME",TypeDef->TypeName);
                boost::algorithm::replace_all(propty,"_SELF_ATTR",field->ValName);
            }    
        }

        TSString supername = "";
        if (TypeDef->Extend.lock())
        {
            supername += ",bases<";
            supername += TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
            supername += "::DataType> ";
        }
        TSString full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
        std::vector<TSString> Names = TSMetaType4Link::Split(full_type_name,PACKAGE_SPLIT,false);
        TSString PackName;
        TSString PackNameAS;

        for (unsigned int i = 0; i < Names.size(); i++)
        {
            TSString item = Names[i];
            PackName += item;
            PackNameAS += item;
            if (i < Names.size()-1)
            {
                PackNameAS += '_';
            }

            PackName += "::";
        }

        boost::algorithm::replace_all(pythonclass,"_SUPER_NS",supername);
        boost::algorithm::replace_all(pythonclass,"_PROPERTY_NAME",propty);
        boost::algorithm::replace_all(pythonclass,"_SELF_NS",PackName);
        boost::algorithm::replace_all(pythonclass,"_SELF_CLASS",PackNameAS);
        boost::algorithm::replace_all(pythonclassExtend,"_SELF_NS",PackName);
        boost::algorithm::replace_all(pythonclassExtend,"_SELF_CLASS",PackNameAS);

    }
    return pythonclass;
}

void TSGenSupportPlugin::GeneratePythonServerPy( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = __PYTHON_SERVICE_SERVER__;
    TSString registerString,methodImpFunc,methodImpWhihPackage;
 
    TSString tmp = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    
    boost::algorithm::replace_all(content,"PACKAGENAME", tmp);
    boost::algorithm::replace_all(tmp,"::","_");
    boost::algorithm::replace_all(content,"PACKAGEALLNAME", tmp);
    boost::algorithm::replace_all(content,"PACKAGE", tmp);
    BOOST_FOREACH(TSFunctionDefPtr funcPtr ,TypeDef->Functions)
    {
        registerString += "    App.RegisterMethod(\"" + tmp + "_" + funcPtr->ValName + "\"," + tmp + "_" + "MethodImp." + funcPtr->ValName + ");\n";

        TSString funcparam,funcparamWithself;
        funcparamWithself += "self";
        for(int i = 0; i < funcPtr->Params.size(); i++)
        {
            funcparam += funcPtr->Params[i]->ValName;
            funcparamWithself += funcPtr->Params[i]->ValName;

            if( i < (funcPtr->Params.size() - 1))
            {
                funcparam += ",";
                funcparamWithself += ",";
            }
        }

        TSString rnt = "        rnt = \" \"\n        return rnt\n\n";
     //   methodImpFunc += "    def " + funcPtr->ValName + "(" + funcparamWithself + "):\n" + rnt;
        methodImpWhihPackage += "    @staticmethod\n    def " + funcPtr->ValName + "(" + funcparam + "):\n" + rnt;

    }
    boost::algorithm::replace_all(content,"_REGISTERMETHOD_", registerString);
    boost::algorithm::replace_all(content,"_PYTHONFUNCWHITOUTSELF_", methodImpWhihPackage);
    boost::algorithm::replace_all(content,"_PYTHONFUNC_", methodImpFunc);
    boost::algorithm::replace_all(content,"SOLUTIONNAME", GetBaseName(TypeDef->Parse.lock()->GetFullFileName()));

    if(ProjFile != NULL)
    {
        ProjFile->AppendLast(content);
    }
}

void TSGenSupportPlugin::GeneratePythonProxyPy( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = __PYTHON_SERVICE_PROXY__;
    TSString funcName,funcParam,funcparamInit;
    BOOST_FOREACH(TSFunctionDefPtr funcPtr ,TypeDef->Functions)
    {
        funcName = funcPtr->ValName;
        funcParam += "(";
        for(int i =0; i <funcPtr->Params.size(); i ++)
        {
            funcparamInit += "            " + funcPtr->Params[i]->ValName + " = \" \"\n";

            funcParam += funcPtr->Params[i]->ValName;

            if( i < (funcPtr->Params.size() - 1))
            {
                funcParam += ",";
            }

        }

        funcParam += ")";
        boost::algorithm::replace_all(content,"_FUNCNAME_", funcName);
        boost::algorithm::replace_all(content,"_FUNCPARAM_", funcParam);
        break;
    }


    TSString tmp = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);    
    boost::algorithm::replace_all(content,"_PARAMSINIT_", funcparamInit);
    boost::algorithm::replace_all(content,"PACKAGENAME", tmp);
    boost::algorithm::replace_all(tmp,"::","_");
    boost::algorithm::replace_all(content,"PACKAGE", tmp);
    boost::algorithm::replace_all(content,"SOLUTIONNAME", GetBaseName(TypeDef->Parse.lock()->GetFullFileName()));
    if(ProjFile != NULL)
    {
        ProjFile->AppendLast(content);
    }
}

TSString TSGenSupportPlugin::InvokerAndInitializeFunc( TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName,bool IsService, bool buildProxyFlag)
{
    TSString funcString = "";
    TSString returnString = "";
    if( IsService )
    {
		if (!buildProxyFlag)
		{
			funcString = "bool ServerSkel::Initialize( const TSString & DomainId )\n{\n" + Table() + "if(TSRPCServer::Initialize(DomainId))\n" + Table() + "{\n";
		}
       
       BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
       {
		   /*TSString funName = Function->ValName;
		   std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		   if (iterTopic == serviceName.end())
		   {
			   continue;
		   }*/
           TSString outStr,inStr,funcStr,pythonFun, funParamsPython, pythonFuncOutEnd;
		   TSString tmp = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef, true);
		   pythonFun += "#ifdef _PYTHON\n";
		   pythonFun += "PyGILThreadLock Pylock;\n";
		   pythonFun += "boost::python::object obj = TSPythonManager::Instance()->GetMethod(\"" + tmp + "_" + Function->ValName + "\");\n";
           for (unsigned int i = 0; i < Function->Params.size(); i++)
           {
               TSParamDefPtr param = Function->Params[i];

               if( param->Type == TSParamDef::In)
               {
                    funcStr +="Req->" + param->ValName + ",";                    
               }
               else if( param->Type == TSParamDef::Out)
               {
                    outStr +=  Table() + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + TSIDL_BLANK +param->ValName + ";\n";
                    inStr += Table() + "Resp->" + param->ValName + " = " + param->ValName + ";\n";
                    funcStr +=param->ValName + ",";

               }
               else if( param->Type == TSParamDef::Inout)
               {
                    inStr += Table() + "Resp->" + param->ValName + " = Req->" + param->ValName + ";\n";
                    funcStr +="Req->" + param->ValName + ",";
               }

			   if (param->Type != TSParamDef::In)
			   {
				   pythonFun += "    boost::python:: list py" + param->ValName + ";\n";
				   funParamsPython += "py" + param->ValName;
				   if (param->Def->Type == TSTypeDef::Array)
				   {
					   TSVectorBuiltinTypeDefPtr tmpArrayType = TS_CAST(param->Def, TSVectorBuiltinTypeDefPtr);
					   if (tmpArrayType != NULL)
					   {
						   if (param->Type == TSParamDef::Inout)
						   {
							   pythonFun += "    py" + param->ValName + " = ConvertVector(Req->" + param->ValName + ");\n";
							   pythonFuncOutEnd += "    Req->" + param->ValName + " = ConvertPList<" + TSBuiltinPlugin::GetSuitFullTypeName(tmpArrayType->Value) + ">(py" + param->ValName + ");\n";
						   }
						   else
						   {
							   pythonFun += "    py" + param->ValName + " = ConvertVector(" + param->ValName + ");\n";
							   pythonFuncOutEnd += "    " + param->ValName + " = ConvertPList<" + TSBuiltinPlugin::GetSuitFullTypeName(tmpArrayType->Value) + ">(py" + param->ValName + ");\n";
						   }

						  
					   }
				   }
				   else
				   {
					   if (param->Type == TSParamDef::Inout)
					   {
						   pythonFun += "    py" + param->ValName + ".append(Req->" + param->ValName + ");\n";
						   pythonFuncOutEnd += "     Req->" + param->ValName + " = extract<" + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + ">(py" + param->ValName + "[0]);\n";
					   }
					   else
					   {
						   pythonFun += "    py" + param->ValName + ".append(" + param->ValName + ");\n";
						   pythonFuncOutEnd += "    " + param->ValName + " = extract<" + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + ">(py" + param->ValName + "[0]);\n";
					   }
					  
					   
				   }

			   }
			   else
			   {
				   funParamsPython += "Req->"+ param->ValName;
			   }
			   if (i != Function->Params.size() - 1)
			   {
				   funParamsPython += ",";
			   }
           }
		   pythonFun += "    boost::python::object rntobj = obj(" + funParamsPython + ");\n"; //+ pythonFuncOutEnd;
		   pythonFun += pythonFuncOutEnd;
		   if (TSBuiltinPlugin::GetSuitFullTypeName(Function->Def) != "void")
		   {
			   pythonFun += "    " + TSBuiltinPlugin::GetSuitFullTypeName(Function->Def) + " rnt = extract<" + TSBuiltinPlugin::GetSuitFullTypeName(Function->Def) + ">(rntobj);\n";
		   }
		   pythonFun += "#else\n";

		   
		   boost::algorithm::replace_all(tmp, "::", "_");
           returnString += "void " + Function->ValName + "RequestHandler(void * UserData,UINT32 RequestTopic,void * RequestDataIn,UINT32 ResponseTopic,void * ResponseDataOut )\n{\n";
           returnString += Table() + "ServerSkel * Skel = (ServerSkel*)UserData;\n" + Table() + Function->ValName + "::Request * Req = (" + Function->ValName + "::Request *)RequestDataIn;\n"
                            + outStr;
		   returnString += pythonFun + Table();
           if(TSBuiltinPlugin::GetSuitFullTypeName(Function->Def) == "void")
           {
			  
			   returnString += Table() + "Skel->GetMethodT<Method>()->" + Function->ValName
				   + "(" + funcStr + ");\n";
			   returnString += "#endif\n";
			   returnString += Table() + Function->ValName + "::Response * Resp = (" +Function->ValName
                   + "::Response *)ResponseDataOut;\n" + inStr + "}\n";
           }
           else
           {
			   returnString += Table() + TSBuiltinPlugin::GetSuitFullTypeName(Function->Def) + " rnt = Skel->GetMethodT<Method>()->" + Function->ValName
				   + "(" + funcStr + ");\n";
			   returnString += "#endif\n";
			   returnString += Table() + Function->ValName + "::Response * Resp = (" + Function->ValName
                                + "::Response *)ResponseDataOut;\n" + inStr + Table() + "Resp->rnt = rnt;\n}\n";
           }
		   if (!buildProxyFlag)
		   {
			   funcString += Table(2) + "RegisterMethod(\"" + Function->ValName + "\"," + GetTopicName(TypeDef) + "_" + Function->ValName + "_" + "Request,"
				   + GetTopicName(TypeDef) + "_" + Function->ValName + "_" + "Response,&" + Function->ValName + "RequestHandler," + "NULL);\n";
		   }
       }
    }
    else//代理
    {
		if (!buildProxyFlag)
		{
			funcString = "bool Proxy::Initialize( const TSString & DomainId )\n{\n" + Table() + "if(TSServerLocator::Initialize(DomainId))\n" + Table() + "{\n";
		}
       
       BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
       {
		   /*TSString funName = Function->ValName;
		   std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		   if (iterTopic == serviceName.end())
		   {
			   continue;
		   }*/
           returnString += "void " + Function->ValName + "AsyncResponseInvoker(void * UserData,UINT32 ResponseTopic,void * ResponseData,void * Invoker,bool IsTimeout)\n{\n"
                           + Table() + "if(ResponseTopic == " + GetTopicName(TypeDef) +"_" +  Function->ValName + "_Response)\n" + Table() + "{\n" + Table(2) + "if(!IsTimeout)\n" + Table(2) + "{\n"
                           + Table(3) + Function->ValName + "::Response * resp = (" + Function->ValName +"::Response*)ResponseData;\n"+ Table(3) + "(*(" + GetFuncAsyncCallBackStr(Function,true)
                           + "*)Invoker)(" + GetFuncOutParamsString(Function) + ");\n"+ Table(2) + "}\n" + Table(2) + "delete (" + GetFuncAsyncCallBackStr(Function,true) + "*)Invoker;\n" + Table() +"}\n}";
		   if (!buildProxyFlag)
		   {
			   funcString += Table(2) + "RegisterMethod(\"" + Function->ValName + "\"," + GetTopicName(TypeDef) + "_" + Function->ValName + "_" + "Request,"
				   + GetTopicName(TypeDef) + "_" + Function->ValName + "_" + "Response," + "NULL,&" + Function->ValName + "AsyncResponseInvoker);\n";
		   }
       }
    }
	if (!buildProxyFlag)
	{
		funcString += Table(2) + "return true;\n" + Table() + "}\n" + Table() + "return false;\n" + "}";
	}
    
    boost::algorithm::replace_all(returnString,",)",")");
    boost::algorithm::replace_all(funcString,",)",")");
    return returnString +"\n" + funcString + "\n";
}

TSString TSGenSupportPlugin::GetFuncParamsString( TSFunctionDefPtr Function )
{
    TSString paramsString = "";
    std::vector<TSParamDefPtr> Params = Function->Params;
    TSString reqArrow = "Req->";
    if( Params.size() == 0)
    {
        return paramsString;
    }
    else if( Params.size() == 1)
    {
        return reqArrow + Params[0]->ValName;
    }

    for(int i = 0; i < Params.size() - 1; i++ )
    {
        paramsString += reqArrow + Params[i]->ValName + ",";
    }
    paramsString += reqArrow + Params[Params.size() - 1]->ValName;
    return paramsString;
}

TSString TSGenSupportPlugin::GetFuncOutParamsString( TSFunctionDefPtr Function )
{
    TSString returnString;
    std::vector<TSParamDefPtr> Params = Function->Params;
    TSString resp = "resp->";

    if(Function->DefTypeName != "void")
    {
        returnString += resp + "rnt";
        if(Params.size() == 0 )
        {
            return returnString;
        }
        else
        {
            returnString += ",";
        }
    }

    for( int i = 0; i < Params.size(); i++)
    {
        if( Params.size() == 0)
        {
            break;
        }
        if( Params[i]->Type != TSParamDef::In)
        {
            returnString += resp + Params[i]->ValName + ",";
        }
    }
    return returnString;
}

TSString TSGenSupportPlugin::GetFuncInParamsString( TSFunctionDefPtr Function )
{
    TSString returnString;

    return returnString;
}

TSString TSGenSupportPlugin::GenTopicWithName( TSTypeDefWithFuncAndFiledPtr TypeDef,TSString TopicName, std::vector<TSString>& serviceName)
{
    TSString result;
    BOOST_FOREACH(TSFunctionDefPtr Func,TypeDef->Functions)
    {
		TSString funName = Func->ValName;
		/*std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/

        result += __DATA_TYPE_TOPIC__;
        TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
            __NAMESPACE_END__,
            result,
            TypeDef,
            funName);

        TSIDLUtils::ReplaceTypeFullName(__SUPER_TYPE__,
            result,TypeDef);

        TSIDLUtils::ReplaceInitList(__CONSTRUCT_LIST__,
            result,TypeDef);

        /*TSIDLUtils::ReplaceInitList(__FIELD_COPYASSIGN__,
            result,TypeDef);*/

        TSIDLUtils::ReplaceInitList(_EXTEND_,
            result,TypeDef);

        TSIDLUtils::ReplaceInitList(_EXTEND_,
            result,TypeDef);

        /*TSIDLUtils::ReplaceInitList(__FIELD_COPYASSIGN__,
            result,TypeDef);*/

        TSIDLUtils::ReplaceFiledsLessCompare(__FIELD_LESSCOMPARE__,
            "other",result,TypeDef);

        TSIDLUtils::ReplaceLinkerVersion(result,"_LINKER_VERSION_");

        if (TypeDef->Extend.lock())
        {
            TSString suit_full_type_name = "," + TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
            suit_full_type_name += "::DataType"; 
            boost::algorithm::replace_all(result,"_SUPER_VERSION_",suit_full_type_name);
        }
        else
        {
            boost::algorithm::replace_all(result,"_SUPER_VERSION_","");
        }

        TSString full_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
        full_name += PACKAGE_SPLIT + funName;
        boost::algorithm::replace_all(result,__TYPE_FULLNAME__,full_name);
        boost::algorithm::replace_all(result,"__DATATYPE__",TopicName);
        boost::algorithm::replace_all(result,"_LINKNAME_SPCE_",full_name);

        TSString parmsInit,defineAttribute,paramsOutInit,AttributeOut,paramsOutEque,paramsEque;
        parmsInit += Table(2) + "SuperType::operator=(other);\n";
        paramsOutInit += Table(2) + "SuperType::operator=(other);\n";
        paramsEque += Table(2) + "if(SuperType::operator==(other)";
        paramsOutEque += Table(2) + "if(SuperType::operator==(other)";
        BOOST_FOREACH(TSParamDefPtr param,Func->Params)
        {
            if (param->Type != TSParamDef::Out)
            {

                parmsInit += Table(2) + param->ValName + " = other." + param->ValName +  ";" + TSIDL_NEWLINE;
                paramsEque += TSIDL_NEWLINE + Table(3) + " && " + param->ValName + " == other." + param->ValName;
                defineAttribute += Table() + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + TSIDL_BLANK + 
                    param->ValName + ";" + TSIDL_NEWLINE;
            }

            if(param->Type != TSParamDef::In)
            {

                paramsOutInit += Table(2) + param->ValName + " = other." + param->ValName +  ";" + TSIDL_NEWLINE;
                paramsOutEque += TSIDL_NEWLINE + Table(3) + " && " + param->ValName + " == other." + param->ValName;
                AttributeOut += Table() + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + TSIDL_BLANK + 
                    param->ValName + ";" + TSIDL_NEWLINE;
            }
        }
        if (Func->DefTypeName != "void")
        {
            paramsOutInit += Table(2) + "rnt = other.rnt;" + TSIDL_NEWLINE;
            AttributeOut += Table() + TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + TSIDL_BLANK + 
                "rnt" + ";" + TSIDL_NEWLINE;
        }
        paramsEque += ")\n";
        paramsEque += Table(2) + "{\n";
        paramsEque += Table(3) + "return true; \n";
        paramsEque += Table(2) + "}\n";
        paramsOutEque += ")\n";
        paramsOutEque += Table(2) + "{\n";
        paramsOutEque += Table(3) + "return true; \n";
        paramsOutEque += Table(2) + "}\n";
        if ("Request" == TopicName)
        {
			boost::algorithm::replace_all(result, __FIELD_COPYASSIGN__, parmsInit);
            boost::algorithm::replace_all(result, __FIELD_ASSIGN__, parmsInit);
            boost::algorithm::replace_all(result, __FIELD_EQUAL__, paramsEque);           
            boost::algorithm::replace_all(result, __FIELD_DECLARE__, defineAttribute);

        }
        else if ("Response" == TopicName)
        {
			boost::algorithm::replace_all(result, __FIELD_COPYASSIGN__, paramsOutInit);
            boost::algorithm::replace_all(result, __FIELD_ASSIGN__, paramsOutInit);
            boost::algorithm::replace_all(result, __FIELD_EQUAL__, paramsOutEque);
            boost::algorithm::replace_all(result, __FIELD_DECLARE__, AttributeOut);  
        }
    }
    return result;
}

TSString TSGenSupportPlugin::GenSupportWithName( TSTypeDefWithFuncAndFiledPtr TypeDef,TSString TopicName, std::vector<TSString>& serviceName)
{
    TSString result;
    BOOST_FOREACH(TSFunctionDefPtr Func,TypeDef->Functions)
    {
		TSString funName = Func->ValName;
		/*std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/

        result += __DATA_TYPE_SUPPORT__;
        TSIDLUtils::ReplaceFullNameSpaceString("__NAMESPACE_BEGINS__",
            "__NAMESPACE_ENDS__",
            result,
            TypeDef,
            funName,
            true);

        TSString supername;
        if (TypeDef->Extend.lock())
        {
            supername = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
        }
        else
        {
            supername = "TSBasicService";
        }
        TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
        selfns += PACKAGE_SPLIT + funName;

        boost::algorithm::replace_all(result,"__DATATYPE__",TopicName);

        boost::algorithm::replace_all(result,"__SUPPROTTYPE__",TopicName + "Support");
        boost::algorithm::replace_all(result,"_SUPER_NS",supername);
        boost::algorithm::replace_all(result,"_SELF_NS",selfns);
        boost::algorithm::replace_all(result,"_TOPIC_TYPE_",TopicName);
        TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,result,TypeDef);

        TSString MashalFieldReq,DeMashalFieldReq,MashalFieldRes,DeMashalFieldRes;
		UINT32 SelfVersionCodeReq = 0;
		UINT32 SelfVersionCodeRes = 0;		
        MashalFieldReq += "Marshal_Supper;\n";
        DeMashalFieldReq += "Demarshal_Supper;\n";
        MashalFieldRes += "Marshal_Supper;\n";
        DeMashalFieldRes += "Demarshal_Supper;\n";
        
        BOOST_FOREACH(TSParamDefPtr param,Func->Params)
        {
            if (param->Type != TSParamDef::Out)
            {
                MashalFieldReq += "Marshal_Filed(" + 
                    param->ValName + ",TopSimDataInterface::kNone);" + TSIDL_NEWLINE;
                DeMashalFieldReq += "Demarshal_Filed(" + 
                    param->ValName + ",TopSimDataInterface::kNone);" + TSIDL_NEWLINE;
				SelfVersionCodeReq += GetCRC32(param->DefTypeName);
            }

            if(param->Type != TSParamDef::In)
            {
                MashalFieldRes += "Marshal_Filed(" + 
                    param->ValName + ",TopSimDataInterface::kNone);" + TSIDL_NEWLINE;
                DeMashalFieldRes += "Demarshal_Filed(" + 
                    param->ValName + ",TopSimDataInterface::kNone);" + TSIDL_NEWLINE;
				SelfVersionCodeRes += GetCRC32(param->DefTypeName);
            }
        }

        if (Func->DefTypeName != "void")
        {
            MashalFieldRes += "Marshal_Filed(rnt,TopSimDataInterface::kNone);" + TSIDL_NEWLINE;
            DeMashalFieldRes += "Demarshal_Filed(rnt,TopSimDataInterface::kNone);" + TSIDL_NEWLINE;
			SelfVersionCodeRes += GetCRC32(Func->DefTypeName);
        }

        if ("Request" == TopicName)
        {
            boost::algorithm::replace_all(result, "_MARSHAL_FILED", MashalFieldReq);
            boost::algorithm::replace_all(result, "_DEMARSHAL_FILED", DeMashalFieldReq);
			boost::algorithm::replace_all(result, "_ATTRABUTE_NUMBER_", boost::lexical_cast<TSString,UINT32>(SelfVersionCodeReq));
        }
        else if ("Response" == TopicName)
        {
            boost::algorithm::replace_all(result, "_MARSHAL_FILED", MashalFieldRes);
            boost::algorithm::replace_all(result, "_DEMARSHAL_FILED", DeMashalFieldRes);
			boost::algorithm::replace_all(result, "_ATTRABUTE_NUMBER_", boost::lexical_cast<TSString,UINT32>(SelfVersionCodeRes));
        }
		TSString superVersion = "#define PARENTTYPEVERSIONMACRO 3877837633\n#define SUPERVERSIONMACRO PARENTTYPEVERSIONMACRO";
		TSString UnSuperVersion = "#undef SUPERVERSIONMACRO\n#undef PARENTTYPEVERSIONMACRO";
		boost::algorithm::replace_all(result, "_SUPPER_VERSIONMACRO", superVersion);
		boost::algorithm::replace_all(result, "_UNDEF_SUPPERVERSION_", UnSuperVersion);
        boost::algorithm::replace_all(result, "_HASFINDSET_STRUCT_", "");
        boost::algorithm::replace_all(result, "_PARENT_FINDSET", "");
        boost::algorithm::replace_all(result, "_HASFINDSET_STRUCT_", "");
        boost::algorithm::replace_all(result, "_FINDSET_FLAG", "TopSimDataInterface::kNone");
        boost::algorithm::replace_all(result, "_TOPIC_FLAG", GetTopicFlag(TypeDef->Type));
    }
    return result;
}

TSString TSGenSupportPlugin::GenSupportCPPWithName( TSTypeDefWithFuncAndFiledPtr TypeDef,TSString TopicName, std::vector<TSString>& serviceName)
{
    TSString result;
    BOOST_FOREACH(TSFunctionDefPtr Func,TypeDef->Functions)
    {
		TSString funName = Func->ValName;
		/*std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/

        result += __DATA_DEFINE_SUPPORT__ + TSIDL_NEWLINE;
        TSIDLUtils::ReplaceFullNameSpaceString("__NAMESPACE_BEGINS__",
            "__NAMESPACE_ENDS__",
            result,
            TypeDef,
            funName,
            true);
		UINT32 SelfVersionCodeReq = 0;
		UINT32 SelfVersionCodeRes = 0;
		BOOST_FOREACH(TSParamDefPtr param,Func->Params)
		{
			if (param->Type != TSParamDef::Out)
			{
				SelfVersionCodeReq += GetCRC32(param->DefTypeName);
			}

			if(param->Type != TSParamDef::In)
			{
				SelfVersionCodeRes += GetCRC32(param->DefTypeName);
			}
		}

		if (Func->DefTypeName != "void")
		{
			SelfVersionCodeRes += GetCRC32(Func->DefTypeName);
		}

		if ("Request" == TopicName)
		{
			boost::algorithm::replace_all(result, "_ATTRABUTE_NUMBER_", boost::lexical_cast<TSString,UINT32>(SelfVersionCodeReq));
		}
		else if ("Response" == TopicName)
		{
			boost::algorithm::replace_all(result, "_ATTRABUTE_NUMBER_", boost::lexical_cast<TSString,UINT32>(SelfVersionCodeRes));
		}

        TSString supername;
        if (TypeDef->Extend.lock())
        {
            supername = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
        }
        else
        {
            supername = "TSBasicService";
        }
        TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
        selfns += PACKAGE_SPLIT + funName;

		boost::algorithm::replace_all(result, "_SUPPER_VERSIONMACRO", "");
		boost::algorithm::replace_all(result, "_UNDEF_SUPPERVERSION_", "");
        boost::algorithm::replace_all(result,"__DATATYPE__",TopicName);
        boost::algorithm::replace_all(result,"__SUPPROTTYPE__",TopicName + "Support");
        boost::algorithm::replace_all(result,"_SUPER_NS",supername);
        boost::algorithm::replace_all(result,"_SELF_NS",selfns);
        TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,result,TypeDef);

        boost::algorithm::replace_all(result,"_REG_FIELD","REG_BASE(SUP_NAMESPACE::DataType);");
    }
    return result;
}

TSString TSGenSupportPlugin::GenDefineWithName( TSTypeDefWithFuncAndFiledPtr TypeDef,TSString TopicName, std::vector<TSString>& serviceName, bool IsHeader)
{
    TSString result;
    BOOST_FOREACH(TSFunctionDefPtr Func,TypeDef->Functions)
    {
		TSString funName = Func->ValName;
		/*std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/

        if (IsHeader)
        {
            result += __DATA_DESC_DEFINE__ + TSIDL_NEWLINE;
        }
        else
        {
            result += __DATA_DEFINE_DEFINE__ + TSIDL_NEWLINE;
        }

        TSIDLUtils::ReplaceFullNameSpaceString("__NAMESPACE_BEGINS__",
            "__NAMESPACE_ENDS__",
            result,
            TypeDef,
            funName,
            true);
        TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,result,TypeDef);

        TSString supername;
        if (TypeDef->Extend.lock())
        {
            supername = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
        }
        else
        {
            supername = TSIDL_TOPIC_BASIC;
        }
        TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
        selfns += PACKAGE_SPLIT + funName; 
        TSString selfname = GetTopicName(TypeDef);
        selfname += "_" + funName + "_" + TopicName;

        boost::algorithm::replace_all(result,"_SUPER_NAME",supername);
        boost::algorithm::replace_all(result,"_SUPER_NS","");
        boost::algorithm::replace_all(result,"_SELF_NS",selfns);
        boost::algorithm::replace_all(result,"_SELF_NAME",selfname);
        boost::algorithm::replace_all(result,"_SELF_DESCRIPTION",selfname);
        boost::algorithm::replace_all(result,"__DATATYPE__",TopicName);
        boost::algorithm::replace_all(result,"__SUPPROTTYPE__",TopicName + "Support");
    }
    return result;
}

void TSGenSupportPlugin::GenerateServerSkelHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = SERVICESKEL_HEADER_CODE;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);
    TSString funHandler;
    BOOST_FOREACH(TSFunctionDefPtr func,TypeDef->Functions)
    {
        funHandler += Table() + "virtual " + func->ValName + "::ResponsePtr " + func->ValName + 
            "Handler(TSBasicService::DataTypePtr Request);\n"; 
    }

    TSString serverPath = GetBaseName(TypeDef->Parse.lock()->GetFullFileName()) + "_xidl/cpp/generic/";
    serverPath += TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(serverPath,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

    TSString TopicPath = GetBaseName(TypeDef->Parse.lock()->GetFullFileName()) + "_xidl/cpp";

    boost::algorithm::replace_all(content,"_TOPIC_PATH_",TopicPath);
    boost::algorithm::replace_all(content,"_SERVICE_PATH_",serverPath);
    boost::algorithm::replace_all(content,"_FUNS_HANDLER_",funHandler);
    ProjFile->AppendLast(content);
}

void TSGenSupportPlugin::GenerateServerSkelCpp( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, bool buildServiceFlag )
{
    TSString content = SERVICESKEL_CPP_CODE;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSString functions;
	functions = InvokerAndInitializeFunc(TypeDef, serviceName, true, buildServiceFlag);

	/*BOOST_FOREACH(TSFunctionDefPtr func, TypeDef->Functions)
	{
		TSString funName = func->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}
		if (TypeDef->Type == TSTypeDef::Service)
		{
			functions  = InvokerAndInitializeFunc(TypeDef, serviceName, true, buildServiceFlag);
		}
	}*/

    TSString TopicPath = GetBaseName(TypeDef->Parse.lock()->GetFullFileName()) + "_xidl/cpp";
    boost::algorithm::replace_all(content,"_TOPIC_PATH_",TopicPath);
    boost::algorithm::replace_all(content,"_FUNS_HANDLER_",functions);
    ProjFile->AppendLast(content);
}

TSString TSGenSupportPlugin::GetIncludeHeadFile( TSTypeDefWithFuncAndFiledPtr TypeDef, const TSString & PathName)
{
    TSString result;
    std::vector<TSString> paramOrfield;
    BOOST_FOREACH(TSFunctionDefPtr func, TypeDef->Functions)
    {
        for (unsigned int i = 0; i < func->Params.size(); i++)
        {
            TSParamDefPtr param = func->Params[i];
            result += "#include <generic/" + TSBuiltinPlugin::GetSuitFullTypeName(param->Def,true) + "/" + PathName + ".h>\n";
            paramOrfield.push_back(param->DefTypeName);
        }
        result += "#include <generic/" + TSBuiltinPlugin::GetSuitFullTypeName(func->Def,true) + "/" + PathName + ".h>\n";
        paramOrfield.push_back(func->DefTypeName);
    }
    for (unsigned int i = 0; i < TypeDef->Fileds.size(); i++)
    {
        TSFieldDefPtr field = TypeDef->Fileds[i];
        result += "#include <generic/" + TSBuiltinPlugin::GetSuitFullTypeName(field->Def,true) + "/" + PathName + ".h>\n";
        paramOrfield.push_back(field->DefTypeName);
    }
    boost::algorithm::replace_all(result,"::",".");
    return result;
}

void TSGenSupportPlugin::GenerateTopicHeader( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName)
{
    TSString content = __DATA_TYPE_TOPIC__;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceTypeFullName(__SUPER_TYPE__,
        content,TypeDef);

    TSIDLUtils::ReplaceInitList(__CONSTRUCT_LIST__,
        content,TypeDef);
   
    TSIDLUtils::ReplaceFiledsAssign(__FIELD_COPYASSIGN__,
        "other",content,TypeDef);

    TSIDLUtils::ReplaceFiledsAssign(_EXTEND_,
        "other",content,TypeDef);

    TSIDLUtils::ReplaceFiledsAssign(__FIELD_ASSIGN__,
        "other",content,TypeDef);

    TSIDLUtils::ReplaceFiledsEqual(__FIELD_EQUAL__,
        "other",content,TypeDef);

    TSIDLUtils::ReplaceFiledsLessCompare(__FIELD_LESSCOMPARE__,
        "other",content,TypeDef);

    TSIDLUtils::ReplaceFiledsDeclare(__FIELD_DECLARE__,
        content,TypeDef);

    TSIDLUtils::ReplaceLinkerVersion(content,"_LINKER_VERSION_");

   if (TypeDef->Extend.lock())
    {
        TSString suit_full_type_name = "," + TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
        suit_full_type_name += "::DataType"; 
        boost::algorithm::replace_all(content,"_SUPER_VERSION_",suit_full_type_name);
    }
    else
    {
        boost::algorithm::replace_all(content,"_SUPER_VERSION_","");
    }

    TSString full_name = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(content,__TYPE_FULLNAME__,full_name);
    boost::algorithm::replace_all(content,"__DATATYPE__","DataType");
    boost::algorithm::replace_all(content,"_LINKNAME_SPCE_",full_name);

    if(TypeDef->Type == TSTypeDef::Service)
    {
        content = "";
        content += GenTopicWithName(TypeDef,"Request", serviceName);
        content += GenTopicWithName(TypeDef,"Response", serviceName);
    }

    TSIDLUtils::ReplaceExportMacro(__EXPORT_MACRO__,content,TypeDef);

    ProjFile->AppendLast(content);
}

TSString TSGenSupportPlugin::GetServiceFunction( TSTypeDefWithFunctionPtr TypeDef, std::vector<TSString>& serviceName )
{
    TSString result;
    if (TypeDef->Type == TSTypeDef::Service)
    {
        BOOST_FOREACH(TSFunctionDefPtr func, TypeDef->Functions)
        {
			/*TSString funName = func->ValName;
			std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
			if (iterTopic == serviceName.end())
			{
				continue;
			}*/
            result += TSBuiltinPlugin::GetSuitFullTypeName(func->Def) + " Proxy::" +
                TSIDLUtils::GetFuncWithoutResultStr(func,true) + "\n";
            result += "{" + TSIDL_NEWLINE;
            result += Table() + func->ValName + "::RequestPtr Request = boost::make_shared<" + func->ValName + "::Request>();\n";
            result += Table() + "UINT32 ErrorCode;\n";
            result += SERVICE_FUN;
            result += Table() + func->ValName + "::Response* Response = (" + func->ValName + "::Response*)SendRequestSync(\""+
                func->ValName + "\",Request.get(),ErrorCode);\n";
            result += Table() + "if(ErrorCode != TSRPC_SUCCESS)\n" + Table() + "{\n" + Table(2) + " throw TSException(TSValue_Cast<TSString>(ErrorCode));\n" + "}\n";
            TSString asyncStr,handlStr;
            std::vector<TSParamDefPtr> paramOut;
            for (unsigned int i = 0; i < func->Params.size(); i++)
            {
                TSParamDefPtr param = func->Params[i];
                if (param->Type != TSParamDef::Out)
                { 
                    asyncStr += "Request->" + param->ValName + " = " + param->ValName + ";\n";                   
                }

                if (param->Type != TSParamDef::In)
                {
                    result += Table() + param->ValName + " = " + "Response->" + 
                        param->ValName + ";\n";
                    handlStr += Table() + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + TSIDL_BLANK +param->ValName + " = " +
                        "Response->" + param->ValName + ";\n";
                    paramOut.push_back(param);
                }
            }
            if (func->DefTypeName != "void")
            {
                result += Table() + "return Response->rnt;\n";
                handlStr += Table() + TSBuiltinPlugin::GetSuitFullTypeName(func->Def) + " rnt = Response->rnt;\n";
            }
            result += "}" + TSIDL_NEWLINE;

            result += TSIDLUtils::GetFuncAsyncStr(func,"Proxy",true,false) + "\n";
            result += "{" + TSIDL_NEWLINE;
            result += Table() + func->ValName + "::RequestPtr Request = boost::make_shared<" + func->ValName + "::Request>();\n";
            result += SERVICE_FUN;
            result += Table() + "UINT32 ErrorCode;\n";
            result += Table() + "SendRequestAsync(\"" + func->ValName + "\",Request.get(),new " + TSIDLUtils::GetFuncAsyncCallBackStr(func,true)
                + "(func),ErrorCode);\n" + "}\n";

            TSString topicName = TSIDLUtils::GetTopicName(TypeDef);
            TSString topicFunName = topicName + "_" + func->ValName;
            boost::algorithm::replace_all(result,"_TOPIC_NAME_",topicName);
            boost::algorithm::replace_all(result,"_TOPICNAME_FUNNAME_",topicFunName);
            boost::algorithm::replace_all(result,"_FUN_NAME_",func->ValName);
            boost::algorithm::replace_all(result,"_REQUEST_PARAM_",asyncStr);
        }

    }
    return result;
}