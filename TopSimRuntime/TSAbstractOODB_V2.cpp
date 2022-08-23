#include "stdafx.h"

#include <QDateTime>
#include <QDebug>
#include <QCheckBox>
#include <QLineEdit>
#include <QWidget>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <HRControls/HRMessageBox.h>

#include <boost/filesystem.hpp>

#include <TopSimRuntime/TSTimeUtil.h>
#include <TopSimRuntime/pugixml.hpp>
#include <TopSimRuntime/TSFileSystem.h>

#include "HRControls/HRSpinBox.h"
#include "HRControls/HRStringListDialog.h"
#include "HRUtil/HRExpression.h"
#include "HRUtil/HRTimeUtil.h"
#include "HRAppConfig.h"
#include "HRBasePropertyWidget.h"
//#include "HRScenarioUtil.h"

#define MAXSECONDS (60*60*24*365)

template<class T>
inline T * CreateBasePropertyWidget(HRPropertyWidget * Wrapper,const QString & TipText)
{
	QHBoxLayout * RootLayout = new QHBoxLayout(Wrapper);
	RootLayout->setContentsMargins(0, 0, 0, 0);
	T * t = new T(Wrapper);

	if (!TipText.isEmpty())
	{
		t->setToolTip(TipText);
	}

	t->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

	RootLayout->addWidget(t);
	Wrapper->setLayout(RootLayout);

	return t;
};

HRCheckBoxPropertyWidget::HRCheckBoxPropertyWidget( XPropertyCreateParamsPtr CreateParams ) 
	: HRPropertyWidget(CreateParams)
{
	_CheckBox = CreateBasePropertyWidget<QCheckBox>(this, CreateParams->Config->TipText);

	_CheckBox->setChecked(CreateParams->Variant.Value<bool>());

	connect(_CheckBox,SIGNAL(stateChanged(int)),this,SLOT(on_CheckBox_stateChanged(int)));
}

void HRCheckBoxPropertyWidget::Validate()
{
	GetCreateParams()->Variant = TSVariant::FromValue(
		_CheckBox->checkState() == Qt::Checked);
}

void HRCheckBoxPropertyWidget::focusInEvent( QFocusEvent * event )
{
	_CheckBox->setFocus();
}

void HRCheckBoxPropertyWidget::on_CheckBox_stateChanged( int State )
{
	HRPropertyWidget::EmitValueChangedSignal(
		TSVariant::FromValue(_CheckBox->checkState() == Qt::Checked));
}

HRSpinBoxPropertyWidget::HRSpinBoxPropertyWidget( XPropertyCreateParamsPtr CreateParams )
	: HRPropertyWidget(CreateParams)
{
	_HRSpinBox = CreateBasePropertyWidget<HRSpinBox>(this,CreateParams->Config->TipText);

	switch(CreateParams->Variant.GetType())
	{
	case TSVariant::VT_UINT8:
		_HRSpinBox->setMaximum(255);
		_HRSpinBox->setMinimum(0);
		break;
	case TSVariant::VT_INT8:
		_HRSpinBox->setMaximum(127);
		_HRSpinBox->setMinimum(-128);
		break;
	case TSVariant::VT_UINT16:
		_HRSpinBox->setMaximum(65535);
		_HRSpinBox->setMinimum(0);
		break;
	case TSVariant::VT_INT16:
		_HRSpinBox->setMaximum(32767);
		_HRSpinBox->setMinimum(-32767-1);
		break;
	case TSVariant::VT_UINT32:
	case TSVariant::VT_UINT64:
		_HRSpinBox->setMaximum(2147483647);
		_HRSpinBox->setMinimum(0);
		break;
	default:
		_HRSpinBox->setMaximum(2147483647);
		_HRSpinBox->setMinimum(-2147483647-1);
		break;
	}

	QString tipText = CreateParams->Config->TipText;

	if (CreateParams->Config->MaxVal != "")
	{
		int maxVal = CreateParams->Config->MaxVal.toInt();
		_HRSpinBox->setMaximum(maxVal);

		QString tipInfo = (tipText.isEmpty() ? QString(FromAscii("最大值:%1")).arg(maxVal) : QString(FromAscii("\r\n最大值:%1")).arg(maxVal));
		tipText.append(tipInfo);
	}

	if (CreateParams->Config->MinVal != "")
	{
		int minVal = CreateParams->Config->MinVal.toInt();
		_HRSpinBox->setMinimum(minVal);

		QString tipInfo = (tipText.isEmpty() ? QString(FromAscii("最小值:%1")).arg(minVal) : QString(FromAscii("\r\n最小值:%1")).arg(minVal));
		tipText.append(tipInfo);
	}

	_HRSpinBox->setToolTip(tipText);

	if (CreateParams->Config->Exp2UI != "")
	{
		HRExpression Exp;
		DOUBLE result;
		Exp.SetVariantValue("$",CreateParams->Variant.Value<INT32>());
		if (Exp.Calc(QString2TSString(CreateParams->Config->Exp2UI).c_str(),result) == 0)
		{
			_HRSpinBox->setValue(result);
		}
		else
		{
			_HRSpinBox->setValue(0);
		}
	}
	else
	{
		_HRSpinBox->setValue(CreateParams->Variant.Value<INT32>());
	}

	connect(_HRSpinBox, SIGNAL(valueChanged(int)), this, SLOT(on_HRSpinBox_valueChanged(int)));
}

