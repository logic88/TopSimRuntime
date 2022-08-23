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

	// ���ƻ���ڵ�����
	static void	HideActiveWindows(QList<QWidget *> &windowList);
	static void	RestoreActiveWindows(const QList<QWidget *> &windowList);

	// ������ֿؼ�
	static void ClearLayoutWidgets(QLayout *layout);

	// �ļ�������ȥ����,�����ڶ�������Ϊȥ����
	static int AssociateFileType(const TSString& ext, const TSString& prog = TSString());

	// ����ʾ��ȫ����ʾ�����б�
	static void MultiScreenMaximumShow(const QList<QWidget *>& widgets);

	// ����ת��Ϊƴ������
	static QString Chinese2Spell(const QString &chineseString);

	// ����ת��
	static void UnicodeToUTF8(char *utf, wchar_t* wszString);

	// �������߶εĽ��㣨��˹���꣩
	static bool CalCrossDot( const QLineF & FirstLine,
		const QLineF & SecondLine,QPointF & PtCross);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�����߶�������Ľ��㣨��˹���꣩e. </summary>
	///
	/// <remarks>	TSMF Team, 2016/3/26. </remarks>
	///
	/// <param name="Polygon"> 	�����. </param>
	/// <param name="Line">	   	�߶�. </param>
	/// <param name="PtCrosss">	[in,out] ����. </param>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static bool CalCrossLine(const QPolygonF & Polygon,
		const QLineF & Line,std::vector<QPointF> & PtCrosss);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary> �����߶��ھ����ڵĲ���. 
	/// 		  
	/// 		  ��������Ļ�ڻ����߶Σ����˵���Ļ�ⲿ��</summary>
	/// 
	/// <remarks>	huaru, 2015/6/19. </remarks>
	///
	/// <param name="Range">	   	����. </param>
	/// <param name="Line">		   	�߶�. </param>
	/// <param name="ContainsLine">	�����ڲ��ֵ��߶�. </param>
	///
	/// <returns> ������������߶β��� true, �߶�������޽��� false  . </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static bool LineInRange(const QRectF &Range,const QLineF &Line,QLineF &ContainsLine);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	����Ԫ���Է�����ĸ�. </summary>
	///
	/// <param name="N">		ϵ������������������ĸ���. </param>
	/// <param name="A[11]">	������󣬼�Ϊ���Է������ϵ�������ټ��ϵȺ��ұ�������ɵľ���Ҫ������ʽΪN*(N+1). </param>
	/// <param name="Xn">   	[in,out] ���б�. </param>
	/// <param name="Err">  	�����������. </param>
	///
	/// <returns>	ϵ�����������ʽ��������0����ʾ������������Է������޽�. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static DOUBLE Simul( const int N, DOUBLE A[11][11], 
		std::vector<DOUBLE> & Xn, const DOUBLE Err /*= EPS*/ );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�����Իع�������õ�����ʽ�ĸ���ϵ��. </summary>
	///
	/// <param name="X"> 	�Ա����������б�. </param>
	/// <param name="Y"> 	������������б�. </param>
	/// <param name="M"> 	���λع�Ӳ������б���ȡ�Ĳ��������. </param>
	/// <param name="N"> 	����ʽ�Ĵ���. </param>
	/// <param name="A0">	[in,out] ����ʽ�ĳ�����. </param>
	/// <param name="Bn">	[in,out] ����ʽ��������. </param>
	///
	/// <returns>	����. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static DOUBLE Regression( const std::vector<DOUBLE> &X, 
		const std::vector<DOUBLE> &Y, int M, int N, DOUBLE &A0, std::vector<DOUBLE> & Bn );

	static void FillTree(QTreeWidget* widget, const boost::property_tree::ptree &tree, QString filter, QString fileExt);

	static void FillTreeItem(QTreeWidgetItem* item, const boost::property_tree::ptree &tree, 
		QString filter, QString fileExt);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	��ȡ����λ�ø߶ȵı�ݷ��������õ��η���. </summary>
    ///
    /// <remarks>	TSMF Team, 2016/3/26. </remarks>
    ///
    /// <param name="Framework">	���η������ڿ��. </param>
    /// <param name="Lon">			���ȣ��ȣ�. </param>
    /// <param name="Lat">			γ�ȣ��ȣ�. </param>
    ///
    /// <returns>	�߶ȣ��ף�. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    static INT16 GetHeightInt(/*TSIFrameworkPtr Framework, */DOUBLE Lon, DOUBLE Lat );

	static void Month2Number(QString & timeString);
	
	static void AdjustParentsLayout(QWidget *childWidget);

	static QString SelectImplementTypeName(int typeId, const QString &dispInfo);

	static TSVariant CreateObjectInstance( int typeId, int typeFlag);

	static TSVariant CreateObjectInstance( const QString &className, int typeFlag);

	static void AdjustWidgetSize(QWidget *widget, int marginWidth = 0, int marginHeight = 0);

	static TSString GenerateTemplateName(const TSString& baseTemplName,int startNum);

	// ��ȡsin/cos����(36)
	static std::vector<FLOAT> GetCircleSplitSinArray();
	static std::vector<FLOAT> GetCircleSplitCosArray();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Revise ll. </summary>
	///
	/// <remarks>	������, 2014/12/26. </remarks>
	///
	/// <param name="llCenter">	[in]���ģ��ȣ� </param>
	/// <param name="orill">   	[in]����λ�ã��ȣ� </param>
	/// <param name="oridis">  	[in]������� </param>
	/// <param name="revdis">  	[in]ʵ�ʾ��� </param>
	///
	/// <returns>	�������λ�ã��ȣ� </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static TSVector2d ReviseLL(TSVector2d llCenter, TSVector2d orill, 
		double oridis, double revdis, bool useWgs84 = false);

	static QString ToAppRelativePath(const QString &path);

	static QLabel* CreateLabelWidget(const QString & labelName, const XPropertyConfigPtr &config);
	static QString GetProperLabelText(const XPropertyConfigPtr &config, const TSMetaProperty *prop);
	static int GetProperOrder(const XPropertyConfigPtr &config, int def = 1);
};

#endif


