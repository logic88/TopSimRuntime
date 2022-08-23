#ifndef HRCLASSSELECTDIALOG_H
#define HRCLASSSELECTDIALOG_H

#include <QDialog>
#include "HRUICommon.h"

namespace Ui {class HRClassSelectDialog;};

class HRUICOMMON_EXPORT HRClassSelectDialog : public QDialog
{
	Q_OBJECT

public:
	HRClassSelectDialog(QWidget *parent, int id, bool multiSelect);
	~HRClassSelectDialog();

	std::vector<int> GetSelectedClassId() const;

	QStringList GetSelectedClassName() const;

private slots:
	void on_pushButton_Ok_clicked();
	void on_pushButton_Cancel_clicked();
	void on_listWidget_doubleClicked();
 
private:
	int _Id;
	Ui::HRClassSelectDialog *ui;
};

#endif // HRCLASSSELECTDIALOG_H
