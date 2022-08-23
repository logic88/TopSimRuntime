#include "stdafx.h"

#include <boost/regex.hpp>
//#include <boost/math/special_functions.hpp>

#include <QApplication>
#include <QDesktopWidget>
#include <QIcon>
#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <HRControls/HRMessageBox.h>
#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QTextStream>
#include <QLayout>
#include <QScrollArea>
#include <QListWidgetItem>
#include <QLabel>
#include <private/qlayoutengine_p.h>
#include <QDebug>

#include <TopSimRuntime/TSConversion.h>
#ifdef JSIM_SDK
#include "TopSimRuntime/TSCoordinate.h"
#endif

#include <HRControls/HRStringListDialog.h>

#include "HRAppConfig.h"
#include "HRUIUtil.h"
#include "HRPropertyWidget.h"

bool HRUIUtil::IsVailidString(const QString &inputString )
{
	return IsVailidString(QString2TSString(inputString));
}

bool HRUIUtil::IsVailidString( const TSString &inputString )
{
	boost::regex expression("[*\\/~!@#$%^&+-]+");
	std::string strName = TSString2Ascii(inputString);

	if (strName.empty())
	{
		return false;
	}

	if(boost::regex_search(strName, expression)) 
	{
		return false;
	}

	return true;
}
 
QDateTime HRUIUtil::TSTimeToDateTime(const TSTime &time)
{
	return QDateTime::fromTime_t((time - boost::posix_time::ptime(boost::gregorian::date(1970,1,1))).total_seconds());
}

void HRUIUtil::HideActiveWindows( QList<QWidget *> &windowList )
{
	windowList.clear();
	QWidget * w = qApp->activeWindow();
	Qt::WindowType wt = w->windowType(); 

	QMainWindow * mainWindow = qobject_cast<QMainWindow*>(w);

	while (w &&( mainWindow == NULL) )
	{	
		if ( wt == Qt::Dialog || wt == Qt::Window)
		{
			w->hide();
			windowList.push_back(w);
		}

		w = w->parentWidget();
		mainWindow = qobject_cast<QMainWindow*>(w);		
		if (w)
		{
			wt = w->windowType();
		}
	}
}

void HRUIUtil::RestoreActiveWindows( const QList<QWidget *> &windowList )
{
	for (int i = windowList.size()-1; i >= 0; i--)
	{
		windowList[i]->show();		 
	}

	if (windowList.size() > 0)
	{
		qApp->setActiveWindow(windowList[0]);	 
	}
}

#include <QProcess>
#include <QDir>
#ifdef _WIN32
#include <ShlObj.h>
#endif
int HRUIUtil::AssociateFileType( const TSString &ext, const TSString &prog /*= TSString()*/ )
{
	/*QString assocCmd = QString("cmd.exe /C \"assoc %1=xsim%1 && ftype xsim%1=\"%2\" \"%3\"\"")
						.arg(ext.c_str())
						.arg(QDir::toNativeSeparators(prog.c_str()))
						.arg("%1");*/
#ifdef _WIN32
	QString assocCmd;
	if (prog.empty())
	{
		assocCmd = QString("cmd.exe /C \"REG DELETE HKCU\\Software\\Classes\\%1 /f && "
			"REG DELETE HKCU\\Software\\Classes\\XSIM%2 /f\"")
			.arg(TSString2QString(ext))
			.arg(TSString2QString(ext).toUpper());
	}
	else
	{
		assocCmd = QString("cmd.exe /C \"REG ADD HKCU\\Software\\Classes\\%1 /ve /t REG_SZ /d XSIM%2 /f && "
			"REG ADD HKCU\\Software\\Classes\\XSIM%2\\shell\\open\\command /ve /t REG_SZ /d \"%3\" /f\"")
			.arg(TSString2QString(ext))
			.arg(TSString2QString(ext).toUpper())
			.arg(QString("\"\"\"") + QDir::toNativeSeparators(TSString2QString(prog)) + "\"\"\" \"\"\"%1\"\"\"");
	}

	int ret = QProcess::execute(assocCmd);
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
#endif

	return 0;
}

void HRUIUtil::MultiScreenMaximumShow( const QList<QWidget *> & widgets )
{
	int screenCount = qApp->desktop()->screenCount();
	for (int i = 0; i < widgets.size(); ++i)
	{
		int index = i % screenCount;
		if (qApp->desktop()->isVirtualDesktop())
		{
			QRect screenRect = qApp->desktop()->availableGeometry(index);
			widgets[i]->resize(screenRect.size());
			widgets[i]->move(screenRect.topLeft());
			widgets[i]->showMaximized();
		}
		else
		{
			widgets[i]->setParent(qApp->desktop()->screen(index));
			widgets[i]->showMaximized();
		}
	}
}

bool HRUIUtil::IsVaildName( const TSString & strName,bool bIsInfo /*= true*/ )
{
	boost::regex expression("[*\\\\/~!@#$%^&+]+"); 

	if (!strName.empty())
	{
		if(boost::regex_search(TSString2Ascii(strName), expression)) 
		{
			if(bIsInfo)
			{
				HRMessageBox::information(QApplication::activeWindow(),FromAscii("提示信息"),FromAscii("名称不能包含特殊字符('*\\/~!@#$%^&+')!"));
			}
			return false;
		}
	}
	else
	{
		if(bIsInfo)
		{
			HRMessageBox::information(QApplication::activeWindow(),FromAscii("提示信息"),FromAscii("名称不能为空!"));
		}
		return false;
	}
	return true;
}

