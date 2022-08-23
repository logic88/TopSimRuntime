#ifndef __HRMAPTOOLDIALOG__H__
#define __HRMAPTOOLDIALOG__H__

#include "HRUICommon.h"
#include <QDialog>
#include <QGroupBox>

class  HRUICOMMON_EXPORT HRCheckableGroupBox : public QGroupBox
{
	Q_OBJECT

public:
	HRCheckableGroupBox(QWidget * parent = 0);

public:
	void SetWidgetsEnabled(bool Enabled);

private slots:
	void OnCheckedSlot(bool checked);
};

#endif // __HRMAPTOOLDIALOG__H__
