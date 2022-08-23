#include "stdafx.h"

#include <boost/foreach.hpp>

#include "TSCSharpPlugin.h"
#include "TSIDLUtils.h"

#include "temp/topic.temp.h"
#include "temp/support.temp.h"
#include "temp/define.temp.h"
#include "temp/CSCallBack.temp.h"
#include "temp/Interceptor.temp.h"
#include "temp/CallBack.temp.h"
#include "temp/Interface.temp.h"
#include "temp/Proxy.temp.h"
#include "temp/Method.temp.h"
#include "temp/RemoteMehod.temp.h"
#include "temp/Servant.temp.h"
#include "temp/Service.temp.h"
#include "temp/Updater.temp.h"
#include "temp/MessageSender.h"

using namespace TSIDLUtils;

BEGIN_METADATA(TSCSharpPlugin)
    REG_BASE(TSIIDLPlugin);
END_METADATA()

struct TSCSharpPluginPrivate
{
    TSIBuiltinPluginPtr _BuiltinPlugin;
};

TSCSharpPlugin::TSCSharpPlugin( void )
    :_p(new TSCSharpPluginPrivate)
{
	_topicFlagStruct = new TSCSTopicFlag;
	_topicFlagStruct->buildTopic = false;
	_topicFlagStruct->buildSupport = false;
	_topicFlagStruct->buildDefine = false;

	_serviceFlagStruct = new TSCSServiceFlag;
	_serviceFlagStruct->buildServiceSkel = false;
	_serviceFlagStruct->buildSupport = false;
	_serviceFlagStruct->buildTopic = false;
	_serviceFlagStruct->buildDefine = false;
	_serviceFlagStruct->buildProxy = false;
	_serviceFlagStruct->buildMethod = false;
	_serviceFlagStruct->buildMethodImpl = false;
}

TSCSharpPlugin::~TSCSharpPlugin( void )
{
    delete _p;
	delete _topicFlagStruct;
	delete _serviceFlagStruct;
}

TSString TSCSharpPlugin::GetName()
{
    return GB18030ToTSString("CSharp文件生成插件");
}

void TSCSharpPlugin::OnPluginLoaded()
{
    _p->_BuiltinPlugin = GetPluginT<TSIBuiltinPlugin>();
    ASSERT(_p->_BuiltinPlugin && "TSCSharpPlugin::OnPluginLoaded");
}

bool TSCSharpPlugin::DoParse()
{
    return true;
}

bool TSCSharpPlugin::DoBuild()
{
    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
    if(Cp != "c#")
    {
        return true;
    }
    BuildTopic();

    BuildSupport();

    BuildDefine();

    BuildProxy();

    BuildMethod();

    BuildServiceSkel();

    BuildMethodImpl();

    return true;
}

void TSCSharpPlugin::Clean()
{

}

void TSCSharpPlugin::BuildTopic()
{
    TSProjFilePtr TopicFile;

    std::vector<TSEnumDefPtr> EnumDefs = _p->_BuiltinPlugin->GetAllEnumDefs();
    BOOST_FOREACH(TSEnumDefPtr EnumDef, EnumDefs)
    {
        TSString ParseFileBaseName = 
            GetBaseName(EnumDef->Parse.lock()->GetFullFileName());
        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(EnumDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                Dir->SetProFileType(None);
                TopicFile = Dir->GetOrCreateProjFile("Topic",".cs");
                TopicFile->SetFileType(None);
                TSIDLUtils::GenerateEnum(TopicFile,EnumDef);
            }
        }
    }

    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	//std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	//GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	//std::vector<TSString> tempServiceVec;
	//获得要生成的服务
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildTopicVec, _serviceFlagStruct->buildTopic);

	//std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

	//获取主题选择工具中选中的主题
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

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
        if (TypeDef->Type != TSTypeDef::Topic && TypeDef->Type != TSTypeDef::Struct &&
            TypeDef->Type != TSTypeDef::Service)
        {
            continue;
        }

		//过滤掉没有选中的节点以及可能重复生成的节点
		/*if (FilterNotSelectNode(TypeDef, indexVec, structVec, _storeStructRec.buildTopicRec))
		{
			continue;
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
		//存储生成的服务
		_storeStructRec.buildTopicRec.push_back(TypeDef->TypeName);
	
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
                Dir->SetProFileType(None);
                TopicFile = Dir->GetOrCreateProjFile("Topic",".cs");
                TopicFile->SetFileType(None);
                GenerateTopic(TopicFile,TypeDef);
                if (TypeDef->Type == TSTypeDef::Service)
                {
                    GenerateTopicWithService(TopicFile,TypeDef, indexServiceVec, _topicFlagStruct->buildTopic);
                    GenerateTopicWithService(TopicFile,TypeDef, indexServiceVec, _topicFlagStruct->buildTopic,false);
                }
            }
        }
    }
    std::vector<TSSolutionPtr> solutions = SolutionMgr()->GetSolutions(); 
    std::vector<TSSolutionPtr>::const_iterator its = solutions.begin();
    BOOST_FOREACH(TSSolutionPtr its,solutions)
    {
        const ProjMap_T &Projs = its->GetProjects();
        for (ProjMap_T::const_iterator iter = Projs.begin(); iter != Projs.end(); iter++)
        {
            const ProjFileMap_T files = iter->second->GetProjFiles();
            TSString baseDir = iter->second->GetShortName();
            for (ProjFileMap_T::const_iterator iterfile = files.begin(); iterfile != files.end(); iterfile++)
            {
                TSString FileName = iterfile->first;
                if (FileName == "Topic.cs" && !_topicFlagStruct->buildTopic)
                {
                    TSString content = __HEADER_INCLUDE_TOPIC_4CS__;
                    iterfile->second->AppendFirst(content);
                }
            }
        }
    }  
	_topicFlagStruct->buildTopic = true;
	_serviceFlagStruct->buildTopic = true;
}

void TSCSharpPlugin::BuildMethod()
{
    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	//std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	//GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	//std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildMethodVec, _serviceFlagStruct->buildMethod);

	//std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
		if (TypeDef->Type == TSTypeDef::Struct)
		{
			continue;
		}
		else if (TypeDef->Type == TSTypeDef::Service)
		{
			/*if (_serviceFlagStruct->buildMethod)
			{
				continue;
			}*/
			if (!vecSelectService.empty())
			{
				if (vecSelectService[0] == "N")
				{
					continue;
				}
			}
		}
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(TSSolutionPtr Solution = 
            SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
        {
            if (TypeDef->Type != TSTypeDef::Service)
            {
                continue;
            }
            TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

            TSString Path = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
            boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

            TSProjFilePtr MethodFile = Dir->GetOrCreateProjFile(Path + "/Method",".cs");
            MethodFile->SetFileType(Generic);
            GenerateMethod(MethodFile,TypeDef, indexServiceVec);
            MethodFile->AppendFirst(METHOD_HEAD_4CS_CODER);
        }
    }
	_serviceFlagStruct->buildMethod = true;
}

void TSCSharpPlugin::GenerateMethod( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName)
{
    TSString content = METHOD_CODER_SERIVICE_4CS;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSString funcs,funsAsync,funsDeLegate;
    bool haveRetrurn = false;
    BOOST_FOREACH( TSFunctionDefPtr Function, TypeDef->Functions)
    {
		/*TSString funName = Function->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
        std::vector<TSParamDefPtr> vecParams; 
        haveRetrurn = false;
        TSString TypeName = GetCSharpValueType(Function->Def);
        funcs += Table(2) + "public abstract " + TypeName + 
            TSIDL_BLANK + Function->ValName + "(";
        funsAsync += Table(2) + "public abstract void " + Function->ValName  + "Async(";
        funsDeLegate += Table(2) + "public delegate void " + Function->ValName + "Async(";
         if (Function->DefTypeName != "void" && 
            Function->DefTypeName != "Void")
        {
            funsAsync += TypeName + " rnt";
            funsDeLegate += TypeName + " rnt"; 
            haveRetrurn = true;
        }
        for (unsigned int i = 0; i < Function->Params.size(); i++)
        {
            TSParamDefPtr Param = Function->Params[i];
            TSString TypeName = GetCSharpValueType(Param->Def);
                 
            if (Param->Type == TSParamDef::Inout)
            {
                funcs += "ref ";
                vecParams.push_back(Param);
                
            }
            else if (Param->Type == TSParamDef::Out)
            {
                funcs += "out ";
                vecParams.push_back(Param);

            }
            funcs += TypeName + TSIDL_BLANK + Param->ValName;
            if (Function->Params.size() != i + 1)
            {
                funcs += ",";
            }
        }
        funcs += ");"+ TSIDL_NEWLINE;
        for(unsigned int j = 0; j < vecParams.size(); j++)
        {
            TSString ParTypeName  = GetCSharpValueType(vecParams[j]->Def);
            if (vecParams[j]->Type == TSParamDef::Inout || vecParams[j]->Type == TSParamDef::Out)
            {
                if (haveRetrurn)
                {
                    funsAsync += ",ref ";
                    funsDeLegate += ",ref ";
                }
                else
                {
                    funsAsync += "ref ";
                    funsDeLegate += "ref ";
                }
            }
            funsAsync += ParTypeName + TSIDL_BLANK + vecParams[j]->ValName;
            funsDeLegate += ParTypeName + TSIDL_BLANK + vecParams[j]->ValName;
            if (vecParams.size() != j + 1)
            {
                if (!haveRetrurn)
                {
                    funsAsync += ",";
                    funsDeLegate += ",";
                }
                
            }
        }

        funsAsync += ");" + TSIDL_NEWLINE;
        funsDeLegate += ");" + TSIDL_NEWLINE;
    }

    TSString MethodFun = funcs + funsDeLegate;
    boost::algorithm::replace_all(funcs,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

    //boost::algorithm::replace_all(content, "_PARENT_", "");
    boost::algorithm::replace_all(content, "_METHOD_FUN_", MethodFun);
    ProjFile->AppendLast(content);
}

void TSCSharpPlugin::BuildProxy()
{
    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	//std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	//GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	//std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildProxyVec, _serviceFlagStruct->buildProxy);

	//std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
		if (TypeDef->Type == TSTypeDef::Struct)
		{
			continue;
		}
		else if (TypeDef->Type == TSTypeDef::Service)
		{
			//服务不再重复生成
			/*if (_serviceFlagStruct->buildProxy)
			{
				continue;
			}*/
			//没有选中服务不生成服务
			if (!vecSelectService.empty())
			{
				if (vecSelectService[0] == "N")
				{
					continue;
				}
			}
		}
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(TSSolutionPtr Solution = 
            SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
        {
            if (TypeDef->Type != TSTypeDef::Service)
            {
                continue;
            }
            TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

            TSString Path = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
            boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

            TSProjFilePtr ProxyFile = Dir->GetOrCreateProjFile(Path + "/Proxy",".cs");
            ProxyFile->SetFileType(Generic);
            GenerateProxy(ProxyFile,TypeDef, indexServiceVec);
            ProxyFile->AppendFirst(SERVICE_SERVICE_HEADER_4CS);
        }
    }
	_serviceFlagStruct->buildProxy = true;
}