QString HRUIUtil::Chinese2Spell(const QString &chineseString)
{
	static const int pyValue[] = {
		-20319,-20317,-20304,-20295,-20292,-20283,-20265,-20257,-20242,-20230,-20051,-20036,
		-20032,-20026,-20002,-19990,-19986,-19982,-19976,-19805,-19784,-19775,-19774,-19763,
		-19756,-19751,-19746,-19741,-19739,-19728,-19725,-19715,-19540,-19531,-19525,-19515,
		-19500,-19484,-19479,-19467,-19289,-19288,-19281,-19275,-19270,-19263,-19261,-19249,
		-19243,-19242,-19238,-19235,-19227,-19224,-19218,-19212,-19038,-19023,-19018,-19006,
		-19003,-18996,-18977,-18961,-18952,-18783,-18774,-18773,-18763,-18756,-18741,-18735,
		-18731,-18722,-18710,-18697,-18696,-18526,-18518,-18501,-18490,-18478,-18463,-18448,
		-18447,-18446,-18239,-18237,-18231,-18220,-18211,-18201,-18184,-18183,-18181,-18012,
		-17997,-17988,-17970,-17964,-17961,-17950,-17947,-17931,-17928,-17922,-17759,-17752,
		-17733,-17730,-17721,-17703,-17701,-17697,-17692,-17683,-17676,-17496,-17487,-17482,
		-17468,-17454,-17433,-17427,-17417,-17202,-17185,-16983,-16970,-16942,-16915,-16733,
		-16708,-16706,-16689,-16664,-16657,-16647,-16474,-16470,-16465,-16459,-16452,-16448,
		-16433,-16429,-16427,-16423,-16419,-16412,-16407,-16403,-16401,-16393,-16220,-16216,
		-16212,-16205,-16202,-16187,-16180,-16171,-16169,-16158,-16155,-15959,-15958,-15944,
		-15933,-15920,-15915,-15903,-15889,-15878,-15707,-15701,-15681,-15667,-15661,-15659,
		-15652,-15640,-15631,-15625,-15454,-15448,-15436,-15435,-15419,-15416,-15408,-15394,
		-15385,-15377,-15375,-15369,-15363,-15362,-15183,-15180,-15165,-15158,-15153,-15150,
		-15149,-15144,-15143,-15141,-15140,-15139,-15128,-15121,-15119,-15117,-15110,-15109,
		-14941,-14937,-14933,-14930,-14929,-14928,-14926,-14922,-14921,-14914,-14908,-14902,
		-14894,-14889,-14882,-14873,-14871,-14857,-14678,-14674,-14670,-14668,-14663,-14654,
		-14645,-14630,-14594,-14429,-14407,-14399,-14384,-14379,-14368,-14355,-14353,-14345,
		-14170,-14159,-14151,-14149,-14145,-14140,-14137,-14135,-14125,-14123,-14122,-14112,
		-14109,-14099,-14097,-14094,-14092,-14090,-14087,-14083,-13917,-13914,-13910,-13907,
		-13906,-13905,-13896,-13894,-13878,-13870,-13859,-13847,-13831,-13658,-13611,-13601,
		-13406,-13404,-13400,-13398,-13395,-13391,-13387,-13383,-13367,-13359,-13356,-13343,
		-13340,-13329,-13326,-13318,-13147,-13138,-13120,-13107,-13096,-13095,-13091,-13076,
		-13068,-13063,-13060,-12888,-12875,-12871,-12860,-12858,-12852,-12849,-12838,-12831,
		-12829,-12812,-12802,-12607,-12597,-12594,-12585,-12556,-12359,-12346,-12320,-12300,
		-12120,-12099,-12089,-12074,-12067,-12058,-12039,-11867,-11861,-11847,-11831,-11798,
		-11781,-11604,-11589,-11536,-11358,-11340,-11339,-11324,-11303,-11097,-11077,-11067,
		-11055,-11052,-11045,-11041,-11038,-11024,-11020,-11019,-11018,-11014,-10838,-10832,
		-10815,-10800,-10790,-10780,-10764,-10587,-10544,-10533,-10519,-10331,-10329,-10328,
		-10322,-10315,-10309,-10307,-10296,-10281,-10274,-10270,-10262,-10260,-10256,-10254
	};

	static const char* pyName[] = {
		"A","Ai","An","Ang","Ao","Ba","Bai","Ban","Bang","Bao","Bei","Ben",
		"Beng","Bi","Bian","Biao","Bie","Bin","Bing","Bo","Bu","Ba","Cai","Can",
		"Cang","Cao","Ce","Ceng","Cha","Chai","Chan","Chang","Chao","Che","Chen","Cheng",
		"Chi","Chong","Chou","Chu","Chuai","Chuan","Chuang","Chui","Chun","Chuo","Ci","Cong",
		"Cou","Cu","Cuan","Cui","Cun","Cuo","Da","Dai","Dan","Dang","Dao","De",
		"Deng","Di","Dian","Diao","Die","Ding","Diu","Dong","Dou","Du","Duan","Dui",
		"Dun","Duo","E","En","Er","Fa","Fan","Fang","Fei","Fen","Feng","Fo",
		"Fou","Fu","Ga","Gai","Gan","Gang","Gao","Ge","Gei","Gen","Geng","Gong",
		"Gou","Gu","Gua","Guai","Guan","Guang","Gui","Gun","Guo","Ha","Hai","Han",
		"Hang","Hao","He","Hei","Hen","Heng","Hong","Hou","Hu","Hua","Huai","Huan",
		"Huang","Hui","Hun","Huo","Ji","Jia","Jian","Jiang","Jiao","Jie","Jin","Jing",
		"Jiong","Jiu","Ju","Juan","Jue","Jun","Ka","Kai","Kan","Kang","Kao","Ke",
		"Ken","Keng","Kong","Kou","Ku","Kua","Kuai","Kuan","Kuang","Kui","Kun","Kuo",
		"La","Lai","Lan","Lang","Lao","Le","Lei","Leng","Li","Lia","Lian","Liang",
		"Liao","Lie","Lin","Ling","Liu","Long","Lou","Lu","Lv","Luan","Lue","Lun",
		"Luo","Ma","Mai","Man","Mang","Mao","Me","Mei","Men","Meng","Mi","Mian",
		"Miao","Mie","Min","Ming","Miu","Mo","Mou","Mu","Na","Nai","Nan","Nang",
		"Nao","Ne","Nei","Nen","Neng","Ni","Nian","Niang","Niao","Nie","Nin","Ning",
		"Niu","Nong","Nu","Nv","Nuan","Nue","Nuo","O","Ou","Pa","Pai","Pan",
		"Pang","Pao","Pei","Pen","Peng","Pi","Pian","Piao","Pie","Pin","Ping","Po",
		"Pu","Qi","Qia","Qian","Qiang","Qiao","Qie","Qin","Qing","Qiong","Qiu","Qu",
		"Quan","Que","Qun","Ran","Rang","Rao","Re","Ren","Reng","Ri","Rong","Rou",
		"Ru","Ruan","Rui","Run","Ruo","Sa","Sai","San","Sang","Sao","Se","Sen",
		"Seng","Sha","Shai","Shan","Shang","Shao","She","Shen","Sheng","Shi","Shou","Shu",
		"Shua","Shuai","Shuan","Shuang","Shui","Shun","Shuo","Si","Song","Sou","Su","Suan",
		"Sui","Sun","Suo","Ta","Tai","Tan","Tang","Tao","Te","Teng","Ti","Tian",
		"Tiao","Tie","Ting","Tong","Tou","Tu","Tuan","Tui","Tun","Tuo","Wa","Wai",
		"Wan","Wang","Wei","Wen","Weng","Wo","Wu","Xi","Xia","Xian","Xiang","Xiao",
		"Xie","Xin","Xing","Xiong","Xiu","Xu","Xuan","Xue","Xun","Ya","Yan","Yang",
		"Yao","Ye","Yi","Yin","Ying","Yo","Yong","You","Yu","Yuan","Yue","Yun",
		"Za", "Zai","Zan","Zang","Zao","Ze","Zei","Zen","Zeng","Zha","Zhai","Zhan",
		"Zhang","Zhao","Zhe","Zhen","Zheng","Zhi","Zhong","Zhou","Zhu","Zhua","Zhuai","Zhuan",
		"Zhuang","Zhui","Zhun","Zhuo","Zi","Zong","Zou","Zu","Zuan","Zui","Zun","Zuo"
	};

	QRegExp regexp("^[\\x4e00-\\x9fa5]$");

	QString pyString = "";
	int chrAsc = 0;
	int i1 = 0;
	int i2 = 0;

	for(int i = 0; i<chineseString.length(); i++)
	{
		if(regexp.indexIn(chineseString.mid(i,1))> -1)
		{
			QByteArray ba = QString2TSString(chineseString.mid(i, 1)).c_str();
			QString codecedString;
			QTextStream out(&codecedString);
			out.setCodec("GB18030");
			out << ba.data();
			int i1 = (short)codecedString.at(0).unicode();
			int i2 = (short)codecedString.at(1).unicode();

			chrAsc = i1*256 + i2 - 65536;
			if(chrAsc > 0 && chrAsc < 160)
			{
				pyString += chineseString.mid(i, 1);
			}
			else
			{
				if(chrAsc == -9254)
				{
					pyString += "Zhen";
				}
				else
				{
					for(int j=395; j>=0; j--)
					{
						if(pyValue[j] <= chrAsc)
						{
							pyString += pyName[j];
							break;
						}
					}
				}
			}
		}
		else
		{
			pyString += chineseString.mid(i, 1);
		}
	}

	return pyString;
}

