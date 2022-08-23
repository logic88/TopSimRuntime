#include "stdafx.h"
#include "HRComplexLinkButton.h"

#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QDialog>
#include <QApplication>
#include <QHBoxLayout>
#include <QtnHuaruHelper.h>
#include "HRAppManager.h"

#include "HRIPropertyWidgetFactory.h"

HRComplexLinkButton::HRComplexLinkButton(XPropertyCreateParamsPtr CreateParams)
	:HRPropertyWidget(CreateParams),_PopDialog(0),_Saved(false)
{
	QHBoxLayout * rootLayout = new QHBoxLayout(this);
	rootLayout->setContentsMargins(0, 0, 0, 0);

	_Widget = new QLabel(this);

	_Widget->setText(QtnHuaruThemeServices::Instance()->GetThemeLinkStyle()+QString("<a href=\"#1\">%1</a>").arg(GetText()));
	if (CreateParams->Config->TipText.trimmed().isEmpty() == false)
	{
		_Widget->setToolTip(CreateParams->Config->TipText);
	}

	_Widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

	rootLayout->addWidget(_Widget);
	this->setLayout(rootLayout);

	connect(_Widget,SIGNAL(linkActivated( const QString &)),this,SLOT(OnlinkActivated( const QString &)));
}

HRComplexLinkButton::~HRComplexLinkButton()
{

}

void HRComplexLinkButton::Validate()
{
	if (_Saved)
	{
		GetCreateParams()->Variant = GetPropertyVar();
	}
}

QString HRComplexLinkButton::GetText()
{
	TSVariant val = GetCreateParams()->Variant;
	QString text = FromAscii("����˴��༭");

	if (_Saved)
	{
		val = GetPropertyVar();
	}

	if (TSNObject * pNObj = (TSNObject *)TSMetaType::Cast(val.GetType(),TSNObject::GetMetaTypeIdStatic(),val.GetDataPtr()))
	{
		text = TSString2QString(pNObj->GetName());
	}
	else if (val.GetType()==TSVariant::VT_ARRAY)
	{
		TSVariantArray arr = val.Value<TSVariantArray>();

		if (arr.empty())
		{
			text = FromAscii("��");
		}
		else if (TSNObject * pNObj = (TSNObject *)TSMetaType::Cast(arr[0].GetType(),
			TSNObject::GetMetaTypeIdStatic(),arr[0].GetDataPtr()))
		{
			text = TSString2QString(pNObj->GetName());

			if (!text.isEmpty())
			{
				if (arr.size() > 1)
				{
					text+=",...";
				}
			}
		}
	}

	if (text.isEmpty() || text==FromAscii("None"))
	{
		text = FromAscii("����˴��༭");
	}

	return text;
}

void HRComplexLinkButton::OnlinkActivated(const QString &)
{
	if (!_PopDialog)
	{
		TSVariant Val = GetCreateParams()->Variant;
		if (Val.GetTypeFlag()!=TSVariant::Is_None && !Val.IsNull())
		{
			Val = TSMetaType::DeepCopy(Val,true);
		}

		HRIPropertyWidgetFactoryPtr PropWidgetFac = QueryModuleT<HRIPropertyWidgetFactory>();

		if (!PropWidgetFac)
		{
			DEF_LOG_ERROR("%1") << toAsciiData("���Թ���������δʵ�֡�");
			return;
		}

		HRPropertyWidget * PropWidget = PropWidgetFac->CreatePropertyWidget(
			MakeCreateParams(Val,GetCreateParams()->Mode,GetCreateParams()->Property,
			GetCreateParams()->OwnerCreateParams,
			GetCreateParams()->UIContext)
			,0,true,true);

		QString windowTitle;
		if (GetCreateParams()->Config && !GetCreateParams()->Config->LabelText.isEmpty())
		{
			windowTitle = GetCreateParams()->Config->LabelText;
		}

		if (windowTitle.isEmpty() && GetCreateParams()->Property)
		{
			windowTitle = TSString2QString(GetCreateParams()->Property->Name);
		}

		windowTitle = QString(FromAscii("%1-�����༭")).arg(windowTitle);

		_PopDialog = PropWidgetFac->ConstructPropertyDialog(PropWidget, windowTitle);

		connect(_PopDialog,SIGNAL(finished(int)),this,SLOT(OnDialogFinished( int)));
	}

	_PopDialog->setModal(true);
	_PopDialog->show();
}

void HRComplexLinkButton::OnDialogFinished( int result )
{
	if (result==QDialog::Accepted)
	{
		_Saved = true;

		_Widget->setText(QtnHuaruThemeServices::Instance()->GetThemeLinkStyle()+QString("<a href=\"#1\">%1</a>").arg(GetText()));
	}
}

TSVariant HRComplexLinkButton::GetPropertyVar()
{
	TSVariant Var;

	HRIPropertyWidgetFactoryPtr PropertyWidgetFac = QueryModuleT<HRIPropertyWidgetFactory>();

	if (PropertyWidgetFac)
	{
		HRPropertyWidget * PropWidget = PropertyWidgetFac->GetPropertyWidget(_PopDialog);

		if (PropWidget)
		{
			Var = PropWidget->GetCreateParams()->Variant; 
		}
		else
		{
			DEF_LOG_ERROR("%1") << toAsciiData("HRComplexLinkButton�й���������Ϊ�� ��");
		}
	}
	else
	{
		DEF_LOG_ERROR("%1") << toAsciiData("HRComplexLinkButton��ʹ�����Թ���������δʵ�֡�");
	}

	return Var;
}

