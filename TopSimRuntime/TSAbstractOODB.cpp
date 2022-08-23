#include "stdafx.h"
#include "HRAppManager.h"

#include <boost/timer.hpp>
#include <boost/filesystem.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <QDir>
#include <QUuid>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>
#include <QTextCodec>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>

#include <tbb/concurrent_unordered_set.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/task.h>
#include <TopSimRuntime/TSTimeUtil.h>
#include <TopSimRuntime/TSOCIDbAccess.h>
#include <TopSimRuntime/TSSqliteDbAccess.h>
#include <TopSimRuntime/TSConversion.h>
#include <TopSimRuntime/TSXMLArchive.h>
#include <TopSimRuntime/TSFileSystem.h>
#include <TopSimRuntime/TSOSMisc.h>
#include <TopSimRuntime/TSDynamicLib.h>
#include <TopSimDataInterface/TSDataInteractManager.h>

#include <HRUtil/HRIWorker.h>
#include <HRUtil/HREventBus.h>
#include <HRUtil/HRSmartAssert.h>
#include <HRUtil/HRPerfCollector.h>

#include <HRControls/HRMessageBox.h>
#include <HRControls/HRSingleApplication.h>

#include "HRAppConfig.h"
#include "HRGlobalConfig.h"
#include "HRIPluginManager.h"
#include "HRDefaultScheduler.h"
#include "internal/NamedItemHelper.h"
#include "internal/HRHandleTaskEventObject.h"

#define MAX_PATH 512
#define PERF_LOG_DIR "perf"

extern "C" TOPSIM_RT_DECL bool _UseWGS84;

typedef TSUnorderedMap<UINT32, TSString> SystemTemplate_M;

SINGLETON_IMPLEMENT(HRAppManager);

struct HRLogger : public HRILogger
{
public:
	HRLogger(HRIWorkerPtr worker, const TSString & LoggerName): _Worker(worker), _LoggerName(LoggerName)
	{
 		boost::filesystem::create_directory("log");

		TSString fileName;
		if ("perf" == LoggerName)
		{
			boost::filesystem::create_directory("log/" + LoggerName);

			char bufFileName[128];
			time_t now_time = time(NULL);
			fileName = "log/" + LoggerName + "/" + GetAppConfig<TSString>("Root.AppName") + "_" + LoggerName + "_%Y%m%d%H%M%S.log";

			strftime(bufFileName, sizeof(char) * 128, fileName.c_str(), localtime(&now_time));
			fileName = bufFileName;
		}
		else
		{
			boost::filesystem::create_directory("log/sys");
			fileName = "log/sys/" + LoggerName + ".log";
		}
		
		_LogFileStream.open(fileName);
	}

	~HRLogger(){}

public:
	void WriteLog()
	{
		std::string str;
		std::size_t size = _LogQueue.unsafe_size();
		while (size-- > 0 && _LogQueue.try_pop(str))
		{
			if (!str.empty())
			{
				_LogFileStream << str << "\n";
			}
		}

		Flush();
	}

	void Write(const char* message, unsigned int length)
	{
		_LogQueue.push(message);
		_Worker->Schedule(boost::bind(&HRLogger::WriteLog, this));
	}

	void Flush()
	{ 
		_LogFileStream.flush();
	}

private:
	TSString _LoggerName;
	std::ofstream _LogFileStream;
	HRIWorkerPtr _Worker;
	tbb::concurrent_queue<std::string> _LogQueue;
};

struct HRModuleConfigure
{
	HRModuleConfigure()
		:Initialized(false)
		,AutoInitialize(true)
	{

	}

	TSString          Name;
	int               InterfaceType;
	int               RelaizedType;
	bool			  AutoInitialize;
	bool			  Initialized;
	HRIModulePtr      Instance;
	std::vector<int>  DependenceInterfaces;
};
typedef boost::shared_ptr<HRModuleConfigure> HRModuleConfigurePtr;

