#include "stdafx.h"
#include "HRPropertyWidgetGroup.h"

#include <QFontMetrics>
#include <QTextEdit>
#include <QGroupBox>
#include <QTextCodec>
#include <QApplication>
#include <QFormLayout>
#include <QLabel>

#include <HRUtil/HRPerfCollector.h>
#include <HRUtil/HRSmartAssert.h>
#include <HRControls/HRMessageBox.h>

#include "HRAppManager.h"
#include "HRIPropertyWidgetFactory.h"
#include "HRPropertyWidget.h"
#include "HRUIUtil.h"
#include "HRAppCenter.h"
#include "HRCheckableGroupBox.h"

struct HRPropertyWidgetGroup::HRPropertyWidgetGroupPrivate
{
	std::vector<PropertyWidgetInfo> _Widgets;
	QFormLayout *_Layout;
	TSVariant _DupObject;
	QWidget * _MainWidget;
	HRCheckableGroupBox * _GroupBox;

	HRPropertyWidgetGroup::PropertyWidgetGroup _UnGroupWidgets;
	std::map<QString, HRPropertyWidgetGroup::PropertyWidgetGroup *> _Groups;
};

HRPropertyWidgetGroup::HRPropertyWidgetGroup(XPropertyCreateParamsPtr CreateParams)
	: HRPropertyWidget(CreateParams), _p(new HRPropertyWidgetGroupPrivate())
{
	_p->_MainWidget = new QWidget();
	_p->_MainWidget->setObjectName("group_MainWidget");

	if (CreateParams->Property)
	{
		QString GroupBoxText = TSString2QString(CreateParams->Property->Name);

		if (CreateParams->Config && CreateParams->Config->LabelText != "NULL" && CreateParams->Config->LabelText != "")
		{
			GroupBoxText = CreateParams->Config->LabelText;
		}

		_p->_GroupBox = new HRCheckableGroupBox(this);
		_p->_GroupBox->setTitle(GroupBoxText);

		if (CreateParams->Config)
		{
			_p->_GroupBox->setChecked(CreateParams->Config->DefaultExpand);
			_p->_GroupBox->setCheckable(CreateParams->Config->GroupCheckable);
		}

		QVBoxLayout * groupLayout = new QVBoxLayout();
		_p->_GroupBox->setLayout(groupLayout);
		groupLayout->addWidget(_p->_MainWidget);

		QVBoxLayout * rootLayout = new QVBoxLayout();
		rootLayout->setContentsMargins(0, 0, 0, 0);
		this->setLayout(rootLayout);
		rootLayout->addWidget(_p->_GroupBox);
	}
	else
	{
		QVBoxLayout * vlayout = new QVBoxLayout();
		vlayout->setContentsMargins(0, 0, 0, 0);
		this->setLayout(vlayout);
		vlayout->addWidget(_p->_MainWidget);
	}
}

void BuildPropWidgetList(
	std::vector<HRPropertyWidgetGroup::PropertyWidgetInfo> & widgetInfos,
	XPropertyCreateParamsPtr createParams, TSVariant &val)
{
	XPropertyCreateParamsPtr makeParam = boost::make_shared<XPropertyCreateParams>();;
	*makeParam = *createParams;
	makeParam->Variant = val;

	if (HRPropertyWidget *widget = HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->CreatePropertyWidget(
		makeParam))
	{
		HRPropertyWidgetGroup::PropertyWidgetInfo tempWidgetInfo;
		tempWidgetInfo.order = HRUIUtil::GetProperOrder(createParams->Config, (widgetInfos.size() + 1) * 1000);
		tempWidgetInfo.label = HRUIUtil::GetProperLabelText(makeParam->Config, makeParam->Property);
		tempWidgetInfo.widget = widget;
		widgetInfos.push_back(tempWidgetInfo);
	}

	std::vector<XPropertyConfigExPtr> pseudoPropConfigs =
		HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->GetPseudoPropConfigs(val.GetType());
	for (std::vector<XPropertyConfigExPtr>::iterator it = pseudoPropConfigs.begin();
		it != pseudoPropConfigs.end();
		++it)
	{
		XPropertyCreateParamsPtr makeParam =
			boost::make_shared<XPropertyCreateParams>(TSVariant(),
				createParams->Mode, reinterpret_cast<TSMetaProperty*>(0), createParams->OwnerCreateParams);
		XPropertyConfigExPtr exconfig = *it;
		makeParam->Config = *it;
		if (HRPropertyWidget* widget =
			HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->CreatePseudoPropertyWidget(exconfig, makeParam))
		{
			HRPropertyWidgetGroup::PropertyWidgetInfo tempWidgetInfo;
			tempWidgetInfo.order = HRUIUtil::GetProperOrder(createParams->Config, (widgetInfos.size() + 1) * 1000);
			tempWidgetInfo.label = exconfig->LabelText.isEmpty() ? exconfig->PropertyName : exconfig->LabelText;
			tempWidgetInfo.widget = widget;
			widgetInfos.push_back(tempWidgetInfo);
		}
	}
}