void HRUIUtil::UnicodeToUTF8(char *utf, wchar_t* wszString)  
{  
#ifdef _WIN32

	//预转换，得到所需空间的大小  
	int u8Len = ::WideCharToMultiByte(CP_UTF8, NULL, wszString, (int)wcslen(wszString), NULL, 0, NULL, NULL);
	//同上，分配空间要给'\0'留个空间  
	//UTF8虽然是Unicode的压缩形式，但也是多字节字符串，所以可以以char的形式保存  
	//转换    //unicode版对应的strlen是wcslen  
	::WideCharToMultiByte(CP_UTF8, NULL, wszString, (int)wcslen(wszString), utf, u8Len, NULL, NULL);
	//最后加上'\0'  
	utf[u8Len] = '\0';
#else
	TSWideString WString = wszString;
	TSUtf8String Utf8String = WideString2Utf8(WString);
	memcpy(utf, Utf8String.data(), Utf8String.size());
#endif
} 

void GetEquationParam( DOUBLE firstPointx, DOUBLE firstPointy, DOUBLE secondPointx, DOUBLE secondPointy, double& A, double& B, double& C )
{
	if(firstPointx == secondPointx)
	{
		A = 1;
		B = 0;
		C = -firstPointx;
	}
	else
	{
		B = 1;
		A = -(firstPointy - secondPointy) * 1.0 / (firstPointx - secondPointx);
		C = -(firstPointy + A * firstPointx);
	}
}

