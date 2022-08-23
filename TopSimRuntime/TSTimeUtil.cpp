#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#include <boost/foreach.hpp>

#ifndef _WRS_KERNEL 
#include <boost/filesystem.hpp>
#endif

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimUtil/TSFileSystem.h>
#include <TopSimUtil/TSDynamicLib.h>
#include <TopSimUtil/pugixml.hpp>
#else
#include <TopSimRuntime/TSFileSystem.h>
#include <TopSimRuntime/TSDynamicLib.h>
#include <TopSimRuntime/pugixml.hpp>
#include <TopSimRuntime/TSLogsUtil.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TSTopicTypeManager.h"

SINGLETON_IMPLEMENT(TSTopicTypeManager);

/*
  主题类型包装
*/
struct TSTopicTypeWarpper : public boost::intrusive_ref_counter<TSTopicTypeWarpper>
{
	TSString               TypeName;		//类型名称
	TSString               TopicName;		//主题名称
	TSString               Desc;			//描述
	TSTypeSupportPtr       Support;			//类型支持
	TSTopicHelperPtr       TopicHelper;		//主题继承关系
};

typedef boost::intrusive_ptr<TSTopicTypeWarpper> TSTopicTypeWarpperPtr;

typedef TSConUnorderedMap<TSString,UINT32> TSTopicIdMapType_T;
typedef std::vector<TSTopicTypeWarpperPtr> TSTopicWarpperArray;
typedef TSConUnorderedMap<TSString,TSTypeSupportPtr> TSTypeSupportMap_T;
typedef TSConUnorderedMap<UINT32,UINT32> TSTopicCRCMapType_T;

/*
  主题类型管理私有结构
*/
struct TSTopicTypeManagerPrivate
{
	TSTopicTypeManagerPrivate();

	UINT32                               _LastTopicIdIndex;		//最近一个主题注册的索引
	TSTypeSupportMap_T                   _TypeSupports;			//Map<TypeName, TSTypeSupport对象智能指针>
	TSTopicWarpperArray                  _TopicWarppers;		//已注册的主题包装数组
	TSTopicIdMapType_T                   _TopicIdMaps;			//主题注册Map<TopicName，主题注册ID(主题句柄)>
	TSTopicCRCMapType_T                  _TopicCRCMaps;			//主题注册Map<CRC(TopicName)，主题注册ID(主题句柄)>
	TSString                             _CfgPath;				//配置文件路径
	std::string              			 _WrokDirectoryPath;	//工作目录路径
	pugi::xml_document                   _Docment;				//xml文本对象
	TSConUnorderedMap<TSString,TSString> _ExtendCfgs;			//附加配置项
	bool                                 _IsPostInitialized;	//是否是构建过继承关系
	bool                                 _IsInitialized;
};

TSTopicTypeManagerPrivate::TSTopicTypeManagerPrivate()
{
	_LastTopicIdIndex = 0;
	_IsPostInitialized = false;
	_IsInitialized = false;
}

/* 解析配置文件中Modules节点并加载响应库 */
void TSTopicTypeManager::LoadModules()
{
	pugi::xpath_node_set nodes = _p->_Docment.select_nodes(TS_TEXT("Config/Modules/Module"));
	for(pugi::xpath_node_set::const_iterator it = nodes.begin();it != nodes.end(); ++it)
	{
		if(it->node().attribute("Enabled").as_bool())
		{
			TSString ModuleName = it->node().attribute("Name").as_string();
			if(!ModuleName.empty())
			{
                TSString DepPath = it->node().attribute("DependencePath").as_string();
                TSString ModuleFile = DepPath.empty() ? ModuleName : DepPath + TS_TEXT("/") + ModuleName;
				LoadModule(ModuleFile);
			}
		}
	}
}