void HRSpinBoxPropertyWidget::Validate()
{
	GetCreateParams()->Variant = getVariantValue();
}

TSVariant HRSpinBoxPropertyWidget::getVariantValue()
{
	TSVariant var;

	if (GetCreateParams()->Config->Exp2ML != "")
	{
		HRExpression Exp;
		DOUBLE result;
		Exp.SetVariantValue("$",_HRSpinBox->value());

		if (Exp.Calc(QString2TSString(GetCreateParams()->Config->Exp2ML).c_str(),result) == 0)
		{
			var = result;
		}
		else
		{
			var = 0;
		}
	}
	else
	{
		switch(GetCreateParams()->Variant.GetType())
		{
		case TSVariant::VT_UINT8:
			var = (UINT8)_HRSpinBox->value();
			break;
		case TSVariant::VT_INT8:
			var =(INT8) _HRSpinBox->value();
			break;
		case TSVariant::VT_UINT16:
			var = (UINT16)_HRSpinBox->value();
			break;
		case TSVariant::VT_INT16:
			var = (INT16)_HRSpinBox->value();
			break;
		case TSVariant::VT_UINT32:
			var = (UINT32)_HRSpinBox->value();
			break;
		case TSVariant::VT_INT64:
			var = (INT64)_HRSpinBox->value();
			break;
		case TSVariant::VT_UINT64:
			var = (UINT64)_HRSpinBox->value();
			break;
		case TSVariant::VT_HANDLE:
			var = TSHANDLE(_HRSpinBox->value());
			break;
		default:
			var = _HRSpinBox->value();
			break;
		}
	}

	return var;
}

void HRSpinBoxPropertyWidget::focusInEvent( QFocusEvent * event )
{
	_HRSpinBox->setFocus();
}

void HRSpinBoxPropertyWidget::on_HRSpinBox_valueChanged( int i )
{
	HRPropertyWidget::EmitValueChangedSignal(getVariantValue());
}

QString HRSpinBoxPropertyWidget::GetText() OVER_RIDE
{
	return QString::number(_HRSpinBox->value());
}

