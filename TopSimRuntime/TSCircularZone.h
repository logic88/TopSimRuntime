#ifndef TSPROPERTYGROUPWIDGET_H__
#define TSPROPERTYGROUPWIDGET_H__

#include <QWidget>
#include <QVBoxLayout>

#include "HRCheckableGroupBox.h"
#include "HRPropertyWidget.h"

class QFormLayout;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	构造器控件组.  </summary>
///
/// <remarks>	龙成亮, 2016/5/13. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

class HRUICOMMON_EXPORT HRPropertyWidgetGroup : public HRPropertyWidget
{
	Q_OBJECT
public:
	HRPropertyWidgetGroup(XPropertyCreateParamsPtr CreateParams);
	~HRPropertyWidgetGroup();

	static QFormLayout* WrapWidget( HRPropertyWidget *widget, const QString &label, 
		const XPropertyConfigPtr &config, bool isBasicData = false, bool inGroupBox = false);

	static QFormLayout* WrapWidget( QLayout *layout, const QString &label, 
		const XPropertyConfigPtr &config, bool isBasicData= false, bool inGroupBox = false );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	构建子控件. </summary>
	///
	/// <remarks>	龙成亮, 2016/5/13. </remarks>
	///
	/// <param name="createParam">	创建参数. </param>
	/// <param name="val">		  	创建值. </param>
	///
	/// <returns>	null if it fails, else. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	struct PropertyWidgetInfo
	{
		PropertyWidgetInfo() : widget(NULL), isBasicData(false) {}

		/// <summary> 标签 </summary>
		QString label;

		/// <summary> 显示顺序 </summary>
		int order;

		/// <summary> 是否是基础数据 </summary>
		bool isBasicData;

		/// <summary> 构造器控件 </summary>
		HRPropertyWidget *widget;
	};

	struct PropertyWidgetGroup
	{
		HRCheckableGroupBox * GroupBox;
		std::vector< std::pair< XPropertyConfigExPtr,PropertyWidgetInfo> >  WidgetInfos;
	};

	void BuildWidgets(const XPropertyCreateParamsPtr &createParam, TSVariant &val);
	void BuildWidgets(const XPropertyCreateParamsPtr &createParam);

	void AddWidget(const PropertyWidgetInfo &widgetInfo);

	const std::vector<PropertyWidgetInfo> &GetWidgets() const;

	void ClearWidgets();

	QFormLayout *CreateLayout(bool showLabel = true);

	void UpdateState();

protected:
	void Validate() OVER_RIDE;
	bool DefaultShowLable() const OVER_RIDE;
	void BindVariant(TSVariant &val) const;

protected slots:
	void OnValueChangedSlot(const HRPropertyWidget* widget, const TSVariant& val);

protected:
	void showEvent(QShowEvent *ev) OVER_RIDE;
	FRIEND_TEST(HRPropertyWidgetGroup, BasicTest);
	struct HRPropertyWidgetGroupPrivate;
	HRPropertyWidgetGroupPrivate* _p;
};
#endif // TSPROPERTYGROUPWIDGET_H__
