#ifndef TSPROPERTYGROUPWIDGET_H__
#define TSPROPERTYGROUPWIDGET_H__

#include <QWidget>
#include <QVBoxLayout>

#include "HRCheckableGroupBox.h"
#include "HRPropertyWidget.h"

class QFormLayout;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	�������ؼ���.  </summary>
///
/// <remarks>	������, 2016/5/13. </remarks>
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
	/// <summary>	�����ӿؼ�. </summary>
	///
	/// <remarks>	������, 2016/5/13. </remarks>
	///
	/// <param name="createParam">	��������. </param>
	/// <param name="val">		  	����ֵ. </param>
	///
	/// <returns>	null if it fails, else. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	struct PropertyWidgetInfo
	{
		PropertyWidgetInfo() : widget(NULL), isBasicData(false) {}

		/// <summary> ��ǩ </summary>
		QString label;

		/// <summary> ��ʾ˳�� </summary>
		int order;

		/// <summary> �Ƿ��ǻ������� </summary>
		bool isBasicData;

		/// <summary> �������ؼ� </summary>
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
