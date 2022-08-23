#include "stdafx.h"
#include "HRRibbonAppSkeleton.h"

#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QLineEdit>
#include <HRControls/HRInputDialog.h>

#include <TopSimRuntime/TSConversion.h>

#include <QtnRibbonBar.h>
#include <QtnRibbonPage.h>
#include <QtnRibbonGroup.h>
#include <QtnRibbonSliderPane.h>
#include <QtnRibbonStatusBar.h>
#include <QtnRibbonBackstageView.h>
#include <QtnRibbonSystemPopupBar.h>
#include <QtnRibbonQuickAccessBar.h>
#include <QtnHuaruHelper.h>

#include "HRUserConfig.h"
#include "HRUIUtil.h"
#include <HRUtil/HREventBus.h>
#include "HRAppManager.h"
#include "HRAppConfig.h"
#include "QtnHuaruHelper.h"
#include "HREventParams.h"
#include "HREventDecl.h"

BEGIN_METADATA(HRRibbonAppSkeleton)
	REG_BASE(HRIAppSkeleton);
END_METADATA()

struct RibbonSkeletonGroup : public HRIAppSkeleton::Group
{
	RibbonSkeletonGroup(Qtitan::RibbonGroup *group_)
	{
		group = group_;
	}

	~RibbonSkeletonGroup() 
	{
		
	}
	void SetObjectName(const QString &Name)
	{
		group->setObjectName(Name);
	}

	Qtitan::RibbonGroup *GetGroup() const
	{
		return group;
	}

	QString GetTitle() OVER_RIDE
	{
		return group->title();
	}

	QAction *AddSeparator() OVER_RIDE
	{
		return group->addSeparator();
	}

	void AddAction(QAction *act, Qt::ToolButtonStyle style = Qt::ToolButtonTextUnderIcon, QMenu* menu = Q_NULL) OVER_RIDE
	{
		group->addAction(act, style,menu);
	}

	QAction *AddAction(const QIcon &icon, const QString &text, Qt::ToolButtonStyle style = Qt::ToolButtonTextUnderIcon,QMenu* menu = Q_NULL) OVER_RIDE
	{
		return group->addAction(icon, text, style,menu);
	}

	QMenu *AddMenu(const QIcon &icon, const QString &text, Qt::ToolButtonStyle style /* = Qt::ToolButtonTextUnderIcon */) OVER_RIDE
	{
		return AddMenu("",icon, text, style);
	}
	QMenu *AddMenu(const QString &name,const QIcon &icon, const QString &text, Qt::ToolButtonStyle style = Qt::ToolButtonTextUnderIcon) OVER_RIDE
	{
		return group->addMenu(name,icon, text, style);
	}

	void RemoveAction(QAction *act) OVER_RIDE
	{
		group->removeAction(act);
	}

	int GetActionCount() OVER_RIDE
	{
		return group->actions().count();
	}

	void Clear() OVER_RIDE
	{
		group->clear();
	}

	QAction *GetOrAddOptionAction() OVER_RIDE
	{
		group->setOptionButtonVisible(true);
		return group->optionButtonAction();
	}

	void AddWidget(QWidget *w) OVER_RIDE
	{
		group->addWidget(w);
	}

	void RemoveWidget(QWidget *w) OVER_RIDE
	{
		group->remove(w);
	}

	void SetVisible(bool visible) OVER_RIDE
	{
		group->setVisible(visible);
	}

	bool IsActionsHide() const
	{
		QList<QAction*> acts = group->actions();

		for (int i=0;i<acts.size(); ++i)
		{
			if (acts[i]->isVisible())
			{
				return false;
			}
		}

		return true;
	}

	void UpdateAsHide()
	{
		if (!IsActionsHide() != group->isVisible())
		{
			SetVisible(!IsActionsHide());
		}
	}

private:
	Qtitan::RibbonGroup *group;
};

struct RibbonSkeletonPage : public HRIAppSkeleton::Page
{
	RibbonSkeletonPage(const QString &title)
	{
		page = new Qtitan::RibbonPage(0, title);
	}

	~RibbonSkeletonPage()
	{
		for (std::vector<RibbonSkeletonGroup *>::iterator it = groups.begin(); it != groups.end(); ++it)
		{
			delete *it;
		}
		groups.clear();
	}

	Qtitan::RibbonPage *GetPage() const
	{
		return page;
	}

	QString GetTitle() OVER_RIDE
	{
		return page->title();
	}