void HRPropertyWidgetGroup::BuildWidgets(const XPropertyCreateParamsPtr &createParam, TSVariant &val)
{
	SCOPED_COLLECTOR(TS_TEXT("HRPropertyWidgetGroup->BuildWidgets"));

	BuildPropWidgetList(_p->_Widgets, createParam, val);
}

void HRPropertyWidgetGroup::BuildWidgets(const XPropertyCreateParamsPtr &createParam)
{
	BuildWidgets(createParam, createParam->Variant);
}

void HRPropertyWidgetGroup::Validate()
{
	for (std::size_t i = 0; i < _p->_Widgets.size(); ++i)
	{
		_p->_Widgets[i].widget->Validate();

		if (_p->_Widgets[i].widget->GetCreateParams()->Property == GetCreateParams()->Property)
		{
			continue;
		}
		SMART_ASSERT(_p->_Widgets[i].widget->GetCreateParams()->OwnerCreateParams);
		SMART_ASSERT(_p->_Widgets[i].widget->GetCreateParams()->OwnerCreateParams->Variant.GetDataPtr());

		_p->_Widgets[i].widget->GetCreateParams()->Property->PropertyOperator->SetValue(
			_p->_Widgets[i].widget->GetCreateParams()->OwnerCreateParams->Variant.GetDataPtr(),
			_p->_Widgets[i].widget->GetCreateParams()->Variant);
	}
}

const std::vector<HRPropertyWidgetGroup::PropertyWidgetInfo> & HRPropertyWidgetGroup::GetWidgets() const
{
	return _p->_Widgets;
}

bool HRPropertyWidgetGroup::DefaultShowLable() const
{
	return false;
}

HRPropertyWidgetGroup::~HRPropertyWidgetGroup()
{
	std::map<QString, HRPropertyWidgetGroup::PropertyWidgetGroup *>::iterator it = _p->_Groups.begin();
	while (it != _p->_Groups.end())
	{
		delete it->second;
		++it;
	}

	delete _p;
}

void HRPropertyWidgetGroup::OnValueChangedSlot(const HRPropertyWidget* widget, const TSVariant& val)
{
	if (_p->_DupObject.IsValid())
	{
		widget->GetCreateParams()->Property->PropertyOperator->SetValue(_p->_DupObject.GetDataPtr(), val);
		UpdateState();
	}
}

