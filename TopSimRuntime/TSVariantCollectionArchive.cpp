#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#include "TSTypeSupport.h"
#include "TSTypeSupport_p.h"

#if defined(XSIM3_2) || defined(XSIM3_3)
#else
#include "TopSimRuntime/TSLogsUtil.h"
#endif

TSTypeSupport::TSTypeSupport(void)
{
}

TSTypeSupport::~TSTypeSupport(void)
{
}

void * TSTypeSupport::ConstructDataObject()
{
	return ConstructDataObjectStatic();
}

VoidPtr TSTypeSupport::ConstructSmartDataObject()
{
	return ConstructSmartDataObjectStatic();
}

void TSTypeSupport::DestroyDataObject( TSInterObject * obj )
{
	UNUSED(obj);
}

bool TSTypeSupport::Serialize( const TSInterObject * Data,TSISerializer& Ser )
{
	UNUSED(Data);
	UNUSED(Ser);
	return true;
}

bool TSTypeSupport::Deserialize( TSInterObject * Data,TSISerializer& Ser )
{
	UNUSED(Data);
	UNUSED(Ser);
	return true;
}

bool TSTypeSupport::IsTypeMatched( const TSInterObject * Data )
{
	return IsTypeMatchedStatic(Data);
}

TSTypeSupport::TypeFlag TSTypeSupport::GetTypeFlag()
{
	return GetTypeFlagStatic();
}

const TSChar * TSTypeSupport::GetTypeName()
{
	return GetTypeNameStatic();
}

TSTypeSupport::TypeFlag TSTypeSupport::GetTypeFlagStatic()
{
	return TSTypeSupport::TF_InvalidFlag;
}

const TSChar * TSTypeSupport::GetTypeNameStatic()
{
	return TS_TEXT("");
}

void * TSTypeSupport::ConstructDataObjectStatic()
{
	return 0;
}

VoidPtr TSTypeSupport::ConstructSmartDataObjectStatic()
{
	return VoidPtr();
}

void TSTypeSupport::DestroyDataObjectStatic( TSInterObject * obj )
{
	UNUSED(obj);
}

bool TSTypeSupport::SerializeStatic( const TSInterObject * Data,TSISerializer& Ser )
{
	UNUSED(Data);
	UNUSED(Ser);

	return true;
}

bool TSTypeSupport::DeserializeStatic( TSInterObject * Data,TSISerializer& Ser )
{
	UNUSED(Data);
	UNUSED(Ser);

	return true;
}

bool TSTypeSupport::IsTypeMatchedStatic( const TSInterObject * Data )
{
	return Data;
}

bool TSTypeSupport::HasFindSet( TopSimDataInterface::FindSetFlag Key )
{
	if(Key == TopSimDataInterface::kNone)
	{
		return true;
	}

	return false;
}

UINT32 TSTypeSupport::GetTypeVersionHashCode()
{
	return GetTypeVersionHashCodeStatic();
}

UINT32 TSTypeSupport::GetTypeVersionHashCodeStatic()
{
	return DEFAULTVERSIONCODE;
}

ExceptionCallBack G_ExceptionCallBack;
void SetExceptionCallBack( ExceptionCallBack CallBack )
{
	if (CallBack)
	{
		G_ExceptionCallBack = CallBack;
	}
}

void ThrowHashVersionTypeException(const TSChar* TypeName,UINT32 SelfVersion,UINT32 OtherVersion )
{
	TSString Exc = TSString("[") + TypeName + TSString("] ") + TSString(CN_TOPSIMDATAINTERFACE_TSTYPESUPPORT_CPP_136)
		+ TSMetaType::GetTypeModuleFileName(TSMetaType::GetType(TypeName)) + TSString(CN_TOPSIMDATAINTERFACE_TSTYPESUPPORT_CPP_137);

	if (G_ExceptionCallBack)
	{
		G_ExceptionCallBack(Exc,TypeName,SelfVersion,OtherVersion);
	}
	else
	{
		DEF_LOG_ERROR(Exc);
	}
}

void ThrowHashVersionTypeExceptionPSNoMatch(const TSChar* TypeName, UINT32 SelfVersion, UINT32 OtherVersion)
{
	TSString Exc = TSString("[") + TypeName + TSString("] ") + TSString(CN_TOPSIMDATAINTERFACE_TSTYPESUPPORT_CPP_136)
		+ TSMetaType::GetTypeModuleFileName(TSMetaType::GetType(TypeName)) + TSString(CN_TOPSIMDATAINTERFACE_TSTYPESUPPORT_CPP_155);

	if (G_ExceptionCallBack)
	{
		G_ExceptionCallBack(Exc, TypeName, SelfVersion, OtherVersion);
	}
	else
	{
		DEF_LOG_ERROR(Exc);
	}
}