void TSCSharpPlugin::GenerateProxy( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName)
{
    TSString content = PROXY_CODE_SERVICE_4CS;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSString funcCode,funcCodeAsync,registerMethod,funInvoke;
    TSString ServerName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    std::vector<TSFunctionDefPtr> allFuncs = TypeDef->Functions;
    for(size_t i = 0; i < allFuncs.size();++i)
    {
		/*TSString funName = allFuncs[i]->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
		TSString valueOperate,valueOperateAsync,RequestIn,ResponseOut,RespParam;
        if(TSFunctionDefPtr func = allFuncs[i])
        {   
            TSString params,outValue;
            funcCode += TSIDL_2TABLE + "public " + TSIDLUtils::GetFunctionStr(func) + "\n";
            funcCode += TSIDL_2TABLE + "{\n";
            funcCodeAsync += TSIDL_2TABLE + "public " + TSIDLUtils::GetServiceFunAsyncStr(func) + "\n";
            funcCodeAsync += TSIDL_2TABLE + "{\n";
			funInvoke += Table(2) + "static void " + func->ValName + "ResponseInvoker(IntPtr UserData, UInt32 ResponseTopic, IntPtr ResponseData, IntPtr Invoker, bool IsTimeout)\n";
			funInvoke += Table(2) + "{\n";
			funInvoke += Table(3) + "IntPtr wrapData = LinkStudioInvoke.TSRPCInitalizer.TSRPC_CS_RawDataUnwarpper(ResponseData);\n";
			

            TSString handlReq = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + 
                "::" + func->ValName + "::Request.ID";
            TSString handlRes = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + 
                 "::" + func->ValName + "::Response.ID";

            boost::algorithm::replace_all(handlReq,"::","_");
            boost::algorithm::replace_all(handlRes,"::","_");

			funInvoke += Table(3) + "if(ResponseTopic == " + handlRes + ")\n";
			funInvoke += Table(3) + "{\n";
			funInvoke += Table(4) + "if(!IsTimeout)\n";
			funInvoke += Table(4) + "{\n";
			funInvoke += Table(5) + func->ValName + ".Response resp = GCHandle.FromIntPtr(wrapData).Target as " + func->ValName + ".Response;\n";
			funInvoke += Table(5) + "Method." + func->ValName + "Async sync = GCHandle.FromIntPtr(Invoker).Target as Method." + func->ValName + "Async;\n";



			

			registerMethod += TSIDL_2TABLE + TSIDL_2TABLE + "RegisterMethod(\"" + func->ValName + "\"," + handlReq + ",\n";
			registerMethod +=	TSIDL_2TABLE + TSIDL_2TABLE + handlRes + ",null,(LinkStudioInvoke.TSRPCInitalizer.AsyncResponseInvoker)" +
				func->ValName + "ResponseInvoker);\n";
			
			valueOperate += Table(3) + func->ValName + ".Request req = new " + func->ValName + ".Request();\n";
			valueOperate += Table(3) + "UInt32 ErrorCode = 0;\n";
			valueOperateAsync = valueOperate;
			bool IsfirstOutParam = false;
			TSString IsAysOut;
            for (size_t j = 0; j < func->Params.size(); j++)
            {
				TSParamDefPtr paramDef = func->Params[j];
				if (paramDef->Type != TSParamDef::Out)
				{
					RequestIn += Table(3) + "req." + paramDef->ValName + " = " + paramDef->ValName + ";\n";
				}
				if (paramDef->Type != TSParamDef::In)
				{
					ResponseOut += Table(3) + paramDef->ValName + " = resp." + paramDef->ValName + ";\n";
					
					if (!IsfirstOutParam)
					{
					    RespParam += "ref resp." + paramDef->ValName;
						IsfirstOutParam = true;
					}
					else
					{
						RespParam += ",";
						RespParam += "ref resp." + paramDef->ValName;
					}
				}

				if (paramDef->Type == TSParamDef::Out)
				{
					IsAysOut += Table(3) + paramDef->ValName;

					if(TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) == "string")
					{
						IsAysOut += " = \"\";\n";
					}
					else if(TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) == "string[]")
					{
						IsAysOut += " = new string[]{""};\n";
					}
					else if(paramDef->Def->Type == TSTypeDef::Enum)
					{
						IsAysOut +=" = new "
							+ TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "();\n";
					}
					else if(paramDef->Def->Type == TSTypeDef::Builtin)
					{
						IsAysOut +=" = new "
							+ TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "();\n";
					}
					else if(paramDef->Def->Type == TSTypeDef::Array)
					{
						TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(paramDef->Def,TSVectorBuiltinTypeDefPtr);
						if (vectorField->Value->Type != TSTypeDef::Builtin)
						{
							IsAysOut +=  " = new List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">();\n";
						}
						else
						{
							IsAysOut += " = new "
								+  TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "{};\n";
						}
					}                   
					else
					{
						IsAysOut += " = new "
							+  TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "{};\n";
					}
				}

            }
			TSString syncParam;
            if (func->DefTypeName != "void")
            {
				TSString RetTypeName = GetCSharpValueType(func->Def);
				ResponseOut += Table(3) + RetTypeName + " ret;\n"; 
				ResponseOut += Table(3) + "if (ErrorCode == 0)\n";
				ResponseOut += Table(3) + "{\n";
				ResponseOut += Table(4) + "ret = resp.rnt;\n";
				ResponseOut += Table(4) + "return ret;\n";
				ResponseOut += Table(3) + "}\n";
				ResponseOut += Table(3) + "else\n";
				ResponseOut += Table(3) + "{\n";
				ResponseOut += Table(3) + " throw new Exception(\"代理调用服务失败\");\n";
				ResponseOut += Table(3) + "}\n";
                syncParam += "resp.rnt"; 
            }
			if (!RespParam.empty())
			{
				syncParam += "," + RespParam;
			}
			funInvoke +=  Table(5) + "sync(" + syncParam + ");\n";

			boost::algorithm::replace_all(funInvoke, "(,","(");
			valueOperate += RequestIn;
			
			valueOperate += Table(3) + func->ValName + ".Response resp = SendRequestSync(\"" + 
				func->ValName + "\", req, ref ErrorCode) as " + func->ValName + ".Response;\n";
	        valueOperate += ResponseOut;
			valueOperateAsync += RequestIn + IsAysOut;
			valueOperateAsync += Table(3) + "SendRequestAsync(" + "\"" +  func->ValName  + "\"" + ",req,func,ref ErrorCode);\n";


			funcCode += valueOperate;

            funcCode += TSIDL_2TABLE + "}\n";

            funcCodeAsync += valueOperateAsync;
            funcCodeAsync += TSIDL_2TABLE + "}\n";
			funInvoke += Table(4) + "}\n";
			funInvoke += Table(3) + "}\n";
			funInvoke += Table(2) + "}\n";
            
        }
    }

    TSString ProxyFun = funcCode + funcCodeAsync + funInvoke;
    boost::algorithm::replace_all(ProxyFun,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
     
    boost::algorithm::replace_all(content,"_FUNS_",ProxyFun);
    boost::algorithm::replace_all(content,"_FUN_REGISTERMETHOD_",registerMethod);
    ProjFile->AppendLast(content);
}

void TSCSharpPlugin::BuildRemoteMethod()
{
 
    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(TSSolutionPtr Solution = 
            SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
        {
            if (TypeDef->Type != TSTypeDef::Service)
            {
                continue;
            }
            TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

            TSString Path = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
            boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

            TSProjFilePtr RemoteMethod = Dir->GetOrCreateProjFile(Path + "/RemoteMethod",".cs");
            RemoteMethod->SetFileType(Generic);
            GenerateRemoteMethod(RemoteMethod,TypeDef);
            RemoteMethod->AppendFirst(REMOTEMETHOD_HEADER_4CS_SERVICE);
        }
    }

}

void TSCSharpPlugin::GenerateRemoteMethod( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = REMOTEMETHODCODER_SERVICE_4CS;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSString suit_full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    TSString suit_full_Request_name,suit_Remove_Topic,funcstr,funcAsyncStr,funcAsyncHandler;
    TSString SetAsync;
    BOOST_FOREACH(TSFunctionDefPtr func,TypeDef->Functions)
    {
        TSString funContent = "";
        TSString suit_Request = suit_full_type_name;
        TSString suit_Response = suit_full_type_name;
        suit_full_Request_name += Table(3) + "PublishTopic(";
        suit_Request += "::" + func->ValName + "::Request";
        suit_full_Request_name += suit_Request +".ID);\n";
        suit_full_Request_name += "SubscribeTopic(";
        suit_Response += "::" + func->ValName + "::Response";
        suit_full_Request_name += suit_Response + ".ID);\n";
        suit_Remove_Topic += Table(3) + "UnpublishTopic(";
        suit_Remove_Topic += suit_Request + ".ID);\n";
        suit_Remove_Topic += Table(3) + "UnsubscribeTopic(";
        suit_Remove_Topic += suit_Response + ".ID);\n";

        TSString suit_Request_ID = suit_Request;
        TSString suit_Response_ID = suit_Response;
        boost::algorithm::replace_all(suit_Request_ID, "::","_");
        boost::algorithm::replace_all(suit_Response_ID, "::","_");
        boost::algorithm::replace_all(suit_Request, "::",".");
        boost::algorithm::replace_all(suit_Response, "::",".");

        SetAsync += Table(3) + "GetOperation().SetAsyncHandler(" + suit_Response_ID + ".ID,(TSIOperation.ResponseHandlerType)" 
            + func->ValName + "AsyncHandler);\n";
        TSString TypeName = GetCSharpValueType(func->Def);
        funcstr += Table(2) + "public " + TypeName + TSIDL_BLANK;
        funcAsyncStr += Table(2) + "public " + TSIDLUtils::GetServiceFunAsyncStr(func) + "\n";
        funcAsyncStr += Table(2) + "{\n";
        funcAsyncHandler += Table(2) + "public void " + func->ValName + "AsyncHandler(object Response,object Invoker)\n";
        funcAsyncHandler += Table(2) + "{\n";
        funcstr += TSIDLUtils::GetFuncWithoutResultStr(func,true) + "\n";
        funcstr += Table(2) + "{\n";
        TSString ResponseOut,RequestIn,IsAysOut,ResOut,callBackParam; 
        BOOST_FOREACH(TSParamDefPtr paramDef,func->Params)
        {
            if (paramDef->Type != TSParamDef::Out)
            {
                RequestIn += Table(3) + "Request." + paramDef->ValName + " = " + paramDef->ValName + ";\n";
            }
            if (paramDef->Type != TSParamDef::In)
            {
                ResponseOut += Table(3) + paramDef->ValName + " = Response." + paramDef->ValName + ";\n";
                 ResOut += Table(3) + GetCSharpValueType(paramDef->Def) + TSIDL_BLANK + paramDef->ValName + 
                        " = Res." + paramDef->ValName + ";\n";             
            }
            if (paramDef->Type == TSParamDef::Out)
            {
                IsAysOut += Table(3) + paramDef->ValName;

                if(TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) == "string")
                {
                     IsAysOut += " = \"\";\n";
                }
                else if(TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) == "string[]")
                {
                    IsAysOut += " = new string[]{""};\n";
                }
                else if(paramDef->Def->Type == TSTypeDef::Enum)
                {
                    IsAysOut +=" = new "
                        + TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "();\n";
                }
                else if(paramDef->Def->Type == TSTypeDef::Builtin)
                {
                    IsAysOut +=" = new "
                        + TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "();\n";
                }
                else if(paramDef->Def->Type == TSTypeDef::Array)
                {
                    TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(paramDef->Def,TSVectorBuiltinTypeDefPtr);
                    if (vectorField->Value->Type != TSTypeDef::Builtin)
                    {
                        IsAysOut +=  " = new List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">();\n";
                    }
                    else
                    {
                        IsAysOut += " = new "
                            +  TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "{};\n";
                    }
                }                   
                else
                {
                    IsAysOut += " = new "
                        +  TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "{};\n";
                }
            }
        }

        funcstr += Table(3) + suit_Request + " Request =  GetLocator().MakeRequest<" + suit_Request + ">();\n";
        funcstr += Table(3) + "Request.Method = " + suit_Request_ID + ".ID;\n";
        funcstr += RequestIn;
        funcstr += Table(3) + suit_Response + " Response = GetLocator().InvokeOperation(" + suit_Request_ID + ".ID,\n";
        funcstr += Table(4) + "Request) as " + suit_Response + ";\n";
        funcstr += ResponseOut;
        funcAsyncStr += Table(3) + suit_Request + " Request =  GetLocator().MakeRequest<" + suit_Request + ">();\n";
        funcAsyncStr += Table(3) + "Request.Method = " + suit_Request_ID + ".ID;\n";
        funcAsyncStr += RequestIn;
        funcAsyncStr += IsAysOut;
        funcAsyncStr += Table(3) + "GetLocator().InvokeOperationAsync(" + suit_Request_ID + ".ID,\n";
        funcAsyncStr += Table(4) + "Request, func);\n";

        if (func->DefTypeName != "void" &&
            func->DefTypeName != "Void")
        {
            funcstr += Table(3) + "return Response.rnt;\n";
            TSString TypeName = GetCSharpValueType(func->Def);
            ResOut += Table(3) + TypeName + " rnt = Res.rnt;\n";
        }
        funcAsyncHandler += Table(3) + suit_Response + " Res = Response as " +suit_Response +  ";\n";
        funcAsyncHandler += ResOut;
        funcAsyncHandler += Table(3) + "Method." + func->ValName + "Async callback = Invoker as Method." + 
            func->ValName + "Async;\n";
        funcAsyncHandler += Table(3) + "callback" + GetFuncParamWithoutFuncName(func) + ";\n";
        funcAsyncHandler += Table(2) + "}\n";
        funcstr += Table(2) + "}\n";
        funcAsyncStr += Table(2) + "}\n";
    }
    TSString funCode = funcstr + funcAsyncStr + funcAsyncHandler;

    boost::algorithm::replace_all(funCode, "::","_");
    boost::algorithm::replace_all(content, "_FUNS_",funCode);

    ProjFile->AppendLast(content);
}

void TSCSharpPlugin::BuildServer()
{
    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(TSSolutionPtr Solution = 
            SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
        {
            if (TypeDef->Type != TSTypeDef::Service)
            {
                continue;
            }
            TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

            TSString Path = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
            boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

            TSProjFilePtr ServiceFile = Dir->GetOrCreateProjFile(Path + "/Server",".cs");
            ServiceFile->SetFileType(Generic);
            GenerateServer(ServiceFile,TypeDef);
            ServiceFile->AppendFirst(SERVICE_SERVICE_HEADER_4CS);
        }
    }
}

void TSCSharpPlugin::GenerateServer( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
	bool FirstOrNot = false;
    TSString content = SERVICE_SERVICE_CODER_4CS;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
     ProjFile->AppendLast(content);
}

void TSCSharpPlugin::BuildServiceSkel()
{
    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	//std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	//GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	//std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildServiceVec, _serviceFlagStruct->buildServiceSkel);

	//std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
		if (TypeDef->Type != TSTypeDef::Service)
		{
			continue;
		}
		else if (TypeDef->Type == TSTypeDef::Service)
		{
			//服务不再重复生成
			/*if (_serviceFlagStruct->buildServiceSkel)
			{
				continue;
			}*/
			//如果没有选中服务则不生成服务
			if (!vecSelectService.empty())
			{
				if (vecSelectService[0] == "N")
				{
					continue;
				}
			}
		}
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(TSSolutionPtr Solution = 
            SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
        {
            if (TypeDef->Type != TSTypeDef::Service)
            {
                continue;
            }
            TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);

            TSString Path = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
            boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

            TSProjFilePtr ServiceSkelFile = Dir->GetOrCreateProjFile(Path + "/ServiceSkel",".cs");
            ServiceSkelFile->SetFileType(Generic);
            GenerateServiceSkel(ServiceSkelFile,TypeDef, indexServiceVec);
            ServiceSkelFile->AppendFirst(SERVICE_SERVICE_HEADER_4CS);
        }
    }
	_serviceFlagStruct->buildServiceSkel = true;
}