void TSTopicTypeManager::LoadModule( const TSString& ModuleFileName )
{

#ifndef  _WRS_KERNEL
	HMODULE hModule =NULL;

	if(!boost::filesystem::path(ModuleFileName.c_str()).is_complete())
	{
		TSString FullModuleFileName = (boost::filesystem::path(constchar_cast(ModuleFileName)).remove_filename()/ModuleFileName.c_str()).string();
		hModule = LoadSuitDynamicLibrary(FullModuleFileName);
	}
	if (!hModule)
	{
        TSString FullModuleFileName = (_p->_WrokDirectoryPath + "/" + ModuleFileName);
        hModule = LoadSuitDynamicLibrary(FullModuleFileName);
	}
	if (!hModule)
	{
		TSString FullModuleFileName = (boost::filesystem::path(constchar_cast(ModuleFileName)).remove_filename()/"bin"/ModuleFileName.c_str()).string();
		hModule = LoadSuitDynamicLibrary(FullModuleFileName);
	}
	if (!hModule)
	{
		TSString FullModuleFileName = (boost::filesystem::path(constchar_cast(ModuleFileName)).remove_filename()/"../bin"/ModuleFileName.c_str()).string();
		hModule = LoadSuitDynamicLibrary(FullModuleFileName);
	}
	if (!hModule)
	{
		TSString FullModuleFileName = (boost::filesystem::path(constchar_cast(ModuleFileName)).remove_filename()/".."/ModuleFileName.c_str()).string();
		hModule = LoadSuitDynamicLibrary(FullModuleFileName);
	}
	if (!hModule)
	{
		hModule = LoadSuitDynamicLibrary(TS_TEXT("bin/") + ModuleFileName);
	}
	if (!hModule)
	{
		hModule = LoadSuitDynamicLibrary(TS_TEXT("../bin/") + ModuleFileName);
	}
	if (!hModule)
	{
		hModule = LoadSuitDynamicLibrary(TS_TEXT("../") + ModuleFileName);
	}

	if (hModule==NULL)
	{
        DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_119)  << ModuleFileName;
	}
	else
	{
		DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_123) << ModuleFileName;
	}
#endif //#ifndef  _WRS_KERNEL
}

void TSTopicTypeManager::SetCfg( const TSString& cfgPath ,const TSString& workDirectoryPath)
{
	if (!_p->_IsInitialized)
	{
		_p->_IsInitialized =  true;

		_p->_CfgPath = cfgPath;
		_p->_WrokDirectoryPath = workDirectoryPath;

		if (_p->_WrokDirectoryPath.empty())
		{
			_p->_WrokDirectoryPath = boost::filesystem::current_path().string();
		}

#if !defined(XSIM3_2) && !defined(XSIM3_3)
		if (TSLoggerPtr Logger = TSLoggerManager::Instance()->GetDefaultLogger())
		{
			Logger->SetLoggingSeverity(Logger->GetLoggingSeverity() | TSILoggingSeverity::_LOG_KERNEL);
		}
#endif

		pugi::xml_parse_result result = _p->_Docment.load_file(_p->_CfgPath.c_str());
		if (result.status != pugi::status_ok)
		{
			result = _p->_Docment.load_file((TSString("config/") + _p->_CfgPath.c_str()).c_str());
		}

		if (result.status == pugi::status_ok)
		{
			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_140) << cfgPath;
			//解析附加配置内容
			LoadExtendCfgs();

			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_143);
			LoadPreloadModules();

			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_146);
			LoadModules();

			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_149);
			LoadObjectsModules();

			//构建继承关系
			PostModulesLoaded();
		}
		else
		{
			throw TSException(TSString(CN_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_157) + cfgPath);
		}
	}
}

