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
	/* ���� */
	enum TypeFlag
	{
		TF_InvalidFlag  = 0,		//��Ч����
		TF_ObjectFlag   = 1 << 0,	//1��������
		TF_MessageFlag  = 1 << 1,	//2��Ϣ����
		TF_DataFlag     = 1 << 2,	//4��������
		TF_LocalClass   = 1 << 3,	//8������
		TF_Service      = 1 << 4,	//16����
		TF_Command      = 1 << 5	//32����
	};

	/* ��ȡ���� */
	virtual TypeFlag                     GetTypeFlag();
	/* ��ȡ���� */
	virtual const TSChar *               GetTypeName();
	/* �Ѿ��ҵ��ļ��� */
	virtual bool                         HasFindSet(TopSimDataInterface::FindSetFlag Key);
	/* �������ݶ��� */
	virtual void *                       ConstructDataObject();
	/* �������ݶ�������ָ�� */
	virtual VoidPtr                      ConstructSmartDataObject();
	/* �������ݶ��� */
	virtual void                         DestroyDataObject(TSInterObject * obj);
	/* ���л����� */
	virtual bool                         Serialize(const TSInterObject * Data,TSISerializer& Ser);
	/* �����л����� */
	virtual bool                         Deserialize(TSInterObject * Data,TSISerializer& Ser);
	/* �����Ƿ�ƥ�� */
	virtual bool                         IsTypeMatched(const TSInterObject * Data);
	/* ��ȡ���Ͱ汾��HashCode */
	virtual UINT32                       GetTypeVersionHashCode();
	/* ��ȡ���ͱ�ʶ */
	static TypeFlag                      GetTypeFlagStatic();
	/* ��ȡ�������� */
	static const TSChar *                GetTypeNameStatic();
	/* �������ݶ��� */
	static void *                        ConstructDataObjectStatic();
	/* �������ݶ��������ָ��*/
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


