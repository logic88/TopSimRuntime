#include "stdafx.h"
#include "HRClassSelectDialog.h"

#include <TopSimRuntime/TSMetaType.h>

#include "ui_HRClassSelectDialog.h"

HRClassSelectDialog::HRClassSelectDialog(QWidget *parent, int id, bool multiSelect)
	: QDialog(parent), _Id(id)
{
	ui = new Ui::HRClassSelectDialog();
	ui->setupUi(this);
	setWindowTitle(FromAscii("类别选择对话框"));

	ui->listWidget->setSelectionMode(multiSelect ? QAbstractItemView::MultiSelection : QAbstractItemView::SingleSelection);

	std::vector<int> deriveds;
	TSMetaType::GetAllDerivedTypes(_Id, deriveds);

	for (int i=0; i<deriveds.size(); ++i)
	{
		QListWidgetItem * item = new QListWidgetItem( TSString2QString( TSMetaType::GetTypeName(deriveds[i]) ), ui->listWidget);
		item->setData(Qt::UserRole, deriveds[i]);
	}
}

HRClassSelectDialog::~HRClassSelectDialog()
{
	delete ui;
}

void HRClassSelectDialog::on_pushButton_Ok_clicked()
{
	accept();
}

void HRClassSelectDialog::on_pushButton_Cancel_clicked()
{
	reject();
}

std::vector<int> HRClassSelectDialog::GetSelectedClassId() const
{
	std::vector<int> itemList;

	QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();

	for (int i = 0;i < selectedItems.size();i++)
	{
		itemList.push_back(selectedItems[i]->data(Qt::UserRole).value<int>());
	}

	return itemList;
}

QStringList HRClassSelectDialog::GetSelectedClassName() const
{
	QStringList itemList;

	QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();

	for (int i = 0;i < selectedItems.size();i++)
	{
		itemList.push_back(selectedItems[i]->text());
	}

	return itemList;
}

void HRClassSelectDialog::on_listWidget_doubleClicked()
{
	accept();
}