	HRIAppSkeleton::Group *AddGroup(const QString &title) OVER_RIDE
	{
		Qtitan::RibbonGroup *group_ = page->addGroup(title);
		RibbonSkeletonGroup *group = new RibbonSkeletonGroup(group_);
		groups.push_back(group);
		return group;
	}

	HRIAppSkeleton::Group *GetOrAddGroup(const QString &title) OVER_RIDE
	{
		for (std::vector<RibbonSkeletonGroup *>::iterator it = groups.begin(); it != groups.end(); ++it)
		{
			if ((*it)->GetTitle() == title)
			{
				return *it;
			};
		}
		return AddGroup(title);
	}

	void RemoveGroup(HRIAppSkeleton::Group *group) OVER_RIDE
	{
		for (std::vector<RibbonSkeletonGroup *>::const_iterator it = groups.begin(); it != groups.end(); ++it)
		{
			if ((*it) == group)
			{
				page->removeGroup(reinterpret_cast<RibbonSkeletonGroup *>(group)->GetGroup());
				delete *it;
				groups.erase(it);
				return;
			}
		}
	}

	HRIAppSkeleton::Group *GetGroup(int index) OVER_RIDE
	{
		SMART_ASSERT(page->groupCount() > index);
		return groups[index];
	}

	int GetGroupCount() OVER_RIDE
	{
		return page->groupCount();
	}

	void SetVisible(bool visible)
	{
		page->setVisible(visible);
	}

	bool IsGroupsHide() const
	{
		for (int i=0; i<groups.size(); ++i)
		{
			if (!groups[i]->IsActionsHide())
			{
				return false;
			}
		}

		return true;
	}

	void UpdateAsHide()
	{
		for (int i=0; i<groups.size(); ++i)
		{
			groups[i]->UpdateAsHide();
		}

		if (!IsGroupsHide() != page->isVisible())
		{
			page->setVisible(!IsGroupsHide());
		}
	}

private:

	Qtitan::RibbonPage *page;
	std::vector<RibbonSkeletonGroup *> groups;
};

struct HRRibbonAppSkeleton::HRRibbonAppSkeletonPrivate
{
	HRRibbonAppSkeletonPrivate()
		:_SystemMenuStyle(HRRibbonAppSkeleton::kPopupBar)
		,_RibbonMinActionEnabled(true)
		,_ActionRibbonMinimize(0)
	{
	}

	HRRibbonAppSkeleton::SystemMenuStyle _SystemMenuStyle;
	bool _RibbonMinActionEnabled;
	QAction *_ActionRibbonMinimize;
	std::vector<RibbonSkeletonPage *> pages;
	QAction *_SystemSeperator;
	QTimer Timer;
};

HRRibbonAppSkeleton::HRRibbonAppSkeleton(QWidget *parent, Qt::WindowFlags flags)
	: HRIAppSkeleton(parent, flags), _p(new HRRibbonAppSkeletonPrivate()) 
{
	InitializeRibbon();
}

HRRibbonAppSkeleton::~HRRibbonAppSkeleton() 
{
	delete _p;
}