bool HRUIUtil::CalCrossDot( const QLineF & FirstLine,const QLineF & SecondLine,QPointF & PtCross)
{
	DOUBLE fjxA, fjxB, fjxC;
	DOUBLE curLineA, curLineB, curLineC;

	GetEquationParam(FirstLine.p1().x(), FirstLine.p1().y(), FirstLine.p2().x(), FirstLine.p2().y(), fjxA, fjxB, fjxC);
	GetEquationParam(SecondLine.p1().x(), SecondLine.p1().y(), SecondLine.p2().x(), SecondLine.p2().y(), curLineA, curLineB, curLineC);

	if (ABS(fjxA * curLineB - fjxB * curLineA) > 0.000001 && ABS(curLineA * fjxB - fjxA * curLineB) > 0.000001)
	{
		DOUBLE fjxDistance = FirstLine.length();
		DOUBLE toA = QLineF(FirstLine.p1(), PtCross).length();
		DOUBLE toB = QLineF(SecondLine.p2(), PtCross).length();

		if (fjxDistance >= toA && fjxDistance >= toB)
		{
			PtCross.setX((curLineC * fjxB - fjxC * curLineB) / (fjxA * curLineB - curLineA * fjxB));
			PtCross.setY((curLineC * fjxA - fjxC * curLineA) / (fjxB * curLineA - fjxA * curLineB));
			return true;
		}
	}
	return false;
}

bool HRUIUtil::CalCrossLine(const QPolygonF & Polygon,const QLineF & Line,std::vector<QPointF> & PtCrosss)
{
	if (Polygon.count() < 3)
	{
		return false;
	}
	QPolygonF::const_iterator itPrev = Polygon.constBegin();
	QPolygonF::const_iterator it = itPrev;
	for (it++; it != Polygon.constEnd(); itPrev++,it++)
	{
		QPointF PtCross;
		if (CalCrossDot(QLineF(*itPrev,*it),Line,PtCross))
		{
			PtCrosss.push_back(PtCross);
		}
	}
	return true;
}

void CrossInRange(const QRectF &Range,const QLineF &Line,std::vector<QPointF> &PtCrosss)
{
	QPointF Cross;
	//左边
	if(QLineF(Range.left(),Range.top(),Range.left(),Range.height()-0.000001).intersect(Line,&Cross)==QLineF::BoundedIntersection)
	{
		PtCrosss.push_back(Cross);
	}
	//下边
	if(QLineF(Range.left(),Range.height(),Range.width()-0.000001,Range.height()).intersect(Line,&Cross)==QLineF::BoundedIntersection)
	{
		PtCrosss.push_back(Cross);
	}
	//右边
	if(QLineF(Range.width(),0.000001,Range.width(),Range.height()).intersect(Line,&Cross)==QLineF::BoundedIntersection)
	{
		PtCrosss.push_back(Cross);
	}
	//上边
	if(QLineF(Range.left()+0.000001,Range.top(),Range.width(),Range.top()).intersect(Line,&Cross)==QLineF::BoundedIntersection)
	{
		PtCrosss.push_back(Cross);
	}
}

bool PointInRect(const QRectF &Range,const QPointF & point)
{
	if (Range.left() < point.x() && point.x() < Range.right() && Range.top() < point.y() && point.y() < Range.bottom())
	{
		return true;
	}
	return false;
}

bool HRUIUtil::LineInRange(const QRectF &Range,const QLineF &Line,QLineF &ContainsLine)
{
	std::vector<QPointF> PtCrosss;
	CrossInRange(Range,Line,PtCrosss);

	if (PointInRect(Range,Line.p1()))
	{
		if (PointInRect(Range,Line.p2()))
		{
			ContainsLine.setPoints(Line.p1(),Line.p2());
		}
		else
		{
			ContainsLine.setPoints(Line.p1(),PtCrosss[0]);
		}
	}
	else if (PointInRect(Range,Line.p2()))
	{
		ContainsLine.setPoints(PtCrosss[0],Line.p2());
	}
	else if (PtCrosss.size() > 1)
	{
		ContainsLine.setPoints(PtCrosss[0],PtCrosss[1]);
	}
	else
	{
		return false;
	}
	return true;
}

void HRUIUtil::ClearLayoutWidgets( QLayout* layout )
{
	if( !layout )
	{
		return;
	}

	QLayoutItem* item;
	while (layout->count() > 0)
	{
		item = layout->takeAt(0);
		if (item->widget())
		{
			delete item->widget();
		}
		else
		{
			ClearLayoutWidgets(item->layout());
		}
		delete item;
	}
}