void TSTopicTypeManager::ReloadCfg(const TSString & cfgPath, const TSString & workDirectoryPath)
{
	if (!_p->_IsInitialized)
	{
		_p->_IsInitialized = true;
#if !defined(XSIM3_2) && !defined(XSIM3_3)
		if (TSLoggerPtr Logger = TSLoggerManager::Instance()->GetDefaultLogger())
		{
			Logger->SetLoggingSeverity(Logger->GetLoggingSeverity() | TSILoggingSeverity::_LOG_KERNEL);
		}
#endif
	}
		_p->_CfgPath = cfgPath;
		_p->_WrokDirectoryPath = workDirectoryPath;

		if (_p->_WrokDirectoryPath.empty())
		{
			_p->_WrokDirectoryPath = boost::filesystem::current_path().string();
		}
		pugi::xml_parse_result result = _p->_Docment.load_file(_p->_CfgPath.c_str());
		if (result.status != pugi::status_ok)
		{
			result = _p->_Docment.load_file((TSString("config/") + _p->_CfgPath.c_str()).c_str());
		}

		if (result.status == pugi::status_ok)
		{
			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_140) << cfgPath;
			//解析附加配置内容
			LoadExtendCfgs();

			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_143);
			LoadPreloadModules();

			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_146);
			LoadModules();

			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_149);
			LoadObjectsModules();

			//构建继承关系
			PostModulesLoaded();
		}
		else
		{
			throw TSException(TSString(CN_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_157) + cfgPath);
		}
}

const TSString& TSTopicTypeManager::GetConfigurePath()
{
	return _p->_CfgPath;
}

TSTopicTypeManager::TSTopicTypeManager()
	:_p(new TSTopicTypeManagerPrivate)
{
	
}


TSTopicTypeManager::~TSTopicTypeManager()
{
	delete _p;
}

/* 解析配置文件中PerLoadModules节点并加载响应库 */
void TSTopicTypeManager::LoadPreloadModules()
{
	pugi::xpath_node_set module_nodes = _p->_Docment.select_nodes(TS_TEXT("Config/PreLoadModules/Module"));
	for(pugi::xpath_node_set::const_iterator it = module_nodes.begin();it != module_nodes.end(); ++it)
	{
		TSString ModuleFileName = it->node().attribute(TS_TEXT("Name")).value();    

		TSString DependencePath = it->node().attribute(TS_TEXT("DependencePath")).value();

		if (!it->node().attribute(TS_TEXT("Enabled")).empty() && !it->node().attribute(TS_TEXT("Enabled")).as_bool())
		{
			continue;
		}
#ifndef _WIN32
		ModuleFileName = TSString("lib") + ModuleFileName;
#endif
#ifndef  _WRS_KERNEL
		if (!DependencePath.empty())
		{
			if(!boost::filesystem::path(DependencePath.c_str()).is_complete())
			{
				DependencePath = _p->_WrokDirectoryPath + "/" + DependencePath;
			}
		}

		HMODULE hModule = NULL;
		boost::filesystem::path ori_current_path;

		if (!DependencePath.empty())
		{
			try
			{
				ori_current_path = boost::filesystem::current_path();

				boost::filesystem::current_path(DependencePath.c_str());
			}
			catch (const TSException &exp)
			{
				throw exp;
			}        
		}

		if(!boost::filesystem::path(ModuleFileName.c_str()).is_complete())
		{
			TSString FullModuleFileName = _p->_WrokDirectoryPath + "/" + ModuleFileName;
			hModule = LoadSuitDynamicLibrary(FullModuleFileName);
		}
		if (!hModule)
		{
			hModule = LoadSuitDynamicLibrary(ModuleFileName.c_str());
		}

		if(hModule)
		{
			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_231) << ModuleFileName;
		}

		if (!ori_current_path.empty())
		{
			try
			{
				boost::filesystem::current_path(ori_current_path);
			}
			catch (const TSException &e)
			{
				throw e;
			}
		}
#endif //#ifndef  _WRS_KERNEL
	}
}

void TSTopicTypeManager::LoadObjectsModules()
{
    if(pugi::xml_node node = _p->_Docment.select_single_node(TS_TEXT("Config/Objects")).node())
    {
        TSString ObjectPath = node.attribute(TS_TEXT("Path")).as_string();
        if (ObjectPath.empty())
        {
            return;
        }
#if !defined(_WRS_KERNEL)
        if(boost::filesystem::path(ObjectPath.c_str()).is_complete())
        {
            boost::filesystem::path p(ObjectPath.c_str());
            if (!boost::filesystem::exists(p))
            {
				DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_262) << p.string();
            }
        }
        else
        {
            boost::filesystem::path p = GetWorkDirPath() + "/" + ObjectPath;

            if (!boost::filesystem::exists(p))
            {
                DEF_LOG_TRACE(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_271) << p.string();
            }
            ObjectPath = p.string();
        }

        std::vector<TSString> FileNames;
        EnumDirectoryFiles(ObjectPath,_MODULE_EXT,FileNames);
        BOOST_FOREACH(TSString FileName,FileNames)
        {
            LoadModule(FileName);
        }
#endif //#if !defined(_WRS_KERNEL) || !defined(__VXWORKS__)
    }
}