struct HRAppManager::HRAppManagerPrivate
{
	HRAppManagerPrivate() 
		:_HandleTaskEventObject(_TaskQueue), _RunMode(HRAppManager::DesignView),_IsUseNetwork(false)
	{
		boost::system::error_code ec;
#ifdef _WIN32
		char szModuleName[MAX_PATH];
		memset(szModuleName, 0, sizeof(szModuleName));
		GetModuleFileName(0, szModuleName, _countof(szModuleName) - 2);
		boost::filesystem::path filePath(szModuleName);
		_WorkingDirectory = TSString2QString(filePath.remove_filename().generic_string().c_str());
#else
		_WorkingDirectory = TSString2QString(boost::filesystem::current_path().string());
#endif
	}

	void InitModules(const std::map<int, HRModuleConfigurePtr> &ModuleConfigs)
	{
		for (std::map<int, HRModuleConfigurePtr>::const_iterator it = ModuleConfigs.begin();
			it != ModuleConfigs.end(); ++it)
		{
			if (std::find(_InitializedTypes.begin(), _InitializedTypes.end(), it->first) == _InitializedTypes.end())
			{
				InitModules2(ModuleConfigs, it->first, it->second);
			}
		}
	}

	void InitModules2(const std::map<int, HRModuleConfigurePtr> &ModuleConfigs, 
		int InterfaceId, const HRModuleConfigurePtr ModuleConfig)
	{
		BOOST_FOREACH(int DependenceType, ModuleConfig->DependenceInterfaces)
		{
			if (std::find(_InitializedTypes.begin(), _InitializedTypes.end(), DependenceType)
				== _InitializedTypes.end())
			{
				std::map<int, HRModuleConfigurePtr>::const_iterator iter = ModuleConfigs.find(DependenceType);
				if (iter == ModuleConfigs.end())
				{
					DEF_LOG_ERROR("%1%2") << "can't find dependenceType:" << TSMetaType::GetTypeName(DependenceType);
				}
				else
				{
					InitModules2(ModuleConfigs, DependenceType, iter->second);					
				}
			}
		}

		ModuleConfig->Instance->Initialize();
		_InitializedModules.push_back(ModuleConfig);
		_InitializedTypes.push_back(InterfaceId);
	}

	typedef std::vector<std::pair<QString,QStringList>> HideObjectListType;

	HRILoggerPtr								_DefaultLogger;
	HRIWorkerPtr								_LoggerWorker;
	UITasks_T									_TaskQueue;
	HRHandleTaskEventObject						_HandleTaskEventObject;
	std::vector<HRAppManager::HRExitProcess>	_ExitProcesses;
	QString										_WorkingDirectory;
	TSGenericHandleTable<HRIModulePtr>			_Modules;
	std::vector<HRModuleConfigurePtr>			_InitializedModules;
	std::vector<int>							_InitializedTypes;
	HRIAppDriverPtr								_Driver;
	HRIAppSkeleton*								_AppSkeleton;
	TSString									_DocumentConfig;
	HRDocumentPtr								_Document;
	HRAppManager::RunMode						_RunMode;
	HRISchedulerPtr								_Scheduler;
	TSDomainPtr									_Domain;
	bool                                        _IsUseNetwork;
};

HRAppManager::HRAppManager()
	: _p(new HRAppManagerPrivate())
{
	_p->_Scheduler = boost::make_shared<HRDefaultScheduler>();
	HREventBus::SetMainScheduler(_p->_Scheduler);
}

HRAppManager::~HRAppManager()
{
	delete _p;
}

bool HRAppManager::UseWGS84()
{
	return _UseWGS84;
}

void HRAppManager::InitApplicationTextCodec(QApplication* app)
{
	QDir rootDir = QFileInfo(QApplication::applicationFilePath()).path();
	QDir::setCurrent(rootDir.canonicalPath());	

	//XSim5.4版本将文件夹名称改为小写，并兼容之前版本
	QString PluginsPath = rootDir.canonicalPath() + "/plugins";

	QDir PluginsDir(PluginsPath);
	if (!PluginsDir.exists())
	{
		PluginsPath = rootDir.canonicalPath() + "/Plugins";
	}

	QCoreApplication::addLibraryPath(PluginsPath);

 	QString translatorFileName = app->applicationDirPath() + FromAscii("/qt_zh_CN.qm");

 	QTranslator *translator = new QTranslator(app);

 	if (app && translator->load(translatorFileName))
 	{
 		app->installTranslator(translator);
 	}
}