void TSCSharpPlugin::GenerateServiceSkel( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName)
{
	bool FirstOrNot = false;
    TSString content = SERVICE_SERVICEKEL_CODER_4CS;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSString suit_full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    TSString suit_full_Request_name,suit_Remove_Topic,funcstr,funcAdd;
    TSString ServerName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true) + "::ServerSkel";
	TSString registerMethod,requstHandler;
    BOOST_FOREACH(TSFunctionDefPtr func,TypeDef->Functions)
    {
		/*TSString funName = func->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
		TSString paramIndex;
        TSString suit_Request = suit_full_type_name;
        TSString suit_Response = suit_full_type_name;
        suit_full_Request_name += Table(3) + "PublishTopic(";
        suit_Request += "::" + func->ValName + "::Request";
        suit_full_Request_name += suit_Request +".ID);\n";
        suit_full_Request_name += "SubscribeTopic(";
        suit_Response += "::" + func->ValName + "::Response";
        suit_full_Request_name += suit_Response + ".ID);\n";
        suit_Remove_Topic += Table(3) + "UnpublishTopic(";
        suit_Remove_Topic += suit_Request + ".ID);\n";
        suit_Remove_Topic += Table(3) + "UnsubscribeTopic(";
        suit_Remove_Topic += suit_Response + ".ID);\n";
        boost::algorithm::replace_all(suit_full_Request_name, "::","_");

        boost::algorithm::replace_all(suit_Remove_Topic, "::","_");
        TSString suit_Request_ID = suit_Request;
        boost::algorithm::replace_all(suit_Request_ID, "::","_");
        boost::algorithm::replace_all(suit_Request, "::",".");

        TSString suit_Response_ID = suit_Response;
        boost::algorithm::replace_all(suit_Response_ID, "::","_");
        boost::algorithm::replace_all(suit_Response, "::",".");

		registerMethod += Table(4) + "RegisterMethod(\"" + func->ValName + "\"" + "," + suit_Request_ID + ".ID,\n";
		registerMethod += Table(4) + suit_Response_ID + ".ID,\n";
		registerMethod += Table(4) + "(LinkStudioInvoke.TSRPCInitalizer.MethodRequestHandler)" + func->ValName + "RequestHandler,null);\n";
		requstHandler += Table(2) + "static void " + func->ValName + "RequestHandler(IntPtr UserData, UInt32 RequestTopic, IntPtr RequestDataIn, UInt32 ResponseTopic, IntPtr ResponseDataOut)\n";
		requstHandler += Table(2) + "{\n";
		requstHandler += Table(3) + "IntPtr wrapData = LinkStudioInvoke.TSRPCInitalizer.TSRPC_CS_RawDataUnwarpper(RequestDataIn);\n";
		requstHandler += Table(3) + "ServerSkel Skel = GCHandle.FromIntPtr(UserData).Target as ServerSkel;\n";
		requstHandler += Table(3) + func->ValName + ".Request Req = GCHandle.FromIntPtr(wrapData).Target as " + func->ValName + ".Request;\n";
		

        funcstr += Table(2) + "private  object " + func->ValName + "Handler(object Request)\n";
        funcstr += Table(2) + "{\n";
        funcstr += Table(3) + func->ValName + ".Request Req = Request as " + func->ValName + ".Request;\n";
        
        funcstr += Table(3) + func->ValName + ".Response Response = new " + func->ValName + ".Response();\n";
        TSString ResponseFun,ParamStar,ResponseOut,IsAysOut; 
        TSString rntStr = "";
        for(int i = 0; i < func->Params.size(); i++)
        {
            if (TSParamDefPtr paramDef = func->Params[i])
            {
                if (paramDef->Type != TSParamDef::Out)
                {
                    if (paramDef->Type == TSParamDef::Inout)
                    {
                        ResponseFun += "ref Req." + paramDef->ValName;
                    }
                    else
                    {
                        ResponseFun += "Req." + paramDef->ValName;
                    }
                }
                else
                {
         
                    ResponseFun += "out " + paramDef->ValName;
                }

				if (paramDef->Type == TSParamDef::Out)
				{
					IsAysOut += Table(3) + TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + " " + paramDef->ValName;

					if(TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) == "string")
					{
						IsAysOut += " = \"\";\n";
					}
					else if(TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) == "string[]")
					{
						IsAysOut += " = new string[]{""};\n";
					}
					else if(paramDef->Def->Type == TSTypeDef::Enum)
					{
						IsAysOut +=" = new "
							+ TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "();\n";
					}
					else if(paramDef->Def->Type == TSTypeDef::Builtin)
					{
						IsAysOut +=" = new "
							+ TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "();\n";
					}
					else if(paramDef->Def->Type == TSTypeDef::Array)
					{
						TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(paramDef->Def,TSVectorBuiltinTypeDefPtr);
						if (vectorField->Value->Type != TSTypeDef::Builtin)
						{
							IsAysOut +=  " = new List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">();\n";
						}
						else
						{
							IsAysOut += " = new "
								+ TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "{};\n";
						}
					}                   
					else
					{
						IsAysOut += " = new "
							+  TSBuiltinPlugin::GetSuitFullTypeName(paramDef->Def) + "{};\n";
					}
				}


                if (i != func->Params.size() - 1)
                {
                    ResponseFun += ",";
                }

                if (paramDef->Type != TSParamDef::In)
                {
					if (paramDef->Type == TSParamDef::Out)
					{
						ResponseOut += Table(3) + "Resp." + paramDef->ValName + " = " + paramDef->ValName + ";\n";
					}
					else
					{
					   ResponseOut += Table(3) + "Resp." + paramDef->ValName + " = Req." + paramDef->ValName + ";\n";
					}
                    
                }
            }
        }
		requstHandler += IsAysOut;
		if (func->DefTypeName != "void" && 
			func->DefTypeName != "Void")
		{          
			ResponseOut += Table(3) + "Resp.rnt = rnt;\n";
			requstHandler += Table(3) + GetCSharpValueType(func->Def) + " rnt = "; 

		}
		requstHandler += "((Skel.GetMethodT<TSMethod>()) as Method)." + func->ValName + "(";
        requstHandler += ResponseFun;
		requstHandler += ");\n";
		requstHandler += Table(3) + "IntPtr OutData = LinkStudioInvoke.TSRPCInitalizer.TSRPC_CS_RawDataUnwarpper(ResponseDataOut);\n";
		requstHandler += Table(3) +  func->ValName + ".Response Resp = GCHandle.FromIntPtr(OutData).Target as " + func->ValName + ".Response;\n";


        
        
		requstHandler += ResponseOut;
        requstHandler += Table(2) + "}\n";
    }

	 boost::algorithm::replace_all(content, "_FUN_REGISTERMETHOD_",registerMethod);

    boost::algorithm::replace_all(content, "_FUNC_HANDLER_",requstHandler);
    boost::algorithm::replace_all(content, "_SERVERSKEL_TYPENAME_",ServerName);
    boost::algorithm::replace_all(suit_full_type_name, "::","_");
    ProjFile->AppendLast(content);
}

void TSCSharpPlugin::BuildMethodImpl()
{
    std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	//std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	//GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	//std::vector<TSString> tempServiceVec;
	//获得要生成的服务，如果追加的服务为空则返回true
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildMethodImplVec, _serviceFlagStruct->buildMethodImpl);

	//std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
		if (TypeDef->Type == TSTypeDef::Struct)
		{
			continue;
		}
		else if (TypeDef->Type == TSTypeDef::Service)
		{
			/*if (tempServiceFlag)
			{
				continue;
			}
			if (_serviceFlagStruct->buildMethodImpl)
			{
				continue;
			}*/
			if (!vecSelectService.empty())
			{
				if (vecSelectService[0] == "N")
				{
					continue;
				}
			}
		}
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());

        if(TSSolutionPtr Solution = 
            SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
        {
            if (TypeDef->Type != TSTypeDef::Service)
            {
                continue;
            }
            TSString Path = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);

			TSString fullPath = Solution->GetFullPath();
			boost::algorithm::replace_all(fullPath, "\\", "\/");
			boost::algorithm::replace_all(fullPath, "c#", "csharp");
            boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

			if (Path == ParseFileBaseName)
			{
				Path = ParseFileBaseName + "." + Path;
			}

            TSProjectPtr Dir  = Solution->GetOrCreateProj(Path);
            Dir->SetProFileType(Impl);
            TSProjFilePtr MethodImplFile = Dir->GetOrCreateProjFile("MethodImpl",".cs");
            MethodImplFile->SetFileType(Impl);

			TSString implPath = Path;
			boost::algorithm::replace_all(implPath, "::", ".");
			implPath = fullPath + "/impl/" + implPath;
			//设置impl目录路径
			TSIDLPluginMgr::Instance()->SetImplFullPath(implPath);

            GenerateMethodImpl(MethodImplFile,TypeDef, indexServiceVec);
            MethodImplFile->AppendFirst(METHOD_HEAD_4CS_CODER);
        }
    }
	//_serviceFlagStruct->buildMethodImpl = true;
}

