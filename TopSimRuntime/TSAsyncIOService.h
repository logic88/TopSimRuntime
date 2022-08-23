#ifndef HRComplexPropertyPanel_H
#define HRComplexPropertyPanel_H

#include <QVBoxLayout>

#include "HRPropertyWidget.h"
#include "HRPropertyWidgetGroup.h"

class HRUICOMMON_EXPORT HRComplexPropertyPanel : public HRPropertyWidgetGroup
{
	Q_OBJECT
public:
	HRComplexPropertyPanel(XPropertyCreateParamsPtr CreateParams);
	~HRComplexPropertyPanel();
protected:
	void Validate() OVER_RIDE;
	TSVariant _Variant;
};

#endif // HRComplexPropertyPanel_H
