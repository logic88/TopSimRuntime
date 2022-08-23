#ifndef HRArrayPropertyPanel_H
#define HRArrayPropertyPanel_H

#include "HRPropertyWidget.h"

#include "HRUICommon.h"

namespace Ui {class HRArrayPropertyPanel;}
class QTableWidgetItem;
class QListWidgetItem;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	ģ���������Ա༭�ؼ�.  </summary>
///
/// <remarks>	���ڱ༭�������͵��������Ա༭���ṩ����Ԫ�ص���ɾ�Ĳ鹦�ܡ�
/// 			TSMF Team, 2014/4/9. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

// �����﷨������ǩ="��ʾ�б�:����Ŀ��;ʹ������;ʹ������|�������б�:TSSensor;TSComdev|��ʶ��ϵ:Friendly|...."��

class HRUICOMMON_EXPORT HRArrayPropertyPanel : public HRPropertyWidget
{
	Q_OBJECT
public:
	HRArrayPropertyPanel(XPropertyCreateParamsPtr CreateParams);
	~HRArrayPropertyPanel();
protected:
	void Validate();
	void FillArrayItem(QListWidgetItem *item, TSVariant & val);
	bool DefaultShowLable() const;

protected slots:
	void on_pushButton_Add_clicked();
	void on_pushButton_Delete_clicked();
	void on_pushButton_Edit_clicked();
	void on_pushButton_Clear_clicked();
	void on_pushButton_Up_clicked();
	void on_pushButton_Down_clicked();
	void on_listWidget_Main_itemClicked(QListWidgetItem * item);
	void on_listWidget_Main_itemDoubleClicked( QListWidgetItem * item );
	void OnPropertyDialogFinishedSlot(int result);
protected:
	DEPRECATED void ParseTagCompatible(const QString &tag);
	void EditItem(QListWidgetItem * item);
	void UpdateEditButtonState( QListWidgetItem * item);
	QString CreateDisplayPropPreview(const TSVariant &val);
private:
	Ui::HRArrayPropertyPanel * ui;
	struct HRArrayPropertyPanelPrivate;
	HRArrayPropertyPanelPrivate *_p;
};

#endif // HRArrayPropertyPanel_H
