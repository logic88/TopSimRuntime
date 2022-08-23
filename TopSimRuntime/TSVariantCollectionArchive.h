#ifndef __TSTYPESUPPOTR_H__
#define __TSTYPESUPPOTR_H__

#include "TSISerializer.h"
#include "TopSimDataInterface.h"

#if !defined(XSIM3_3) && !defined(XSIM3_2)
#include "TSRDGTypes.h"
#endif // !defined(XSIM3_3) && !defined(XSIM3_2)

#define  DEFAULTVERSIONCODE 0

struct TSTypeSupportPrivate;
class TOPSIMDATAINTERFACE_DECL TSTypeSupport : public boost::intrusive_ref_counter<TSTypeSupport>
{
public:
	TSTypeSupport(void);
	virtual ~TSTypeSupport(void);

public:
	/* 类型 */
	enum TypeFlag
	{
		TF_InvalidFlag  = 0,		//无效类型
		TF_ObjectFlag   = 1 << 0,	//1对象类型
		TF_MessageFlag  = 1 << 1,	//2消息类型
		TF_DataFlag     = 1 << 2,	//4数据类型
		TF_LocalClass   = 1 << 3,	//8本地类
		TF_Service      = 1 << 4,	//16服务
		TF_Command      = 1 << 5	//32命令
	};

	/* 获取类型 */
	virtual TypeFlag                     GetTypeFlag();
	/* 获取名称 */
	virtual const TSChar *               GetTypeName();
	/* 已经找到的集合 */
	virtual bool                         HasFindSet(TopSimDataInterface::FindSetFlag Key);
	/* 构造数据对象 */
	virtual void *                       ConstructDataObject();
	/* 构造数据对象智能指针 */
	virtual VoidPtr                      ConstructSmartDataObject();
	/* 销毁数据对象 */
	virtual void                         DestroyDataObject(TSInterObject * obj);
	/* 序列化对象 */
	virtual bool                         Serialize(const TSInterObject * Data,TSISerializer& Ser);
	/* 反序列化对象 */
	virtual bool                         Deserialize(TSInterObject * Data,TSISerializer& Ser);
	/* 类型是否匹配 */
	virtual bool                         IsTypeMatched(const TSInterObject * Data);
	/* 获取类型版本的HashCode */
	virtual UINT32                       GetTypeVersionHashCode();
	/* 获取类型标识 */
	static TypeFlag                      GetTypeFlagStatic();
	/* 获取类型名称 */
	static const TSChar *                GetTypeNameStatic();
	/* 构造数据对象 */
	static void *                        ConstructDataObjectStatic();
	/* 构造数据对象的智能指针*/
	static VoidPtr                       ConstructSmartDataObjectStatic();
	static void                          DestroyDataObjectStatic(TSInterObject * obj);
	static bool                          SerializeStatic(const TSInterObject * Data,TSISerializer& Ser);
	static bool                          DeserializeStatic(TSInterObject * Data,TSISerializer& Ser);
	static bool                          IsTypeMatchedStatic(const TSInterObject * Data);
	static UINT32                        GetTypeVersionHashCodeStatic();
	 
};

typedef boost::intrusive_ptr<TSTypeSupport> TSTypeSupportPtr;
typedef boost::function<void(TSString,const TSChar*,UINT32,UINT32)> ExceptionCallBack;
void TOPSIMDATAINTERFACE_DECL SetExceptionCallBack(ExceptionCallBack CallBack);
void TOPSIMDATAINTERFACE_DECL ThrowHashVersionTypeException(const TSChar* TypeName,UINT32 SelfVersion,UINT32 OtherVersion);
void TOPSIMDATAINTERFACE_DECL ThrowHashVersionTypeExceptionPSNoMatch(const TSChar* TypeName, UINT32 SelfVersion, UINT32 OtherVersion);
#endif


