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

	// 初始化应用骨架
	bool Initialize() OVER_RIDE;

	// 清理应用骨架
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

	// 创建action
	virtual void CreateActions() {}

	// 创建系统弹出菜单
	virtual void CreateSystemPopupBar(Qtitan::RibbonSystemPopupBar*);

	// 创建BackstageView
	virtual void CreateBackstageView(Qtitan::RibbonBackstageView*) {}

	// 创建快捷访问菜单
	virtual void CreateQuickAccessBar(Qtitan::RibbonQuickAccessBar*) {}

	// 创建工具条
	virtual void CreateRibbonBar(Qtitan::RibbonBar*) {}

	// 创建状态条
	virtual void CreateStatusBar(Qtitan::RibbonStatusBar*) {}

	enum SystemMenuStyle
	{
		kPopupBar,
		kBackstageView,
	};

	// 设置系统菜单样式
	void SetSystemMenuStyle(SystemMenuStyle style);

	// 设置是否启用ribbon工具条最小化
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