void HRPropertyWidgetGroup::UpdateState()
{
	SCOPED_COLLECTOR(TS_TEXT("HRPropertyWidgetGroup->UpdatePropertyState"));

	HRValueChangedCtxPtr Ctx = boost::make_shared<HRValueChangedCtx>();

	//执行脚本

	std::map<QString, HRPropertyWidgetGroup::PropertyWidgetGroup *>::iterator it = _p->_Groups.begin();

	while (it != _p->_Groups.end())
	{
		PropertyWidgetGroup * Group = it->second;

		for (size_t i = 0; i < Group->WidgetInfos.size(); ++i)
		{
			if (XPropertyConfigExPtr Config = Group->WidgetInfos[i].first)
			{
				if (TSScriptInterpreter * Interpreter =
					HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->GetPropertyScriptInterpreter(Config))
				{
					TSString GroupName = QString2TSString(Group->WidgetInfos[i].first->Group);
					TSString PropertyName = QString2TSString(Group->WidgetInfos[i].first->PropertyName);
					TSString FunctionName = QString2TSString(Config->ScriptFunctionName);

					if (FunctionName.empty())
					{
						const MetaPropertyCollection & Props = TSMetaType::GetProperties(_p->_DupObject.GetType());

						for (MetaPropertyCollection::const_iterator it = Props.begin(); it != Props.end(); ++it)
						{
							if ((*it)->Name == PropertyName)
							{
								FunctionName = TSMetaType::GetTypeName((*it)->PropertyOperator->GetDummyVariant().GetType());
								boost::algorithm::replace_all(FunctionName, "::", "_");
								break;
							}
						}
					}

					try
					{
						if (!FunctionName.empty())
						{
							TSVariant Val = TSVariant::FromValue(Ctx);
                            TSVariant ReturnVal;
							//Load 直接传入参数调用接口，脚本执行未响应

							TSString ScriptFile = QString2TSString(Config->ScriptFile);

							Interpreter->Load(ScriptFile.c_str(), (ScriptFile + "~").c_str());

							Interpreter->RunFunction(FunctionName.c_str(), &ReturnVal, "TSVariant,TSString,TSString,TSVariant", &_p->_DupObject, &PropertyName, &GroupName, &Val);
						}
					}
					catch (...)
					{
						DEF_LOG_WARNING(toAsciiData("脚本[%1]执行接口[%2]异常...")) << QString2TSString(Config->ScriptFile) << FunctionName;
					}
				}
			}
		}

		++it;
	}

	for (size_t i = 0; i < _p->_UnGroupWidgets.WidgetInfos.size(); ++i)
	{
		if (XPropertyConfigExPtr Config = _p->_UnGroupWidgets.WidgetInfos[i].first)
		{
			if (TSScriptInterpreter * Interpreter =
				HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->GetPropertyScriptInterpreter(_p->_UnGroupWidgets.WidgetInfos[i].first))
			{
				TSString GroupName = QString2TSString(_p->_UnGroupWidgets.WidgetInfos[i].first->Group);
				TSString PropertyName = QString2TSString(_p->_UnGroupWidgets.WidgetInfos[i].first->PropertyName);
				TSString FunctionName = QString2TSString(Config->ScriptFunctionName);

				if (FunctionName.empty())
				{
					const MetaPropertyCollection & Props = TSMetaType::GetProperties(_p->_DupObject.GetType());

					for (MetaPropertyCollection::const_iterator it = Props.begin(); it != Props.end(); ++it)
					{
						if ((*it)->Name == PropertyName)
						{
							FunctionName = TSMetaType::GetTypeName((*it)->PropertyOperator->GetDummyVariant().GetType());
							boost::algorithm::replace_all(FunctionName, "::", "_");
							break;
						}
					}
				}

				try
				{
					if (!FunctionName.empty())
					{
                        TSVariant ReturnVal;
						TSVariant Val = TSVariant::FromValue(Ctx);

						TSString ScriptFile = QString2TSString(Config->ScriptFile);

						//Load 直接传入参数调用接口，脚本执行未响应
						Interpreter->Load(ScriptFile.c_str(), (ScriptFile + "~").c_str());

						Interpreter->RunFunction(FunctionName.c_str(), &ReturnVal, "TSVariant,TSString,TSString,TSVariant", &_p->_DupObject, &PropertyName, &GroupName, &Val);
					}
				}
				catch (...)
				{
					DEF_LOG_WARNING(toAsciiData("脚本[%1]执行接口[%2]异常...")) << QString2TSString(Config->ScriptFile) << FunctionName;
				}
			}
		}
	}

	//设置状态

	std::map<TSString, bool>::iterator propIt = Ctx->PropsMap.begin();

	while (propIt != Ctx->PropsMap.end())
	{
		for (size_t i = 0; i < _p->_Widgets.size(); ++i)
		{
			if (_p->_Widgets[i].widget->GetCreateParams()->Property->Name == propIt->first)
			{
				_p->_Widgets[i].widget->setEnabled(propIt->second);
				break;
			}
		}

		++propIt;
	}

	////发送信号，触发父窗口的状态更新

	emit ValueChangedSignal(this, _p->_DupObject);

	HRUIUtil::AdjustParentsLayout(this);
}

