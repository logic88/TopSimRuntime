#include "stdafx.h"

#include <QGroupBox>
#include <QFormLayout>

#include "HRComplexPropertyPanel.h"
#include "HRIPropertyWidgetFactory.h"
#include "HRPropertyWidget.h"
#include "HRUIUtil.h"
#include "HRAppManager.h"

#include <HRUtil/HRSmartAssert.h>

HRComplexPropertyPanel::HRComplexPropertyPanel(XPropertyCreateParamsPtr CreateParams)
	: HRPropertyWidgetGroup(CreateParams)
{
	_Variant = CreateParams->Variant;

	if (_Variant.IsNull() && 
		(_Variant.GetTypeFlag() == TSVariant::Is_SmartPointer ||
		_Variant.GetTypeFlag() == TSVariant::Is_Pointer))
	{
		_Variant = HRUIUtil::CreateObjectInstance(_Variant.GetType(),  _Variant.GetTypeFlag());
	}
	
	if (_Variant.IsNull())
	{
		return;
	}

	BindVariant(CreateParams->Variant);

	std::vector<int> parentTypes;
	TSMetaType::GetAllParentTypes(_Variant.GetType(), parentTypes);
	std::reverse(parentTypes.begin(), parentTypes.end());
	parentTypes.push_back(_Variant.GetType());

	bool readOnly = false;
	
	for (std::size_t i = 0; i < parentTypes.size(); i++)
	{	
		int parentType = parentTypes[i];

#ifdef _DEBUG
		QString thisTypeName = TSMetaType::GetTypeName(parentType);
#endif

		const MetaPropertyCollection &properties = TSMetaType::GetProperties(parentType, false);

		for (std::size_t j = 0;j < properties.size();j++)
		{
			const TSMetaProperty *prop = properties[j];				
#ifdef NOTEST
			if ((TESTBIT(prop->Flag,TSMetaProperty::NoDesignable)))
			{
				continue;
			}

			readOnly = 
				(TESTBIT(prop->Flag, TSMetaProperty::NoDesignWrite) ||
				TESTBIT(prop->PropertyOperator->GetFlag(), PROP_FLAG_READONLY));
#endif
			XPropertyConfigExPtr config = HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->GetPropConfig(_Variant.GetType(), 
				TSString2QString(prop->Name));

			if (config)
			{
				if (!TESTBIT(config->ShowModes, CreateParams->Mode) )
				{
					continue;
				}
			}

			if (!CreateParams->Group.isEmpty() && (!config || config->Group != CreateParams->Group))
			{
				continue;
			}


			TSVariant var = prop->PropertyOperator->GetValue(_Variant.GetDataPtr());
			if (!var.IsValid())
			{
				var = prop->PropertyOperator->GetDummyVariant();
			}

			XPropertyCreateParamsPtr subMakeParam = boost::make_shared<XPropertyCreateParams>();
			*subMakeParam = *CreateParams;
			subMakeParam->Variant = var;
			subMakeParam->Property = prop;
			subMakeParam->Config = config;
			subMakeParam->UIContext = CreateParams->UIContext;
			subMakeParam->OwnerCreateParams = CreateParams;
			subMakeParam->Group = "";

			if (HRPropertyWidget *widget = HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->CreatePropertyWidget(subMakeParam))
			{
				if(readOnly)
				{
					widget->setEnabled(false);
				}

				HRPropertyWidgetGroup::PropertyWidgetInfo tempWidgetInfo;
				tempWidgetInfo.order = HRUIUtil::GetProperOrder(subMakeParam->Config, ((int)GetWidgets().size() + 1 ) * 1000);
				tempWidgetInfo.label = HRUIUtil::GetProperLabelText(subMakeParam->Config, subMakeParam->Property);
				tempWidgetInfo.widget = widget;				
				AddWidget(tempWidgetInfo);
			}
		}
	}

	std::vector<XPropertyConfigExPtr> pseudoProps = 
		HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->GetPseudoPropConfigs(_Variant.GetType());
	for (std::vector<XPropertyConfigExPtr>::const_iterator it = pseudoProps.begin(); it != pseudoProps.end(); ++it)
	{
		XPropertyConfigExPtr config = *it;
		if (!CreateParams->Group.isEmpty() && (!config || config->Group != CreateParams->Group))
		{
			continue;
		}

		XPropertyCreateParamsPtr makeParam = 
			boost::make_shared<XPropertyCreateParams>(TSVariant(), 
			CreateParams->Mode, reinterpret_cast<TSMetaProperty*>(0), CreateParams->OwnerCreateParams);
		XPropertyConfigExPtr exconfig = *it;
		makeParam->Config = config;
		if (HRPropertyWidget* widget = 
			HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->CreatePseudoPropertyWidget(exconfig, makeParam))
		{
			HRPropertyWidgetGroup::PropertyWidgetInfo tempWidgetInfo;
			tempWidgetInfo.order = HRUIUtil::GetProperOrder(makeParam->Config, ((int)GetWidgets().size() + 1 ) * 1000);
			tempWidgetInfo.label = HRUIUtil::GetProperLabelText(makeParam->Config, makeParam->Property);
			tempWidgetInfo.widget = widget;				
			AddWidget(tempWidgetInfo);
		}
	}

	setLayout(CreateLayout());
}

HRComplexPropertyPanel::~HRComplexPropertyPanel()
{

}

void HRComplexPropertyPanel::Validate()
{
	GetCreateParams()->Variant = _Variant;

	HRPropertyWidgetGroup::Validate();
}
