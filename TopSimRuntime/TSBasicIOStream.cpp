#include "stdafx.h"
#include <HRUtil/HREventBus.h>
#include "HRIWidgetPlugin.h"

struct HRIWidgetPluginPrivate
{
	HRIWidgetPluginParamPtr _PluginParam;
};

HRIWidgetPlugin::HRIWidgetPlugin()
	: _p(new HRIWidgetPluginPrivate)
{

}

HRIWidgetPlugin::~HRIWidgetPlugin()
{
	delete _p;
}

void HRIWidgetPlugin::Initialize( HRIWidgetPluginParamPtr Param )
{
	_p->_PluginParam = Param;
}

void HRIWidgetPlugin::Cleanup()
{
	HREventBus::Instance()->UnSubscribeByObject(this);

	WaitAndForceDestroy();
}

HRIWidgetPluginParamPtr HRIWidgetPlugin::GetPluginParam()
{
	return _p->_PluginParam;
}