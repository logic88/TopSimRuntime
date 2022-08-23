#ifndef HRBasePropertyWidget_h__
#define HRBasePropertyWidget_h__

#include "HRPropertyWidget.h"

class QHBoxLayout;
class QCheckBox;
class QSpinBox;
class HRSpinBox;
class QDateTime;
class HRDoubleSpinBox;
class QLineEdit;
class QDateTimeEdit;
class TSBattleTimeEditor;

class HRUICOMMON_EXPORT HRCheckBoxPropertyWidget : public HRPropertyWidget
{
	Q_OBJECT

public:
	HRCheckBoxPropertyWidget(XPropertyCreateParamsPtr CreateParams);

	void Validate();

protected:
	virtual void focusInEvent ( QFocusEvent * event );

	private slots:
		void on_CheckBox_stateChanged(int State);

private:
	QCheckBox * _CheckBox;
};

class HRUICOMMON_EXPORT HRSpinBoxPropertyWidget : public HRPropertyWidget
{
	Q_OBJECT

public:
	HRSpinBoxPropertyWidget(XPropertyCreateParamsPtr CreateParams);

	void Validate() OVER_RIDE;
	QString GetText() OVER_RIDE;

	TSVariant getVariantValue();

protected:
	virtual void focusInEvent ( QFocusEvent * event );

	private slots:
		void on_HRSpinBox_valueChanged(int i);

private:
	HRSpinBox * _HRSpinBox;
};

class HRUICOMMON_EXPORT HRDoubleSpinBoxPropertyWidget : public HRPropertyWidget
{
	Q_OBJECT

public:
	HRDoubleSpinBoxPropertyWidget(XPropertyCreateParamsPtr CreateParams);

	void Validate();
	QString GetText() OVER_RIDE;

public:
	TSVariant getVariantValue();

protected:
	virtual void focusInEvent ( QFocusEvent * event );

	private slots:
		void on_HRDoubleSpinBox_valueChanged(double d);

private:
	HRDoubleSpinBox * _HRDoubleSpinBox;
};

class HRUICOMMON_EXPORT HRLineEditPropertyWidget : public HRPropertyWidget
{
	Q_OBJECT

public:
	HRLineEditPropertyWidget(XPropertyCreateParamsPtr CreateParams);

	void Validate();
	QString GetText() OVER_RIDE;

public:
	TSVariant getVariantValue();

protected:
	virtual void focusInEvent ( QFocusEvent * event );

	private slots:
		void on_LineEdit_editingFinished();
		void on_LineEdit_textEdited(const QString & text);

private:
	QLineEdit * _LineEdit;
};

class HRUICOMMON_EXPORT HRDateTimePropertyWidget : public HRPropertyWidget
{
	Q_OBJECT

public:
	HRDateTimePropertyWidget(XPropertyCreateParamsPtr CreateParams);

	void Validate();
	QString GetText() OVER_RIDE;

public:
	TSVariant getVariantValue();

protected:
	virtual void focusInEvent ( QFocusEvent * event );

	private slots:
		void on_DateTimeEdit_dateTimeChanged(const QDateTime &date);

private:
	QDateTimeEdit * _DateTimeEdit;
};

class HRUICOMMON_EXPORT TSBattleTimePropertyWidget : public HRPropertyWidget
{
	Q_OBJECT

public:
	TSBattleTimePropertyWidget(XPropertyCreateParamsPtr CreateParams);

	void Validate();
	QString GetText() OVER_RIDE;

public:
	TSVariant getVariantValue();

protected:
	virtual void focusInEvent ( QFocusEvent * event );

private slots:
		void on_DateTimeEdit_dateTimeChanged(const QDateTime &date);
		void on_CheckBox_stateChanged(int state);
		void on_SpinBox_valueChanged ( int );
private:
	TSBattleTimeEditor * _DateTimeEdit;
	QSpinBox           * _SpinBox;
	QCheckBox          * _CheckBox;
};

#endif // HRBasePropertyWidget_h__