void HRPropertyWidgetGroup::ClearWidgets()
{
	for (std::vector<PropertyWidgetInfo>::iterator it = _p->_Widgets.begin();
		it != _p->_Widgets.end(); ++it)
	{
		delete (*it).widget;
	}
	_p->_Widgets.clear();
}

QFormLayout* HRPropertyWidgetGroup::WrapWidget(HRPropertyWidget *widget,
	const QString &label, const XPropertyConfigPtr &config, bool isBasicData, bool inGroupBox)
{
	QFormLayout *layout = new QFormLayout();
	layout->setMargin(0);

	QLabel *labelWidget = HRUIUtil::CreateLabelWidget(label, config);
	if (labelWidget)
	{
		if (isBasicData)
		{
			labelWidget->setObjectName("label_basicData");
		}
		layout->addRow(labelWidget, widget);
	}
	else
	{
		layout->addRow(widget);
	}
	return layout;
}

QFormLayout* HRPropertyWidgetGroup::WrapWidget(QLayout *layout,
	const QString &label, const XPropertyConfigPtr &config, bool isBasicData, bool inGroupBox)
{
	QFormLayout *wrappedLayout = new QFormLayout();
	wrappedLayout->setMargin(0);
	QLabel *labelWidget = HRUIUtil::CreateLabelWidget(label, config);
	if (labelWidget)
	{
		if (isBasicData)
		{
			labelWidget->setObjectName("label_basicData");
		}
		QWidget *widget = new QWidget;
		widget->setLayout(layout);
		wrappedLayout->addRow(labelWidget, widget);
	}
	else
	{
		wrappedLayout->addRow(layout);
	}
	return wrappedLayout;
}