void TSCSharpPlugin::GenerateMethodImpl( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName)
{
    TSString content = METHODIMPL_SERVICE_CODER_4CS;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    TSString funcstr;
    BOOST_FOREACH(TSFunctionDefPtr Func,TypeDef->Functions)
    {
		/*TSString funName = Func->ValName;
		std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
        TSString TypeName = GetCSharpValueType(Func->Def);
        funcstr += Table(2) + " public override " + TypeName + " " +
            TSIDLUtils::GetFuncWithoutResultStr(Func) + "\n";
        funcstr += Table(2) + "{\n";
        funcstr += Table(3) + "throw new NotImplementedException();\n";
        funcstr += Table(2) + "}\n";
    }
    boost::algorithm::replace_all(content,"_FUNS_",funcstr);
    ProjFile->AppendLast(content);
}

void TSCSharpPlugin::GenerateTopicWithService( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName, bool FirstOrNot,const bool & Requst)
{
    
    BOOST_FOREACH(TSFunctionDefPtr Func,TypeDef->Functions)
    {
		TSString funName = Func->ValName;
		/*std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
        TSString content = __DATA_TYPE_TOPIC_4CS__;
        TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
            __NAMESPACE_END__,
            content,
            TypeDef,
            funName,
			FirstOrNot);
        TSIDLUtils::ReplaceInitList(__CONSTRUCT_LIST__,
            content,TypeDef);
        TSIDLUtils::ReplaceFiledsAssign(_EXTEND_,
            "other",content,TypeDef);
        TSIDLUtils::ReplaceFiledsAssign(__FIELD_COPYASSIGN__,
            "other",content,TypeDef);

       if(Requst)
       {
            boost::algorithm::replace_all(content, "DataType","Request");
       }
       else
       {
            boost::algorithm::replace_all(content, "DataType","Response");
       }
        
        TSIDLUtils::ReplaceFieldDeclareWithService(__FIELD_DECLARE__,
            content,Func,Requst);
        ProjFile->AppendLast(content);
    }
    
}

//过滤掉没有选中的节点以及可能重复生成的节点
bool TSCSharpPlugin::FilterNotSelectNode(TSTypeDefPtr TypeDef, std::vector<TSString>& indexVec, std::vector<TSString>& structVec, std::vector<TSString>& storeStructRec)
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

void TSCSharpPlugin::GenerateTopic( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = __DATA_TYPE_TOPIC_4CS__;
    TSString custon,paramIint,writeBuffer, readBuffer;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSIDLUtils::ReplaceInitList(__CONSTRUCT_LIST__,
        content,TypeDef);

    TSIDLUtils::ReplaceFiledsAssign(__FIELD_COPYASSIGN__,
        "other",content,TypeDef);

    TSIDLUtils::ReplaceFiledsAssign(_EXTEND_,
        "other",content,TypeDef);

    TSIDLUtils::ReplaceFiledsAssign(__FIELD_ASSIGN__,
        "other",content,TypeDef);

    TSIDLUtils::ReplaceFiledsDeclare(__FIELD_DECLARE__,
        content,TypeDef);

    TSString suit_full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
    if (TypeDef->Type == TSTypeDef::Service)
    {
        suit_full_type_name = "TSBasicService";
    }
    TSString TypeFullName;
    if(!suit_full_type_name.empty())
    {
         TypeFullName = ":" + suit_full_type_name + ".DataType";
         boost::algorithm::replace_all(content, "_NEW_LIZE_", "new");
		 if (TypeDef->Type == TSTypeDef::Service)
		 {
			  writeBuffer += Table(3) + "base.Serialize(buffer,MarshaledFlag);" + TSIDL_NEWLINE;
		 }
		 else
		 {
			writeBuffer += Table(3) + "base.Serialize(buffer,MarshaledFlag);" + TSIDL_NEWLINE;
		 }
          
         readBuffer += Table(3) + "base.Deserialize(buffer);" + TSIDL_NEWLINE;
    }
    else if(TypeDef->Type != TSTypeDef::Topic &&
        TypeDef->Type != TSTypeDef::Struct)
    {
        TypeFullName = " : TSBasicSdo";
        
    }

    boost::algorithm::replace_all(content, "_NEW_LIZE_", "");
    boost::algorithm::replace_all(content,__SUPER_TYPE__,TypeFullName);
    boost::algorithm::replace_all(content, "__BASE_SERIALIZE__", "");
    boost::algorithm::replace_all(content, "__BASE_DESERIALIZE__", "");

    for(size_t fieldId = 0; fieldId < TypeDef->Fileds.size(); ++fieldId)
    {
        if(TSFieldDefPtr field = TypeDef->Fileds[fieldId])
        {
 
            if (field->Def->Type == TSTypeDef::Array)
            {
                TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(field->Def,TSVectorBuiltinTypeDefPtr);
                if (vectorField->Value->Type != TSTypeDef::Builtin)
                {
                    custon += Table(3) + field->ValName +  " = new List<";      
                    custon += TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">();" + TSIDL_NEWLINE;
                    writeBuffer += Table(3) + "buffer.WriteInt32(" + field->ValName + ".Count);" + TSIDL_NEWLINE;
                    writeBuffer += Table(3) + "foreach (var item in " + field->ValName + ")" + TSIDL_NEWLINE;
                    writeBuffer += Table(3) + "{" + TSIDL_NEWLINE;
                    if (vectorField->Value->Type == TSTypeDef::Enum)
                    {
                        writeBuffer += Table(4) + "buffer.WriteInt32((int)item);" + TSIDL_NEWLINE;
                    }
                    else
                    {
                        writeBuffer += Table(4) + "item.Serialize(buffer,MarshaledFlag);" + TSIDL_NEWLINE;
                    }
                  
                    writeBuffer += Table(3) + "}" + TSIDL_NEWLINE;
                    readBuffer += Table(3) + "uint " + field->ValName + "Capacity = buffer.ReadUInt32();" + TSIDL_NEWLINE;
                    readBuffer += Table(3) + "for(uint i = 0; i < " + field->ValName + "Capacity; i++)" + TSIDL_NEWLINE;
                    readBuffer += Table(3) + "{" + TSIDL_NEWLINE;
                    if (vectorField->Value->Type == TSTypeDef::Enum)
                    {
                        readBuffer += Table(4) + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + " topic = (" +
                             TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ")buffer.ReadInt32();\n";
                    }
                    else
                    {
                        readBuffer += Table(4) + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + " topic = " + 
                            "new " + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + "();\n"; 
                        readBuffer += Table(4) + "topic.Deserialize(buffer);\n";
                    }
                    
                    readBuffer += Table(4) + field->ValName + ".Add(topic);\n";
                    readBuffer += Table(3) + "}" + TSIDL_NEWLINE;
                }
                else
                {   
                    TSString fiedType = TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value);
                    if (fiedType == "TSVector2d" || fiedType == "TSVector3d" || fiedType == "TSVector2f" || fiedType == "TSVector3f"
                        || fiedType == "TSHANDLE" || fiedType == "handle" || fiedType == "TSModelTmplOrClassId" || fiedType == "TSTOPICHANDLE")
                    {
                        custon += Table(3) + field->ValName +  " = new List<";      
                        custon += TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">();" + TSIDL_NEWLINE;
                        writeBuffer += Table(3) + "buffer.WriteInt32(" + field->ValName + ".Count);" + TSIDL_NEWLINE;
                        writeBuffer += Table(3) + "foreach (" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + " item in " + 
                        field->ValName + ")" + TSIDL_NEWLINE;
                        writeBuffer += Table(3) + "{" + TSIDL_NEWLINE;
                        writeBuffer += Table(4) + "buffer.Write" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + "(item);" + TSIDL_NEWLINE;
                        writeBuffer += Table(3) + "}" + TSIDL_NEWLINE;
                        readBuffer += Table(3) + field->ValName + ".Capacity = buffer.ReadInt32();" + TSIDL_NEWLINE;
                        readBuffer += Table(3) + "for(uint i = 0; i < " + field->ValName + ".Capacity; i++)" + TSIDL_NEWLINE;
                        readBuffer += Table(3) + "{" + TSIDL_NEWLINE;
                        readBuffer += Table(4) + field->ValName + ".Add(buffer.Read" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + 
                        "());" + TSIDL_NEWLINE;
                        readBuffer += Table(3) + "}" + TSIDL_NEWLINE;

                    }
                    else
                    {
                        custon += Table(3) + field->ValName +  " = new " + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + "[0];\n";      
                   
                        writeBuffer += Table(3) + "buffer.WriteArray(" + field->ValName +");\n";

                        if (fiedType == "sbyte")
                        {
                            readBuffer += Table(3) + "this." + field->ValName + " = buffer.ReadSByteForArray();\n";
                        }
                        else if (fiedType == "byte")
                        {
                            readBuffer += Table(3) + "this." + field->ValName + " = buffer.ReadByteForArray();\n";
                        }
                        else
                        {
                            readBuffer += Table(3) + "this." + field->ValName + " = buffer.Read" +  fiedType +"ForArray();\n";
                        }                 
                    }
                }                             
            }
            else if (field->Def->Type == TSTypeDef::Map)
            {
                custon += Table(3) + field->ValName +  " = new " + TSBuiltinPlugin::GetSuitFullTypeName(field->Def) + "();\n";
                TSMapBuiltinTypeDefPtr mapField = TS_CAST(field->Def,TSMapBuiltinTypeDefPtr);
                writeBuffer += Table(3) + "int " + field->ValName +"count = " + field->ValName + ".Count;\n";
                writeBuffer += Table(3) + "buffer.WriteInt32(" + field->ValName +"count);" + TSIDL_NEWLINE;
                writeBuffer += Table(3) + "foreach (KeyValuePair<" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + 
                    ", " + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + "> kvp in " + 
                    field->ValName + ")" + TSIDL_NEWLINE;
                writeBuffer += Table(3) + "{\n";
				writeBuffer += Table(4) + "buffer.Write" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + "(kvp.Key);" + TSIDL_NEWLINE;      
                readBuffer += Table(3) + "int " + field->ValName + "count = buffer.ReadInt32();" + TSIDL_NEWLINE;
                readBuffer += Table(3) + "for (int i = 0; i < " + field->ValName + "count; i++)\n";
                readBuffer += Table(3) + "{\n";

                 if(mapField->Value->Type  == TSTypeDef::Array)
                 {
                     writeBuffer += Table(4) + "buffer.WriteArray"  + "(kvp.Value);" + TSIDL_NEWLINE;
                     if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) == "sbyte[]")
                     {
                         readBuffer += Table(4) + field->ValName + ".Add(buffer.Read" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + "(),"
                             "buffer.ReadSByteForArray());\n";
                     }
                     else if (TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) == "byte[]")
                     {
                         readBuffer += Table(4) + field->ValName + ".Add(buffer.Read" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + "(),"
                            "buffer.ReadByteForArray());\n";
                     }
                     else
                     {
                         readBuffer += Table(4) + field->ValName + ".Add(buffer.Read" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + 
                             "(), buffer.Read" +TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + "());" + TSIDL_NEWLINE;
                     }
                 }
                 else
                 {
                     writeBuffer += Table(4) + "buffer.Write" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + "(kvp.Value);" + TSIDL_NEWLINE;
                     readBuffer += Table(4) + field->ValName + ".Add(buffer.Read" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + 
                         "(), buffer.Read" +TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + "());" + TSIDL_NEWLINE;
                 }

                 writeBuffer += Table(3) + "}\n";
                 readBuffer += Table(3) + "}\n";
                 boost::algorithm::replace_all(readBuffer, "[]()",   "ForArray()");
            }
            else if (field->Def->Type !=  TSTypeDef::Builtin &&
                field->Def->Type != TSTypeDef::Enum )
            {
                custon += Table(3) + field->ValName + " = new " + TSBuiltinPlugin::GetSuitFullTypeName(field->Def) + "();\n";
                TSIDL_NEWLINE;
               // if(field->HaveKey)
                //{
                    writeBuffer += Table(3) + "UInt32 KeyType_" + field->ValName + " = " + boost::lexical_cast<TSString>(field->FindSetEnumType) + ";\n";
                    writeBuffer += Table(3) + "UInt32 Offset_" + field->ValName + " = buffer.TellW();\n";
                //}
					writeBuffer += Table(3) + "UInt32 shaledFlag_" + field->ValName + "=  MarshaledFlag == 0 ? MarshaledFlag : KeyType_" + field->ValName  + ";\n";
					writeBuffer += Table(3) + field->ValName + ".Serialize(buffer,shaledFlag_" + field->ValName + ");" + TSIDL_NEWLINE;
               // if (field->HaveKey)
                //{
                    writeBuffer += Table(3) + "UInt32 Size_" + field->ValName + " = buffer.TellW() - Offset_" + field->ValName + ";\n";
					
                    writeBuffer += Table(3) + "if (0 != shaledFlag_" + field->ValName +")\n";
                    writeBuffer += Table(3) + "{\n";
                    writeBuffer += Table(3) + Table() + "buffer.MarshalKey(shaledFlag_" + field->ValName + ",Offset_" + 
                        field->ValName + ",Size_" + field->ValName + ");\n";
                    writeBuffer += Table(3) + "}\n";

                //}
                readBuffer += Table(3) + field->ValName + ".Deserialize(buffer);" + TSIDL_NEWLINE;
            }
            else if (field->Def->Type == TSTypeDef::Enum)
            {
                //if(field->HaveKey)
                //{
                    writeBuffer += Table(3) + "UInt32 KeyType_" + field->ValName + " = " + boost::lexical_cast<TSString>(field->FindSetEnumType) + ";\n";
                    writeBuffer += Table(3) + "UInt32 Offset_" + field->ValName + " = buffer.TellW();\n";
                //}
                writeBuffer += Table(3) + "buffer.WriteINT32((int)" + field->ValName + ");" + TSIDL_NEWLINE;
                //if (field->HaveKey)
                //{
                    writeBuffer += Table(3) + "UInt32 Size_" + field->ValName + " = buffer.TellW() - Offset_" + field->ValName + ";\n";
					writeBuffer += Table(3) + "UInt32 shaledFlag_" + field->ValName +"=  MarshaledFlag == 0 ? MarshaledFlag : KeyType_" + field->ValName  + ";\n";
					writeBuffer += Table(3) + "if (0 != shaledFlag_" + field->ValName + ")\n";
					writeBuffer += Table(3) + "{\n";
					writeBuffer += Table(3) + Table() + "buffer.MarshalKey(shaledFlag_" + field->ValName + ",Offset_" + 
                        field->ValName + ",Size_" + field->ValName + ");\n";
                    writeBuffer += Table(3) + "}\n";

                //}
                readBuffer += Table(3) + "this." + field->ValName + " = (" + TSBuiltinPlugin::GetSuitFullTypeName(field->Def) + 
                    ")buffer.ReadINT32();" + TSIDL_NEWLINE;
            }
            else
            {
                //if(field->HaveKey)
                //{
                    writeBuffer += Table(3) + "UInt32 KeyType_" + field->ValName + " = " + boost::lexical_cast<TSString>(field->FindSetEnumType) + ";\n";
                    writeBuffer += Table(3) + "UInt32 Offset_" + field->ValName + " = buffer.TellW();\n";
                //}
                writeBuffer += Table(3) + "buffer.Write" + TSBuiltinPlugin::GetSuitFullTypeName(field->Def) + "(" + field->ValName + ");" + TSIDL_NEWLINE;
                //if (field->HaveKey)
                //{
                    writeBuffer += Table(3) + "UInt32 Size_" + field->ValName + " = buffer.TellW() - Offset_" + field->ValName + ";\n";
					writeBuffer += Table(3) + "UInt32 shaledFlag_" + field->ValName +"=  MarshaledFlag == 0 ? MarshaledFlag : KeyType_" + field->ValName  + ";\n";
					writeBuffer += Table(3) + "if (0 != shaledFlag_" + field->ValName + ")\n";
					writeBuffer += Table(3) + "{\n";
					writeBuffer += Table(3) + Table() + "buffer.MarshalKey(shaledFlag_" + field->ValName + ",Offset_" + 
						field->ValName + ",Size_" + field->ValName + ");\n";
                    writeBuffer += Table(3) + "}\n";

                //}
                readBuffer += Table(3) + "this." + field->ValName + " = buffer.Read" + 
                    TSBuiltinPlugin::GetSuitFullTypeName(field->Def) + "();" + TSIDL_NEWLINE;
            }

        }
    }

    boost::algorithm::replace_all(content,"_CUSTON_",custon);
    boost::algorithm::replace_all(content, "_BUFFER_WRITE_", writeBuffer);     
    boost::algorithm::replace_all(content, "_BUFFER_READ_", readBuffer);

    boost::algorithm::replace_all(content,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

    boost::algorithm::replace_all(content, "Readbool",   "ReadBool");
    boost::algorithm::replace_all(content, "Readsbyte",  "ReadInt8");
    boost::algorithm::replace_all(content, "Readbyte",   "ReadUInt8");
    boost::algorithm::replace_all(content, "Readshort",  "ReadInt16");
    boost::algorithm::replace_all(content, "Readushort", "ReadUInt16");
    boost::algorithm::replace_all(content, "Readint64",  "ReadInt64");
    boost::algorithm::replace_all(content, "Readint16",  "ReadInt16");
    boost::algorithm::replace_all(content, "Readint8",   "ReadInt8");
    boost::algorithm::replace_all(content, "Readint32",  "ReadInt32");
    boost::algorithm::replace_all(content, "Readint",    "ReadInt32");
    boost::algorithm::replace_all(content, "Readlong",   "ReadInt64");
    boost::algorithm::replace_all(content, "Readulong",  "ReadUInt64");
    boost::algorithm::replace_all(content, "Readfloat64", "ReadFloat");
    boost::algorithm::replace_all(content, "Readfloat32", "ReadFloat");
    boost::algorithm::replace_all(content, "Readfloat",   "ReadFloat");

    boost::algorithm::replace_all(content, "Readdouble", "ReadDouble");
    boost::algorithm::replace_all(content, "ReadSBYTE",  "ReadInt8");
    boost::algorithm::replace_all(content, "ReadBYTE",   "ReadUInt8");
    boost::algorithm::replace_all(content, "ReadSHORT",  "ReadInt16");
    boost::algorithm::replace_all(content, "ReadUSHORT", "ReadUInt16");
    boost::algorithm::replace_all(content, "ReadINT",  "ReadInt");

    boost::algorithm::replace_all(content, "ReadLONG",   "ReadInt64");
    boost::algorithm::replace_all(content, "ReadULONG",  "ReadUInt64");

    boost::algorithm::replace_all(content, "ReadUINT", "ReadUInt");
    boost::algorithm::replace_all(content, "ReadFLOAT",  "ReadFloat");
    boost::algorithm::replace_all(content, "ReadDOUBLE", "ReadDouble");
    boost::algorithm::replace_all(content, "Readstring", "ReadString");
    boost::algorithm::replace_all(content, "Readuint64", "ReadUInt64");
    boost::algorithm::replace_all(content, "Readuint32", "ReadUInt32");
    boost::algorithm::replace_all(content, "Readuint16", "ReadUInt16");
    boost::algorithm::replace_all(content, "Readuint8",  "ReadUInt8");
    boost::algorithm::replace_all(content, "Readuint",   "ReadUInt32");
    boost::algorithm::replace_all(content, "Writebool",   "WriteBool"); 
    boost::algorithm::replace_all(content, "WriteBYTE",   "WriteUInt8"); 
    boost::algorithm::replace_all(content, "WriteSBYTE",  "WriteInt8");
    boost::algorithm::replace_all(content, "WriteSHORT",  "WriteInt32");
    boost::algorithm::replace_all(content, "WriteUSHORT", "WriteUInt32");

    boost::algorithm::replace_all(content, "WriteLONG",   "WriteInt64");
    boost::algorithm::replace_all(content, "WriteULONG",  "WriteUInt64");
    boost::algorithm::replace_all(content, "WriteUINT",   "WriteUInt");
    boost::algorithm::replace_all(content, "WriteFLOAT",  "WriteFloat");
    boost::algorithm::replace_all(content, "WriteDOUBLE", "WriteDouble");
    boost::algorithm::replace_all(content, "Writebyte",   "WriteUInt8"); 
    boost::algorithm::replace_all(content, "Writesbyte",  "WriteInt8");
    boost::algorithm::replace_all(content, "Writeshort",  "WriteInt16");
    boost::algorithm::replace_all(content, "WriteINT",    "WriteInt");
    boost::algorithm::replace_all(content, "Writeushort", "WriteUInt16");
    boost::algorithm::replace_all(content, "Writestring", "WriteString");
    boost::algorithm::replace_all(content, "Writeuint64", "WriteUInt64");
    boost::algorithm::replace_all(content, "Writeuint32", "WriteUInt32");
    boost::algorithm::replace_all(content, "Writeuint16", "WriteUInt16");
    boost::algorithm::replace_all(content, "Writeuint8",  "WriteUInt8");
    boost::algorithm::replace_all(content, "Writeuint",   "WriteUInt32");
    boost::algorithm::replace_all(content, "Writeint64",  "WriteInt64");
    boost::algorithm::replace_all(content, "Writeint32",  "WriteInt32");
    boost::algorithm::replace_all(content, "Writeint16",  "WriteInt16");
    boost::algorithm::replace_all(content, "Writeint8",   "WriteInt8");
    boost::algorithm::replace_all(content, "Writeint",    "WriteInt32");
    boost::algorithm::replace_all(content, "Writelong",   "WriteInt64");
    boost::algorithm::replace_all(content, "Writeulong",  "WriteUInt64");

    boost::algorithm::replace_all(content, "Writefloat64",  "WriteFloat");
    boost::algorithm::replace_all(content, "Writefloat32",  "WriteFloat");
    boost::algorithm::replace_all(content, "Writefloat",    "WriteFloat");

    boost::algorithm::replace_all(content, "Writedouble", "WriteDouble");
 

    boost::algorithm::replace_all(content, "ReadTSHANDLE",  "ReadTSTOPICHANDLE");
    boost::algorithm::replace_all(content, "ReadTSBasicSdo", "ReadTSObjectHandle");
    boost::algorithm::replace_all(content, "WriteTSHANDLE", "WriteTSTOPICHANDLE");
    boost::algorithm::replace_all(content, "WriteTSBasicSdo", "WriteTSObjectHandle");

    ProjFile->AppendLast(content);
}

void TSCSharpPlugin::BuildSupport()
{
    TSProjFilePtr SupportFile;

    std::vector<TSTypeDefWithFuncAndFiledPtr> TypeDefs = _p->_BuiltinPlugin->GetAllTypeDefWithFuncAndFiled();

	//std::map<TSString, TSString> serviceMap;
	//获得服务和其依赖的结构体存入serviceMap容器中
	//GetServiceAndStructMap(TypeDefs, serviceMap);

	//获取主题选择工具中选中的服务
	std::vector<TSString> vecSelectService = TSIDLPluginMgr::Instance()->GetSelectServiceVector();
	//std::vector<TSString> tempServiceVec;
	//获得要生成的服务
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildSupportVec, _serviceFlagStruct->buildSupport);

	//std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

	//获取主题选择工具中选中的主题
	std::vector<TSString> vecSelectTopic = TSIDLPluginMgr::Instance()->GetSelectTopicVector();
	//std::vector<TSString> tempTopicVec;
	//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
	//bool emptyTopicFlag = GetGenerateTopic(vecSelectTopic, tempTopicVec, _topicVecStruct.buildSupportVec, _topicFlagStruct->buildSupport);

	//std::vector<TSString> indexVec;
	//if (!tempTopicVec.empty() && !vecSelectTopic.empty())
	//{
	//	//用来存储没有被选中的主题
	//	indexVec = GetTopicInVec(tempTopicVec, TypeDefs);
	//}

    BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr TypeDef,TypeDefs)
    {
        if (TypeDef->Type != TSTypeDef::Topic && TypeDef->Type != TSTypeDef::Struct &&
            TypeDef->Type != TSTypeDef::Service)
        {
            continue;
        }

		//过滤掉没有选中的节点以及可能重复生成的节点
		/*if (FilterNotSelectNode(TypeDef, indexVec, structVec, _storeStructRec.buildSupportRec))
		{
			continue;
		}*/

		//如果没有选中服务，那么跳过服务的生成
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

		_storeStructRec.buildSupportRec.push_back(TypeDef->TypeName);

        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
        if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
        {
            if(TSSolutionPtr Solution = 
                SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
            {
                TSProjectPtr Dir = Solution->GetOrCreateProj(ParseFileBaseName);
                SupportFile = Dir->GetOrCreateProjFile("Support",".cs");
                SupportFile->SetFileType(None);
                GenerateSupport(SupportFile,TypeDef);
                if (TypeDef->Type == TSTypeDef::Service)
                {
                    GenerateSupportWithService(SupportFile,TypeDef, indexServiceVec);
                    GenerateSupportWithService(SupportFile,TypeDef, indexServiceVec,false);
                }
            }
        }  
    }
    std::vector<TSSolutionPtr> solutions = SolutionMgr()->GetSolutions(); 
    std::vector<TSSolutionPtr>::const_iterator its = solutions.begin();
    BOOST_FOREACH(TSSolutionPtr its,solutions)
    {
        const ProjMap_T &Projs = its->GetProjects();
        for (ProjMap_T::const_iterator iter = Projs.begin(); iter != Projs.end(); iter++)
        {
            const ProjFileMap_T files = iter->second->GetProjFiles();
            TSString baseDir = iter->second->GetShortName();
            for (ProjFileMap_T::const_iterator iterfile = files.begin(); iterfile != files.end(); iterfile++)
            {
                TSString FileName = iterfile->first;
                if (FileName == "Support.cs" && !_topicFlagStruct->buildSupport)
                {
                    TSString content = __HEADER_INCLUDE_TOPIC_4CS__;
                    iterfile->second->AppendFirst(content);
                }
            }
        }
    }
	_topicFlagStruct->buildSupport = true;
}

void TSCSharpPlugin::GenerateSupportWithService( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef, std::vector<TSString>& serviceName,const bool & Requst /*= true*/ )
{ 
    BOOST_FOREACH(TSFunctionDefPtr Func,TypeDef->Functions)
    {
		TSString funName = Func->ValName;
		/*std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
		if (iterTopic == serviceName.end())
		{
			continue;
		}*/
        TSString content = __DATA_DEFINE_SUPPORT_4CS__;
        TSString FlagType = "TF_DataFlag";
        if (TypeDef->Type == TSTypeDef::Topic || TypeDef->Type == TSTypeDef::Struct)
        {
            FlagType = "TF_DataFlag"; 
        }
        else if (TypeDef->Type == TSTypeDef::Service)
        {
            FlagType = "TF_Service";
        }
        else
        {
            FlagType = "TF_InvalidFlag";
        }

        TSString getTypeName = TSBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
        if (Requst)
        {
			
			boost::algorithm::replace_all(content,"_HASHVERSION_CODE_",TSIDLUtils::GenerateTypeVersionNum4Service(Func,true));
            boost::algorithm::replace_all(content, "Support","RequestSupport");
            boost::algorithm::replace_all(content, "DataType","Request");
            getTypeName += "::" + Func->ValName + "::Request";
        }
        else
        {
			boost::algorithm::replace_all(content,"_HASHVERSION_CODE_",TSIDLUtils::GenerateTypeVersionNum4Service(Func,false));
            boost::algorithm::replace_all(content, "Support","ResponseSupport");
            boost::algorithm::replace_all(content, "DataType","Response");
            getTypeName += "::" + Func->ValName + "::Response";
        }
        
        TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
            __NAMESPACE_END__,
            content,
            TypeDef,
            funName);
       
        boost::algorithm::replace_all(content,"_FLAG_TYPE_",FlagType);
        boost::algorithm::replace_all(content,"_SELF_NAME_",getTypeName);
        boost::algorithm::replace_all(content,"__DATA_FINDSET__","");
        ProjFile->AppendLast(content);
    }
}