bool HRAppManager::InitApplication(QApplication* app, bool singleApp)
{
	if (GetAppConfig<bool>("Root.EnablePerfCollector"))
	{
		HRPerfCollector::SetLogger(boost::bind(&HRILogger::Write, CreateLogger(PERF_LOG_DIR), _1, _2));
	}

	if (app)
	{
		_p->_WorkingDirectory = app->applicationDirPath();
	}

	if (GetAppConfig<bool>("Root.SingleApp"))
	{
		if (HRSingleApplication *sapp = qobject_cast<HRSingleApplication *>(app))
		{
			if (sapp->isRunning())
			{
				sapp->sendMessage(FromAscii("已有程序运行"));
				return false;
			}
		}
	}

	if(GetAppConfig<bool>("Root.Cmd.UseNetwork",false))
	{
		_p->_IsUseNetwork = true;
	}

	_UseWGS84 = HRGlobalConfig::GetDefault().UseWGS84;

	_p->_DocumentConfig = GetAppConfig<TSString>("Root.AppConfig");

	if(_p->_DocumentConfig.empty())
	{
		_p->_DocumentConfig = "config/" DEF_APP_CONF_NAME;
	}

	_p->_Document = boost::make_shared<pugi::xml_document>();
	pugi::xml_parse_result result =_p->_Document->load_file(_p->_DocumentConfig.c_str());

	if(result.status != pugi::status_ok)
	{
		DEF_LOG_ERROR(toAsciiData("%1 配置文件解析失败！")) << _p->_DocumentConfig;

		return false;
	}

	return true;
}

void HRAppManager::ScheduleMainThreadTask( const HRICallablePtr& callable, int delayMillisecs )
{
	if (delayMillisecs <= 0)
	{
		if (_p->_Scheduler->GetMainThreadWorker()->IsCurrent())
		{
			if (callable && !callable->IsCancelled()) HR_IGNORE_EXCEPTION_TRY(callable->Invoke());
		}
		else
		{
			_p->_TaskQueue.push(callable);
			QCoreApplication::postEvent(&_p->_HandleTaskEventObject, new HRUITaskEvent());
		}
	}
	else
	{
		HRHandleTimerEventObject *timerEventObject = new HRHandleTimerEventObject(callable);
		timerEventObject->MoveMainThreadExecute(delayMillisecs);
	}
}

void HRAppManager::Cleanup()
{
	if (GetAppConfig<bool>("Root.EnablePerfCollector"))
	{
		_p->_LoggerWorker.reset();
	}

	for (std::vector<HRExitProcess>::const_iterator iter = _p->_ExitProcesses.begin(); 
		iter != _p->_ExitProcesses.end(); ++iter)
	{
		if (*iter)
		{
			HR_IGNORE_EXCEPTION_TRY((*iter)());
		}
	}
	_p->_ExitProcesses.clear();

	// 优先清理插件管理器
	HRIPluginManagerPtr PluginManager = QueryModuleT<HRIPluginManager>();
	if (PluginManager)
	{
		PluginManager->Cleanup();
	}

	TSGenericHandleTable<HRIModulePtr>::Iterator iter = _p->_Modules.Begin();
	for (; iter; ++iter)
	{
		HRIModulePtr Module = *iter;

		if (!Module)
		{
			continue;
		}

		if (Module == PluginManager)
		{
			continue;
		}

		Module->Cleanup();
	}

	if(_p->_Domain)
	{
		DataManager->DestroyDomain(_p->_Domain);
		_p->_Domain.reset();
	}
}

