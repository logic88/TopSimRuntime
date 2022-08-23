#include "stdafx.h"

#include <boost/foreach.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/filesystem.hpp>

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimUtil/TSDynamicLib.h>
#include <TopSimUtil/pugixml.hpp>
#else
#include <TopSimRuntime/TSDynamicLib.h>
#include <TopSimRuntime/TSFileSystem.h>
#include <TopSimRuntime/pugixml.hpp>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TSIDLPluginMgr.h"

#define MaxDeep 1

SINGLETON_IMPLEMENT(TSIDLPluginMgr);

BEGIN_METADATA(TSIIDLPlugin)
	END_METADATA()

struct TSIDLPluginConfigure
{
	TSString         Name;
	int              InterfaceType;
	int              RelaizedType;
	std::vector<int> DependenceInterfaces;
};
STRUCT_PTR_DECLARE(TSIDLPluginConfigure);

struct TSIDLPluginMgrPrivate
{
	TSIDLPluginMgrPrivate()
		:_RunOptions(GB18030ToTSString("����ѡ��"))
	{
		_IsIncludePathParse = false;
        _IsLibraryInclude = false;
        _IsService = false;
		_Pipe = NULL;
		_ConfigFileName = "XIDL.config";
        _CodeType = "c++";
        _FileNameWithXidl = true;
        _GeneratePy = false;
        _Isimport = true;
        _GenerateXsimfile = false;
		_SortDeep = MaxDeep;
		_CmdLineOptionsFlag = false;
	}

	boost::program_options::variables_map        _VariablesMap;					//����ѡ��洢��
	boost::program_options::options_description  _CmdLineOptions;				//�����в���������
	TSConUnorderedMap<TSString,TSString>         _KeyNameMaps;
	boost::program_options::options_description  _RunOptions;					//����ѡ��������
	bool                                         _CmdLineOptionsFlag;
	TSString                                     _CurParseFile;
	bool                                         _IsIncludePathParse;
    bool                                         _IsLibraryInclude;
    bool                                         _IsService;
	std::vector<TSString>                        _IncludePaths;
    std::vector<TSString>                        _LibPaths;
	std::vector<TSString>                        _Files;
	DocumentPtr                                  _Doc;
	std::vector<TSString>                        _UrOpts;
	TSString                                     _ConfigFileName;				//XIDL�����ļ�:XIDL.config
    TSString                                     _CodeType;
    bool                                         _FileNameWithXidl;
    bool                                         _Isimport;
    bool                                         _GeneratePy;
    bool                                         _GenerateXsimfile;
	TSString                                  _ImplPath;
	TSString                                  _GenericPath;
    std::vector<std::pair<TSString,TSIDLPluginMgr::ParserUtilPtr>> _Utils;
	//TSConUnorderedMap<TSString,TSIDLPluginMgr::ParserUtilPtr> _Utils;
	PluginMap_T                  _BuildPlugins;
	std::vector<TSIIDLPluginPtr> _FlatBuildPlugins;
	TSGramCommon::OutputPipe *      _Pipe;
    TSString                      _FilePath;									//XIDL�ļ�·��
	TSString                   _SDKPath;										//LINKSDK��������
	UINT32                     _SortDeep;
	std::map<TSString, UINT32> _XidlBegin;										//XIDL�ļ���Ч���ݿ�ʼλ��<XIDL�ļ����ݣ���Чλ�ÿ�ʼ����>

	void BuildFlatPlugin_pp(const TSConUnorderedMap<int,TSIDLPluginConfigurePtr> & PluginConfigures,
		int InterfaceType,
		TSIDLPluginConfigurePtr Conf)
	{
		BOOST_FOREACH(int DependenceType,Conf->DependenceInterfaces)
		{
			if(_BuildPlugins.find(DependenceType)
				== _BuildPlugins.end())
			{
				TSConUnorderedMap<int,TSIDLPluginConfigurePtr>::const_iterator it = PluginConfigures.find(DependenceType);
				ASSERT(it != PluginConfigures.end() && "PluginConfigures.end()");
				BuildFlatPlugin_pp(PluginConfigures,DependenceType,it->second);
			}
		}
		TSIIDLPluginPtr Plugin = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(Conf->RelaizedType),TSIIDLPluginPtr);
		Plugin->OnPluginLoaded();
		_FlatBuildPlugins.push_back(Plugin);
		_BuildPlugins[InterfaceType] = Plugin;
	}

	void BuildFlatPlugin_p(const TSConUnorderedMap<int,TSIDLPluginConfigurePtr> & PluginConfigures)
	{
		TSConUnorderedMap<int,TSIDLPluginConfigurePtr>::const_iterator it = PluginConfigures.begin();
		while(it != PluginConfigures.end())
		{
			if(_BuildPlugins.find(it->first)
				== _BuildPlugins.end())
			{
				BuildFlatPlugin_pp(PluginConfigures,it->first,it->second);
			}

			++it;
		}
	}
};

bool HasOption( const TSString & Key )
{
	return TSIDLPluginMgr::Instance()->HasOption(Key);
}

