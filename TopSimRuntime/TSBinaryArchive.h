#ifndef  __HRLINEEDITWITHCANCLEBTN_H__
#define __HRLINEEDITWITHCANCLEBTN_H__

#include <QLabel>
#include <QMouseEvent>
#include <QLineEdit>

#include "HRUICommon.h"

class HRLabel : public QLabel
{
	Q_OBJECT
public:
	HRLabel(QWidget* parent = 0);
	~HRLabel();
protected:
	void focusInEvent(QFocusEvent* event);
	void mousePressEvent(QMouseEvent* event);

signals:
	void clicked();

private:
	QString _LabelUrl;
};

class HRUICOMMON_EXPORT HRLineEditWithCancleBtn : public QLineEdit
{
	Q_OBJECT
public:
	HRLineEditWithCancleBtn(QWidget* parent = 0);
	virtual ~HRLineEditWithCancleBtn();

signals:
	void clearContents();
protected:
	void resizeEvent(QResizeEvent* event);
private slots:
	void OnTextChanged(const QString& text);
private:
	HRLabel* _Label;
};
#endif//__HRLINEEDITWITHCANCLEBTN_H__