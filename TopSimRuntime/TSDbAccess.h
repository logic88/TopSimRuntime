#ifndef __HRUIUtil_H__
#define __HRUIUtil_H__

#include <QIcon>
#include <QWidget>
#include <QString>
#include <QDateTime>

#include <boost/property_tree/ptree.hpp>

#ifdef FHANDLE
#undef FHANDLE
#endif

#ifdef _WIN32
#include <Windows.h>
#define FHANDLE	HANDLE 
#else
#include <stdio.h>
#include <fcntl.h>
#define FHANDLE int
#endif

#include <HRUtil/HRUtil.h>
#include "HRUICommon.h" 

class QWidget;
class QLayout;
class QTreeWidget;
class QTreeWidgetItem;
class QLabel;

class HRUICOMMON_EXPORT HRUIUtil
{
public:
	static bool IsVailidString(const QString &inputString);
	static bool IsVailidString(const TSString &inputString);
	static bool IsVaildName(const TSString& strName, bool bIsInfo = true); 

	static QWidget *GetRootWidget(QWidget* widget);
	static QDateTime TSTimeToDateTime(const TSTime& time);

	template <class T>
	static bool ActiveTopLevelWidget(QWidget * widget)
	{
		QWidget * ParentWidget = widget->parentWidget();

		while (ParentWidget && ParentWidget->parentWidget())
		{
			ParentWidget = ParentWidget->parentWidget();
		}

		T * ActiveWindow = qobject_cast<T *>(ParentWidget);

		if (ActiveWindow)
		{
			ActiveWindow->activateWindow();

			return true;
		}

		return false;
	}

	static DOUBLE NormalizeAngle(DOUBLE); //ensure angle between 0~360
	static DOUBLE NormalizeAngleWithin180(DOUBLE angle, DOUBLE against); //ensure diff < 180

	// 控制活动窗口的显隐
	static void	HideActiveWindows(QList<QWidget *> &windowList);
	static void	RestoreActiveWindows(const QList<QWidget *> &windowList);

	// 清除布局控件
	static void ClearLayoutWidgets(QLayout *layout);

	// 文件关联和去关联,不带第二个参数为去关联
	static int AssociateFileType(const TSString& ext, const TSString& prog = TSString());

	// 多显示器全屏显示窗口列表
	static void MultiScreenMaximumShow(const QList<QWidget *>& widgets);

	// 汉字转换为拼音函数
	static QString Chinese2Spell(const QString &chineseString);

	// 编码转换
	static void UnicodeToUTF8(char *utf, wchar_t* wszString);

	// 计算两线段的交点（高斯坐标）
	static bool CalCrossDot( const QLineF & FirstLine,
		const QLineF & SecondLine,QPointF & PtCross);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	计算线段与区域的交点（高斯坐标）e. </summary>
	///
	/// <remarks>	TSMF Team, 2016/3/26. </remarks>
	///
	/// <param name="Polygon"> 	多边形. </param>
	/// <param name="Line">	   	线段. </param>
	/// <param name="PtCrosss">	[in,out] 交点. </param>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static bool CalCrossLine(const QPolygonF & Polygon,
		const QLineF & Line,std::vector<QPointF> & PtCrosss);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary> 计算线段在矩形内的部分. 
	/// 		  
	/// 		  用于在屏幕内绘制线段，过滤掉屏幕外部分</summary>
	/// 
	/// <remarks>	huaru, 2015/6/19. </remarks>
	///
	/// <param name="Range">	   	矩形. </param>
	/// <param name="Line">		   	线段. </param>
	/// <param name="ContainsLine">	矩形内部分的线段. </param>
	///
	/// <returns> 计算出矩形内线段部分 true, 线段与矩形无交集 false  . </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static bool LineInRange(const QRectF &Range,const QLineF &Line,QLineF &ContainsLine);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	求解多元线性方程组的根. </summary>
	///
	/// <param name="N">		系数增广矩阵的行数，解的个数. </param>
	/// <param name="A[11]">	增广矩阵，即为线性方程组的系数矩阵再加上等号右边那列组成的矩阵，要求矩阵格式为N*(N+1). </param>
	/// <param name="Xn">   	[in,out] 解列表. </param>
	/// <param name="Err">  	计算允许误差. </param>
	///
	/// <returns>	系数矩阵的行列式，若返回0，表示参数错误或线性方程组无解. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static DOUBLE Simul( const int N, DOUBLE A[11][11], 
		std::vector<DOUBLE> & Xn, const DOUBLE Err /*= EPS*/ );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	非线性回归分析，得到多项式的各项系数. </summary>
	///
	/// <param name="X"> 	自变量采样点列表. </param>
	/// <param name="Y"> 	因变量采样点列表. </param>
	/// <param name="M"> 	本次回归从采样点列表中取的采样点个数. </param>
	/// <param name="N"> 	多项式的次数. </param>
	/// <param name="A0">	[in,out] 多项式的常数项. </param>
	/// <param name="Bn">	[in,out] 多项式的其他项. </param>
	///
	/// <returns>	方差. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static DOUBLE Regression( const std::vector<DOUBLE> &X, 
		const std::vector<DOUBLE> &Y, int M, int N, DOUBLE &A0, std::vector<DOUBLE> & Bn );

	static void FillTree(QTreeWidget* widget, const boost::property_tree::ptree &tree, QString filter, QString fileExt);

	static void FillTreeItem(QTreeWidgetItem* item, const boost::property_tree::ptree &tree, 
		QString filter, QString fileExt);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	获取所在位置高度的便捷方法（调用地形服务）. </summary>
    ///
    /// <remarks>	TSMF Team, 2016/3/26. </remarks>
    ///
    /// <param name="Framework">	地形服务所在框架. </param>
    /// <param name="Lon">			精度（度）. </param>
    /// <param name="Lat">			纬度（度）. </param>
    ///
    /// <returns>	高度（米）. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    static INT16 GetHeightInt(/*TSIFrameworkPtr Framework, */DOUBLE Lon, DOUBLE Lat );

	static void Month2Number(QString & timeString);
	
	static void AdjustParentsLayout(QWidget *childWidget);

	static QString SelectImplementTypeName(int typeId, const QString &dispInfo);

	static TSVariant CreateObjectInstance( int typeId, int typeFlag);

	static TSVariant CreateObjectInstance( const QString &className, int typeFlag);

	static void AdjustWidgetSize(QWidget *widget, int marginWidth = 0, int marginHeight = 0);

	static TSString GenerateTemplateName(const TSString& baseTemplName,int startNum);

	// 获取sin/cos数组(36)
	static std::vector<FLOAT> GetCircleSplitSinArray();
	static std::vector<FLOAT> GetCircleSplitCosArray();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Revise ll. </summary>
	///
	/// <remarks>	龙成亮, 2014/12/26. </remarks>
	///
	/// <param name="llCenter">	[in]中心（度） </param>
	/// <param name="orill">   	[in]估算位置（度） </param>
	/// <param name="oridis">  	[in]估算距离 </param>
	/// <param name="revdis">  	[in]实际距离 </param>
	///
	/// <returns>	修正后的位置（度） </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static TSVector2d ReviseLL(TSVector2d llCenter, TSVector2d orill, 
		double oridis, double revdis, bool useWgs84 = false);

	static QString ToAppRelativePath(const QString &path);

	static QLabel* CreateLabelWidget(const QString & labelName, const XPropertyConfigPtr &config);
	static QString GetProperLabelText(const XPropertyConfigPtr &config, const TSMetaProperty *prop);
	static int GetProperOrder(const XPropertyConfigPtr &config, int def = 1);
};

#endif


