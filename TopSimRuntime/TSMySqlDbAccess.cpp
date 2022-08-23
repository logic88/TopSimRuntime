#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#include "TSBasicTypeSupport.h"

#define SUPNAME         TOPIC_Base
#define NAME            TOPIC_Basic_Sdo
#define DESC            TS_TEXT(CN_TOPSIMDATAINTERFACE_TSBASICTYPESUPPORT_CPP_7)

#define SUP_NAMESPACE    
#define NAMESPACE       TSBasicSdo
#define DATA_TYPE       NAMESPACE::DataType
#define SUPPORT_TYPE    TypeSupport 
#define EXPORT_MACRO    TOPSIMDATAINTERFACE_DECL
#define BEGIN_NAMESPACE namespace TSBasicSdo{
#define END_NAMESPACE   }

#define HASHTYPEVERSIONMACRO 01010101011
#define SELFVERSONMACRO     HASHTYPEVERSIONMACRO

TYPESUPPORT_IMPL;
TOPICHELPER_IMPL;

BEGIN_EXTERNAL_METADATA(DATA_TYPE)
	REG_PROP_FIELD2(Handle);
END_EXTERNAL_METADATA()

#undef SUPNAME 
#undef NAME
#undef DESC
#undef SUP_NAMESPACE
#undef NAMESPACE
#undef DATA_TYPE
#undef SUPPORT_TYPE
#undef EXPORT_MACRO   
#undef BEGIN_NAMESPACE 
#undef END_NAMESPACE
#undef SUPERVERSIONMACRO
#undef PARENTTYPEVERSIONMACRO