TOPSIMIDL_DECL TSString GetBaseName( const TSString & FileFullPath )
{
	return boost::filesystem::basename(FileFullPath);
}

TOPSIMIDL_DECL TSString GetAbsoluteDir( const TSString & FileFullPath )
{
	boost::filesystem::path p = FileFullPath;
	if(p.has_parent_path())
	{
		return p.parent_path().string();
	}
	else
	{
		return p.string();
	}
}

TSIDLPluginMgr * PluginMgr()
{
	return TSIDLPluginMgr::Instance();
}

TSIDLPluginMgr::TSIDLPluginMgr(void)
	:_p(new TSIDLPluginMgrPrivate)
{
    _IsTopicOrService = false;
    _IsPlugin = false;
    _HaveXsim = false;
}


TSIDLPluginMgr::~TSIDLPluginMgr(void)
{
	delete _p;
}

bool TSIDLPluginMgr::LoadPlugins()
{
	_p->_Doc.reset();
	_p->_Doc = boost::make_shared<pugi::xml_document>();
    boost::filesystem::path pfile;
    const char* link;
    if (HasOption(SetSDKKey))
    {
        TSString sdkName = Option<TSString>(SetSDKKey);
        link = getenv(sdkName.c_str());
    }
    else if (HasOption(LorisKey))
    {
        link = getenv("LorisSDK");
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
    TSString filePath = pfile.string() + "/";
	pugi::xml_parse_result ret = _p->_Doc->load_file(_p->_ConfigFileName.c_str());
	if(ret.status != pugi::status_ok)
	{
#if defined(XSIM3_2) || defined(XSIM3_3)
		ret = _p->_Doc->load_file(("./config/" + _p->_ConfigFileName).c_str());
#else
		ret = _p->_Doc->load_file((TSDir::GetFileDir(TSDir::GetAppFileFullPath()) + "/config/" + _p->_ConfigFileName.c_str()).c_str());
#endif
	}
	if(ret.status == pugi::status_ok)
	{
		TSConUnorderedMap<int,TSIDLPluginConfigurePtr> PluginConfigures;

		pugi::xpath_node_set PluginNodes = _p->_Doc->select_nodes("Config/Plugins/Plugin");
		for(pugi::xpath_node_set::const_iterator it = PluginNodes.begin();
			it != PluginNodes.end(); ++it)
		{
			TSString Name = it->node().attribute("Name").as_string();
			TSString InterfaceName = it->node().attribute("Interface").as_string();
			TSString RealizedName  = it->node().attribute("Realized").as_string();
			TSString DependnceInterfaces = it->node().attribute("DependenceInterfaces").as_string();
            TSString ModuleFileName = "";
            ModuleFileName = it->node().attribute("Module").as_string();
#ifndef _WIN32
			ModuleFileName = "lib" + ModuleFileName;
#endif
            if (!ModuleFileName.empty())
            {
                TSString CurrentPath = boost::filesystem::current_path().string();
                TSString FilePath = CurrentPath + "/" + ModuleFileName;
                HMODULE hModule = NULL;
                hModule = LoadSuitDynamicLibrary(FilePath.c_str());
                if (!hModule)
                {
                    hModule = LoadSuitDynamicLibrary(FilePath.c_str());
                }

                if(hModule)
                {
                    std::cout << "LoadDynamicLibrary: " << FilePath << " OK!" << std::endl;
                }
            }

			std::vector<TSString> Dependences = TSMetaType4Link::Split(DependnceInterfaces,",",false);

			if(int InterfaceType = TSMetaType::GetType(InterfaceName.c_str()))
			{
				if(TSMetaType::CanConvert(InterfaceType,TSIIDLPlugin::GetMetaTypeIdStatic()))
				{
					if(int RealizedType = TSMetaType::GetType(RealizedName.c_str()))
					{
						if(TSMetaType::CanConvert(RealizedType,InterfaceType))
						{
							//success
							TSIDLPluginConfigurePtr Configure = boost::make_shared<TSIDLPluginConfigure>();
							Configure->Name = Name;
							Configure->InterfaceType = InterfaceType;
							Configure->RelaizedType = RealizedType;
							for(size_t i = 0; i < Dependences.size(); ++i)
							{
								if(int Type = TSMetaType::GetType(Dependences[i].c_str()))
								{
									if(TSMetaType::CanConvert(Type,TSIIDLPlugin::GetMetaTypeIdStatic()))
									{
										Configure->DependenceInterfaces.push_back(Type);
									}
								}
							}
							PluginConfigures[InterfaceType] = Configure;
						}
					}
                    else
                    {
                        std::cerr << GB18030ToTSString("���ʵ����ȡʧ��") << std::endl;
                        continue;
                    }
				}
                std::cout << GB18030ToTSString("�Ѽ��ز����").c_str() << InterfaceName << "(" << Name <<")" << std::endl;
			}
            else
            {
                std::cout << GB18030ToTSString("�����") << InterfaceName << "(" << Name <<")" 
					<< GB18030ToTSString("ʧ��") << std::endl;
            }
            
		}
		_p->BuildFlatPlugin_p(PluginConfigures);

		return true;
	}
    else
    {
        return false;
    }

}

void TSIDLPluginMgr::CleanPlugins()
{
	std::vector<TSIIDLPluginPtr>::reverse_iterator it = _p->_FlatBuildPlugins.rbegin();
	while(it != _p->_FlatBuildPlugins.rend())
	{
		(*it)->Clean();
		++it;
	}

	_p->_FlatBuildPlugins.clear();
	_p->_BuildPlugins.clear();
	_p->_KeyNameMaps.clear();
}

TSIIDLPluginPtr TSIDLPluginMgr::GetPlugin( int InterfaceMetaTypeId ) const
{
	return _p->_BuildPlugins[InterfaceMetaTypeId];
}

const PluginMap_T & TSIDLPluginMgr::GetPlugins() const
{
	return _p->_BuildPlugins;
}

void TSIDLPluginMgr::SetTopicNameMap(std::map<TSString, TSString> topicMap)
{
	_topicMap = topicMap;
}

void TSIDLPluginMgr::SetServiceNameVec(std::vector<TSString> serviceVec)
{
	_serviceVec = serviceVec;
}

std::vector<TSString> TSIDLPluginMgr::GetServiceNameVec()
{
	return _serviceVec;
}

std::vector<TSString> TSIDLPluginMgr::GetSelectServiceVector()
{
	return _selectServiceVector;
}

void TSIDLPluginMgr::SetCurrentFileTopicNameMap(std::map<std::pair<TSString, TSString>, TSString> topicVec)
{
	_currentFileTopicVec = topicVec;
}

std::map<TSString, TSString> TSIDLPluginMgr::GetTopicNameMap()
{
	return _topicMap;
}

std::map<std::pair<TSString, TSString>, TSString> TSIDLPluginMgr::GetCurrentFileTopicNameMap()
{
	return _currentFileTopicVec;
}

void TSIDLPluginMgr::DoParse()
{
	//std::map<TSString, TSString> topicMap;
    try
    {
		//C++��C#��JAVA �̳л���TSIIDLPlugin��TSIIDLPlugin����ָ����ò�ͬ���Ե�����DoParse��Ա����
        BOOST_FOREACH(TSIIDLPluginPtr Plugin,_p->_FlatBuildPlugins)
        {
            if(!Plugin->DoParse())
            {
                break;
            }
        }
    }
    catch(TSException &exp)
    {
        throw exp;
    }
    catch(...)
    {
        throw TSException(GB18030ToTSString("����δ֪�쳣").c_str());
    }
}

const std::vector<TSIIDLPluginPtr> & TSIDLPluginMgr::GetFlatPlugins()
{
	return _p->_FlatBuildPlugins;
}

void TSIDLPluginMgr::SetPipe(TSGramCommon::OutputPipe * Pipe)
{
	_p->_Pipe = Pipe;
}

TSGramCommon::OutputPipe * TSIDLPluginMgr::GetPipe()
{
	return _p->_Pipe;
}

void TSIDLPluginMgr::TransferSelectTopic(std::vector<TSString>& vecTopic)
{
	_selectTopicVector = vecTopic;
}

void TSIDLPluginMgr::TransferSelectService(std::vector<TSString>& vecService)
{
	_selectServiceVector = vecService;
}

std::vector<TSString> TSIDLPluginMgr::GetSelectTopicVector()
{
	return _selectTopicVector;
}

void TSIDLPluginMgr::TranferSelectFileName(TSString fileName)
{
	_selectFileName = fileName;
}

TSString TSIDLPluginMgr::GetSelectFileName()
{
	return _selectFileName;
}

bool TSIDLPluginMgr::IsLinkCodeGenTool()
{
	return _isTool;
}

void TSIDLPluginMgr::SetLinkCodeGenToolFlag(bool isTool)
{
	_isTool = isTool;
}

void TSIDLPluginMgr::DoBuild()
{
	BOOST_FOREACH(TSIIDLPluginPtr Plugin,_p->_FlatBuildPlugins)
	{
		if(!Plugin->DoBuild())
		{
			continue;
		}
	}
}

bool TSIDLPluginMgr::ParseFile(const TSString & FileName)
{
	TSString FileFullPath = GetFileFullPath(FileName);

    _p->_FilePath = FileFullPath;
	_p->_Utils.clear();
    
    if (HasOption(CodeTypeKey))
    {
        TSString codeType = Option<TSString>(CodeTypeKey);
       _p->_CodeType = codeType;
	   std::cout << "codeType: " << codeType << std::endl;
    }

    if (HasOption(GeneratePyKey))
    {
        _p->_GeneratePy = true;
    }

    if (HasOption(GenerateXsimfileKey))
    {
        _p->_GenerateXsimfile = true;
    }
    
    if (HasOption(FileNameWithXidlKey))
    {
        _p->_FileNameWithXidl = false;
    }

    if(HasOption(GImportKey))
    {
        _p->_Isimport = false;
    }

    if (HasOption(LorisKey))
    {
        return true;
    }

	if(TSIDLGram::ParserPtr Parse = ParseFile_p(FileFullPath))
	{
		ParserUtilPtr Util = boost::make_shared<ParserUtil>();
		Util->Parse = Parse;
		Util->SourceName = FileName;
        _p->_Utils.push_back(std::pair<TSString,TSIDLPluginMgr::ParserUtilPtr>(FileFullPath,Util));
		//_p->_Utils[FileFullPath] = Util;
		_p->_CurParseFile = FileFullPath;

		return true;
	}

	return false;
}

UINT32 TSIDLPluginMgr::GetLineCount( TSIDLGram::Parser::Node * AstNode )
{
	ParserUtilPtr Util = GetParserUtil(GetCurParseFileFullPath());
	ASSERT(Util && "Util");
	return Util->Parse->GetLineCount(AstNode);
}

TSString TSIDLPluginMgr::GetLineMsg( UINT32 nLine )
{
	ParserUtilPtr Util = GetParserUtil(GetCurParseFileFullPath());
	ASSERT(Util && "Parse");
	return Util->Parse->GetLineMsg(nLine);
}

UINT32 TSIDLPluginMgr::GetMaxLineCount()
{
	ParserUtilPtr Util = GetParserUtil(GetCurParseFileFullPath());
	ASSERT(Util && "Util");
	return Util->Parse->GetMaxLine();
}

void TSIDLPluginMgr::LogErrorMsg( const TSString & Msg,TSIDLGram::Parser::Node * AstNode )
{
	UINT32 nLine = GetLineCount(AstNode);
	TSString LineMsg = GetLineMsg(nLine);

	TSString LogMsg = TSStringUtil::Format("(%u):%s ����:%s.\n",nLine,Msg.c_str(),LineMsg.c_str());

	if(GetPipe())
	{
		GetPipe()->Output(LogMsg.c_str());
	}
	else
	{
		STD_COUT << LogMsg;
	}
}

/* ��ʼ������ */
bool TSIDLPluginMgr::InitOption(int argc,char * argv[])
{
	//����XIDL.config�����ļ�
	pugi::xml_document doc;
	pugi::xml_parse_result ret = doc.load_file(_p->_ConfigFileName.c_str());
	if(ret.status != pugi::status_ok)
	{
#if defined(XSIM3_2) || defined(XSIM3_3)
		ret = doc.load_file(("./config/" + _p->_ConfigFileName).c_str());
#else
		ret = doc.load_file((TSDir::GetFileDir(TSDir::GetAppFileFullPath()) + "/config/" + _p->_ConfigFileName).c_str());
#endif
	}
	if(ret.status == pugi::status_ok)
	{
		if (!_p->_CmdLineOptionsFlag)
		{
			boost::program_options::options_description_easy_init easy_init = _p->_RunOptions.add_options();

			easy_init
			(TS_TEXT("Version,b"), boost::program_options::value<TSString>(), GB18030ToTSString("Linker�汾.(XSIM3_2;XSIM3_3;XSIM4_3;XSIM_5)").c_str())
			(TS_TEXT("Compile,c"), boost::program_options::value<TSString>(), GB18030ToTSString("���빤��.").c_str())
			(TS_TEXT("Debug,d"), GB18030ToTSString("��ͣ���Թ���.").c_str())
			(TS_TEXT("ExportFile,e"), boost::program_options::value<TSString>(), GB18030ToTSString("��������ļ���·��.").c_str())
			(TS_TEXT("File,f"), boost::program_options::value<TSString>(), GB18030ToTSString("IDL�ļ���·��������ļ���','����.").c_str())
			(TS_TEXT("GImport,g"), GB18030ToTSString("ָ���Ƿ����ɵ����xidl,Ĭ������").c_str())
			(TS_TEXT("Help,h"), GB18030ToTSString("��ӡIDL����.").c_str())
			(TS_TEXT("IncludePath,i"), boost::program_options::value<TSString>(), GB18030ToTSString("��Ӱ���·��,���·����';'�ָ�.").c_str())
			(TS_TEXT("LibPath,l"), boost::program_options::value<TSString>(), GB18030ToTSString("��ӿ�·��,��·����';'�ָ�.").c_str())
			(TS_TEXT("ExportMacro,m"), boost::program_options::value<TSString>(), GB18030ToTSString("���������ļ��ĵ�����.").c_str())
			(TS_TEXT("NodeFile,n"), boost::program_options::value<TSString>(), GB18030ToTSString("���Խڵ��ļ���·��.").c_str())
			(TS_TEXT("GFileOnly,o"), GB18030ToTSString("ֻ�����ļ�.").c_str())
			(TS_TEXT("FileNameWithXidl,p"), GB18030ToTSString("ָ�����ɵ��ļ�Ŀ¼�޺�׺_xidl��Ĭ�Ϻ�׺��_xidl").c_str())
			(TS_TEXT("GSql,q"), GB18030ToTSString("����SQL�ӿ�.").c_str())
			(TS_TEXT("SavePath,s"), boost::program_options::value<TSString>(), GB18030ToTSString("���뱣���ļ�·��,Ĭ�ϵ�ǰִ��·��.").c_str())
			(TS_TEXT("CodeType,t"), boost::program_options::value<TSString>(), GB18030ToTSString("�������������(c++,c#,java),ʹ��','�ָ�").c_str())
			(TS_TEXT("Gstdafx,u"), GB18030ToTSString("����Ԥ����ͷ�ļ�.").c_str())
			(TS_TEXT("VS,v"), boost::program_options::value<TSString>(), GB18030ToTSString("Visual Studio�汾.vs2010(VC100��Ĭ��),vs2013,vs2015,vs2017(VC140��Ĭ��)).").c_str())
			(TS_TEXT("GExe,w"), GB18030ToTSString("����exe��.").c_str())
			(TS_TEXT("GenerateXsimfile,j"), GB18030ToTSString("ָ�����ɵ��ļ��Ƿ�����xsim����ļ���Ĭ�ϲ�����").c_str())
			(TS_TEXT("GListEntity,y"), boost::program_options::value<TSString>(), GB18030ToTSString("ָ������ʵ���б��xml�ļ���·��.").c_str())
			(TS_TEXT("GeneratePy,z"), GB18030ToTSString("ָ�����ɵ��ļ��Ƿ���python��ʹ��ʾ����Ĭ�ϲ�����").c_str())
			(TS_TEXT("Python"), GB18030ToTSString("����python����ģ��").c_str());//XSIM3.2Debug�汾��Ҫȥ��һЩ�ַ�����Ȼ�ᱨboost n == name.size()-2����
		
			//�������ò���ѡ�����ӵ�ѡ������������
			pugi::xpath_node_set OptNodes = doc.select_nodes("Config/Options/Option");
			for (pugi::xpath_node_set::const_iterator it = OptNodes.begin();
				it != OptNodes.end(); ++it)
			{
				TSString Name = it->node().attribute("Name").as_string();
				TSString Type = it->node().attribute("Type").as_string();
				TSString Key = it->node().attribute("Key").as_string();
				TSString Desc = it->node().attribute("Description").as_string();
				if (Type == "int")
				{
					easy_init
					(Name.c_str(), boost::program_options::value<int>(), Desc.c_str());
				}
				else if (Type == "double")
				{
					easy_init
					(Name.c_str(), boost::program_options::value<double>(), Desc.c_str());
				}
				else if (Type == "string")
				{
					easy_init
					(Name.c_str(), boost::program_options::value<TSString>(), Desc.c_str());
				}
				else
				{
					easy_init
					(Name.c_str(), Desc.c_str());
				}

				//�����õ�������ӵ�_KeyNameMaps��
				std::vector<TSString> NameVec = TSMetaType4Link::Split(Name, ",", false);
				TSString KeyValue = Key.empty() ? NameVec[0] : Key;

				if (_p->_KeyNameMaps.find(KeyValue) == _p->_KeyNameMaps.end())
				{
					if (NameVec.size() > 0)
					{
						_p->_KeyNameMaps[KeyValue] = NameVec[0];
					}
				}
				else
				{
					std::cout << KeyValue << GB18030ToTSString("�������ظ�.") << std::endl;
				}
			}
			_p->_CmdLineOptions.add(_p->_RunOptions);
		}

		_p->_KeyNameMaps[FileKey] = TS_TEXT("File");
		_p->_KeyNameMaps[DebugKey] = TS_TEXT("Debug");
		_p->_KeyNameMaps[HelpKey] = TS_TEXT("Help");
		_p->_KeyNameMaps[CodeTypeKey] = TS_TEXT("CodeType");
		_p->_KeyNameMaps[SavePathKey] = TS_TEXT("SavePath");
		_p->_KeyNameMaps[VSKey] = TS_TEXT("VS");
		_p->_KeyNameMaps[NodeFileKey] = TS_TEXT("NodeFile");
		_p->_KeyNameMaps[LibPathKey] = TS_TEXT("LibPath");
		_p->_KeyNameMaps[GImportKey] = TS_TEXT("GImport");
		_p->_KeyNameMaps[ExportFileKey] = TS_TEXT("ExportFile");
		_p->_KeyNameMaps[ExportMacroKey] = TS_TEXT("ExportMacro");
		_p->_KeyNameMaps[GListEntityKey] = TS_TEXT("GListEntity");
		_p->_KeyNameMaps[GFileOnlyKey] = TS_TEXT("GFileOnly");
		_p->_KeyNameMaps[CompileKey] = TS_TEXT("Compile");
		_p->_KeyNameMaps[GSqlKey] = TS_TEXT("GSql");
		_p->_KeyNameMaps[GstdafxKey] = TS_TEXT("Gstdafx");
		_p->_KeyNameMaps[GExeKey] = TS_TEXT("GExe");
		_p->_KeyNameMaps[VerSionKey] = TS_TEXT("Version");
		_p->_KeyNameMaps[IncludePathKey] = TS_TEXT("IncludePath");
		_p->_KeyNameMaps[FileNameWithXidlKey] = TS_TEXT("FileNameWithXidl");
		_p->_KeyNameMaps[GeneratePyKey] = TS_TEXT("GeneratePy");
		_p->_KeyNameMaps[GenerateXsimfileKey] = TS_TEXT("GenerateXsimfile");
		_p->_KeyNameMaps[GenPython] = TS_TEXT("Python");

		_p->_CmdLineOptionsFlag = true;

		//������������
		pugi::xpath_node_set pathNodes = doc.select_nodes("Config/Environment");
		for(pugi::xpath_node_set::const_iterator it = pathNodes.begin();
			it != pathNodes.end(); ++it)
		{
			_p->_SDKPath = it->node().attribute("Name").as_string();
			break;
		}

		//�����б����
		pugi::xpath_node_set pathDeepNodes = doc.select_nodes("Config/SortDeep");
		for(pugi::xpath_node_set::const_iterator it = pathDeepNodes.begin();
			it != pathDeepNodes.end(); ++it)
		{
			_p->_SortDeep = it->node().attribute("Deep").as_int();
			break;
		}

		try
		{
			boost::program_options::command_line_parser par(argc, argv);

			boost::program_options::command_line_parser bc = par.options(_p->_CmdLineOptions);

			boost::program_options::command_line_parser au = bc.allow_unregistered();

			boost::program_options::parsed_options res = au.run();

			BOOST_AUTO(pr, res);
			boost::program_options::store(pr,_p->_VariablesMap);
			boost::program_options::notify(_p->_VariablesMap);

			_p->_UrOpts = collect_unrecognized(pr.options , boost::program_options::include_positional);

			return true;
		}
		catch(boost::program_options::invalid_syntax &)
		{
			if(GetPipe())
			{
				GetPipe()->Output(GB18030ToTSString("�������﷨����.\n").c_str());

				std::ostringstream os;
				os << _p->_RunOptions;
				GetPipe()->Output(os.str().c_str());
			}
			else
			{
				STD_COUT << GB18030ToTSString("�������﷨����.");
				STD_COUT << _p->_RunOptions << std::endl;
			}
		}
		catch (boost::program_options::validation_error & e)
		{
			TSString ErrorMsg = TSString(GB18030ToTSString("��Ч�Ĳ���ֵ(")) + e.get_option_name();

			if(GetPipe())
			{
				GetPipe()->Output(ErrorMsg.c_str());

				std::ostringstream os;
				os << _p->_RunOptions;
				GetPipe()->Output(os.str().c_str());
			}
			else
			{
				STD_COUT << ErrorMsg.c_str();
				STD_COUT << _p->_RunOptions << std::endl;
			}
		}
		catch (boost::program_options::unknown_option &e)
		{
			if(GetPipe())
			{
				GetPipe()->Output(e.what());

				std::ostringstream os;
				os << _p->_RunOptions;
				GetPipe()->Output(os.str().c_str());
			}
			else
			{
				STD_COUT << e.what();
				STD_COUT << _p->_RunOptions << std::endl;
			}
		}
	}

	return false;
}

bool TSIDLPluginMgr::HasOption( const TSString & Key )
{
	return _p->_VariablesMap.count(Key.c_str());
}
  
const boost::program_options::variable_value & TSIDLPluginMgr::GetOptionVal( const TSString & Key )
{
	return _p->_VariablesMap[_p->_KeyNameMaps[Key].c_str()];
}

void TSIDLPluginMgr::SetConfigFileName( const TSString & FileName )
{
	_p->_ConfigFileName = FileName;
}

const std::vector<TSString> & TSIDLPluginMgr::UnrecognizedOpts()
{
	return _p->_UrOpts;
}

void TSIDLPluginMgr::PrintHelps()
{
	if(GetPipe())
	{
		std::ostringstream os;
		os << _p->_RunOptions;
		GetPipe()->Output(os.str().c_str());
	}
	else
	{
		STD_COUT << _p->_RunOptions << std::endl;
	}
}

/* ��ȡ����xidl�ļ�·�� */
const std::vector<TSString> & TSIDLPluginMgr::GetOptionFiles()
{
    if(HasOption(FileKey))
    {
        TSString FileNames = Option<TSString>(FileKey);

        std::vector<TSString> Files = TSMetaType4Link::Split(FileNames,",",false);

        _p->_Files.insert(_p->_Files.end(),Files.begin(),Files.end());
    }
	else
	{
		std::cout << GB18030ToTSString("�������󣬻�ȡxidl�ļ�ʧ�ܣ�") << std::endl;
	}
	return _p->_Files;
}

const TSString & TSIDLPluginMgr::GetCurParseFileFullPath()
{
	return _p->_CurParseFile;
}

TSString TSIDLPluginMgr::GetFileFullPath( const TSString & FileName )
{
	TSString FileFullName;

	const std::vector<TSString> & IncludePaths = GetIncludePaths();
	BOOST_FOREACH(const TSString & Path, IncludePaths)
	{
		if (boost::filesystem::exists((Path + TS_TEXT("/") + FileName).c_str()))
		{
			FileFullName = Path + TS_TEXT("/") + FileName;
			break;
		}
	}

	if (FileFullName.empty())
	{
		FileFullName = boost::filesystem::system_complete(boost::filesystem::path(FileName)).string();
	}

	if(!boost::filesystem::exists(FileFullName.c_str()))
	{
        FileFullName = "";  
		if(boost::filesystem::exists(boost::filesystem::current_path() / FileName.c_str()))
		{
			FileFullName = (boost::filesystem::current_path() / FileName.c_str()).string();
		}

        if (FileFullName.empty())
        {
            TSString::size_type separatorPos = _p->_FilePath.find_last_of("/");
            TSString newPath = _p->_FilePath.substr(0,separatorPos);
            if (!newPath.empty())
            {
                FileFullName = newPath + "/" + FileName;
            }  
        }
	}
  

	if(!boost::filesystem::exists(FileFullName.c_str()))
	{
        std::cout<< FileName << GB18030ToTSString(" �ļ�������.")<<std::endl;
		return TSString();
	}

	boost::algorithm::replace_all(FileFullName, "\\", "/");

	return FileFullName;
}

const std::vector<TSString> & TSIDLPluginMgr::GetIncludePaths()
{
	if(!_p->_IsIncludePathParse)
	{
		if(HasOption(IncludePathKey))
		{
			TSString Names = Option<TSString>(IncludePathKey);
			std::vector<TSString> Paths = TSMetaType4Link::Split(Names,";",false);
			_p->_IncludePaths.insert(_p->_IncludePaths.end(),Paths.begin(),Paths.end());
		}
		_p->_IsIncludePathParse = true;
	}

	return _p->_IncludePaths;
}

TSIDLGram::ParserPtr TSIDLPluginMgr::ParseFile_p( const TSString & FileFullPath )
{
	if(TSIDLGram::ParserPtr Parse = TSIDLGram::BuildFile(FileFullPath,GetPipe()))
	{
		ProcessPreCompiles(Parse);
		return Parse;
	}

	return TSIDLGram::ParserPtr();
}

void TSIDLPluginMgr::ProcessPreCompiles(TSIDLGram::ParserPtr Parse)
{
	TSIDLGram::Visit(Parse->TextPars->GetAstRoot(),
		boost::bind(&TSIDLPluginMgr::VisitPreCompile_p,
		this,_1,Parse));
}

void TSIDLPluginMgr::VisitPreCompile_p(TSIDLGram::Parser::Node * Node,TSIDLGram::ParserPtr Parse)
{
	if(IS_TYPE(Node,TSIDLGram::ImportPath))
	{
		TSString ImportPath = NodeToString(Node);
		TSString FullImportPath = GetFileFullPath(ImportPath);

		if(FullImportPath.empty())
		{
			if(GetPipe())
			{
				GetPipe()->Output((GB18030ToTSString("����Ԥ����ָ��ʱ,�Ҳ����ļ�:") + NodeToString(Node) + ".").c_str());
			}
			else
			{
				STD_COUT << (GB18030ToTSString("����Ԥ����ָ��ʱ,�Ҳ����ļ�:") + NodeToString(Node) + ".").c_str();
			}
		}

        TSString basname = GetBaseName(FullImportPath);
        Parse->ImportFileNames.push_back(basname);
		if(TSIDLGram::ParserPtr ImportParse = ParseFile_p(FullImportPath))
		{
			ParserUtilPtr Util = boost::make_shared<ParserUtil>();
			Util->Parse = ImportParse;
			Util->SourceName = ImportPath;
            Util->IsImportFile = true;
            bool IsFirstImport = true;

            for(int utilSize = 0; utilSize < _p->_Utils.size(); utilSize ++)
            {
                if( _p->_Utils[utilSize].first == FullImportPath)
                {
                    IsFirstImport = false;
                }
            }
            if( IsFirstImport )
            {
                 _p->_Utils.push_back(std::pair<TSString,TSIDLPluginMgr::ParserUtilPtr>(FullImportPath,Util));
            }
			//_p->_Utils[FullImportPath] = Util;
		}
	}
}

const TSIDLPluginMgr::ParseMap_T & TSIDLPluginMgr::GetParserUtils()
{
	return _p->_Utils;
}

void TSIDLPluginMgr::CleanParse()
{
    std::vector<std::pair<TSString,TSIDLPluginMgr::ParserUtilPtr>> tmp;
     _p->_Utils.swap(tmp);
}

TSIDLPluginMgr::ParserUtilPtr TSIDLPluginMgr::GetParserUtil( const TSString & FileFullName )
{
    for(int i= 0;i < _p->_Utils.size(); i++)
    {
        if( _p->_Utils[i].first == FileFullName)
        {
            return _p->_Utils[i].second;
        }
    }

	return ParserUtilPtr();
}

bool TSIDLPluginMgr::IsImportParse( ParserUtilPtr ParseUtil )
{
	return IsImportParse(ParseUtil->Parse);
}

bool TSIDLPluginMgr::IsImportParse( TSIDLGram::ParserPtr Parse )
{
	return Parse->GetFullFileName() != _p->_CurParseFile;
}

DocumentPtr TSIDLPluginMgr::GetConfigureDocument()
{
	return _p->_Doc;
}

bool TSIDLPluginMgr::GetTopicOrService()
{
    return _IsTopicOrService;
}

void TSIDLPluginMgr::SetTopicOrService( bool IsTopicOrService )
{
    _IsTopicOrService = IsTopicOrService;
}

void TSIDLPluginMgr::SetIsPlugin( bool IsPlugin )
{
    _IsPlugin = IsPlugin;
}

bool TSIDLPluginMgr::GetIsPlugin()
{
    return _IsPlugin;
}

TSString TSIDLPluginMgr::GetCodeType()
{
    return _p->_CodeType;
}

bool TSIDLPluginMgr::GetFileNameWithXidl()
{
    return _p->_FileNameWithXidl;
}

bool TSIDLPluginMgr::GetGeneratePy()
{
    return _p->_GeneratePy;
}

bool TSIDLPluginMgr::GetGenerateXsimfile()
{
    return _p->_GenerateXsimfile;
}

void TSIDLPluginMgr::SetFileNameWithXidl(bool IsXidl)
{
    _p->_FileNameWithXidl = IsXidl;
}

bool TSIDLPluginMgr::GetIsGenerateImportFile()
{
    return _p->_Isimport;
}

void TSIDLPluginMgr::SetIsGenerateImportFile(bool Isimport)
{
    _p->_Isimport = Isimport;
}

bool TSIDLPluginMgr::GetIsIncludePath()
{
    return _p->_IsIncludePathParse;
}

void TSIDLPluginMgr::SetIsIncludePath(bool IncludePath)
{
    _p->_IsIncludePathParse = IncludePath;
}

void TSIDLPluginMgr::SetPareseMap( TSString FileFullPath,ParserUtilPtr Util )
{
    _p->_Utils.push_back(std::pair<TSString,TSIDLPluginMgr::ParserUtilPtr>(FileFullPath,Util));
    //_p->_Utils[Key] = Util;
}

void TSIDLPluginMgr::SetCurParseFileFullPath( const TSString & FilePath )
{
    _p->_CurParseFile = FilePath;
}

void TSIDLPluginMgr::SetImplFullPath(const TSString& FilePath)
{
	_p->_ImplPath = FilePath;
}

const TSString& TSIDLPluginMgr::GetImplFullPath()
{
	return _p->_ImplPath;
}

void TSIDLPluginMgr::SetGenericFullPath(const TSString& FilePath)
{
	_p->_GenericPath = FilePath;
}

const TSString& TSIDLPluginMgr::GetGenericFullPath()
{
	return _p->_GenericPath;
}

TSString TSIDLPluginMgr::GetPravatFilePath()
{
   return _p->_FilePath;
}

TSString TSIDLPluginMgr::GetFullIdlPath( const TSString & FileName )
{
    return GetFileFullPath(FileName);
}

const std::vector<TSString> & TSIDLPluginMgr::GetLibraryPaths()
{
    if (!_p->_IsLibraryInclude)
    {
        if (HasOption(LibPathKey))
        {
            TSString libPaths = Option<TSString>(LibPathKey);
            std::vector<TSString> veclibPaths = TSMetaType4Link::Split(libPaths,";",false);
            _p->_LibPaths.insert(_p->_LibPaths.end(),veclibPaths.begin(),veclibPaths.end());
        }
        _p->_IsLibraryInclude =true;
    }
    return _p->_LibPaths;
}

void TSIDLPluginMgr::SetService( bool IsService )
{
    _p->_IsService = IsService;
}

bool TSIDLPluginMgr::GetService()
{
    return _p->_IsService;
}

void TSIDLPluginMgr::SetHaveXimPlugin( bool IsPlugin )
{
    _HaveXsim = IsPlugin;
}

bool TSIDLPluginMgr::GetHaveXimPlugin()
{
   return _HaveXsim;
}

TSString TSIDLPluginMgr::GetSDKPath()
{
	
	return _p->_SDKPath;
}

UINT32 TSIDLPluginMgr::GetSortDeep()
{

	return _p->_SortDeep;
}

void TSIDLPluginMgr::SetSortDeep(const UINT32 & Deep)
{
	_p->_SortDeep = Deep;
}

void TSIDLPluginMgr::SetIncludePaths( std::vector<TSString> Ipaths )
{
	_p->_IncludePaths = Ipaths;
}

void TSIDLPluginMgr::setXidlBegin(TSString FileName, UINT32 nLine)
{
	_p->_XidlBegin.insert(std::make_pair(FileName, nLine));
}

bool TSIDLPluginMgr::IsDefualtXidlFile(TSString FileName)
{
	if (FileName == "TopSimBrain_Inter" || FileName == "TopSimCore_Inter" ||
		FileName == "TopSimMMF_Inter" || FileName == "TopSimModels_Inter")
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::map<TSString, UINT32> TSIDLPluginMgr::GetXidlBegin()
{
	return _p->_XidlBegin;
}

TSString GetPluginContent( const TSString & PluginName )
{
    const TSIDLPluginMgr::ParseMap_T & parsemap = PluginMgr()->GetParserUtils();

    TSIDLPluginMgr::ParseMap_T::const_iterator iterU = parsemap.begin();

    TSString result = "";
    for (; iterU != parsemap.end(); iterU++)
    {
        TSString xidlstr = iterU->second->Parse->GetPluginContent(PluginName);
        result = xidlstr + result;
    }
    
    return result;
}