bool HRAppManager::RestoreDefaultConfiguration(const char* configFileName)
{
	static const char* configList[] = {
		"global.config",
	};

	QString p = QDir::currentPath();
	TSString fileName(configFileName);
	if (fileName.empty())
	{
		int len = sizeof(configList)/sizeof(configList[0]);
		for (int i = 0; i < len; ++i)
		{
			QString OriFilePath = _p->_WorkingDirectory + "/config/" + configList[i];
			QFile OriFile(OriFilePath);
			if (OriFile.exists())
			{
				if (!OriFile.remove())
				{
					DEF_LOG_WARNING("%1%2%3%4%5") << "remove config file " << QString2TSString(p) << QString2TSString(QDir::separator()) << QString2TSString(OriFilePath) << " failed!";
				}
			}

			if (!QFile::copy(QString(":/") + configList[i], OriFilePath))
			{
				DEF_LOG_WARNING("%1%2%3%4%5") << "restore config file " << QString2TSString(p) << QString2TSString(QDir::separator()) << QString2TSString(OriFilePath) << " failed!";
				return false;
			}
		}
	}
	else
	{
		QString OriFilePath = _p->_WorkingDirectory + "/config/" + TSString2QString(fileName);
		QFile OriFile(OriFilePath);
		if (OriFile.exists())
		{
			if (!OriFile.remove())
			{
				DEF_LOG_WARNING("%1%2%3%4%5") << "remove config file " << QString2TSString(p) << QString2TSString(QDir::separator()) << QString2TSString(OriFilePath) << " failed!";
			}
		}

		if (!QFile::copy(":/" + TSString2QString(fileName), OriFilePath))
		{
			DEF_LOG_WARNING("%1%2%3%4%5") << "restore config file " << QString2TSString(p) << QString2TSString(QDir::separator()) << QString2TSString(OriFilePath) << " failed!";
			return false;
		}
	}

	return false;
}

const QString& HRAppManager::GetWorkingDirectory() const
{
	return _p->_WorkingDirectory;
}

void HRAppManager::SetWorkingDirectory( const QString& dir )
{
	_p->_WorkingDirectory = dir;
}

void HRAppManager::RegisterExitProcess( const HRExitProcess& process )
{
	_p->_ExitProcesses.push_back(process);
}

HRILoggerPtr HRAppManager::CreateLogger( const TSString& loggerName )
{
	if (!_p->_LoggerWorker)
	{
		_p->_LoggerWorker = _p->_Scheduler->CreateWorker();
	}

	return boost::make_shared<HRLogger>(_p->_LoggerWorker, loggerName);
}

HRILoggerPtr HRAppManager::GetDefaultLogger()
{
	if (!_p->_DefaultLogger)
	{
		_p->_DefaultLogger = CreateLogger(GetAppConfig<TSString>("Root.AppName"));
	}

	return _p->_DefaultLogger;
}

HRIModulePtr HRAppManager::GetModule( int metaTypeId )
{
	HRIModulePtr Module = _p->_Modules.GetObjectFromHandle(metaTypeId);

	if (!Module)
	{
		TSGenericHandleTable<HRIModulePtr>::Iterator it = _p->_Modules.Begin();

		for (; it; ++it)
		{
			if (!(*it))
			{
				continue;
			}

			if (TSMetaType::CanConvert((*it)->GetMetaTypeId(), metaTypeId))
			{
				return *it;
			}
		}
	}

	return Module;
}