void TSCSharpPlugin::GenerateSupport( TSProjFilePtr ProjFile,TSTypeDefWithFiledPtr TypeDef )
{
    TSString content = __DATA_DEFINE_SUPPORT_4CS__;
    TSString findcontent = __DATA_FINDSET__;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSString FindSetContent;

	if(TypeDef->Extend.lock())
	{
		FindSetContent = TSString("if(") +
			TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true) + ".Support.HasFindSetT(Flag)){return true;};\n";

		boost::algorithm::replace_all(FindSetContent,"::",".");
	}

    BOOST_FOREACH(TSFieldDefPtr field,TypeDef->Fileds)
    {
        if (TESTBIT(field->FindSet,TSFieldDef::Handle))
        {
			FindSetContent += __DATA_FINDSET_CONTENT_;
			boost::algorithm::replace_all(FindSetContent,"_KHANDLER_FINDSET_","kHandleType");
        }
        else if(TESTBIT(field->FindSet,TSFieldDef::SpacialRange))
        {
		   FindSetContent += __DATA_FINDSET_CONTENT_;
		   boost::algorithm::replace_all(FindSetContent,"_KHANDLER_FINDSET_","kSpacialRangeType");
        }
        else if (TESTBIT(field->FindSet,TSFieldDef::ModelOrTmplId))
        {
			FindSetContent += __DATA_FINDSET_CONTENT_;
			boost::algorithm::replace_all(FindSetContent,"_KHANDLER_FINDSET_","kModelTmplOrClassIdType");
        }
        else if (TESTBIT(field->FindSet,TSFieldDef::Key))
        {
			FindSetContent += __DATA_FINDSET_CONTENT_;
			boost::algorithm::replace_all(FindSetContent,"_KHANDLER_FINDSET_","kKey");
        }
    }
    TSString FlagType;
    if (TypeDef->Type == TSTypeDef::Topic || TypeDef->Type == TSTypeDef::Struct)
    {
        FlagType = "TF_DataFlag"; 
    }
    else if (TypeDef->Type == TSTypeDef::Service)
    {
        FlagType = "TF_Service";
    }
    else
    {
        FlagType = "TF_InvalidFlag";
    }

	BOOST_FOREACH(TSFieldDefPtr field,TypeDef->Fileds)
	{

	}

	TSString HashVersionCode = TSIDLUtils::GenerateTypeVersionNum(TypeDef);
    TSString selName = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
	boost::algorithm::replace_all(content,"_HASHVERSION_CODE_",HashVersionCode);
    boost::algorithm::replace_all(content,"_SELF_NAME_",selName);
    boost::algorithm::replace_all(findcontent,"__DATA_FINDSET_CONTENT_",FindSetContent);
	boost::algorithm::replace_all(content,"__DATA_FINDSET__",findcontent);
    boost::algorithm::replace_all(content,"_FLAG_TYPE_",FlagType);
    ProjFile->AppendLast(content);
}

