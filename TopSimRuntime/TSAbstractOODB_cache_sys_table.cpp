#include "stdafx.h"
#include "HRArrayPropertyPanel.h"

#include <QPointer>
#include <QMessageBox>
#include <QItemDelegate>
#include <QTreeWidgetItem>

#include <HRUtil/HRExpression.h>
#include <HRUtil/HRTimeUtil.h>
#include <HRControls/HRStringListDialog.h>

#include "HRUIUtil.h"
#include "HRAppManager.h"
#include "HRPropertyWidget.h"
#include "HRComplexLinkButton.h" 
#include "HRClassSelectDialog.h"
#include "HRIPropertyWidgetFactory.h"

#include "ui_HRArrayPropertyPanel.h"

Q_DECLARE_METATYPE(TSVariant);
Q_DECLARE_METATYPE(TSHANDLE);
Q_DECLARE_METATYPE(QListWidgetItem*);

#define ROW_DATA_INDEX (Qt::UserRole+1)


enum RelationShip
{

	/// <summary> 友好.  </summary>
	Friendly,
	/// <summary> 敌对.  </summary>
	Hostility,
	/// <summary> 中立.  </summary>
	Neutral,
	/// <summary> 未知.  </summary>
	Unknown
};

struct RelationshiStrPair
{
	RelationShip rel;
	QString str;
};

static const RelationshiStrPair kRelEnumArray[] =
{
	{RelationShip::Friendly, "friendly"},
	{RelationShip::Hostility, "hostility"},
	{RelationShip::Neutral, "neutral"},
	{RelationShip::Unknown, "unknown"},
};

int StrToRel(const QString &str)
{
	QString lowerTrimmedStr = str.trimmed().toLower();
	for (int i = 0; i < TS_ARRAYSIZE(kRelEnumArray); ++i)
	{
		if (kRelEnumArray[i].str == lowerTrimmedStr)
		{
			return kRelEnumArray[i].rel;
		}
	}
	return -1;
}

struct HRArrayPropertyPanel::HRArrayPropertyPanelPrivate
{
	HRArrayPropertyPanelPrivate()
		: _IncludeSelf(true)
		, _IdentRelation(-1)
		, _AllowRepeatedValue(false)
		, _ArrayCapacity(65535)
	{

	}

	// 待显示的属性名称集合
	// 标签="...|显示列表:攻击目标;使用武器;使用数量|...."
	QStringList _DisplayPropertyNames;

	// TSHandle、TSTemplateId、TSClassificationId、TSModelTmplOrClassId
	// 标签="...|类名称列表:TSBSE;TSComdev|...."
	QStringList _IncludeClasses;

	// 实体标识关系
	// 标签="...|标识关系:Friendly|..."
	/// <summary> The identifier relation </summary>
	int _IdentRelation;

	// 是否包含类型自身
	// 标签="...|包含自己:false|..."
	bool _IncludeSelf;

	bool _AllowRepeatedValue;
	int _ArrayCapacity;
};

HRArrayPropertyPanel::HRArrayPropertyPanel(XPropertyCreateParamsPtr CreateParams)
	: HRPropertyWidget(CreateParams)
	, _p(new HRArrayPropertyPanelPrivate())
{
	ui = new Ui::HRArrayPropertyPanel();
	ui->setupUi(this);
	ui->pushButton_Up->setText(FromAscii("上移"));
	ui->pushButton_Up->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui->pushButton_Down->setText(FromAscii("下移"));
	ui->pushButton_Down->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	if (CreateParams->Property)
	{
		QString groupBoxTitle = TSString2QString(CreateParams->Property->Name);
		if (CreateParams->Config && CreateParams->Config->LabelText != "NULL" && CreateParams->Config->LabelText != "")
		{
			groupBoxTitle = CreateParams->Config->LabelText;
		}
		ui->groupBox_Wrapper->setTitle(groupBoxTitle);
	}

	if (GetCreateParams()->Config)
	{
		ParseTagCompatible(GetCreateParams()->Config->Tag);
	}

	ui->listWidget_Main->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->listWidget_Main->setSelectionMode(QAbstractItemView::SingleSelection);

	TSVariantArray vArray = CreateParams->Variant.Value<TSVariantArray>();
	for (std::size_t i = 0; i < vArray.size(); i++)
	{
		QListWidgetItem *item = new QListWidgetItem();
		ui->listWidget_Main->addItem(item);
		FillArrayItem(item, vArray[i]);
	}
}