void HRAppManager::LoadModules()
{
	if (_p->_Document)
	{
		PreloadModules();

		pugi::xpath_node_set nodes = _p->_Document->select_nodes(toAsciiData(DOCUMENT_PREFIX "模块列表/模块"));
		
		std::map<int, HRModuleConfigurePtr> ModuleConfigs;

		for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			if (it->node().attribute(toAsciiData("启用")).empty() || it->node().attribute(toAsciiData("启用")).as_bool())
			{
				int InterfaceId = TSMetaType::GetType(it->node().attribute(toAsciiData("接口")).value());
				int RealizedId = TSMetaType::GetType(it->node().attribute(toAsciiData("实现")).value());

				if (_p->_Modules.GetObjectFromHandle(InterfaceId))
				{
					DEF_LOG_ERROR("%1%2") << it->node().attribute(toAsciiData("名称")).value() << " conflicting!";
					continue;
				}
				if (!InterfaceId || !RealizedId)
				{
					DEF_LOG_ERROR("%1%2") << it->node().attribute(toAsciiData("名称")).value() << " load failed!";

					continue;
				}

				if (!TSMetaType::IsAbstract(RealizedId))
				{
					if (TSMetaType::CanConvert(RealizedId, InterfaceId))
					{
						if (TSMetaType::CanConvert(RealizedId, HRIModule::GetMetaTypeIdStatic()))
						{
							if (HRIModulePtr Module = TS_STATIC_CAST(TSMetaType::ConstructSmart(RealizedId, NULL, false), HRIModulePtr))
							{
								HRIModulePtr result = _p->_Modules.VerifyHandle(Module, InterfaceId);

								HRModuleConfigurePtr ModuleConfigure = boost::make_shared<HRModuleConfigure>();
								ModuleConfigure->Name = it->node().attribute(toAsciiData("名称")).as_string();
								ModuleConfigure->Initialized = false;
								ModuleConfigure->AutoInitialize = it->node().attribute(toAsciiData("自动初始化")).empty() || it->node().attribute(TS_TEXT("自动初始化")).as_bool();
								ModuleConfigure->Instance = Module;
								TSString DependenceInterfaces = it->node().attribute(toAsciiData("依赖项列表")).as_string();
								std::vector<TSString> Dependences = TSStringUtil::Split(DependenceInterfaces, ",", false);

								for (int i = 0; i < Dependences.size(); ++i)
								{
									if (int Type = TSMetaType::GetType(Dependences[i].c_str()))
									{
										if (TSMetaType::CanConvert(Type, HRIModule::GetMetaTypeIdStatic()))
										{
											ModuleConfigure->DependenceInterfaces.push_back(Type);
										}
									}
								}

								ASSERT(result == Module);
								ModuleConfigs[InterfaceId] = ModuleConfigure;
							}
						}
						else
						{
							DEF_LOG_ERROR("%1%2") << it->node().attribute(toAsciiData("名称")).value() << " Realized is not inherited from HRIModule!";
						}
					}
					else
					{
						DEF_LOG_ERROR("%1%2%3") << it->node().attribute(toAsciiData("名称")).value() << " Realized is not inherited from !"<< it->node().attribute(toAsciiData("接口")).value();
					}	
				}
				else
				{
					DEF_LOG_ERROR("%1%2") << it->node().attribute(TS_TEXT("Name")).value() << " Realized is abstract!";
				}
			}
		}

		_p->InitModules(ModuleConfigs);
	}
}

HRIAppDriverPtr HRAppManager::GetDriver()
{
	return _p->_Driver;
}

void HRAppManager::SetDriver( HRIAppDriverPtr driver )
{
	_p->_Driver = driver;
}

void HRAppManager::SetAppSkeleton( HRIAppSkeleton* skeleton )
{
	_p->_AppSkeleton = skeleton;
}

HRIAppSkeleton * HRAppManager::GetAppSkeleton()
{
	return _p->_AppSkeleton;
}

HRDocumentPtr HRAppManager::GetDocument()
{
	return _p->_Document;
}

void HRAppManager::PreloadModules()
{
	if (HRDocumentPtr pluginDoc = HRGetAppManager()->GetDocument())
	{
		pugi::xpath_node_set nodes = pluginDoc->select_nodes(toAsciiData(DOCUMENT_PREFIX "预加载模块列表/模块"));
		for (std::size_t i = 0; i < nodes.size(); i++)
		{
			TSString ModuleFileName = nodes[i].node().attribute(toAsciiData("名称")).value();
#ifndef _WIN32
			ModuleFileName = "lib" + ModuleFileName;
#endif
			bool enabled = nodes[i].node().attribute(toAsciiData("启用")).as_bool(false);
			TSString dependencePath = nodes[i].node().attribute(toAsciiData("依赖路径")).value();

			if(enabled)
			{
				HMODULE module = NULL;

				LoadModuleLibrary(module, ModuleFileName, dependencePath);

				if(!module)
				{
					DEF_LOG_ERROR("%1%2") << ModuleFileName << toAsciiData(" 加载失败!");
				}
			}
		}
	}
}

