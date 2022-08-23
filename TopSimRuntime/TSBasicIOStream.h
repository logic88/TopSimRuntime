#ifndef HRIWIDGETPLUGIN_H
#define HRIWIDGETPLUGIN_H

#include "HRIPlugin.h"

struct HRIWidgetPluginParam : public HRIPluginParam
{
	/// <summary> ��  </summary>
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
	/// <summary>	����������������ʱ���ã�һ������Initialize�г�ʼ������Դ�����紴����Page/Group/Action/Widget/Dock���򿪵��ļ���ȡ������Ҫ�������Դ </summary>
	///
	/// <remarks>	������, 2015/1/4. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void Cleanup();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ�����������Initialize�����Ϊͳһ��������ָ��. </summary>
	///
	/// <remarks>	������, 2015/1/4. </remarks>
	///
	/// <returns>	The plugin parameter. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	HRIWidgetPluginParamPtr GetPluginParam();

private:
	HRIWidgetPluginPrivate * _p;
};

CLASS_PTR_DECLARE(HRIWidgetPlugin);
#endif // HRIWIDGETPLUGIN_H
