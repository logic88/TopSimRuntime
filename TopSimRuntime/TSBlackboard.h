#ifndef HRPropertyDialog_H
#define HRPropertyDialog_H
#include <QDialog>

#include "HRPropertyWidget.h"
#include <HRControls/HRVerifiableDialog.h>

class QVBoxLayout;

class HRUICOMMON_EXPORT HRPropertyDialog : public HRVerifiableDialog
{
	Q_OBJECT
public:
	HRPropertyDialog(QWidget *parent = 0);
	HRPropertyDialog(HRPropertyWidget * PropertyWidget, QWidget *parent);
	~HRPropertyDialog();

	void SetPropertyWidget(HRPropertyWidget * PropertyWidget);
	HRPropertyWidget * GetPropertyWidget();

protected:
	void showEvent(QShowEvent * e);
	void closeEvent(QCloseEvent * e);

	virtual bool Verify();

private:
	struct HRPropertyDialogPrivate;
	HRPropertyDialogPrivate *_p;
};

typedef HRPropertyDialog HRPropertyDialog;

#endif // HRPropertyDialog_H