HRDoubleSpinBoxPropertyWidget::HRDoubleSpinBoxPropertyWidget( XPropertyCreateParamsPtr CreateParams ) : HRPropertyWidget(CreateParams)
{
	_HRDoubleSpinBox = CreateBasePropertyWidget<HRDoubleSpinBox>(this,CreateParams->Config->TipText);

	_HRDoubleSpinBox->setMaximum(1.7976931348623158e+308);
	_HRDoubleSpinBox->setMinimum(-1.7976931348623158e+308);

	QString tipText= CreateParams->Config->TipText;

	if (CreateParams->Config->MaxVal != "")
	{
		DOUBLE maxVal = CreateParams->Config->MaxVal.toDouble();
		_HRDoubleSpinBox->setMaximum(maxVal);

		QString tipInfo = (tipText.isEmpty() ? QString(FromAscii("最大值:%1")).arg(maxVal) : QString(FromAscii("\r\n最大值:%1")).arg(maxVal));
		tipText.append(tipInfo);
	}	
	if (CreateParams->Config->MinVal != "")
	{
		DOUBLE minVal = CreateParams->Config->MinVal.toDouble();
		_HRDoubleSpinBox->setMinimum(minVal);

		QString tipInfo = (tipText.isEmpty() ? QString(FromAscii("最小值:%1")).arg(minVal) : QString(FromAscii("\r\n最小值:%1")).arg(minVal));
		tipText.append(tipInfo);
	}

	_HRDoubleSpinBox->setToolTip(tipText);
	_HRDoubleSpinBox->setDecimals(CreateParams->Config->Decimals);

	pugi::xml_document xmldoc;
	boost::filesystem::path p = boost::filesystem::current_path() / "config/global.config";
	if (xmldoc.load_file(p.string().c_str(), pugi::parse_full, pugi::encoding_auto).status == pugi::status_ok)
	{
		pugi::xpath_node_set appNodes = xmldoc.select_nodes(toAsciiData("全局配置/配置项"));
		for (pugi::xpath_node_set::const_iterator cit = appNodes.begin(); cit != appNodes.end(); ++cit)
		{
			TSString nodeName = cit->node().attribute(toAsciiData("名称")).value();

			if (nodeName == toAsciiData("保留小数位"))
			{
				pugi::xpath_node_set nodeset = cit->node().select_nodes(toAsciiData("类型"));
				for (pugi::xpath_node_set::const_iterator citr = nodeset.begin(); citr != nodeset.end(); ++citr)
				{
					TSString Unit = citr->node().attribute(toAsciiData("单位")).as_string();
					int nSecond = citr->node().attribute(toAsciiData("小数位")).as_int();

					if (TSString2QString(Unit) == CreateParams->Config->Unit &&
						nSecond >= 0)
					{
						_HRDoubleSpinBox->setDecimals(nSecond);
					}
				}
			}
		}
	}

	if (CreateParams->Config->Exp2UI != "")
	{
		HRExpression Exp;
		DOUBLE result;
		Exp.SetVariantValue("$",CreateParams->Variant.Value<DOUBLE>());
		if (Exp.Calc(QString2TSString(CreateParams->Config->Exp2UI).c_str(),result) == 0)
		{
			_HRDoubleSpinBox->setValue(result);
		}
		else
		{
			_HRDoubleSpinBox->setValue(0.0);
		}
	}
	else
	{
		_HRDoubleSpinBox->setValue(CreateParams->Variant.Value<DOUBLE>());
	}

	connect(_HRDoubleSpinBox,SIGNAL(valueChanged(double)),this,SLOT(on_HRDoubleSpinBox_valueChanged(double)));
}

void HRDoubleSpinBoxPropertyWidget::Validate()
{
	GetCreateParams()->Variant = getVariantValue();
}

TSVariant HRDoubleSpinBoxPropertyWidget::getVariantValue()
{
	TSVariant var;

	if (GetCreateParams()->Config->Exp2ML!="")
	{
		HRExpression Exp;
		DOUBLE result;
		Exp.SetVariantValue("$",_HRDoubleSpinBox->value());

		if (Exp.Calc(QString2TSString(GetCreateParams()->Config->Exp2ML).c_str(),result) == 0)
		{
			var = result;
		}
		else
		{
			var = 0.0;
		}
	}
	else
	{
		var = _HRDoubleSpinBox->value();
	}

	return var;
}

