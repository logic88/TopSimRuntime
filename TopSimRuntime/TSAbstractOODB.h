#ifndef HRAPPMANAGER_H
#define HRAPPMANAGER_H

#ifndef Q_MOC_RUN

#include "HRIModule.h"
#include "HRILogger.h"

#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>

#include <TopSimRuntime/pugixml.hpp>
#include <TopSimRuntime/TSDbAccess.h>
#include <TopSimRuntime/TSGenericHandleTable.h>
#include <TopSimRuntime/TSDynamicLib.h>

#include <HRUtil/HRIScheduler.h>
#include <HRUtil/HRCallableObject.h>

#endif

class QApplication;
class HRIAppSkeleton;

class HRUICOMMON_EXPORT HRAppManager : public TSSingleton<HRAppManager>
{
	SINGLETON_DECLARE(HRAppManager);

public:
	enum RunMode
	{
		DesignView			= (1 << 0),
		PlaybackView		= (1 << 1),
		RunView				= (1 << 2),
		SceneView			= (1 << 3),
		ConceptView			= (1 << 4),
		AllView				= 0xFFFF,
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	初始化应用程序的编码设置. </summary>
	///
	/// <remarks>	XSIM Team, 2018/8/8. </remarks>
	///
	/// <param name="app">	Qt应用程序. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void InitApplicationTextCodec(QApplication* app);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	初始化应用程序. </summary>
	///
	/// <remarks>	XSIM Team, 2018/8/8. </remarks>
	///
	/// <param name="app">			Qt应用程序. </param>
	/// <param name="singleApp">	App是否单例运行. </param>
	///
	/// <returns>	初始化成功返回true，否则返回false. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool InitApplication(QApplication* app, bool singleApp = false);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取WGS84启用情况. </summary>
	///
	/// <remarks>	XSIM Team, 2018/8/8. </remarks>
	///
	/// <returns>	WGS84是否开启. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UseWGS84();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取驱动类，驱动类全局只有一个 </summary>
	///
	/// <remarks>	孟龙龙, 2018/8/8. </remarks>
	///
	/// <returns>	驱动. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	HRIAppDriverPtr GetDriver();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置驱动类 </summary>
	///
	/// <remarks>	孟龙龙, 2018/8/8. </remarks>
	///
	/// <param name="driver"> 驱动. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetDriver(HRIAppDriverPtr driver);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取应用程序骨架. </summary>
	///
	/// <remarks>	孟龙龙, 2018/8/8. </remarks>
	///
	/// <returns>	应用程序骨架. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	HRIAppSkeleton* GetAppSkeleton();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置应用程序骨架. </summary>
	///
	/// <remarks>	孟龙龙, 2018/8/8. </remarks>
	///
	/// <param name="skeleton">	应用程序骨架. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetAppSkeleton(HRIAppSkeleton* skeleton);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取当前的运行模式 </summary>
	///
	/// <remarks>	孟龙龙, 2018/8/8. </remarks>
	///
	/// <returns>	运行模式. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	RunMode GetRunMode() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置当前的运行模式 </summary>
	///
	/// <remarks>	孟龙龙, 2018/8/8. </remarks>
	///
	/// <param name="mode">	运行模式. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetRunMode( RunMode mode );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	加载模块. </summary>
	///
	/// <remarks>	孟龙龙, 2018/8/8. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadModules();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取当前场景上下文的数据域指针. </summary>
	///
	/// <remarks>	刘涛, 2018/1/3. </remarks>
	///
	/// <returns>	数据域. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSDomainPtr GetContextDomain();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置当前场景上下文的数据域指针. </summary>
	///
	/// <remarks>	刘涛, 2018/1/3. </remarks>
	///
	/// <param name="driver">	数据域. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetContextDomain(TSDomainPtr Domain);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取是否是协同模式. </summary>
	///
	/// <remarks>	刘涛, 2018/1/3. </remarks>
	///
	/// <param name="driver">	数据域. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool IsUseNetwork();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	恢复默认配置文件到config目录下. </summary>
	///
	/// <remarks>	TSMF Team, 2015/1/13. </remarks>
	///
	/// <param name="configFileName">	配置文件名.比如“XSIMEngine.config”,如果为空则恢复系统所有的配置文件 </param>
	///
	/// <returns>	恢复成功返回真，否则返回假. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RestoreDefaultConfiguration(const TSChar* configFileName = TS_TEXT(""));

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	清理App. </summary>
	///
	/// <remarks>	XSIM Team, 2018/8/8. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void Cleanup();

	HRISchedulerPtr GetScheduler() const;

	template <typename CallableType>
	void PostMainThreadTask(const CallableType& callableType, int delayMillisecs = 0)
	{
		ScheduleMainThreadTask(boost::make_shared<HRCallableObject<CallableType>>(callableType), delayMillisecs);
	}

	void ScheduleMainThreadTask(const HRICallablePtr& callable, int delayMillisecs = 0);

	const QString& GetWorkingDirectory() const;
	void SetWorkingDirectory(const QString&);

	typedef boost::function<void (void)> HRExitProcess;
	void RegisterExitProcess(const HRExitProcess& process);

	HRILoggerPtr CreateLogger(const TSString& loggerName);

	HRILoggerPtr GetDefaultLogger();

	HRDocumentPtr GetDocument();

	HRIModulePtr GetModule(int metaTypeId);
	
	TSGenericHandleTable<HRIModulePtr>& GetModules();

	void LoadModuleLibrary(HMODULE& module, const TSString& pluginName, const TSString& dependencePath);

	template<typename T>
	FORCEINLINE T GetModuleT()
	{
		TSGenericHandleTable<HRIModulePtr>::Iterator iter = GetModules().Begin();
		for (; iter; ++iter)
		{
			HRIModulePtr module = *iter;

			if (!module)
				continue;

			if (T object = TS_CAST(module, T))
			{
				return object;
			}
		}

		return 0;
	}

	template<typename T>
	FORCEINLINE boost::shared_ptr<T> QueryModuleT()
	{
		return boost::static_pointer_cast<T>(GetModule(T::GetMetaTypeIdStatic()));
	}

private:
	HRAppManager();
	~HRAppManager();

	void PreloadModules();

private:
	struct HRAppManagerPrivate;
	HRAppManagerPrivate *_p;
};

#define HRGetAppManager()				HRAppManager::Instance()
#define HRGetDriver()					HRGetAppManager()->GetDriver()
#define HRGetWorkingDirectory()			HRGetAppManager()->GetWorkingDirectory()
#define HRGetCurrentWorker()			HRGetAppManager()->GetScheduler()->GetCurrentWorker()
#define HRGetMainThreadWorker()			HRGetAppManager()->GetScheduler()->GetMainThreadWorker()
#define HRGetAsyncWorker()				HRGetAppManager()->GetScheduler()->GetAsyncWorker()
#define HRGetEventWorker()				HRGetAppManager()->GetScheduler()->GetEventWorker()

template<typename T>
boost::shared_ptr<T> QueryModuleT()
{
	return HRGetAppManager()->QueryModuleT<T>();
}

#endif // HRAPPMANAGER_H