void HRArrayPropertyPanel::ParseTagCompatible(const QString& tag)
{
	if (tag.isEmpty())
	{
		_p->_DisplayPropertyNames = QStringList() << FromAscii("名称");
	}
	else
	{
		QString strDisplayList;

		QString strClassList;

		QStringList tagList = tag.split("|");

		for (int i = 0; i < tagList.size(); i++)
		{
			if (tagList[i].indexOf(FromAscii("显示列表")) >= 0)
			{
				strDisplayList = tagList[i].trimmed();
			}
			else if (tagList[i].indexOf(FromAscii("类名称列表")) >= 0)
			{
				strClassList = tagList[i].trimmed();
			}
			else if (tagList[i].indexOf(FromAscii("标识关系")) >= 0)
			{
				_p->_IdentRelation = StrToRel(tagList[i].replace(FromAscii("标识关系:"), ""));
			}
			else if (tagList[i].indexOf(FromAscii("包含自己")) >= 0)
			{
				QString includeSelfStr = tagList[i].replace(FromAscii("包含自己:"), "").trimmed().toLower();
				if (includeSelfStr == "false" || includeSelfStr == "0")
				{
					_p->_IncludeSelf = false;
				}
			}
		}

		if (!strDisplayList.isEmpty())
		{
			_p->_DisplayPropertyNames = strDisplayList.replace(FromAscii("显示列表:"), "").split(";");
		}

		if (!strClassList.isEmpty())
		{
			_p->_IncludeClasses = strClassList.replace(FromAscii("类名称列表:"), "").split(";");
		}
	}
}

void HRArrayPropertyPanel::FillArrayItem(QListWidgetItem *item, TSVariant &val)
{
	// detect if variant can cast to string
	QString dispText;

	if (_p->_DisplayPropertyNames.size() > 0)
	{
		dispText = CreateDisplayPropPreview(val);
	}

	if (dispText.isEmpty())
	{
		dispText = TSString2QString(val.Value<TSString>());
	}

	if (dispText.isEmpty())
	{
		dispText = FromAscii("条目");
	}

	item->setText(dispText);
	item->setData(ROW_DATA_INDEX, QVariant::fromValue(val));

}

HRArrayPropertyPanel::~HRArrayPropertyPanel()
{
	delete ui;
	delete _p;
}

void HRArrayPropertyPanel::Validate()
{
	TSVariantArray vArray;
	for (int i = 0;i < ui->listWidget_Main->count(); i++)
	{
		vArray.push_back(ui->listWidget_Main->item(i)->data(ROW_DATA_INDEX).value<TSVariant>());
	}
	GetCreateParams()->Variant = vArray;
}