DOUBLE HRUIUtil::NormalizeAngle( DOUBLE d)
{
	if (boost::math::isnan(d) || boost::math::isinf(d))
	{
		DEF_LOG_WARNING("%1") << "invalid angle when NormalizeAngle";
		return 0.0;
	}

	if (d < -36000.0 || d > 36000.0)
	{
		return 0.0;
	}

	while (d > 360.0) 
	{
		d -= 360;
	}

	while (d < 0.0) 
	{
		d += 360.0;
	}

	return d;
}

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
DOUBLE HRUIUtil::Simul( const INT32 N, DOUBLE A[11][11], std::vector<DOUBLE> & Xn, const DOUBLE Err /*= EPS*/ )
{
	long i,j,k,intch;
	long Rowk=0, Colk=0;
	DOUBLE Deter, Pivot, aijck;

	std::vector<int> Col(N); 
	std::vector<int> Row(N); 
	std::vector<int> UCol(N); 
	std::vector<int> URow(N); 
	std::vector<int> Ord(N); 

	for (i=0;i<N;i++)
	{
		UCol[i] = URow[i] = -1;
	}

	Deter = 1.0;
	for (k=0;k<N;k++)
	{
		Pivot = 0.0;
		for (i=0; i<N; i++)
		{
			if (URow[i] == -1)
			{
				for (j=0; j<N; j++)
				{
					if (UCol[j] == -1)
					{
						if (ABS (A[i][j]) > ABS (Pivot))
						{
							Pivot = A[i][j];
							Rowk = i;
							Colk = j;
						}
					}
				}
			}
		}
		URow[Rowk] = Rowk;
		UCol[Colk] = Colk;

		Row[k] = Rowk;
		Col[k] = Colk;

		if (ABS (Pivot) < Err)
		{
			return (0.0);
		}

		Deter *= Pivot;

		for (j=0; j<=N; j++)
		{
			A[Rowk][j] /= Pivot;
		}
		for (i=0; i<N; i++)
		{
			aijck = A[i][Colk];
			if (i != Rowk)
			{
				A[i][Colk] = 0.0;
				for (j=0; j<=N; j++)
				{
					if (j != Colk)
					{
						A[i][j] -= (aijck * A[Rowk][j]);
					}
				}
			}
		}
	}

	for (i=0; i<N; i++)
	{
		Ord[Row[i]] = Col[i];
		Xn[Col[i]] = A[Row[i]][N];
	}

	intch = 0;
	for (i=0; i<N-1; i++)
	{
		for (j= i+1; j<N; j++)
		{
			if (Ord[j] < Ord[i])
			{
				k = Ord[j];
				Ord[j] = Ord[i];
				Ord[i] = k;
				intch++;
			}
		}
	}

	if (intch % 2 == 1) 
	{
		Deter = - Deter;
	}

	return (Deter);
}

DOUBLE HRUIUtil::Regression( const std::vector<DOUBLE> &X, const std::vector<DOUBLE> &Y, const int M, const int N, DOUBLE &A0, std::vector<DOUBLE> & Bn )
{
	DOUBLE Sy=0.0, Syy=0.0, Cyy, Coef[11][11], Sd;
	std::vector<DOUBLE> Sx(N*2);
	std::vector<DOUBLE> Syx(N);
	std::vector<DOUBLE> Cyx(N);
	DOUBLE dum, fm, temp, denom;

	for(int i = 0;i < M;i++)
	{
		Sy += Y[i];
		Syy += (Y[i]*Y[i]);
		dum =1.0;
		for(int j=0;j<N;j++)
		{
			dum*=X[i];
			Sx[j]+=dum;
			Syx[j]+=(Y[i]*dum);
		}
		for(int j=N;j<2*N;j++)
		{
			dum*=X[i];
			Sx[j]+=dum;
		}
	}

	fm = M;
	Cyy = Syy -(Sy*Sy/fm);

	for (int i=0;i<N;i++)
	{
		Coef[i][N] = Cyx[i] = Syx[i] - (Sy*Sx[i]/fm);
		for (int j=0;j<N;j++)
		{
			Coef[i][j] = Sx[i+j+1] - (Sx[i]*Sx[j]/fm);
		}
	}

	if (Simul(N, Coef, Bn, EPSILON) == 0)
	{
		return 0;
	}

	dum = Sy;
	temp = Cyy;

	for (int i=0; i<N; i++)
	{
		dum -= (Bn[i] * Sx[i]);
		temp -= (Bn[i] * Cyx[i]);
	}

	A0 = dum /fm;
	denom = M - N - 1;
	Sd = sqrt ((DOUBLE) (temp/denom));

	return(Sd);
}

DOUBLE HRUIUtil::NormalizeAngleWithin180( DOUBLE angle, DOUBLE against )
{
	DOUBLE normAngle = NormalizeAngle(angle);
	DOUBLE normAgainst = NormalizeAngle(against);
	if (ABS(normAngle - normAgainst) / 2 > 90)
	{
		if (normAngle < normAgainst)
		{
			normAngle += 360;
		}
		else
		{
			normAngle -= 360;
		}
	}
	return normAngle;
}

void HRUIUtil::FillTree(QTreeWidget* widget, const boost::property_tree::ptree &tree, QString filter, QString fileExt)
{
	boost::property_tree::ptree p1;
	for(boost::property_tree::ptree::const_iterator it = tree.begin(); 
		it!=tree.end(); ++it)
	{
		p1 = it->second;
		std::string text = p1.get<std::string>("text");
		QString localString = TSString2QString(text.c_str());
		std::string type = p1.get<std::string>("type");
		std::string id = p1.get<std::string>("id");
		if(type == "folder")
		{
			QStringList stringList;
			stringList << localString;
			QTreeWidgetItem * item = new QTreeWidgetItem(stringList);
			item->setData(0, Qt::UserRole, TSString2QString(id));
			item->setIcon(0, QIcon(":/Resources/images/Folder_16x16.png"));
			widget->addTopLevelItem(item);

			boost::optional<boost::property_tree::ptree&> po = p1.get_child_optional("children");

			if(po)
			{
				FillTreeItem(item, *po, filter, fileExt);
			}
		}
	}
}

