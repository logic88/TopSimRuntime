#ifndef HRArrayPropertyPanel_H
#define HRArrayPropertyPanel_H

#include "HRPropertyWidget.h"

#include "HRUICommon.h"

namespace Ui {class HRArrayPropertyPanel;}
class QTableWidgetItem;
class QListWidgetItem;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	模型数组属性编辑控件.  </summary>
///
/// <remarks>	用于编辑复杂类型的数组属性编辑，提供数组元素的增删改查功能。
/// 			TSMF Team, 2014/4/9. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

// 兼容语法：《标签="显示列表:攻击目标;使用武器;使用数量|类名称列表:TSSensor;TSComdev|标识关系:Friendly|...."》

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