void TSCSharpPlugin::BuildDefine()
{
    TSProjFilePtr DefinedFile;

    std::vector<TSTypeDefPtr>  TypeDefs = _p->_BuiltinPlugin->GetAllTypes();
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
	//bool tempServiceFlag = GetGenerateService(vecSelectService, tempServiceVec, _serviceVecStruct.buildDefineVec, _serviceFlagStruct->buildDefine);

	//std::vector<TSString> structVec;
	std::vector<TSString> indexServiceVec;
	//将服务依赖的结构体存入structVec容器中
	//StoreServiceAboutStruct(serviceMap, structVec, indexServiceVec, vecSelectService, tempServiceVec);

	//接收选中的主题
	std::vector<TSString> vecSelectTopic = TSIDLPluginMgr::Instance()->GetSelectTopicVector();
	//std::vector<TSString> tempTopicVec;
	//如果不是第一次选择主题则找到当前选中的主题和上次选中主题的差集，生成差集中的主题
	//bool emptyTopicFlag = GetGenerateTopic(vecSelectTopic, tempTopicVec, _topicVecStruct.buildDefineVec, _topicFlagStruct->buildDefine);

	//std::vector<TSString> indexVec;
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

    BOOST_FOREACH(TSTypeDefPtr TypeDef,TypeDefs)
    {
        if (TypeDef->Type != TSTypeDef::Topic && TypeDef->Type != TSTypeDef::Struct &&
            TypeDef->Type != TSTypeDef::Service)
        {
            continue;
        }

		//过滤掉没有选中的节点以及可能重复生成的节点
		/*if (FilterNotSelectNode(TypeDef, indexVec, structVec, _storeStructRec.buildDefineRec))
		{
			continue;
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
		//_storeStructRec.buildDefineRec.push_back(TypeDef->TypeName);
    
       TSString ParseFileBaseName = 
           GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
       if(TSIDLPluginMgr::Instance()->GetIsGenerateImportFile() || !TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock()))
       {
           if(TSSolutionPtr Solution = 
               SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
           {
               TSProjectPtr Dir  = Solution->GetOrCreateProj(ParseFileBaseName);
               DefinedFile = Dir->GetOrCreateProjFile("Defined",".cs");
               DefinedFile->SetFileType(None);
               if (TypeDef->Type == TSTypeDef::Topic)
               {
				   //过滤掉没有被选中的主题
				   //if (!indexVec.empty())
				   //{
					  // //判断当前主题是否是过滤掉的主题，如果是则跳过当前循环
					  // std::vector<TSString>::iterator iterTopic = std::find(indexVec.begin(), indexVec.end(), TypeDef->TypeName);
					  // if (iterTopic != indexVec.end())
					  // {
						 //  continue;
					  // }
				   //}
                   TSTopicTypeDefPtr topicDef = TS_CAST(TypeDef,TSTopicTypeDefPtr);
                   TSTypeDefWithFiledPtr structDef = TS_CAST(topicDef->Def.lock(),TSTypeDefWithFiledPtr);
                   GenerateDefine(DefinedFile, structDef,topicDef);
               }
               if (TypeDef->Type == TSTypeDef::Service)
               {
                   TSTypeDefWithFuncAndFiledPtr ServiceDef = TS_CAST(TypeDef,TSTypeDefWithFuncAndFiledPtr);
                   GenerateDefineWithService(DefinedFile,ServiceDef,"Service", indexServiceVec);
                   GenerateDefineWithService(DefinedFile,ServiceDef,"Request", indexServiceVec);
                   GenerateDefineWithService(DefinedFile,ServiceDef,"Response", indexServiceVec);
               }
               
           }
       }                
    }
    std::vector<TSSolutionPtr> solutions = SolutionMgr()->GetSolutions(); 
    std::vector<TSSolutionPtr>::const_iterator its = solutions.begin();
    BOOST_FOREACH(TSSolutionPtr its,solutions)
    {
        const ProjMap_T &Projs = its->GetProjects();
        for (ProjMap_T::const_iterator iter = Projs.begin(); iter != Projs.end(); iter++)
        {
            const ProjFileMap_T files = iter->second->GetProjFiles();
            TSString baseDir = iter->second->GetShortName();
            for (ProjFileMap_T::const_iterator iterfile = files.begin(); iterfile != files.end(); iterfile++)
            {
                TSString FileName = iterfile->first;
                if (FileName == "Defined.cs" && !_topicFlagStruct->buildDefine)
                {
                    TSString content = __HEADER_INCLUDE_TOPIC_4CS__;
                    iterfile->second->AppendFirst(content);
                }
            }
        }
    }  
	_topicFlagStruct->buildDefine = true;
}

void TSCSharpPlugin::GenerateDefineWithService( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef,const TSString & Requst, std::vector<TSString>& serviceName)
{
    if (Requst == "Service")
    {
        TSString content = __DATA_DEFINE_DEFINE_4CS__;

        TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
            __NAMESPACE_END__,
            content,
            TypeDef);
        TSString selfname = GetTopicName(TypeDef);
        TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
        boost::algorithm::replace_all(selfns,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

        TSString funcconvert;
        funcconvert = "return To == ID;";
        boost::algorithm::replace_all(content,"__FINDSET__","");
        boost::algorithm::replace_all(content,"_SELF_NAME",selfname);
        boost::algorithm::replace_all(content,"_PACKAGES_",selfns);
        boost::algorithm::replace_all(content,"_FUNC_CONVERT_",funcconvert);
        boost::algorithm::replace_all(content,"_SELF_DESCRIPTION",selfname);

        ProjFile->AppendLast(content);
    }
    else
    {
        BOOST_FOREACH(TSFunctionDefPtr Func,TypeDef->Functions)
        {
			/*TSString funName = Func->ValName;
			std::vector<TSString>::iterator iterTopic = std::find(serviceName.begin(), serviceName.end(), funName);
			if (iterTopic == serviceName.end())
			{
				continue;
			}*/
            TSString content = __DATA_DEFINE_DEFINE_4CS__;
            TSString suit_full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
            suit_full_type_name += "_" + Func->ValName;
            TSString suit_package;
            if (Requst == "Request")
            {
                suit_full_type_name += "_Request";
            }
            else if (Requst == "Response")
            {
                suit_full_type_name += "_Response";
            }
            suit_package = suit_full_type_name;
            boost::algorithm::replace_all(suit_full_type_name,"::","_");
            boost::algorithm::replace_all(suit_package,"_",".");
            boost::algorithm::replace_all(suit_package,"::",".");
            TSString funcconvert = "return To == ID;";

            boost::algorithm::replace_all(content,"_SELF_NAME",suit_full_type_name);
            boost::algorithm::replace_all(content,"_SELF_DESCRIPTION",suit_full_type_name);
            boost::algorithm::replace_all(content,"_PACKAGES_",suit_package);
            boost::algorithm::replace_all(content,"Request.Support","RequestSupport");
            boost::algorithm::replace_all(content,"Response.Support","ResponseSupport");
            boost::algorithm::replace_all(content,"__FINDSET__","");
            boost::algorithm::replace_all(content,"_FUNC_CONVERT_",funcconvert);
            ProjFile->AppendLast(content);
        }
    }
}