void HRDoubleSpinBoxPropertyWidget::focusInEvent( QFocusEvent * event )
{
	_HRDoubleSpinBox->setFocus();
}

void HRDoubleSpinBoxPropertyWidget::on_HRDoubleSpinBox_valueChanged( double d )
{
	HRPropertyWidget::EmitValueChangedSignal(getVariantValue());
}

QString HRDoubleSpinBoxPropertyWidget::GetText() OVER_RIDE
{
	if (GetCreateParams()->Config->Decimals == 0)
	{
		return QString::number(_HRDoubleSpinBox->value(), 'f', 0);
	}
	else
	{
		return QString::number(_HRDoubleSpinBox->value(), 'f', GetCreateParams()->Config->Decimals);
	}
}

HRLineEditPropertyWidget::HRLineEditPropertyWidget( XPropertyCreateParamsPtr CreateParams ) : HRPropertyWidget(CreateParams)
{
	_LineEdit = CreateBasePropertyWidget<QLineEdit>(this,CreateParams->Config->TipText);

	_LineEdit->setMaxLength(TS_MAX_NAME_LENGTH);

	if (GetCreateParams()->Property && GetCreateParams()->Property->Name == toAsciiData("名称"))
	{
		QRegExp rx("[_a-zA-Z0-9\u4e00-\u9fa5^.]*");
		QValidator *validator = new QRegExpValidator(rx, _LineEdit);
		_LineEdit->setValidator(validator);
	}

	if (CreateParams->Config->MaxVal != "")
	{
		_LineEdit->setMaxLength(CreateParams->Config->MaxVal.toInt());
	}

	_LineEdit->setText(TSString2QString(CreateParams->Variant.Value<TSString>()));
	_LineEdit->home(false);//将光标移动到开始出，始终显示最开始的内容
	
	connect(_LineEdit,SIGNAL(editingFinished()),this,SLOT(on_LineEdit_editingFinished()));
	connect(_LineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(on_LineEdit_textEdited(const QString &)));
}

void HRLineEditPropertyWidget::Validate()
{
	GetCreateParams()->Variant = getVariantValue();
}

TSVariant HRLineEditPropertyWidget::getVariantValue()
{
	TSVariant var;

	//TODO::区分字符串与vector类型的属性
	if (GetCreateParams()->Variant.GetType() == TSVariant::VT_STRING)
	{
		var = TSVariant::FromValue(QString2TSString(_LineEdit->text()));
	}
	else if(GetCreateParams()->Variant.GetType() == TSVariant::VT_VECTOR2D)
	{
		TSVector2d v;
		QStringList texts = _LineEdit->text().split(",");
		for(int i = 0; i < texts.size(); ++i)
		{
			if(0 == i)
			{
				v.x = texts[i].toDouble();
			}
			else if(1 == i)
			{
				v.y = texts[i].toDouble();
			}
		}

		var = TSVariant::FromValue(v);
	}
	else if(GetCreateParams()->Variant.GetType() == TSVariant::VT_VECTOR3D)
	{
		TSVector3d v;
		QStringList texts = _LineEdit->text().split(",");
		for(int i = 0; i < texts.size(); ++i)
		{
			if(0 == i)
			{
				v.x = texts[i].toDouble();
			}
			else if(1 == i)
			{
				v.y = texts[i].toDouble();
			}
			else if(2 == i)
			{
				v.z = texts[i].toDouble();
			}
		}

		var = TSVariant::FromValue(v);
	}
	else if(GetCreateParams()->Variant.GetType() == TSVariant::VT_VECTOR2F)
	{
		TSVector2f v;
		QStringList texts = _LineEdit->text().split(",");
		for(int i = 0; i < texts.size(); ++i)
		{
			if(0 == i)
			{
				v.x = texts[i].toFloat();
			}
			else if(1 == i)
			{
				v.y = texts[i].toFloat();
			}
		}

		var = TSVariant::FromValue(v);
	}
	else if(GetCreateParams()->Variant.GetType() == TSVariant::VT_VECTOR3F)
	{
		TSVector3f v;
		QStringList texts = _LineEdit->text().split(",");
		for(int i = 0; i < texts.size(); ++i)
		{
			if(0 == i)
			{
				v.x = texts[i].toFloat();
			}
			else if(1 == i)
			{
				v.y = texts[i].toFloat();
			}
			else if(2 == i)
			{
				v.z = texts[i].toFloat();
			}
		}

		var = TSVariant::FromValue(v);
	}

	return var;
}