pugi::xml_document& TSTopicTypeManager::GetConfigureDoc()
{
	return _p->_Docment;
}

/* 解析配置文件中ExtendConfigs节点配置 */
void TSTopicTypeManager::LoadExtendCfgs()
{
	_p->_ExtendCfgs.clear();//重新初始化前先清空
	pugi::xpath_node_set nodes = _p->_Docment.select_nodes("Config/ExtendConfigs/ExtendConfig");
	for(pugi::xpath_node_set::const_iterator it = nodes.begin();it != nodes.end(); ++it)
	{
		TSString Key = it->node().attribute("Key").as_string();
		TSString Value = it->node().attribute("Value").as_string();
		_p->_ExtendCfgs[Key] = Value;
	}
}

static TSString DummyValue;

const TSString& TSTopicTypeManager::GetExtendCfg( const TSString& Key )
{
	TSConUnorderedMap<TSString,TSString>::iterator it = 
		_p->_ExtendCfgs.find(Key);

	if(it !=_p->_ExtendCfgs.end())
	{
		return it->second;
	}

	return DummyValue;
}

void TSTopicTypeManager::InitNoCacheTopicSet(std::set<UINT32> & noCacheTopicHandle)
{
	noCacheTopicHandle.clear();
	TSString noCacheTopics = GetExtendCfg("NoCacheTopics");
	if (!noCacheTopics.empty())
	{
		UINT32 t_handleValue = 0;

#if defined(XSIM3_3) || defined(XSIM3_2)
		TSStringArray topicArray = TSStringUtil::Split(noCacheTopics, ";", true);
#else
		TSStringArray topicArray = TSStringUtil::Split(noCacheTopics, ";");
#endif
		
		for (std::vector<TSString>::iterator it = topicArray.begin(); it != topicArray.end(); ++it)
		{
			noCacheTopicHandle.insert(GetTopicByTopicName(*it)._value);
		}
	}
}

const TSString& TSTopicTypeManager::GetWorkDirectoryPath()
{
	return _p->_WrokDirectoryPath;
}

void TSTopicTypeManager::SetExtendCfg( const TSString& Key,const TSString& Value )
{
	_p->_ExtendCfgs[Key] = Value;
}

void TSTopicTypeManager::PostModulesLoaded()
{
	for(size_t i = 0; i < _p->_TopicWarppers.size(); ++i)
	{
		for(size_t j = 0; j < _p->_TopicWarppers.size(); ++j)
		{
			if(TSTopicTypeWarpperPtr Parent = _p->_TopicWarppers[i])
			{
				if(TSTopicTypeWarpperPtr Child = _p->_TopicWarppers[j])
				{
					if(Child->TopicHelper->CanConvert(i))
					{
						if(j != i)
						{
							Child->TopicHelper->AddParent(i);
							Parent->TopicHelper->AddChild(j);
						}
					}
				}
			}
		}
	}

	_p->_IsPostInitialized = true;
}

