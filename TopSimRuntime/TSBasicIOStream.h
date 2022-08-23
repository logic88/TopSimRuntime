#ifndef HRIWIDGETPLUGIN_H
#define HRIWIDGETPLUGIN_H

#include "HRIPlugin.h"

struct HRIWidgetPluginParam : public HRIPluginParam
{
	/// <summary> 域  </summary>
	TSDomainPtr					Domain;
};

STRUCT_PTR_DECLARE(HRIWidgetPluginParam);

struct HRIWidgetPluginPrivate;

class HRUICOMMON_EXPORT HRIWidgetPlugin : public HRIPlugin
{
public:
	HRIWidgetPlugin();
	 virtual ~HRIWidgetPlugin();

	virtual void Initialize(HRIWidgetPluginParamPtr Param);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	清理插件，创建销毁时调用，一般清理Initialize中初始化的资源，比如创建的Page/Group/Action/Widget/Dock、打开的文件获取其它需要清理的资源 </summary>
	///
	/// <remarks>	龙成亮, 2015/1/4. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void Cleanup();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取插件参数，与Initialize传入的为统一参数智能指针. </summary>
	///
	/// <remarks>	龙成亮, 2015/1/4. </remarks>
	///
	/// <returns>	The plugin parameter. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	HRIWidgetPluginParamPtr GetPluginParam();

private:
	HRIWidgetPluginPrivate * _p;
};

CLASS_PTR_DECLARE(HRIWidgetPlugin);
#endif // HRIWIDGETPLUGIN_H