void HRLineEditPropertyWidget::focusInEvent( QFocusEvent * event )
{
	_LineEdit->setFocus();
}

void HRLineEditPropertyWidget::on_LineEdit_editingFinished()
{
	HRPropertyWidget::EmitValueChangedSignal(getVariantValue());
	_LineEdit->home(false);//将光标移动到开始出，始终显示最开始的内容
}

void HRLineEditPropertyWidget::on_LineEdit_textEdited( const QString & text )
{

}

QString HRLineEditPropertyWidget::GetText() OVER_RIDE
{
	return _LineEdit->text();
}

HRDateTimePropertyWidget::HRDateTimePropertyWidget( XPropertyCreateParamsPtr CreateParams )
	: HRPropertyWidget(CreateParams)
{
	_DateTimeEdit = CreateBasePropertyWidget<QDateTimeEdit>(this, CreateParams->Config->TipText);
	_DateTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");

	TSVariant Val = GetCreateParams()->Variant;
	QDateTime time = QDateTime::fromTime_t((Val.Value<TSTime>() - TSTimeUtil::Epoch()).total_seconds());
	_DateTimeEdit->setDateTime(time);

	connect(_DateTimeEdit, SIGNAL(dateTimeChanged(const QDateTime &)), this, SLOT(on_DateTimeEdit_dateTimeChanged(const QDateTime &)));
}

void HRDateTimePropertyWidget::Validate()
{
	GetCreateParams()->Variant = getVariantValue();
}

TSVariant HRDateTimePropertyWidget::getVariantValue()
{
	TSVariant var;

	var = TSTime(
		boost::posix_time::from_time_t(_DateTimeEdit->dateTime().toUTC().toTime_t()));

	return var;
}

void HRDateTimePropertyWidget::focusInEvent( QFocusEvent * event )
{
	_DateTimeEdit->setFocus();
}

void HRDateTimePropertyWidget::on_DateTimeEdit_dateTimeChanged( const QDateTime &date )
{
	HRPropertyWidget::EmitValueChangedSignal(getVariantValue());
}

QString HRDateTimePropertyWidget::GetText() OVER_RIDE
{
	return _DateTimeEdit->dateTime().toString(FromAscii("yyyy/MM/dd-hh:mm:ss.zzz"));
}

class TSBattleTimeEditor : public QDateTimeEdit 
{
public:
	TSBattleTimeEditor(QWidget * parent = 0) : QDateTimeEdit(parent),_MousePressed(false)
	{
		this->installEventFilter(this);
		lineEdit()->installEventFilter(this);
	}

protected:
	bool eventFilter ( QObject * watched, QEvent * event )
	{
		if (watched == lineEdit())
		{
			if (QEvent::HoverLeave == event->type())
			{
				_MousePressed = false;
			}
			if (QEvent::MouseButtonPress == event->type())
			{
				_MousePressed = true;
			}
		}
		if (watched == this)
		{
			if (QEvent::Wheel == event->type())
			{
				if (false == _MousePressed)
				{
					event->ignore();
					return true;
				}
			}
		}

		return QWidget::eventFilter(watched, event);
	}
private:
	bool _MousePressed;
};