void TSCSharpPlugin::GenerateDefine( TSProjFilePtr ProjFile,TSTypeDefWithFiledPtr TypeDef,TSTopicTypeDefPtr TopicDef)
{
    TSString content = __DATA_DEFINE_DEFINE_4CS__;
    TSString contFind = __FINDSET__;

    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);

    TSString selfns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    boost::algorithm::replace_all(selfns,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
    TSString selfname = GetTopicName(TopicDef);
	TSString selfdesc = TopicDef->Desc;
	boost::replace_all(selfdesc,"\"","");

    TSString funcconvert;
    TSString superns;
    if (TopicDef->Extend.lock())
    {
        superns = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock());
        boost::algorithm::replace_all(superns,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
        TSString supername = GetTopicName(TopicDef->Extend.lock());
        funcconvert = Table(2) + "if (!" + supername + ".CanConvert(ref To))" + TSIDL_NEWLINE +
            Table(3) + "{" + TSIDL_NEWLINE +
            Table(4) +  "return To == ID;" + TSIDL_NEWLINE +  
            Table(3) + "}" + TSIDL_NEWLINE +
            Table(3) + "return true;" + TSIDL_NEWLINE;
    }
    else
    {
        funcconvert = "return To == ID;";
    }

    boost::algorithm::replace_all(content,"__FINDSET__",contFind);
    boost::algorithm::replace_all(content,"_SELF_NAME",selfname);
    boost::algorithm::replace_all(content,"_PACKAGES_",selfns);
    boost::algorithm::replace_all(content,"_FUNC_CONVERT_",funcconvert);
    boost::algorithm::replace_all(content,"_SELF_DESCRIPTION",selfdesc);

    ProjFile->AppendLast(content);
}