void HRArrayPropertyPanel::on_pushButton_Add_clicked()
{
	if (ui->listWidget_Main->count() >= _p->_ArrayCapacity)
	{
		QMessageBox::information(QApplication::activeWindow(), FromAscii("提示信息"), FromAscii("数组容量已经到达上限，请删除后重试！"));
		return;
	}

	TSVariant dummy;

	if (GetCreateParams()->Property)
	{
		dummy = GetCreateParams()->Property->PropertyOperator->GetDummyVariant().Value<TSVariantArray>()[0];
	}
	else if (GetCreateParams()->Variant.GetType() == TSVariant::VT_ARRAY)
	{
		if (GetCreateParams()->Variant.Value<TSVariantArray>().size() > 0)
		{
			dummy = GetCreateParams()->Variant.Value<TSVariantArray>()[0];
		}
	}

	if (!dummy.IsValid())
	{
		QMessageBox::information(QApplication::activeWindow(), FromAscii("提示信息"), FromAscii("数组类型未给出！"));
		return;
	}

	XPropertyCreateParamsPtr parentCreateParams = GetCreateParams();

	XPropertyCreateParamsPtr itemCreateParams;

	if (_p->_IncludeClasses.size() > 0)
	{
		QList<QStandardItem *> candidateItems;
		for (int i = 0; i < _p->_IncludeClasses.size(); i++)
		{
			int typeId = TSMetaType::GetType(QString2TSString(_p->_IncludeClasses[i]).c_str());

			if (typeId == 0)
			{
				DEF_LOG_ERROR("%1%2") << QString2TSString(_p->_IncludeClasses[i]) << QString2TSString(FromAscii("类型未找到..."));
				continue;
			}

			TSString Alias = QueryModuleT<HRIPropertyWidgetFactory>()->GetClassAlias(typeId);
			QString ItemName = TSString2QString(Alias) + TS_TEXT("(") + _p->_IncludeClasses[i] + TS_TEXT(")");
			if (Alias.empty())
			{
				ItemName = _p->_IncludeClasses[i];
			}

			QStandardItem* item = new QStandardItem(ItemName);
			item->setData(QVariant::fromValue(typeId), Qt::UserRole + 1);
			candidateItems.push_back(item);
		}
		HRStringListDialog listDlg(candidateItems, FromAscii("类型选择"), false, this);
		listDlg.setModal(true);
		if (listDlg.exec() == QDialog::Accepted)
		{
			const QList<QStandardItem *> items = listDlg.GetSelectedItems();
			if (items.empty())
			{
				return;
			}
			int typeId = items[0]->data(Qt::UserRole + 1).value<int>();
			TSVariant val = HRUIUtil::CreateObjectInstance(typeId, dummy.GetTypeFlag());
			QueryModuleT<HRIPropertyWidgetFactory>()->InitWithDefaultValue(typeId, val);
			itemCreateParams = MakeCreateParams(val, parentCreateParams->Mode, parentCreateParams->Property,
				parentCreateParams->OwnerCreateParams, parentCreateParams->UIContext);
		}
		else
		{
			return;
		}
	}
	else
	{
		itemCreateParams = MakeCreateParams(dummy, parentCreateParams->Mode, parentCreateParams->Property,
			parentCreateParams->OwnerCreateParams, parentCreateParams->UIContext);
	}

	HRIPropertyWidgetFactoryPtr PropWidgetFac = QueryModuleT<HRIPropertyWidgetFactory>();

	if (!PropWidgetFac)
	{
		DEF_LOG_ERROR("%1") << QString2TSString(FromAscii("属性构造器工厂未实现"));
		return;
	}

	QString Title;

	if (GetCreateParams()->Property)
	{
		Title = QString(FromAscii("%1-参数设置")).arg(TSString2QString(GetCreateParams()->Property->Name));
	}
	else
	{
		Title = FromAscii("参数设置");
	}

	HRPropertyWidget * PropWidget = PropWidgetFac->CreatePropertyWidget(itemCreateParams);

	QDialog * dlg = PropWidgetFac->ConstructPropertyDialog(PropWidget, Title);

	connect(dlg, SIGNAL(finished(int)), SLOT(OnPropertyDialogFinishedSlot(int)));

	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setModal(true);
	dlg->show();
}

void HRArrayPropertyPanel::on_pushButton_Delete_clicked()
{
	if (QListWidgetItem * item = ui->listWidget_Main->item(
		ui->listWidget_Main->currentRow()))
	{
		delete item;
	}
}

void MoveAndSelectItem(QListWidget *listWidget, int inc)
{
	int index = listWidget->currentIndex().row();

	if (QListWidgetItem*  item = listWidget->takeItem(index))
	{
		listWidget->insertItem(index + inc, item);
		listWidget->clearSelection();
		listWidget->setCurrentItem(item);
		item->setSelected(true);
	}
}