TSBattleTimePropertyWidget::TSBattleTimePropertyWidget( XPropertyCreateParamsPtr CreateParams ) 
	: HRPropertyWidget(CreateParams)
{
	QHBoxLayout * hBoxLayout = new QHBoxLayout(this);

	hBoxLayout->setMargin(0);

	_DateTimeEdit = CreateBasePropertyWidget<TSBattleTimeEditor>(this,CreateParams->Config->TipText);
	hBoxLayout->addWidget(_DateTimeEdit);
	_DateTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");

	_SpinBox = CreateBasePropertyWidget<QSpinBox>(this,CreateParams->Config->TipText);
	_SpinBox->setMaximum(MAXSECONDS);
	_SpinBox->setHidden(true);
	hBoxLayout->addWidget(_SpinBox);

	_CheckBox = new QCheckBox(FromAscii("相对想定时间(秒)"),this);
	_CheckBox->setCheckState(Qt::Unchecked);
	_CheckBox->setHidden(!GetCreateParams()->UIContext);
	hBoxLayout->addWidget(_CheckBox);

	this->setLayout(hBoxLayout);

	QDateTime minTime = QDateTime::fromTime_t((TSTimeUtil::Epoch() - TSTimeUtil::Epoch()).total_seconds());
	QDateTime maxTime = QDateTime::fromTime_t((TSMaxTimeValue - TSTimeUtil::Epoch()).total_seconds());
	_DateTimeEdit->setMaximumDateTime(maxTime);
	_DateTimeEdit->setMinimumDateTime(minTime);

	TSBattleTime defaultTime = CreateParams->Variant.Value<TSBattleTime>();

	if (!GetAppConfig<bool>("Root.UseUTCTime"))
	{
		_DateTimeEdit->setDateTime(HRTimeUtil::TSBattleTime2QDataTime(defaultTime).toLocalTime());
	}
	else
	{
		_DateTimeEdit->setDateTime(HRTimeUtil::TSBattleTime2QDataTime(defaultTime));
	}

	if (GetCreateParams()->UIContext)
	{
		//if (NS_ScenarioBasicInfo::DataTypePtr basicInfo = HRScenarioUtil::GetScenarioBasicInfo(GetCreateParams()->UIContext))
		//{
		//	double duration = (defaultTime - TSTimeUtil::Epoch()).total_seconds() - basicInfo->BattleStartTime;
		//	_SpinBox->setValue(duration);
		//}
	}

	connect(_DateTimeEdit,SIGNAL(dateTimeChanged(const QDateTime &)),this,SLOT(on_DateTimeEdit_dateTimeChanged(const QDateTime &)));
	connect(_SpinBox,SIGNAL(valueChanged( int )),this,SLOT(on_SpinBox_valueChanged (int)));
	connect(_CheckBox,SIGNAL(stateChanged(int)),this,SLOT(on_CheckBox_stateChanged(int)));
}

void TSBattleTimePropertyWidget::on_SpinBox_valueChanged ( int value)
{
}

void TSBattleTimePropertyWidget::on_CheckBox_stateChanged(int state)
{
	if (Qt::Unchecked == state)
	{
		_SpinBox->setHidden(true);
		_DateTimeEdit->setHidden(false);
	}
	if (Qt::Checked == state)
	{
		_SpinBox->setHidden(false);
		_DateTimeEdit->setHidden(true);
	}
}

void TSBattleTimePropertyWidget::Validate()
{
	GetCreateParams()->Variant = getVariantValue();
}

TSVariant TSBattleTimePropertyWidget::getVariantValue()
{
	TSVariant var;

	//QString strTime = _DateTimeEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz");
	var = TSBattleTime (boost::posix_time::from_time_t(_DateTimeEdit->dateTime().toUTC().toTime_t()));

	return var;
}

void TSBattleTimePropertyWidget::focusInEvent( QFocusEvent * event )
{
	_DateTimeEdit->setFocus();
}

void TSBattleTimePropertyWidget::on_DateTimeEdit_dateTimeChanged( const QDateTime &date )
{
	HRPropertyWidget::EmitValueChangedSignal(getVariantValue());
}

QString TSBattleTimePropertyWidget::GetText() OVER_RIDE
{
	return _DateTimeEdit->dateTime().toString(FromAscii("yyyy/MM/dd-hh:mm:ss.zzz"));
}