void HRUIUtil::FillTreeItem(QTreeWidgetItem* item, const boost::property_tree::ptree &tree, QString filter, QString fileExt)
{
	boost::property_tree::ptree p1;
	for(boost::property_tree::ptree::const_iterator it = tree.begin(); 
		it!=tree.end(); ++it)
	{
		p1 = it->second;
		std:: string text = p1.get<std:: string>("text");
		QString localString = TSString2QString(text.c_str());
		std:: string type = p1.get<std:: string>("type");
		std:: string id = p1.get<std:: string>("id");
		QString compareString = filter;
		if(type == "folder" 
			&& (filter == "" || (filter != "" && localString == compareString)))
		{
			QStringList stringList;
			stringList << localString;
			QTreeWidgetItem * childItem = new QTreeWidgetItem(stringList);
			childItem->setData(0, Qt::UserRole, TSString2QString(id));
			childItem->setIcon(0, QIcon(":/Resources/images/Folder_16x16.png"));
			item->addChild(childItem);

			boost::optional<boost::property_tree::ptree&> po = p1.get_child_optional("children");

			if(po)
			{
				FillTreeItem(childItem, *po, filter, fileExt);
			}
		}
		else if(type == "file")
		{
			int indexOfDot = localString.lastIndexOf(".");
			if(indexOfDot > -1)
			{
				QString ext = localString.mid(indexOfDot);
				if(ext == fileExt)
				{
					QString itemData = item->data(0, Qt::UserRole+1).toString();
					if(!itemData.isEmpty())
					{
						itemData += ";";
					}
					itemData += localString;
					itemData += ",";
					itemData += TSString2QString(id.c_str());

					item->setData(0, Qt::UserRole + 1, itemData);
				}
			}
		}
	}
}

INT16 HRUIUtil::GetHeightInt( DOUBLE Lon, DOUBLE Lat )
{
    //if (TSITerrainServicePtr terrsvc = TS_STATIC_CAST(Framework->
    //    GetServiceManager()->GetService(TSITerrainService::GetMetaTypeIdStatic()),TSITerrainServicePtr))
    //{
    //    return terrsvc->GetAlt(DEGTORAD(Lat), DEGTORAD(Lon));
    //}

    return 0;
}

void HRUIUtil::Month2Number(QString & timeString)
{
	if (timeString.contains("Jan"))
	{
		timeString.replace("Jan","01");
	}
	else if(timeString.contains("Feb"))
	{
		timeString.replace("Feb","02");
	}
	else if(timeString.contains("Mar"))
	{
		timeString.replace("Mar","03");
	}
	else if(timeString.contains("Apr"))
	{
		timeString.replace("Apr","04");
	}
	else if(timeString.contains("May"))
	{
		timeString.replace("May","05");
	}
	else if(timeString.contains("Jun"))
	{
		timeString.replace("Jun","06");
	}
	else if(timeString.contains("Jul"))
	{
		timeString.replace("Jul","07");
	}
	else if(timeString.contains("Aug"))
	{
		timeString.replace("Aug","08");
	}
	else if(timeString.contains("Sep"))
	{
		timeString.replace("Sep","09");
	}
	else if(timeString.contains("Oct"))
	{
		timeString.replace("Oct","10");
	}
	else if(timeString.contains("Nov"))
	{
		timeString.replace("Nov","11");
	}
	else if(timeString.contains("Dec"))
	{
		timeString.replace("Dec","12");
	}
}

void HRUIUtil::AdjustParentsLayout( QWidget *childWidget )
{
	QWidget *w = childWidget;
	while (w) 
	{
		if (w->layout())
		{
			w->layout()->activate();
		}

		if (QScrollArea *area = qobject_cast<QScrollArea*>(w)) 
		{
			QEvent e(QEvent::LayoutRequest);
			QCoreApplication::sendEvent(area, &e);
		}
		else if (QDialog *dlg = dynamic_cast<QDialog *>(w))
		{
			QEvent e(QEvent::LayoutRequest);
			QCoreApplication::sendEvent(dlg, &e);
		}

		w = w->parentWidget();
	}
}

QString HRUIUtil::SelectImplementTypeName( int typeId, const QString &dispInfo)
{
	QList<QStandardItem *> listItems;

	std::vector<int> derivedTypes;
	TSMetaType::GetAllDerivedTypes(typeId, derivedTypes);

	if (!TSMetaType::IsAbstract(typeId))
	{
		listItems.push_back(new QStandardItem(TSString2QString(TSMetaType::GetTypeName(typeId))));
	}

	for (std::size_t i = 0; i < derivedTypes.size(); i++)
	{
		if (!TSMetaType::IsAbstract(derivedTypes[i]))
		{
			listItems.push_back(
				new QStandardItem(TSString2QString(TSMetaType::GetTypeName(derivedTypes[i]))));
		}
	}

	HRStringListDialog dlg(listItems, FromAscii("%1的类型选择对话框").arg(dispInfo));
	if (dlg.exec()==QDialog::Accepted)
	{
		const QList<QStandardItem *> &selectedItems = dlg.GetSelectedItems();
		if (!selectedItems.isEmpty())
		{
			return selectedItems.first()->text();
		}
	}
	return "";
}

TSVariant HRUIUtil::CreateObjectInstance( int typeId, int typeFlag)
{
	TSVariant variant;
	if (typeId > 0)
	{
		if (void* pObj = TSMetaType::Construct(typeId))
		{
			if (typeFlag==TSVariant::Is_SmartPointer)
			{
				VoidPtr spObj = TSMetaType::ConstructSmart(typeId, pObj);
				variant.Create(typeId, TSVariant::Is_SmartPointer, &spObj);
			}
			else if (typeFlag==TSVariant::Is_Pointer)
			{
				variant.Create(typeId, TSVariant::Is_Pointer, &pObj);
			}
			else
			{
				variant.Create(typeId, TSVariant::Is_None, pObj);
				delete pObj;
			}
		}
	}
	return variant;
}

