#include "stdafx.h"
#include "HRPropertyWidget.h"
#include <TopSimDataInterface/TSTypeSupport.h>
#include <TopSimDataInterface/TSTopicTypeManager.h>
#include "HRAppManager.h"
#include "HRIPropertyWidgetFactory.h"

HRValueChangedCtx::HRValueChangedCtx()
	: Checkout(true)
	, Flag(0)
{

}

BEGIN_METADATA(HRValueChangedCtx)
REG_FUNC(Enable);
REG_FUNC(Disable);
REG_PROP_FIELD(Checkout);
REG_PROP_FIELD(Tips);
REG_PROP_FIELD(Flag);
END_METADATA()

BEGIN_METADATA(TSIPropertyWidgetConstructor)
END_METADATA()

XPropertyConfig::XPropertyConfig() 
	: DefaultWidgetConstructor(true)
	, Order(-1)
	, LabelText("NULL")
	, Nullable(false)
	, Decimals(4)
	, DefaultExpand(true)
	, IsNeed(false)
	, GroupCheckable(false)
{

}

XPropertyConfig::~XPropertyConfig()
{
}

XPropertyCreateParams::XPropertyCreateParams( const TSVariant &Var,XPropertyConfig::UseMode Mode,const TSMetaProperty * Property,XPropertyCreateParamsPtr  OwnerCreateParams, TSDomainPtr UIContext  )
	:Variant(Var),UIContext(UIContext),Property(Property),OwnerCreateParams(OwnerCreateParams),Mode(Mode)
{

}

XPropertyCreateParams::XPropertyCreateParams( TSTOPICHANDLE TopicHandle, XPropertyConfig::UseMode Mode, TSDomainPtr UIContext )
	:Property(NULL),Mode(Mode)
{
	if(TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(TopicHandle))
	{
		this->VariantTopicHandle = TopicHandle;
		this->Variant = TSVariant(Support->GetTypeName(),TSVariant::Is_SmartPointer);
		this->UIContext = UIContext;
	}
}
		
XPropertyCreateParams::XPropertyCreateParams()
	:Property(NULL)
	,Mode(XPropertyConfig::ComponentDesign)
{

}

XPropertyCreateParams::XPropertyCreateParams( const TSVariant &Var, const TSString & KeyName, XPropertyConfig::UseMode Mode, XPropertyCreateParamsPtr OwnerCreateParams /*= NULL*/ )
	:Property(NULL),Variant(Var),OwnerCreateParams(OwnerCreateParams),Mode(Mode),BBKeyName(KeyName)
{
	
}

XPropertyCreateParamsPtr MakeCreateParams( const TSVariant & Var,XPropertyConfig::UseMode Mode,const TSMetaProperty * Property,XPropertyCreateParamsPtr  OwnerCreateParams, TSDomainPtr UIContext )
{
	XPropertyCreateParamsPtr ret(new XPropertyCreateParams(
		Var,Mode,Property,OwnerCreateParams,UIContext));

	return ret;
}

XPropertyCreateParamsPtr MakeCreateParams(TSTOPICHANDLE TopicHandle, XPropertyConfig::UseMode Mode,TSDomainPtr UIContext)
{
	XPropertyCreateParamsPtr ret(new XPropertyCreateParams(TopicHandle, Mode, UIContext));
	return ret;
}

XPropertyCreateParamsPtr MakeCreateParams( TSTOPICHANDLE TopicHandle, VoidPtr Data, XPropertyConfig::UseMode Mode, TSDomainPtr UIContext )
{
	if(TSTypeSupportPtr Support = TSTopicTypeManager::Instance()->GetTypeSupportByTopic(TopicHandle))
	{
		TSVariant Var(TSMetaType::GetType(Support->GetTypeName()),  &Data, TSVariant::Is_SmartPointer);
		XPropertyCreateParamsPtr ret(new XPropertyCreateParams(Var,Mode,NULL,NULL,UIContext));
		ret->VariantTopicHandle = TopicHandle;

		return ret;
	}

	DEF_LOG_ERROR(toAsciiData("未发现主题Id[%1]的TypeSupport"))<<TopicHandle;
	return XPropertyCreateParamsPtr();
}

 XPropertyCreateParamsPtr MakeCreateParams( const TSVariant &Var
	, const TSString & BBKeyName
	, XPropertyConfig::UseMode Mode
	, XPropertyCreateParamsPtr OwnerCreateParams /*= NULL*/ )
{
	XPropertyCreateParamsPtr ret(new XPropertyCreateParams(
		Var, BBKeyName, Mode, OwnerCreateParams));

	return ret;
}

HRPropertyWidget::HRPropertyWidget(XPropertyCreateParamsPtr CreateParams)
	: _CreateParams(CreateParams)
{
}

HRPropertyWidget::~HRPropertyWidget()
{
}

XPropertyCreateParamsPtr HRPropertyWidget::GetCreateParams() const
{
	return _CreateParams;
}

void HRPropertyWidget::EmitValueChangedSignal( const TSVariant& val )
{
	emit ValueChangedSignal(this, val);
}

