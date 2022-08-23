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
	/// <summary>	��ʼ��Ӧ�ó���ı�������. </summary>
	///
	/// <remarks>	XSIM Team, 2018/8/8. </remarks>
	///
	/// <param name="app">	QtӦ�ó���. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void InitApplicationTextCodec(QApplication* app);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ʼ��Ӧ�ó���. </summary>
	///
	/// <remarks>	XSIM Team, 2018/8/8. </remarks>
	///
	/// <param name="app">			QtӦ�ó���. </param>
	/// <param name="singleApp">	App�Ƿ�������. </param>
	///
	/// <returns>	��ʼ���ɹ�����true�����򷵻�false. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool InitApplication(QApplication* app, bool singleApp = false);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡWGS84�������. </summary>
	///
	/// <remarks>	XSIM Team, 2018/8/8. </remarks>
	///
	/// <returns>	WGS84�Ƿ���. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UseWGS84();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ�����࣬������ȫ��ֻ��һ�� </summary>
	///
	/// <remarks>	������, 2018/8/8. </remarks>
	///
	/// <returns>	����. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	HRIAppDriverPtr GetDriver();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���������� </summary>
	///
	/// <remarks>	������, 2018/8/8. </remarks>
	///
	/// <param name="driver"> ����. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetDriver(HRIAppDriverPtr driver);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡӦ�ó���Ǽ�. </summary>
	///
	/// <remarks>	������, 2018/8/8. </remarks>
	///
	/// <returns>	Ӧ�ó���Ǽ�. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	HRIAppSkeleton* GetAppSkeleton();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	����Ӧ�ó���Ǽ�. </summary>
	///
	/// <remarks>	������, 2018/8/8. </remarks>
	///
	/// <param name="skeleton">	Ӧ�ó���Ǽ�. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetAppSkeleton(HRIAppSkeleton* skeleton);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ��ǰ������ģʽ </summary>
	///
	/// <remarks>	������, 2018/8/8. </remarks>
	///
	/// <returns>	����ģʽ. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	RunMode GetRunMode() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���õ�ǰ������ģʽ </summary>
	///
	/// <remarks>	������, 2018/8/8. </remarks>
	///
	/// <param name="mode">	����ģʽ. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetRunMode( RunMode mode );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	����ģ��. </summary>
	///
	/// <remarks>	������, 2018/8/8. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadModules();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ��ǰ���������ĵ�������ָ��. </summary>
	///
	/// <remarks>	����, 2018/1/3. </remarks>
	///
	/// <returns>	������. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSDomainPtr GetContextDomain();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���õ�ǰ���������ĵ�������ָ��. </summary>
	///
	/// <remarks>	����, 2018/1/3. </remarks>
	///
	/// <param name="driver">	������. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetContextDomain(TSDomainPtr Domain);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ�Ƿ���Эͬģʽ. </summary>
	///
	/// <remarks>	����, 2018/1/3. </remarks>
	///
	/// <param name="driver">	������. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool IsUseNetwork();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�ָ�Ĭ�������ļ���configĿ¼��. </summary>
	///
	/// <remarks>	TSMF Team, 2015/1/13. </remarks>
	///
	/// <param name="configFileName">	�����ļ���.���硰XSIMEngine.config��,���Ϊ����ָ�ϵͳ���е������ļ� </param>
	///
	/// <returns>	�ָ��ɹ������棬���򷵻ؼ�. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RestoreDefaultConfiguration(const TSChar* configFileName = TS_TEXT(""));

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	����App. </summary>
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