void HRAppManager::LoadModuleLibrary( HMODULE& module, const TSString& pluginName, const TSString& dependencePath )
{
	TSString DependencePath = dependencePath;
	TSString asciiPath = DependencePath;

	if (!asciiPath.empty())
	{
		if(!boost::filesystem::path(asciiPath.c_str()).is_complete())
		{
			DependencePath = QString2TSString(QDir::toNativeSeparators(TSString2QString(
				(boost::filesystem::path(QString2TSString(HRGetWorkingDirectory())) / asciiPath.c_str()).string())));
		}			 
	}

	boost::filesystem::path pluginBasePath = boost::filesystem::path(DependencePath.c_str());

	QString currentPath = QDir::currentPath();
	if (!DependencePath.empty())
	{
		QCoreApplication::addLibraryPath(TSString2QString(DependencePath));
		QDir::setCurrent(TSString2QString(DependencePath));
	}

	if(!boost::filesystem::path(pluginName.c_str()).is_complete())
	{
		TSString FullModuleFileName = (pluginBasePath/pluginName.c_str()).string();
		module = LoadSuitDynamicLibrary(FullModuleFileName);
	}
	if (!module)
	{
		module = LoadSuitDynamicLibrary(pluginName);
	}
	if (!module)
	{
		TSString FullModuleFileName = (pluginBasePath/"bin"/pluginName.c_str()).string();
		module = LoadSuitDynamicLibrary(FullModuleFileName);
	}
	if (!module)
	{
		TSString FullModuleFileName = (pluginBasePath/"../bin"/pluginName.c_str()).string();
		module = LoadSuitDynamicLibrary(FullModuleFileName);
	}
	if (!module)
	{
		TSString FullModuleFileName = (pluginBasePath/".."/pluginName.c_str()).string();
		module = LoadSuitDynamicLibrary(FullModuleFileName);
	}
	if (!module)
	{
		module = LoadSuitDynamicLibrary(TSString(TS_TEXT("bin/")) + pluginName);
	}
	if (!module)
	{
		module = LoadSuitDynamicLibrary(TSString(TS_TEXT("../bin/")) + pluginName);
	}
	if (!module)
	{
		module = LoadSuitDynamicLibrary(TSString(TS_TEXT("../")) + pluginName);
	}

	QDir::setCurrent(currentPath);
}

HRAppManager::RunMode HRAppManager::GetRunMode() const
{
	return _p->_RunMode;
}

void HRAppManager::SetRunMode( RunMode mode )
{
	_p->_RunMode = mode;
}

HRISchedulerPtr HRAppManager::GetScheduler() const
{
	return _p->_Scheduler;
}

TSGenericHandleTable<HRIModulePtr>& HRAppManager::GetModules()
{
	return _p->_Modules;
}

TSDomainPtr HRAppManager::GetContextDomain()
{
	if(!_p->_Domain)
	{
		TSString DomainId = GetAppConfig<TSString>("Root.Cmd.AppDomainId");

		if (DomainId.empty())
		{
			DomainId = HRGlobalConfig::GetDefault().AppDomainId;
		}

		if(DomainId.empty())
		{
			DomainId = "150";
		}

		_p->_Domain = DataManager->GetDomain(DomainId);

		if (!_p->_Domain)
		{
			_p->_Domain = DataManager->CreateDomain(DomainId);
		}
	}

	return _p->_Domain;
}

void HRAppManager::SetContextDomain( TSDomainPtr Domain )
{
	_p->_Domain = Domain;
}

bool HRAppManager::IsUseNetwork()
{
	return _p->_IsUseNetwork;
}