void HRArrayPropertyPanel::on_pushButton_Up_clicked()
{
	MoveAndSelectItem(ui->listWidget_Main, -1);
}

void HRArrayPropertyPanel::on_pushButton_Down_clicked()
{
	MoveAndSelectItem(ui->listWidget_Main, 1);
}

void HRArrayPropertyPanel::EditItem(QListWidgetItem * item)
{
	TSVariant value = item->data(ROW_DATA_INDEX).value<TSVariant>();
	if (!value.IsValid())
	{
		QMessageBox::information(QApplication::activeWindow(), FromAscii("提示信息"), FromAscii("数据异常！"));
		return;
	}

	int nValType = value.GetType();
	if (nValType == TSVariant::VT_MODELTMPLORCLASSID ||
		nValType == TSVariant::VT_TEMPLATEID ||
		nValType == TSVariant::VT_CLASSIFICATIONID ||
		nValType == TSVariant::VT_HANDLE)
	{
		return;
	}

	if (!value.IsNull())
	{
		value = TSMetaType::DeepCopy(value, true);
	}

	HRIPropertyWidgetFactoryPtr PropWidgetFac = QueryModuleT<HRIPropertyWidgetFactory>();

	if (!PropWidgetFac)
	{
		DEF_LOG_ERROR("%1") << toAsciiData("属性构造器工厂没有实现！");
		return;
	}

	QString Title = FromAscii("参数设置");

	if (GetCreateParams()->Property)
	{
		Title = QString(FromAscii("%1-参数设置")).arg(TSString2QString(GetCreateParams()->Property->Name));
	}

	HRPropertyWidget * PropWidget = PropWidgetFac->CreatePropertyWidget(
		MakeCreateParams(value,
			GetCreateParams()->Mode,
			GetCreateParams()->Property,
			GetCreateParams()->OwnerCreateParams,
			GetCreateParams()->UIContext), 0, true, true);

	QDialog * editDlg = PropWidgetFac->ConstructPropertyDialog(PropWidget, Title);

	PropWidgetFac->SetPropertyDialogUserData(editDlg, QVariant::fromValue(item));

	editDlg->setAttribute(Qt::WA_DeleteOnClose);
	connect(editDlg, SIGNAL(finished(int)), SLOT(OnPropertyDialogFinishedSlot(int)));

	editDlg->setModal(true);
	editDlg->show();
}

void HRArrayPropertyPanel::on_pushButton_Edit_clicked()
{
	if (QListWidgetItem * item = ui->listWidget_Main->item(
		ui->listWidget_Main->currentRow()))
	{
		EditItem(item);
	}
}

void HRArrayPropertyPanel::on_pushButton_Clear_clicked()
{
	ui->listWidget_Main->clear();
}

void HRArrayPropertyPanel::on_listWidget_Main_itemDoubleClicked(QListWidgetItem * item)
{
	UpdateEditButtonState(item);
	if (ui->pushButton_Edit->isEnabled())
	{
		EditItem(item);
	}
}

void HRArrayPropertyPanel::on_listWidget_Main_itemClicked(QListWidgetItem * item)
{
	UpdateEditButtonState(item);
}

void HRArrayPropertyPanel::UpdateEditButtonState(QListWidgetItem * item)
{
	TSVariant Val = item->data(ROW_DATA_INDEX).value<TSVariant>();
	if (!Val.IsValid())
	{
		ui->pushButton_Edit->setEnabled(false);
		return;
	}

	int nValType = Val.GetType();

	if (nValType == TSVariant::VT_MODELTMPLORCLASSID ||
		nValType == TSVariant::VT_TEMPLATEID ||
		nValType == TSVariant::VT_CLASSIFICATIONID ||
		nValType == TSVariant::VT_HANDLE)
	{
		ui->pushButton_Edit->setEnabled(false);
		return;
	}

	ui->pushButton_Edit->setEnabled(true);
}