bool HRRibbonAppSkeleton::Initialize()
{
	// 创建action
	CreateActions();
	// 创建系统菜单
	if(QAction* systemMenuAction = ribbonBar()->addSystemButton(QtnHuaruThemeServices::Instance()->GetThemeResourceIcon(GetSystemMenuIcon()), GetSystemMenuText())) 
	{
		// 隐藏快速访问菜单
		ribbonBar()->getQuickAccessBar()->hide();

		systemMenuAction->setToolTip(GetSystemTooltip());

		switch (_p->_SystemMenuStyle)
		{
		case kPopupBar:
			{
				if (Qtitan::RibbonSystemPopupBar* popupBar = 
					qobject_cast<Qtitan::RibbonSystemPopupBar*>(systemMenuAction->menu()))
				{
					popupBar->clear();
					if (GetRecentFileListEnabled())
					{
						if (Qtitan::RibbonPageSystemRecentFileList* pageRecentFile = popupBar->addPageRecentFile(FromAscii("最近使用的文件")))
						{
							pageRecentFile->setSize(9);
							connect(pageRecentFile, SIGNAL(openRecentFile(const QString&)), this, SLOT(OnOpenRecentFileSlot(const QString&)));				
							connect(this, SIGNAL(UpdateRecentFileListSignal(const QStringList&)),pageRecentFile, SLOT(updateRecentFileActions(const QStringList&)));
						}
					}
					CreateSystemPopupBar(popupBar);
				}
				break;
			}
		case kBackstageView:
			{
				CreateBackstageView(new Qtitan::RibbonBackstageView(ribbonBar()));
				break;
			}
		}
	}

	// 创建快捷访问工具条
	if (Qtitan::RibbonQuickAccessBar* quickAccessBar = ribbonBar()->quickAccessBar())
	{
		CreateQuickAccessBar(quickAccessBar);
	}

	connect(ribbonBar(), SIGNAL(currentPageChanged(int)), SLOT(OnCurrentPageChangedSlot(int)));

	// 创建ribbon面板
	CreateRibbonBar(ribbonBar());

	if (GetAboutActionEnabled())
	{
		QAction *aboutAct = ribbonBar()->addAction(QtnHuaruThemeServices::Instance()->GetThemeResourceIcon("ribbon_aboutbutton_normal.png"), 
			FromAscii("关于"), Qt::ToolButtonIconOnly);
		aboutAct->setToolTip(FromAscii("显示XSimStudio的功能概述"));
		connect(aboutAct, SIGNAL(triggered()), SLOT(OnAboutActionSlot()));	
	}

	//ribbon min/max action
	if (_p->_RibbonMinActionEnabled)
	{
		_p->_ActionRibbonMinimize = ribbonBar()->addAction(QtnHuaruThemeServices::Instance()->GetThemeResourceIcon("ribbon_minimizebutton_normal.png"), 
			FromAscii("最小化功能区"), Qt::ToolButtonIconOnly);
		_p->_ActionRibbonMinimize->setStatusTip(FromAscii("只显示Ribbon页的名称"));
		_p->_ActionRibbonMinimize->setShortcut(FromAscii("Alt+F1"));
		connect(_p->_ActionRibbonMinimize, SIGNAL(triggered()), this, SLOT(OnToggleMaximizeSlot()));
		connect(ribbonBar(), SIGNAL(minimizationChanged(bool)), this, SLOT(OnMinimizationChangedSlot(bool)));
	}

	if (ribbonBar()->getPageCount() > 0)
	{
		HRCurrPageChangedParamPtr param = boost::make_shared<HRCurrPageChangedParam>();
		param->page = GetPage(0);
		HREventBus::Instance()->Post(SE_UI_CURR_PAGE_CHANGED, TSVariant::FromValue(param));

		if (IsExclusivePage(param->page->GetTitle()))
		{
			HREventBus::Instance()->Post(SE_UI_EXCLUSIVE_PAGE_CHANGED, TSVariant::FromValue(QString2TSString(param->page->GetTitle())));
		}
	}

	if (GetStatusBarEnabled())
	{
		Qtitan::RibbonStatusBar* statusBar = new Qtitan::RibbonStatusBar();
		setStatusBar(statusBar);
	}

	if (!HRIAppSkeleton::Initialize())
	{
		return false;
	}

	connect(&_p->Timer, SIGNAL(timeout()), this, SLOT(OnTimeoutSlot()));
	_p->Timer.start(500);  

	return true;
}

void HRRibbonAppSkeleton::OnAboutActionSlot()
{
	ShowAboutDialog();
}

void HRRibbonAppSkeleton::OnToggleMaximizeSlot()
{
	ribbonBar()->setMinimized(!ribbonBar()->isMinimized());
}

void HRRibbonAppSkeleton::OnMinimizationChangedSlot(bool minimized)
{
	_p->_ActionRibbonMinimize->setChecked(minimized);
	_p->_ActionRibbonMinimize->setIcon(minimized ? QtnHuaruThemeServices::Instance()->GetThemeResourceIcon("ribbon_maximizebutton_normal.png") :  
		QtnHuaruThemeServices::Instance()->GetThemeResourceIcon("ribbon_minimizebutton_normal.png"));
}

void HRRibbonAppSkeleton::Cleanup()
{
	HRIAppSkeleton::Cleanup();

	for (std::vector<RibbonSkeletonPage *>::iterator it = _p->pages.begin(); it != _p->pages.end(); ++it)
	{
		delete *it;
	}
	_p->pages.clear();
}

