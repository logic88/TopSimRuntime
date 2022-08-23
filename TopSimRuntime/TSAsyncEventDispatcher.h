#ifndef HRComplexLinkButton_H
#define HRComplexLinkButton_H

#include <QCommandLinkButton>
#include "HRPropertyWidget.h"
#include "HRUICommon.h"

class QDialog;
class QToolButton;
class QLabel;

class HRUICOMMON_EXPORT HRComplexLinkButton : public HRPropertyWidget
{
	Q_OBJECT
public:
	HRComplexLinkButton(XPropertyCreateParamsPtr CreateParams);
	~HRComplexLinkButton();
	virtual void Validate();
protected slots:
	virtual void OnlinkActivated(const QString &);
	void OnDialogFinished ( int result );
	QString GetText();
	TSVariant GetPropertyVar();

protected:
	QLabel *_Widget;
	bool _Saved;
	QDialog * _PopDialog;
};

#endif // HRComplexLinkButton_H