QString HRArrayPropertyPanel::CreateDisplayPropPreview(const TSVariant &val)
{
	QString itemText;

	XPropertyCreateParamsPtr OwnerCreateParam = MakeCreateParams(val
		, GetCreateParams()->Mode, GetCreateParams()->Property, GetCreateParams());

	for (int i = 0; i < _p->_DisplayPropertyNames.size(); i++)
	{
		const QString &propertyName = _p->_DisplayPropertyNames[i];

		if (TSMetaProperty * Prop = TSMetaType::GetProperty(val.GetType(),
			TSMetaType::GetPropertyH(val.GetType(), QString2TSString(propertyName))))
		{
			TSVariant displayValue = Prop->PropertyOperator->GetValue(val.GetDataPtr());

			TSString strTypeName = TSMetaType::GetTypeName(displayValue.GetType());

			XPropertyConfigExPtr config = QueryModuleT<HRIPropertyWidgetFactory>()->GetPropConfig(Prop->OwningType, propertyName);

			XPropertyCreateParamsPtr PropCreateParam = MakeCreateParams(displayValue, 
				OwnerCreateParam->Mode, Prop, OwnerCreateParam);

			HRPropertyWidget * PropWidget = QueryModuleT<HRIPropertyWidgetFactory>()->CreatePropertyWidget(PropCreateParam);

			QString Text;

			if (PropWidget)
			{
				Text = PropWidget->GetText();
			}

			if (!Text.isEmpty())
			{
				itemText += QString("%1=%2;").arg(propertyName).arg(Text);
			}
			else
			{
				int displayValueType = displayValue.GetType();
				if (displayValueType >= TSVariant::VT_FIRST_SIMPLE && displayValueType <= TSVariant::VT_LAST_SIMPLE)
				{
					if (displayValueType == TSVariant::VT_HANDLE)
					{
						itemText += QString("%1=%2;").arg(propertyName).arg(QString::number(displayValue.ToHandle()._value));
					}
					else if (displayValueType == TSVariant::VT_STRING)
					{
						itemText += QString("%1=%2;").arg(propertyName).arg(TSString2QString(displayValue.Value<TSString>()));
					}
					else if (displayValueType == TSVariant::VT_BOOL)
					{
						if (displayValue.Value<bool>())
						{
							itemText += QString("%1=%2;").arg(propertyName).arg("true");
						}
						else
						{
							itemText += QString("%1=%2;").arg(propertyName).arg("false");
						}
					}
					else if (displayValueType == TSVariant::VT_TIMEDURATION)
					{
						QString timeStr = TSString2QString(TSValue_Cast<TSString>(TD2CSTR(displayValue.Value<TSTimeDuration>())));
						itemText += QString("%1=%2;").arg(propertyName).arg(timeStr);
					}
					else if (displayValueType == TSVariant::VT_TIME)
					{
						itemText += QString("%1=%2;").arg(propertyName).arg(TSString2QString(TSValue_Cast<TSString>(TIME2CSTR(displayValue.Value<TSTime>()))));
					}
					else if (displayValueType == TSVariant::VT_BATTLE_TIME)
					{
						QDateTime datetime = HRTimeUtil::TSBattleTime2QDataTime(displayValue.Value<TSBattleTime>());
						itemText += QString("%1=%2;").arg(propertyName).arg(datetime.toString("yyyy-MM-dd hh:mm:ss"));
					}
					else if (displayValueType == TSVariant::VT_VECTOR2F)
					{
						TSVector2d Vector2d = displayValue.Value<TSVector2d>();
						QString PropertyValue = QString("(%1,%2)").arg(Vector2d.x).arg(Vector2d.y);
						itemText += QString("%1=%2;").arg(propertyName).arg(PropertyValue);
					}
					else if (displayValueType == TSVariant::VT_VECTOR3F)
					{
						TSVector3f Vector3f = displayValue.Value<TSVector3f>();
						QString PropertyValue = QString("(%1,%2,%3)").arg(Vector3f.x).arg(Vector3f.y).arg(Vector3f.z);
						itemText += QString("%1=%2;").arg(propertyName).arg(PropertyValue);
					}
					else if (displayValueType == TSVariant::VT_VECTOR2D)
					{
						TSVector2d Vector2d = displayValue.Value<TSVector2d>();
						QString PropertyValue = QString("(%1,%2)").arg(Vector2d.x).arg(Vector2d.y);
						itemText += QString("%1=%2;").arg(propertyName).arg(PropertyValue);
					}
					else if (displayValueType == TSVariant::VT_VECTOR3D)
					{
						TSVector3d Vector3d = displayValue.Value<TSVector3d>();
						QString PropertyValue = QString("(%1,%2,%3)").arg(Vector3d.x).arg(Vector3d.y).arg(Vector3d.z);
						itemText += QString("%1=%2;").arg(propertyName).arg(PropertyValue);
					}
					else
					{
						if (config)
						{
							DOUBLE result = 0;
							if (!config->Exp2UI.isEmpty())
							{
								HRExpression Exp;

								if (displayValueType >= TSVariant::VT_UINT8 && displayValueType <= TSVariant::VT_INT64)
								{
									Exp.SetVariantValue("$", displayValue.Value<INT32>());
								}
								else if (displayValueType == TSVariant::VT_FLOAT ||
									displayValueType == TSVariant::VT_DOUBLE)
								{
									Exp.SetVariantValue("$", displayValue.Value<DOUBLE>());
								}

								if (Exp.Calc(QString2TSString(config->Exp2UI).c_str(), result) == 0)
								{
									itemText += QString("%1=%2%3;").arg(propertyName).arg(result).arg(config->Unit.trimmed());
								}
							}
							else if (displayValueType >= TSVariant::VT_UINT8 && displayValueType <= TSVariant::VT_DOUBLE)
							{
								result = displayValue.Value<DOUBLE>();

								itemText += QString("%1=%2%3;").arg(propertyName).arg(result).arg(config->Unit.trimmed());
							}
							else if (displayValueType == TSVariant::VT_STRING)
							{
								itemText += QString("%1=%2%3;").arg(propertyName).arg(TSString2QString(displayValue.ToString())).arg(config->Unit.trimmed());
							}
						}
						else
						{
							itemText += QString("%1=%2;").arg(propertyName).arg(TSString2QString(displayValue.Value<TSString>()));
						}
					}
				}
			}
		}
	}

	return itemText;
}