void TSCSharpPlugin::BuildCallBackImpl(TSTypeDefPtr TypeDef,const TSString & FileName)
{
    if(!TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock())
        || TSIDLPluginMgr::Instance()->GetIsGenerateImportFile())
    {
        TSString ParseFileBaseName = 
            GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
        if(TSSolutionPtr Solution = 
            SolutionMgr()->GetOrCreateSolution(ParseFileBaseName))
        {
            TSProjectPtr Dir  = Solution->GetOrCreateProj(FileName);
            Dir->SetProFileType(Impl);
            TSString Path = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef) + ".CallBackImpl";
            boost::algorithm::replace_all(Path,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
            TSProjFilePtr CallBackImplFile = Dir->GetOrCreateProjFile(Path + "/CallBackImpl",".cs");
            CallBackImplFile->SetFileType(Impl);
            if(TSTypeDefWithFuncAndFiledPtr typeDef = TS_CAST(TypeDef,TSTypeDefWithFuncAndFiledPtr))
            {
                GenerateCallBackImpl(CallBackImplFile,typeDef);
            }
            CallBackImplFile->AppendFirst(CALLBACK_HEADER_CODE_4CS);
        }

    }
}

void TSCSharpPlugin::GenerateCallBackImpl( TSProjFilePtr ProjFile,TSTypeDefWithFuncAndFiledPtr TypeDef )
{
    TSString content = CALLBACKIMPL_4CS_CODER;
    TSIDLUtils::ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        content,
        TypeDef);
    ProjFile->AppendLast(content); 
}

TSString TSCSharpPlugin::GetCSharpValueType( TSTypeDefPtr tmpDef)
{
	TSString TypeName =TSBuiltinPlugin::GetSuitFullTypeName(tmpDef);
    return TypeName;
}

//获得服务函数和依赖的结构体的Map
void TSCSharpPlugin::GetServiceAndStructMap(std::vector<TSTypeDefWithFuncAndFiledPtr>  TypeDefs, std::map<TSString, TSString>& serviceMap)
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

//获得要生成的服务，如果追加的服务为空则返回true
bool TSCSharpPlugin::GetGenerateService(std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec, std::vector<TSString>& serviceVecStruct, bool serviceFlag)
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

//将和服务相关的结构体存入vector中
//serviceMap 服务和依赖结构体组成的Map, structVec服务依赖的结构体容器, indexServiceVec 记录服务的容器, vecSelectService 工具选中的服务, tempServiceVec 实际用来操作的服务容器
void TSCSharpPlugin::StoreServiceAboutStruct(std::map<TSString, TSString>& serviceMap, std::vector<TSString>& serviceVec, std::vector<TSString>& indexServiceVec, std::vector<TSString>& vecSelectService, std::vector<TSString>& tempServiceVec)
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

std::vector<TSString> TSCSharpPlugin::GetVectorDifference(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& topicVec)
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
bool TSCSharpPlugin::GetGenerateTopic(std::vector<TSString>& vecSelectTopic, std::vector<TSString>& tempTopicVec, std::vector<TSString>& topicVecStruct, bool topicFlag)
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

std::vector<TSString> TSCSharpPlugin::GetTopicInVec(std::vector<TSString>& tempTopicVec, std::vector<TSTopicTypeDefPtr>& TypeDefs)
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

std::vector<TSString> TSCSharpPlugin::GetTopicInVec(std::vector<TSString>& tempTopicVec, std::vector<TSTypeDefWithFuncAndFiledPtr>& TypeDefs)
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