TSVariant HRUIUtil::CreateObjectInstance( const QString &className, int typeFlag)
{
	if (!className.isEmpty())
	{
		int typeId = TSMetaType::GetType(QString2TSString(className).c_str());
		return CreateObjectInstance(typeId, typeFlag);
	}
	return TSVariant();
}

void HRUIUtil::AdjustWidgetSize( QWidget *widget, int marginWidth, int marginHeight )
{
	QSize calcSize = widget->size();
	QRect rect = qApp->desktop()->screenGeometry(widget);
	calcSize.setWidth(MIN(calcSize.width(), rect.width() * 0.8) + marginWidth);
	calcSize.setHeight(MIN(calcSize.height(), rect.height() * 0.8) + marginHeight);
	widget->resize(calcSize);
}

TSString HRUIUtil::GenerateTemplateName( const TSString& baseTemplName,int startNum)
{
	QString BaseName = TSString2QString(baseTemplName);
	bool isNumber = false;
	int index = BaseName.lastIndexOf("_");
	if (index > -1)
	{
		QString CopyNumber = BaseName.mid(index + 1);
		if (!CopyNumber.isEmpty())
		{
			unsigned long Num = CopyNumber.toULong(&isNumber, 10);
		}
	}

	if (isNumber)
	{
		return (QString2TSString(BaseName.left(index)) + "_" +  TSValue_Cast<TSString>(startNum));
	}
	else
	{
		return (QString2TSString(QString("%1_%2").arg(BaseName).arg(startNum)));
	}		
}

extern "C" TOPSIM_SYMBOL_IMPORT bool _UseWGS84;

#define GRID_SIZE 0.1f //必须被1整除
#define DATA_SIZE ((int)(180 * 1 / GRID_SIZE * 3))
#define DATA_FILE_NAME "internal/GridData.inl"
#define REVISE_RANGE 10000

#include "internal/GridData.inl"

/**********************************************************************************************
说明：GenerateGridData用于生成GridData.inc文件，GridData.inc包含了计算好的地球网格数据
调用示例：
    GenerateGridData(true, DATA_FILE_NAME);
***********************************************************************************************
const int kDataSize = DATA_SIZE * 2;
static float kGridData[kDataSize] = {0};
static void GenerateGridData(bool genDataFile = false, const char* fileName = "")
{
	static bool inited = false;
	if (!inited)
	{
		inited = true;
	}
	else
	{
		return;
	}

	float lon = 0.0;
	float startLat = -90.0;
	float latStep = GRID_SIZE;
	float lat;
	int index;

	bool useWgs84old = _UseWGS84;
	for (lat = startLat, index = 0; lat < 90; lat += latStep)
	{
		if (index <= kDataSize - 6)
		{
			TSVector3d startPos, topPos, rightPos, topRightPos;

			_UseWGS84 = false;
			TSConversion::LLAToECEF(DEGTORAD(lat), DEGTORAD(lon), 0.0, startPos);
			TSConversion::LLAToECEF(DEGTORAD(lat + latStep), DEGTORAD(lon + latStep), 0.0, topRightPos);
			TSConversion::LLAToECEF(DEGTORAD(lat + latStep), DEGTORAD(lon), 0.0, topPos);
			TSConversion::LLAToECEF(DEGTORAD(lat), DEGTORAD(lon + latStep), 0.0, rightPos);

			kGridData[index++] = TSConversion::GroundRange(startPos, topPos);
			kGridData[index++] = TSConversion::GroundRange(startPos, rightPos);
			kGridData[index++] = TSConversion::GroundRange(startPos, topRightPos);

			_UseWGS84 = true;
			TSConversion::LLAToECEF(DEGTORAD(lat), DEGTORAD(lon), 0.0, startPos);
			TSConversion::LLAToECEF(DEGTORAD(lat + latStep), DEGTORAD(lon + latStep), 0.0, topRightPos);
			TSConversion::LLAToECEF(DEGTORAD(lat + latStep), DEGTORAD(lon), 0.0, topPos);
			TSConversion::LLAToECEF(DEGTORAD(lat), DEGTORAD(lon + latStep), 0.0, rightPos);

			kGridData[index++] = TSConversion::GroundRange(startPos, topPos);
			kGridData[index++] = TSConversion::GroundRange(startPos, rightPos);
			kGridData[index++] = TSConversion::GroundRange(startPos, topRightPos);
		}
		else
		{
			break;
		}
	}

	_UseWGS84 = useWgs84old;

	if (genDataFile)
	{
		std::ofstream fout(fileName);
		fout << "const int kDataSize = " << kDataSize << ";\n";
		fout << "static float kGridData[" << kDataSize << "] = \n{";
		for (int i = 0; i < kDataSize; ++i)
		{
			fout << kGridData[i] << "f";

			if (i < kDataSize - 1)
			{
				if ((i + 1) % 15 ==  0)
				{
					fout << ",\n";
				}
				else
				{
					fout << ",";
				}
			}
		}
		fout << "};";
		fout.close();
	}
}
***************************************************************************************************/