/* 注册主题 */
bool TSTopicTypeManager::RegisterTopic( UINT32 Id,const TSString& TypeName,const TSString& TopicName,const TSString& Desc, TSTopicHelperPtr TopicTopicHelper )
{
	ASSERT(Id < _p->_TopicWarppers.size());
	if(TSTopicTypeWarpperPtr & Warpper = _p->_TopicWarppers[Id])
	{
		TSTypeSupportMap_T::iterator type_it = _p->_TypeSupports.find(TypeName);
		if(type_it != _p->_TypeSupports.end())
		{
			Warpper->TypeName = TypeName;
			Warpper->TopicName = TopicName;
			Warpper->Desc = Desc;
			Warpper->Support = type_it->second;
			Warpper->TopicHelper = TopicTopicHelper;
			//是否已经构建过继承关系
			if (_p->_IsPostInitialized)
			{
				for (size_t j = 0; j < _p->_TopicWarppers.size(); ++j)
				{
					if (TSTopicTypeWarpperPtr & RegisterdWarpper = _p->_TopicWarppers[j])
					{
						if (Warpper->TopicHelper->CanConvert(j))
						{
							Warpper->TopicHelper->AddParent(j);
							RegisterdWarpper->TopicHelper->AddChild(Id);
						}
					}
				}
			}
		}
		else
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_367) << TypeName;
		}
		
	}
	else
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_373) << TypeName;
	}
	
	return false;
}

bool TSTopicTypeManager::RegisterTypeSupport( const TSString& TypeName, TSTypeSupportPtr TypeSupport, int DataMetaTypeId)
{
	//if (_p->_IsPostInitialized)
	//{
	//	TSString ModuleName = TSMetaType::GetTypeModuleFileName(DataMetaTypeId);

	//	DEF_LOG_WARNING("type %1,in module %2 loaded after PostInitialized,callback will lost!") << TypeName << ModuleName;
	//}

	TSTypeSupportMap_T::iterator type_it = _p->_TypeSupports.find(TypeName);
	if(type_it != _p->_TypeSupports.end())
	{
        DEF_LOG_WARNING(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPEMANAGER_CPP_384) << TypeName;
	}
	_p->_TypeSupports[TypeName] = TypeSupport;

	return true;
}

static TSTypeSupportPtr DummyTypeSupport;
TSTypeSupportPtr  TSTopicTypeManager::GetTypeSupportByTopic( TSTOPICHANDLE TopicHandle )
{
	if(TopicHandle._value < _p->_TopicWarppers.size())
	{
		if(TSTopicTypeWarpperPtr & Warpper = _p->_TopicWarppers[TopicHandle._value])
		{
			return Warpper->Support;
		}
	}

	return DummyTypeSupport;
}

static TSTopicHelperPtr DummyTypeHelper;
TSTopicHelperPtr  TSTopicTypeManager::GetTopicHelperByTopic( TSTOPICHANDLE TopicHandle )
{
	if(TopicHandle._value < _p->_TopicWarppers.size())
	{
		if(TSTopicTypeWarpperPtr & Warpper = _p->_TopicWarppers[TopicHandle._value])
		{
			return Warpper->TopicHelper;
		}
	}

	return DummyTypeHelper;
}

static TSString DummyString;

const TSString& TSTopicTypeManager::GetTopicNameByTopic( TSTOPICHANDLE TopicHandle )
{
	if(TopicHandle._value < _p->_TopicWarppers.size())
	{
		if(TSTopicTypeWarpperPtr & Warpper = _p->_TopicWarppers[TopicHandle._value])
		{
			return Warpper->TopicName;
		}
	}

	return DummyString;
}

const TSString& TSTopicTypeManager::GetDescByTopic( TSTOPICHANDLE TopicHandle )
{
	if(TopicHandle >= 0)
	{
		if(TopicHandle._value <= _p->_TopicWarppers.size())
		{
			if(TSTopicTypeWarpperPtr & Warpper = _p->_TopicWarppers[TopicHandle._value])
			{
				return Warpper->Desc;
			}
		}
	}

	return DummyString;
}


const TSString& TSTopicTypeManager::GetTypeNameByTopic( TSTOPICHANDLE TopicHandle )
{
	if(TopicHandle >= 0 )
	{
		if(TopicHandle._value <= _p->_TopicWarppers.size())
		{
			if(TSTopicTypeWarpperPtr & Warpper = _p->_TopicWarppers[TopicHandle._value])
			{
				return Warpper->TypeName;
			}
		}
	}

    return DummyString;
}

TSTypeSupportPtr TSTopicTypeManager::GetSupportByTypeName( const TSString& TypeName )
{
	TSTypeSupportMap_T::iterator type_it = _p->_TypeSupports.find(TypeName);
	if(type_it != _p->_TypeSupports.end())
	{
		return type_it->second;
	}

	return DummyTypeSupport;
}

