#ifndef HRAPPSKELETON_H__
#define HRAPPSKELETON_H__

#include "HRUICommon.h"

#ifndef Q_MOC_RUN
#include <HRUICommon/HRUICommon.h>
#include <HRControls/HRMainWindow.h>
#include <HRUtil/HRSafeInvokeObject.h>

#include "HRIAppSkeleton.h"
#endif //Q_MOC_RUN

namespace Qtitan
{
	class RibbonSystemPopupBar;
	class RibbonBackstageView;
	class RibbonQuickAccessBar;
	class RibbonStatusBar;
}

class QStackedWidget;

class HRUICOMMON_EXPORT HRRibbonAppSkeleton : public HRIAppSkeleton
{
	Q_OBJECT
	TS_MetaType(HRRibbonAppSkeleton, HRIAppSkeleton);
	TS_NOCOPYABLE(HRRibbonAppSkeleton);
public:
	HRRibbonAppSkeleton(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~HRRibbonAppSkeleton();

	// ��ʼ��Ӧ�ùǼ�
	bool Initialize() OVER_RIDE;

	// ����Ӧ�ùǼ�
	void Cleanup() OVER_RIDE;

	Page *AddPage(const QString &title) OVER_RIDE;

	Page *GetOrAddPage(const QString &title) OVER_RIDE;

	void RemovePage(Page *page) OVER_RIDE;

	Page *GetPage(int index) OVER_RIDE;

	int GetPageCount() OVER_RIDE;

	void AddSystemAction(QAction *) OVER_RIDE;

	void UpdateRecentFileList(const QStringList &recentFiles) OVER_RIDE;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// override to initialize ribbon
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// ����action
	virtual void CreateActions() {}

	// ����ϵͳ�����˵�
	virtual void CreateSystemPopupBar(Qtitan::RibbonSystemPopupBar*);

	// ����BackstageView
	virtual void CreateBackstageView(Qtitan::RibbonBackstageView*) {}

	// ������ݷ��ʲ˵�
	virtual void CreateQuickAccessBar(Qtitan::RibbonQuickAccessBar*) {}

	// ����������
	virtual void CreateRibbonBar(Qtitan::RibbonBar*) {}

	// ����״̬��
	virtual void CreateStatusBar(Qtitan::RibbonStatusBar*) {}

	enum SystemMenuStyle
	{
		kPopupBar,
		kBackstageView,
	};

	// ����ϵͳ�˵���ʽ
	void SetSystemMenuStyle(SystemMenuStyle style);

	// �����Ƿ�����ribbon��������С��
	void SetRibbonMinActionEnabled(bool);

signals:
	void UpdateRecentFileListSignal(const QStringList&);

protected slots:
	void OnToggleMaximizeSlot();

	void OnMinimizationChangedSlot(bool minimized);

	void OnCurrentPageChangedSlot( int index );

	void OnOpenRecentFileSlot(const QString& filePath);

	void OnAboutActionSlot();

	void OnTimeoutSlot();

private:
	struct HRRibbonAppSkeletonPrivate;
	HRRibbonAppSkeletonPrivate* _p;
};

#endif // HRAPPSKELETON_H__
