#ifndef __HRPropertyWidget_H_3377cfa9_adf1_4846_8654_6bc3609611ce__
#define __HRPropertyWidget_H_3377cfa9_adf1_4846_8654_6bc3609611ce__

#include <QWidget>

#include "HRUICommon.h"

#ifndef Q_MOC_RUN

#include <TopSimDataInterface/TSTopicTypes.h>
#include <TopSimRuntime/TSScriptInterpreter.h>

#endif

struct HRUICOMMON_EXPORT HRValueChangedCtx
{
	HRValueChangedCtx();

	TS_MetaType(HRValueChangedCtx);

	void Enable(const TSString & PropName)
	{
		PropsMap[PropName] = true;
	}

	void Disable(const TSString & PropName)
	{
		PropsMap[PropName] = false;
	}

	std::map<TSString, bool> PropsMap;
	bool Checkout;				 //������֤
	TSString Tips;				     //��ʾ��Ϣ
	int Flag;                            //�����ı�ʶ 0��UIԼ��Enable Disable, 1��Ĭ��ֵ 2�����Լ��
};

STRUCT_PTR_DECLARE(HRValueChangedCtx);

class HRVerifiableDialog;

struct HRUICOMMON_EXPORT XPropertyConfig 
{
	XPropertyConfig();
	~XPropertyConfig();

	enum UseMode
	{
		ComponentDesign = (1<<0),
		EntityDesign	= (1<<1),
		DeployDesign	= (1<<2),
		RuntimeDesign	= (1<<3),
		All				= 0xFFFF,
	};

	/// <summary> ��ǩ��Deprecated��ʹ��Param���棩 </summary>
	QString Tag;

	/// <summary> �������ڷ���(��������) </summary>
	QString Group;

	/// <summary> ������ʾ���� </summary>
	QString LabelText;

	/// <summary> ��ʾ��Ϣ </summary>
	QString TipText;

	/// <summary> �ؼ���ʾ˳�� </summary>
	int Order;

	/// <summary> ���㾫�� </summary>
	int Decimals;

	/// <summary> ���ֵ </summary>
	QString MaxVal;

	/// <summary> ��Сֵ </summary>
	QString MinVal;

	/// <summary> ��λ </summary>
	QString Unit;

	/// <summary> UI��ʾ���ʽ </summary>
	QString Exp2UI;

	/// <summary> ģ�ʹ洢���ʽ </summary>
	QString Exp2ML;

	/// <summary> �ɿ� </summary>
	bool Nullable;

	QString DefaultValue;

	/// <summary> ʹ��Ĭ�Ϲ����� </summary>
	bool DefaultWidgetConstructor;

	/// <summary> �ű��ļ�·�� </summary>
	QString ScriptFile;

	/// <summary> �ű��������� </summary>
	QString ScriptFunctionName;

	/// <summary> �ؼ�������json���� </summary>
	QString Param;

	/// <summary> Ĭ��չ�� </summary>
	bool DefaultExpand;

	/// <summary>  </summary>
	bool GroupCheckable;

	/// <summary> ��Ҫ����</summary>
	bool IsNeed;
};

struct HRUICOMMON_EXPORT XPropertyCreateParams
{
	enum Flag
	{
		kNullFlag,
		kEditableFlag = 0x01,
		kFullFlag = 0xFFFFFFFF,
	};

public:
	XPropertyCreateParams();
	XPropertyCreateParams(const TSVariant &Var,XPropertyConfig::UseMode Mode,const TSMetaProperty * Property = NULL,
		XPropertyCreateParamsPtr  OwnerCreateParams = NULL, TSDomainPtr Domian = TSDomainPtr());
	XPropertyCreateParams(TSTOPICHANDLE TopicHandle, XPropertyConfig::UseMode Mode, TSDomainPtr Domian = TSDomainPtr());
	XPropertyCreateParams(const TSVariant &Var, const TSString & BBKeyName, XPropertyConfig::UseMode Mode, XPropertyCreateParamsPtr  OwnerCreateParams = NULL);

	/// <summary> ������ʾ���� </summary>
	XPropertyConfigPtr Config;

	/// <summary> ����ָ�� </summary>
	const TSMetaProperty * Property;

	/// <summary> �����߲��� </summary>
	XPropertyCreateParamsPtr  OwnerCreateParams;

	/// <summary> ����ȡֵ������ </summary>
	TSTOPICHANDLE VariantTopicHandle;

	/// <summary> ����ȡֵ </summary>
	TSVariant Variant;

