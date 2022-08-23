#include "stdafx.h"
#include "HRPropertyDialog.h"

#include <QLineEdit>
#include <QApplication>
#include <QObjectList>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <private/qlayoutengine_p.h>

#include <HRControls/HRMessageBox.h>

#include "HRUIUtil.h"
#include "HRAppManager.h"
#include "HRAppConfig.h"

struct HRPropertyDialog::HRPropertyDialogPrivate
{
	QLineEdit *FindFirstLineEdit(QObject *obj)
	{
		const QObjectList &lstObjs = obj->children();
		foreach(QObject *objChild, lstObjs)
		{
			if(QLineEdit *control = dynamic_cast<QLineEdit *>(objChild))
			{
				return control;
			}
			if(QLineEdit *control = FindFirstLineEdit(objChild))
			{
				return control;
			}
		}
		return NULL;
	}

	HRPropertyDialogPrivate() : _PropertyWidget(NULL){}

	HRPropertyWidget * _PropertyWidget;
};

struct TSDummyPropertyWidget : public HRPropertyWidget 
{
	TSDummyPropertyWidget()
		:HRPropertyWidget(XPropertyCreateParamsPtr())
	{
		QHBoxLayout * RootLayout = new QHBoxLayout(this);
		RootLayout->setContentsMargins(0, 0, 0, 0);
		QLabel * label = new QLabel(this);
		label->setText(GetText());
		label->setAlignment(Qt::AlignHCenter);
		label->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
		RootLayout->addWidget(label);
		this->setLayout(RootLayout);
	}

	void Validate()
	{
	
	}

	QString GetText()
	{
		return FromAscii("无效的编辑控件");
	}
};

HRPropertyDialog::HRPropertyDialog(HRPropertyWidget * PropertyWidget, QWidget *parent)
	: HRVerifiableDialog(parent, Qt::WindowCloseButtonHint)
	,_p(new HRPropertyDialogPrivate())
{
	this->setWindowTitle(FromAscii("参数编辑对话框"));
	SetPropertyWidget(PropertyWidget);
}

HRPropertyDialog::HRPropertyDialog( QWidget *parent /*= 0*/ )
	: HRVerifiableDialog(parent, Qt::WindowCloseButtonHint)
	,_p(new HRPropertyDialogPrivate())
{
	this->setWindowTitle(FromAscii("参数编辑对话框"));
}

HRPropertyDialog::~HRPropertyDialog()
{
	delete _p;
}

HRPropertyWidget * HRPropertyDialog::GetPropertyWidget()
{
	return _p->_PropertyWidget;
}

bool HRPropertyDialog::Verify()
{
	if (HRVerifiableDialog::Verify())
	{
#ifdef DEBUG 
		_p->_PropertyWidget->Validate();
#else
		try
		{
			_p->_PropertyWidget->Validate();
		}
		catch (TSException & e)
		{
			HRMessageBox::information(QApplication::activeWindow(),FromAscii("保存失败!"), e.what());
			return false;
		}
#endif
		return true;
	}

	return false;
}

void HRPropertyDialog::showEvent(QShowEvent * e)
{
	HRUIUtil::AdjustWidgetSize(this, 10,48);
	QDialog::showEvent(e);
}

void HRPropertyDialog::closeEvent( QCloseEvent * e )
{
	QDialog::closeEvent(e);
}

void HRPropertyDialog::SetPropertyWidget( HRPropertyWidget * PropertyWidget )
{
	_p->_PropertyWidget = PropertyWidget ? PropertyWidget : new TSDummyPropertyWidget();

	if (!_p->_PropertyWidget->CustomizePropertyDialog(this))
	{
		SetCentralWidget(_p->_PropertyWidget);

		if(QLineEdit *lineEdit = _p->FindFirstLineEdit(_p->_PropertyWidget))
		{
			lineEdit->setFocus();
		}
	}
}
