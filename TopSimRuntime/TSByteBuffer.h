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
	bool Checkout;				 //属性验证
	TSString Tips;				     //提示信息
	int Flag;                            //上下文标识 0：UI约束Enable Disable, 1：默认值 2：属性检查
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

	/// <summary> 标签（Deprecated，使用Param代替） </summary>
	QString Tag;

	/// <summary> 属性所在分组(基础数据) </summary>
	QString Group;

	/// <summary> 属性显示名称 </summary>
	QString LabelText;

	/// <summary> 提示信息 </summary>
	QString TipText;

	/// <summary> 控件显示顺序 </summary>
	int Order;

	/// <summary> 浮点精度 </summary>
	int Decimals;

	/// <summary> 最大值 </summary>
	QString MaxVal;

	/// <summary> 最小值 </summary>
	QString MinVal;

	/// <summary> 单位 </summary>
	QString Unit;

	/// <summary> UI显示表达式 </summary>
	QString Exp2UI;

	/// <summary> 模型存储表达式 </summary>
	QString Exp2ML;

	/// <summary> 可空 </summary>
	bool Nullable;

	QString DefaultValue;

	/// <summary> 使用默认构造器 </summary>
	bool DefaultWidgetConstructor;

	/// <summary> 脚本文件路径 </summary>
	QString ScriptFile;

	/// <summary> 脚本函数名称 </summary>
	QString ScriptFunctionName;

	/// <summary> 控件参数（json对象） </summary>
	QString Param;

	/// <summary> 默认展开 </summary>
	bool DefaultExpand;

	/// <summary>  </summary>
	bool GroupCheckable;

	/// <summary> 必要属性</summary>
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

	/// <summary> 参数显示配置 </summary>
	XPropertyConfigPtr Config;

	/// <summary> 属性指针 </summary>
	const TSMetaProperty * Property;

	/// <summary> 所有者参数 </summary>
	XPropertyCreateParamsPtr  OwnerCreateParams;

	/// <summary> 参数取值主题句柄 </summary>
	TSTOPICHANDLE VariantTopicHandle;

	/// <summary> 参数取值 </summary>
	TSVariant Variant;

	/// <summary> 使用模式 </summary>
	XPropertyConfig::UseMode Mode;

	/// <summary> 所在的UI上下文 </summary>
	TSDomainPtr UIContext;

	///<summary> 黑板KeyId </summary>
	TSString BBKeyName;

	/// <summary> 当前分组 </summary>
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
/// <summary>	属性编辑控件.  </summary>
///
/// <remarks>	模型属性编辑自定义控件的基类，\n
/// 			！！！用户开发自定义属性插件必须从该类继承！！！\n
/// 			Validate接口需完成控件信息的保存工作。\n
/// 			构造参数为待编辑的模型属性参数。\n
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
	/// <summary>	验证数据. </summary>
	///
	/// <remarks>	龙成亮, 2017/1/21. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void Validate() = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取控件文本. </summary>
	///
	/// <remarks>	龙成亮, 2017/1/21. </remarks>
	///
	/// <returns>	The text. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual QString GetText() { return ""; };

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	是否显示标签. </summary>
	///
	/// <remarks>	龙成亮, 2017/1/21. </remarks>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual bool DefaultShowLable() const { return true; };

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	自定义属性弹出框. </summary>
	///
	/// <remarks>	龙成亮, 2017/1/21. </remarks>
	///
	/// <param name="Dlg">	[in,out] If non-null, the dialog. </param>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual bool CustomizePropertyDialog(QDialog *Dlg) { return false; }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置控件（由控件实现响应）. </summary>
	///
	/// <remarks>	龙成亮, 2017/1/21. </remarks>
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