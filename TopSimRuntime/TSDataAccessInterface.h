#ifndef HRUICOMMON_H__
#define HRUICOMMON_H__

#include <config/config.h>

#ifndef Q_MOC_RUN

#include <TopSimRuntime/TopSimRuntime.h>
#include <TopSimRuntime/pugixml.hpp>
#include <TopSimDataInterface/TSDomain.h>

#endif



#ifdef HRUICOMMON_LIB
#	define HRUICOMMON_EXPORT Q_DECL_EXPORT
#else
#	define HRUICOMMON_EXPORT HR_SYMBOL_IMPORT
#	ifdef HR_ENABLE_HRUICOMMON_AUTO_LINK
#		define __LIB_NAME__ "HRUICommon" HR_STATIC_SUFFIX
#		include<TopSimRuntime/TSAutoLink.h>
#	endif
#endif

#ifndef Q_MOC_RUN
#include <boost/smart_ptr.hpp>
#endif//Q_MOC_RUN

#include <HRUtil/HRSmartAssert.h>

#include "HRIAppDriver.h"

#define DOCUMENT_PREFIX           "Ó¦ÓÃÅäÖÃ/"
#define DEF_APP_CONF_NAME         "GlobalAppConfigure.config"

#define HR_SHOW_MESSAGE(Message) HREventBus::Instance()->PostEventBlocking(SE_UI_SHOW_MESSAGE, Message);

STRUCT_PTR_DECLARE(XPropertyConfig);
STRUCT_PTR_DECLARE(XPropertyCreateParams);
CLASS_PTR_DECLARE(HRIModule);
CLASS_PTR_DECLARE(HRIAppDriver);
CLASS_PTR_DECLARE(TSIPropertyWidgetConstructor);
typedef boost::shared_ptr<pugi::xml_document> HRDocumentPtr;
typedef boost::shared_ptr<struct XPropertyConfigEx> XPropertyConfigExPtr;

#endif // HRUICOMMON_H__