void HRArrayPropertyPanel::OnPropertyDialogFinishedSlot(int result)
{
	if (result != QDialog::Accepted)
	{
		return;
	}

	QDialog *dlg = static_cast<QDialog *>(sender());

	HRIPropertyWidgetFactoryPtr PropWidgetFac = QueryModuleT<HRIPropertyWidgetFactory>();

	if (!PropWidgetFac)
	{
		DEF_LOG_ERROR("%1") << QString2TSString(FromAscii("属性构造器工厂未实现。"));

		return;
	}

	QListWidgetItem *item = PropWidgetFac->GetPropertyDialogUserData(dlg).value<QListWidgetItem *>();
	TSVariant val = PropWidgetFac->GetPropertyWidget(dlg)->GetCreateParams()->Variant;

	if (!_p->_AllowRepeatedValue)
	{
		if (TSNObjectPtr objPtr = val.Value<TSNObjectPtr>())
		{
			for (int j = 0; j < ui->listWidget_Main->count(); j++)
			{
				TSVariant curVal = ui->listWidget_Main->item(j)->data(ROW_DATA_INDEX).value<TSVariant>();

				if (TSNObjectPtr curObjPtr = curVal.Value<TSNObjectPtr>())
				{
					if (curObjPtr->GetName() == objPtr->GetName())
					{
						QMessageBox::information(QApplication::activeWindow(), FromAscii("提示信息"), FromAscii("名称已经存在，请重新输入！"));
						return;
					}
				}
			}
		}
	}

	if (!item)
	{
		item = new QListWidgetItem();
		ui->listWidget_Main->addItem(item);
	}

	FillArrayItem(item, val);
}

bool HRArrayPropertyPanel::DefaultShowLable() const
{
	return false;
}

