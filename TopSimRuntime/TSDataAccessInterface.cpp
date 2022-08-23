#include "stdafx.h"
#include "HRUICommon.h"
#include "HREventDecl.h"
#include "HRISceneCreator.h"
#include "HRIIconManager.h"
#include "HRIPluginManager.h"
#include "HRIPropertyWidgetFactory.h"


BEGIN_METADATA(HRIIconManager)
	REG_BASE(HRIModule);
END_METADATA()

BEGIN_METADATA(HRIPluginConstructor)
END_METADATA()

BEGIN_METADATA(HRIPluginManager)
	REG_BASE(HRIModule);
END_METADATA()

BEGIN_METADATA(HRIPropertyWidgetFactory)
	REG_BASE(HRIModule);
END_METADATA()

 BEGIN_METADATA(HRISceneCreator)
 	REG_BASE(HRIModule);
 END_METADATA()

BEGIN_METADATA(HRDomainWrapper)
END_METADATA();