QFormLayout * HRPropertyWidgetGroup::CreateLayout(bool showLabel)
{
	std::stable_sort(_p->_Widgets.begin(), _p->_Widgets.end(),
		[](const PropertyWidgetInfo &l, const PropertyWidgetInfo &r) -> bool
	{
		return l.order < r.order;
	});

	_p->_Layout = new QFormLayout(_p->_MainWidget);
	_p->_Layout->setMargin(0);
	_p->_Layout->setContentsMargins(0, 0, 0, 0);

	//分组

	for (std::size_t i = 0; i < _p->_Widgets.size(); i++)
	{
		if (_p->_Widgets[i].widget->GetCreateParams()->Property)
		{
			if (XPropertyCreateParamsPtr OwnerCreateParam = _p->_Widgets[i].widget->GetCreateParams()->OwnerCreateParams)
			{
				TSString s = _p->_Widgets[i].widget->GetCreateParams()->Property->Name;

				if (XPropertyConfigExPtr config = HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->GetPropConfig(OwnerCreateParam->Variant.GetType(),
					TSString2QString(_p->_Widgets[i].widget->GetCreateParams()->Property->Name)))
				{
					if (!config->Group.isEmpty())
					{
						PropertyWidgetGroup * group;

						std::map<QString, HRPropertyWidgetGroup::PropertyWidgetGroup *>::iterator it = _p->_Groups.find(config->Group);

						if (it == _p->_Groups.end())
						{
							group = new PropertyWidgetGroup;

							HRCheckableGroupBox * groupBox = new HRCheckableGroupBox(this);
							groupBox->setTitle(config->Group);
							QVBoxLayout * groupLayout = new QVBoxLayout();
							groupBox->setLayout(groupLayout);

							groupBox->setChecked(config->DefaultExpand);
							groupBox->setCheckable(config->GroupCheckable);

							group->GroupBox = groupBox;
							group->WidgetInfos.push_back(std::make_pair(config, _p->_Widgets[i]));

							_p->_Groups.insert(std::make_pair(config->Group, group));
						}
						else
						{
							group = it->second;
							group->WidgetInfos.push_back(std::make_pair(config, _p->_Widgets[i]));
						}
					}
					else
					{
						_p->_UnGroupWidgets.WidgetInfos.push_back(std::make_pair(config, _p->_Widgets[i]));
					}
				}
			}
		}
	}

	std::set<QWidget*> AddedWidgets;

	for (std::size_t i = 0; i < _p->_Widgets.size(); i++)
	{
		QLabel *label = NULL;
		if (showLabel && !_p->_Widgets[i].label.isEmpty() && _p->_Widgets[i].widget->DefaultShowLable())
		{
			label = HRUIUtil::CreateLabelWidget(
				_p->_Widgets[i].label, _p->_Widgets[i].widget->GetCreateParams()->Config);
		}

		QWidget * w = _p->_Widgets[i].widget;

		if (_p->_Widgets[i].widget->GetCreateParams()->Property)
		{
			if (XPropertyCreateParamsPtr OwnerCreateParam = _p->_Widgets[i].widget->GetCreateParams()->OwnerCreateParams)
			{
				if (XPropertyConfigExPtr config = HRAppManager::Instance()->QueryModuleT<HRIPropertyWidgetFactory>()->GetPropConfig(OwnerCreateParam->Variant.GetType(),
					TSString2QString(_p->_Widgets[i].widget->GetCreateParams()->Property->Name)))
				{
					std::map<QString, HRPropertyWidgetGroup::PropertyWidgetGroup *>::iterator it = _p->_Groups.find(config->Group);

					if (it != _p->_Groups.end())
					{
						w = it->second->GroupBox;

						QWidget * FormWidget = new QWidget();
						QFormLayout * Layout = new QFormLayout();
						Layout->setMargin(0);
						Layout->setContentsMargins(0, 0, 0, 0);
						FormWidget->setLayout(Layout);

						if (label)
						{
							Layout->addRow(label, _p->_Widgets[i].widget);
						}
						else
						{
							Layout->addRow(_p->_Widgets[i].widget);
						}

						it->second->GroupBox->layout()->addWidget(FormWidget);

						if (AddedWidgets.find(w)
							== AddedWidgets.end())
						{
							_p->_Layout->addRow(it->second->GroupBox);

							AddedWidgets.insert(w);
						}
					}
				}
			}
		};

		if (AddedWidgets.find(w)
			== AddedWidgets.end())
		{
			if (label)
			{
				_p->_Layout->addRow(label, _p->_Widgets[i].widget);
			}
			else
			{
				_p->_Layout->addRow(_p->_Widgets[i].widget);
			}

			AddedWidgets.insert(w);
		}
	}

	if (_p->_Widgets.empty())
	{
		QVBoxLayout * verticalLayout = new QVBoxLayout();
		QSpacerItem * verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
		verticalLayout->addItem(verticalSpacer);

		QHBoxLayout * horizontalLayout = new QHBoxLayout();
		QSpacerItem * horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		horizontalLayout->addItem(horizontalSpacer);

		QLabel * label = new QLabel(this);
		label->setObjectName("label_hintinfo");
		label->setText(FromAscii("没有可以编辑的参数"));
		horizontalLayout->addWidget(label);

		QSpacerItem * horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		horizontalLayout->addItem(horizontalSpacer_2);
		verticalLayout->addLayout(horizontalLayout);

		QSpacerItem * verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
		verticalLayout->addItem(verticalSpacer_2);
		_p->_Layout->addRow(verticalLayout);
	}
	else
	{
		for (std::size_t i = 0; i < _p->_Widgets.size(); ++i)
		{
			HRPropertyWidget* pwidget = _p->_Widgets[i].widget;

#ifdef _DEBUG
			if (pwidget->GetCreateParams()->Property)
			{
				TSString propertyName = pwidget->GetCreateParams()->Property->Name;
			}
#endif
			if (!pwidget->GetCreateParams()->Config)
			{
				continue;
			}

			connect(pwidget, SIGNAL(ValueChangedSignal(const HRPropertyWidget*, const TSVariant&)), this, SLOT(OnValueChangedSlot(const HRPropertyWidget*, const TSVariant&)));
		}
	}



	return _p->_Layout;
}

void HRPropertyWidgetGroup::AddWidget(const PropertyWidgetInfo &widgetInfo)
{
	_p->_Widgets.push_back(widgetInfo);
}

void HRPropertyWidgetGroup::showEvent(QShowEvent *ev)
{
	UpdateState();
	HRPropertyWidget::showEvent(ev);
}

void HRPropertyWidgetGroup::BindVariant(TSVariant &val) const
{
	if (val.IsValid())
	{
		_p->_DupObject = TSMetaType::DeepCopy(val, true);
	}
}