void HRRibbonAppSkeleton::CreateSystemPopupBar( Qtitan::RibbonSystemPopupBar* popupbar)
{
	_p->_SystemSeperator = popupbar->addSeparator();

	//QAction * configAct = new QAction(QtnHuaruThemeServices::Instance()->GetThemeResourceIcon("actconfig.png"), TSString2QString(HR_STR_RES(SR_RIBBON_ACTION_CONFIG)), popupbar);
	//popupbar->addAction(configAct);
	//
	//connect(configAct, SIGNAL(triggered()), SLOT(OnConfigAppSlot()));

	QAction * exitAct = new QAction(QtnHuaruThemeServices::Instance()->GetThemeResourceIcon("exitact.png"), TSString2QString(/*HR_STR_RES(SR_RIBBON_ACTION_EXIT)*/toAsciiData("退出")), popupbar);
	exitAct->setShortcut(FromAscii("Ctrl+Q"));
	popupbar->addAction(exitAct);

	connect(exitAct, SIGNAL(triggered()), SLOT(close()));

// 	popupbar->addPopupBarAction(exitAct, Qt::ToolButtonTextBesideIcon);
// 	popupbar->addPopupBarAction(configAct, Qt::ToolButtonTextBesideIcon);
}

void HRRibbonAppSkeleton::OnOpenRecentFileSlot( const QString& filePath )
{
	EmitOpenRecentFileEvent(filePath);
}

void HRRibbonAppSkeleton::OnCurrentPageChangedSlot( int index )
{
	if (ribbonBar()->getPageCount() > index)
	{
		HRCurrPageChangedParamPtr param = boost::make_shared<HRCurrPageChangedParam>();
		param->page = GetPage(index);
		HREventBus::Instance()->Post(SE_UI_CURR_PAGE_CHANGED, TSVariant::FromValue(param));

		if (IsExclusivePage(param->page->GetTitle()))
		{
			HREventBus::Instance()->Post(SE_UI_EXCLUSIVE_PAGE_CHANGED, TSVariant::FromValue(QString2TSString(param->page->GetTitle())));
		}
	}
}

void HRRibbonAppSkeleton::SetSystemMenuStyle( SystemMenuStyle style )
{
	_p->_SystemMenuStyle = style;
}

void HRRibbonAppSkeleton::SetRibbonMinActionEnabled( bool val )
{
	_p->_RibbonMinActionEnabled = val;
}

HRRibbonAppSkeleton::Page * HRRibbonAppSkeleton::AddPage( const QString &title )
{
	RibbonSkeletonPage *page = new RibbonSkeletonPage(title);
	_p->pages.push_back(page);
	ribbonBar()->addPage(page->GetPage());
	return page;
}

HRRibbonAppSkeleton::Page * HRRibbonAppSkeleton::GetOrAddPage( const QString &title )
{
	for (std::vector<RibbonSkeletonPage *>::iterator it = _p->pages.begin(); it != _p->pages.end(); ++it)
	{
		if ((*it)->GetTitle() == title)
		{
			return *it;
		}
	}
	return AddPage(title);
}

void HRRibbonAppSkeleton::RemovePage( Page *page )
{
	std::vector<RibbonSkeletonPage *>::iterator it = std::find(_p->pages.begin(), _p->pages.end(), page);
	if (it != _p->pages.end())
	{
		RibbonSkeletonPage *page = *it;
		_p->pages.erase(it);
		ribbonBar()->removePage(page->GetPage());
		delete page;
	}
}

HRRibbonAppSkeleton::Page * HRRibbonAppSkeleton::GetPage( int index )
{
	if ((std::size_t)index < _p->pages.size())
	{
		return _p->pages[index];
	}
	return 0;
}

int HRRibbonAppSkeleton::GetPageCount()
{
	return (int)_p->pages.size();
}

void HRRibbonAppSkeleton::UpdateRecentFileList( const QStringList &recentFiles )
{
	emit UpdateRecentFileListSignal(recentFiles);
}

void HRRibbonAppSkeleton::AddSystemAction( QAction *act )
{
	if (Qtitan::RibbonSystemButton* systemButton = ribbonBar()->getSystemButton())
	{
		if (Qtitan::RibbonSystemPopupBar* popupBar = 
			qobject_cast<Qtitan::RibbonSystemPopupBar*>(systemButton->defaultAction()->menu()))
		{
			popupBar->insertAction( _p->_SystemSeperator, act);
		}
	}
}

void HRRibbonAppSkeleton::OnTimeoutSlot()
{
	for (int i=0; i<_p->pages.size(); ++i)
	{
		_p->pages[i]->UpdateAsHide();
	}
}