std::vector<TSTOPICHANDLE> TSTopicTypeManager::GetRegisteredTopics()
{
	std::vector<TSTOPICHANDLE> Topics;
	for(size_t i = 0; i < _p->_TopicWarppers.size(); ++i)
	{
		if(TSTopicTypeWarpperPtr Warpper = _p->_TopicWarppers[i])
		{
			Topics.push_back(i);
		}
	}

	return Topics;
}

std::vector<TSString> TSTopicTypeManager::GetRegisteredTypes()
{
	std::vector<TSString> Types;
	TSTypeSupportMap_T::iterator it = _p->_TypeSupports.begin();
	while(it != _p->_TypeSupports.end())
	{
		Types.push_back(it->first);
		++it;
	}

	return Types;
}

UINT32 TSTopicTypeManager::GenerateTopicByName( const TSString & TopicName )
{
	TSTopicIdMapType_T::iterator it = _p->_TopicIdMaps.find(TopicName);
	UINT32 Index = 0;
	
	if(it != _p->_TopicIdMaps.end())
	{
		Index = it->second;
	}
	else
	{
		Index = ++_p->_LastTopicIdIndex;
		_p->_TopicIdMaps.insert(std::make_pair(TopicName, Index));
		_p->_TopicCRCMaps.insert(std::make_pair(GetCRC32(TopicName), Index));
	}
	if(Index >= _p->_TopicWarppers.size())
	{
		_p->_TopicWarppers.resize(Index + 100);
	}
	TSTopicTypeWarpperPtr & Warpper = _p->_TopicWarppers[Index];

	if (!Warpper)
	{
		Warpper = new TSTopicTypeWarpper();
		Warpper->TopicName = TopicName;
	}
	ASSERT(Warpper && Warpper->TopicName == TopicName);
	return Index;
}

TSTOPICHANDLE TSTopicTypeManager::GetTopicByTopicName( const TSString & TopicName )
{
	TSTopicIdMapType_T::iterator it = _p->_TopicIdMaps.find(TopicName);

	if(it != _p->_TopicIdMaps.end())
	{
		return it->second;
	}

	return TSTOPICHANDLE();
}

TSTOPICHANDLE TSTopicTypeManager::PreRegisterTopicByName( const TSString & TopicName )
{
	TSTopicIdMapType_T::iterator it = _p->_TopicIdMaps.find(TopicName);

	if(it != _p->_TopicIdMaps.end())
	{
		return it->second;
	}
	else
	{
		return GenerateTopicByName(TopicName);
	}
}

TSTOPICHANDLE TSTopicTypeManager::GetMaxRegisterdTopic()
{
	return _p->_TopicWarppers.size();
}

TSTOPICHANDLE TSTopicTypeManager::GetTopicByCRC32( UINT32 CRC32Value )
{
	TSTopicCRCMapType_T::iterator it = _p->_TopicCRCMaps.find(CRC32Value);
	if(it != _p->_TopicCRCMaps.end())
	{
		return it->second;
	}
	
	return TSTOPICHANDLE();
}

TOPSIMDATAINTERFACE_DECL bool RegisterTopic(UINT32 Id,const TSString& TypeName,const TSString& TopicName,const TSString& Desc,
	TSTopicHelperPtr TopicTopicHelper)
{
	return TSTopicTypeManager::Instance()->RegisterTopic(Id, TypeName, TopicName, Desc, TopicTopicHelper);
}

TOPSIMDATAINTERFACE_DECL bool RegisterTypeSupport( const TSString& TypeName, TSTypeSupportPtr TypeSupport ,int DataMetaTypeId)
{
	return TSTopicTypeManager::Instance()->RegisterTypeSupport(TypeName, TypeSupport, DataMetaTypeId);
}

TOPSIMDATAINTERFACE_DECL UINT32 GenerateTopicByName( const TSString & TopicName )
{
	return TSTopicTypeManager::Instance()->GenerateTopicByName(TopicName);
}