TSVector2d HRUIUtil::ReviseLL(TSVector2d llCenter, TSVector2d orill, double oridis, double revdis, bool useWgs84)
{
	static const DOUBLE outOfRange = 6371 * 1000;
	if (ABS(revdis - oridis) > outOfRange)
	{
		DEF_LOG_WARNING("%1") << "ReviseLL failed, data out of range.";
		return orill;
	}

	if (revdis < 10000 && fabs(oridis - revdis) < REVISE_RANGE)
	{
		return orill;
	}

	double tolerance = 1.0E-14;
	double disDiff = revdis - oridis;

	if (ABS(disDiff) < tolerance)
	{
		return orill;
	}

	int midIndex = kDataSize / 2;
	double lonDiff = fabs(orill.x - llCenter.x);
	double latDiff = fabs(orill.y - llCenter.y);
	float invCos = 1;

	int incType = 1; //0: height, 1: width: 2:diagonal
	if (latDiff < tolerance)
	{
		incType = 1;
	}
	else if(lonDiff < tolerance)
	{
		incType = 0;
	}
	else
	{
		invCos = orill.getDistance(llCenter) / lonDiff;
		double invCosPi4 = 1.0 / COS(PI_4);
		if (ABS(invCosPi4 - invCos) < 0.01)
		{
			incType = 2;
		}
		else if (invCos < invCosPi4)
		{
			incType = 1;
		}
		else
		{
			incType = 0;
			invCos = orill.getDistance(llCenter) / latDiff;;
		}
	}

	int _revdis = oridis;
	int gridY = orill.y / GRID_SIZE;
	float incGrid = 0;
	int diffSym = disDiff > 0 ? 1 : -1;
	for (int i = gridY; ABS(revdis - _revdis) > 1000.0;)
	{
		int index = midIndex + i * 6 + incType + (useWgs84 ? 3 : 0);
		if (index > kDataSize || index < 0)
		{
			break;
		}

		double increment = kGridData[index] * invCos;
		_revdis = disDiff > 0 ? _revdis + increment : _revdis - increment;
		incGrid += GRID_SIZE;

		int revSym = revdis - _revdis > 0 ? 1 : -1;
		if(revSym * diffSym < 0)
		{
			if (incGrid > GRID_SIZE)
			{
				incGrid -= GRID_SIZE;
			}
			break;
		}

		if (incType != 1)
		{
			i += diffSym;
		}
	}

	int sx = (orill.x - llCenter.x) > 0 ? 1 : -1;
	int sy = (orill.y - llCenter.y) > 0 ? 1 : -1;
	int symx = disDiff > 0 ? sx : -sx;
	int symy = disDiff > 0 ? sy : -sy;

	TSVector2d revll = orill;
	switch(incType)
	{
	case 0:
		{
			float tanAng = lonDiff / latDiff;
			revll.y = orill.y + symy * incGrid;
			revll.x = orill.x + symx * incGrid * tanAng;
			break;
		}
	case 1:
		{
			float tanAng = latDiff / lonDiff;
			revll.x = orill.x + symx * incGrid;
			revll.y = orill.y + symy * incGrid * tanAng;
			break;
		}
	case 2:
		{
			revll.y = orill.y + symy * incGrid;
			revll.x = orill.x + symx * incGrid;
			break;
		}
	}

	return revll;
}

QString HRUIUtil::ToAppRelativePath( const QString &path )
{
	if (QDir::isAbsolutePath(path))
	{
		return path;
	}
	else
	{
		return qApp->applicationDirPath() + "/" + path;
	}
}

QLabel* HRUIUtil::CreateLabelWidget( const QString & labelName, const XPropertyConfigPtr &config )
{
	QString labelText = labelName;

	if (config && !config->LabelText.isEmpty() && config->LabelText != "NULL")
	{
		labelText = config->LabelText;
		if (labelText.endsWith(":"))
		{
			labelText.remove(labelText.length() - 1, 1);
		}
	}

	if (!labelText.isEmpty())
	{
		if (config && !config->Unit.isEmpty())
		{
			labelText+= "(" + config->Unit + ")";
		}

		if (config->IsNeed)
		{
			labelText = QString("* ") + labelText;
		}

		labelText += ":";

		QLabel *label = new QLabel();

		label->setText(labelText);

	    label->setToolTip(config->TipText);

		return label;
	}					   

	return NULL;
}

QString HRUIUtil::GetProperLabelText( const XPropertyConfigPtr &config, const TSMetaProperty *prop )
{
	if (config && !config->LabelText.isEmpty() && config->LabelText.toUpper() != "NULL")
	{
		return config->LabelText;
	}

	if (prop)
	{
		return TSString2QString(prop->Name);
	}
	return "";
}

int HRUIUtil::GetProperOrder( const XPropertyConfigPtr &config, int def /*= 1*/ )
{
	if (config && config->Order != 0)
	{
		return config->Order;
	}
	return def;
}

QWidget * HRUIUtil::GetRootWidget( QWidget* widget )
{
	QWidget * root = widget;
	while(root)
	{
		if (root->isWindow())
		{
			break;
		}
		root = root->parentWidget();
	}
	return root;
}

std::vector<FLOAT> HRUIUtil::GetCircleSplitSinArray()
{
	std::vector<FLOAT> sinArray;
	for (int i = 0; i <= 36; i++)
	{
		DOUBLE degree = (DOUBLE)DEGTORAD(i*10);
		sinArray.push_back((FLOAT)sin(degree)) ;
	}

	return sinArray;
}

std::vector<FLOAT> HRUIUtil::GetCircleSplitCosArray()
{
	std::vector<FLOAT> cosArray;
	for (int i = 0; i <= 36; i++)
	{
		DOUBLE degree = (DOUBLE)DEGTORAD(i*10);
		cosArray.push_back((FLOAT)cos(degree));
	}

	return cosArray;
}