	/// <summary> ʹ��ģʽ </summary>
	XPropertyConfig::UseMode Mode;

	/// <summary> ���ڵ�UI������ </summary>
	TSDomainPtr UIContext;

	///<summary> �ڰ�KeyId </summary>
	TSString BBKeyName;

	/// <summary> ��ǰ���� </summary>
	QString Group;
};

HRUICOMMON_EXPORT XPropertyCreateParamsPtr MakeCreateParams(
	const TSVariant &Var, XPropertyConfig::UseMode Mode,const TSMetaProperty * Property = NULL,
	XPropertyCreateParamsPtr  OwnerCreateParams = NULL,
	TSDomainPtr UIContext = TSDomainPtr());

HRUICOMMON_EXPORT XPropertyCreateParamsPtr MakeCreateParams(
	TSTOPICHANDLE TopicHandle, XPropertyConfig::UseMode Mode,
	TSDomainPtr UIContext = TSDomainPtr());

HRUICOMMON_EXPORT XPropertyCreateParamsPtr MakeCreateParams(
	TSTOPICHANDLE TopicHandle, VoidPtr Data,XPropertyConfig::UseMode Mode,
	TSDomainPtr UIContext = TSDomainPtr());

HRUICOMMON_EXPORT XPropertyCreateParamsPtr MakeCreateParams(
	const TSVariant &Var, const TSString & BBKeyName, XPropertyConfig::UseMode Mode,
	XPropertyCreateParamsPtr  OwnerCreateParams = NULL);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	���Ա༭�ؼ�.  </summary>
///
/// <remarks>	ģ�����Ա༭�Զ���ؼ��Ļ��࣬\n
/// 			�������û������Զ������Բ������Ӹ���̳У�����\n
/// 			Validate�ӿ�����ɿؼ���Ϣ�ı��湤����\n
/// 			�������Ϊ���༭��ģ�����Բ�����\n
/// 			TSMF Team, 2014/4/9. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

class HRUICOMMON_EXPORT HRPropertyWidget : public QWidget
{
	Q_OBJECT
signals:
	void ValueChangedSignal(const HRPropertyWidget*, const TSVariant&);
public:
	HRPropertyWidget(XPropertyCreateParamsPtr CreateParams);
	~HRPropertyWidget();

	XPropertyCreateParamsPtr GetCreateParams() const;

	void EmitValueChangedSignal(const TSVariant& val);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��֤����. </summary>
	///
	/// <remarks>	������, 2017/1/21. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void Validate() = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ�ؼ��ı�. </summary>
	///
	/// <remarks>	������, 2017/1/21. </remarks>
	///
	/// <returns>	The text. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual QString GetText() { return ""; };

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�Ƿ���ʾ��ǩ. </summary>
	///
	/// <remarks>	������, 2017/1/21. </remarks>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual bool DefaultShowLable() const { return true; };

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�Զ������Ե�����. </summary>
	///
	/// <remarks>	������, 2017/1/21. </remarks>
	///
	/// <param name="Dlg">	[in,out] If non-null, the dialog. </param>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual bool CustomizePropertyDialog(QDialog *Dlg) { return false; }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���ÿؼ����ɿؼ�ʵ����Ӧ��. </summary>
	///
	/// <remarks>	������, 2017/1/21. </remarks>
	///
	/// <param name="key">  	The key. </param>
	/// <param name="value">	The value. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void SetProperty(const std::string &key, const std::string &value) {};

private:
	XPropertyCreateParamsPtr _CreateParams;
};

class HRUICOMMON_EXPORT TSIPropertyWidgetConstructor
{
	TS_MetaType(TSIPropertyWidgetConstructor,TSNullClass);
public:
	virtual HRPropertyWidget * ConstructPropertyWidget(XPropertyCreateParamsPtr CreateParams) = 0;
};

#define DEF_WIDGETCONSTRUCTOR(a) \
class a##Constructor : public TSIPropertyWidgetConstructor\
{\
	TS_MetaType(a##Constructor,TSIPropertyWidgetConstructor);\
public:\
	virtual HRPropertyWidget * ConstructPropertyWidget(XPropertyCreateParamsPtr CreateParams)\
	{\
		return new a(CreateParams);\
	}\
};\
BEGIN_METADATA(a##Constructor)\
REG_BASE(TSIPropertyWidgetConstructor);\
END_METADATA()

#endif // __HRPropertyWidget_H_3377cfa9_adf1_4846_8654_6bc3609611ce__