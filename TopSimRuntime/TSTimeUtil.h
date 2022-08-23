#ifndef __TSTOPICTYPEMANAGER_H__
#define __TSTOPICTYPEMANAGER_H__


#ifdef XSIM3_3
#include <TopSimUtil/pugixml.hpp>
#include "TopSimDataInterface4XSim.h"
#elif XSIM3_2
#include <TopSimUtil/pugixml.hpp>
#include "TopSimDataInterface4XSim3.2.h"
#else
#include <TopSimRuntime/pugixml.hpp>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TSTopicHelper.h"
#include "TSTypeSupport.h"
#include "TopSimDataInterface.h"

#define GROUP_TOPIC 6

struct TSTopicTypeManagerPrivate;
/*
  主题类型管理类
*/
class TOPSIMDATAINTERFACE_DECL TSTopicTypeManager : public TSSingleton<TSTopicTypeManager>
{
	SINGLETON_DECLARE(TSTopicTypeManager);
public:
	 /* 预注册主题根据主题名称 */
	 TSTOPICHANDLE          PreRegisterTopicByName(const TSString & TopicName);
	 /* 根据CRC32(TopicName)获取主题句柄 */
	 TSTOPICHANDLE          GetTopicByCRC32(UINT32 CRC32Value);
	 /* 根据主题名称获取主题句柄 */
	 TSTOPICHANDLE          GetTopicByTopicName(const TSString & TopicName);
	 /* 根据主题句柄获取主题的类型支持对象 */
	 TSTypeSupportPtr       GetTypeSupportByTopic(TSTOPICHANDLE TopicHandle);
	 /* 根据主题句柄获取主题的Helper对象 */
	 TSTopicHelperPtr       GetTopicHelperByTopic(TSTOPICHANDLE TopicHandle);
	 /* 根据主题句柄获取主题名称 */
	 const TSString&        GetTopicNameByTopic(TSTOPICHANDLE TopicHandle);
	 /* 根据主题句柄获取主题类型名称 */
	 const TSString&        GetTypeNameByTopic(TSTOPICHANDLE TopicHandle);
	 /* 根据主题句柄获取主题描述 */
     const TSString&        GetDescByTopic(TSTOPICHANDLE TopicHandle);
	 /* 根据主题名称获取主题支持对象 */
	 TSTypeSupportPtr       GetSupportByTypeName(const TSString& TypeName);
	 
	 /* 获取已经注册的主题句柄数组 */
	 std::vector<TSTOPICHANDLE> GetRegisteredTopics();
	 /* 获取当前最大可容纳的主题注册个数 */
	 TSTOPICHANDLE              GetMaxRegisterdTopic();
	 std::vector<TSString>      GetRegisteredTypes();
	 
	 void SetCfg(const TSString& cfgPath,const TSString& workDirectoryPath = "");
	 void ReloadCfg(const TSString& cfgPath, const TSString& workDirectoryPath = "");
	 const TSString& GetConfigurePath();
	 const TSString& GetWorkDirectoryPath();
	 pugi::xml_document& GetConfigureDoc();
	 
	 void SetExtendCfg(const TSString& Key,const TSString& Value);
	 const TSString& GetExtendCfg(const TSString& Key);
	 void InitNoCacheTopicSet(std::set<UINT32> & noCacheTopicHandle);

public:
	/* 注册主题 */
	bool RegisterTopic(UINT32 Id,const TSString& TypeName,const TSString& TopicName,const TSString& Desc, TSTopicHelperPtr TopicTopicHelper);
	/* 注册主题支持类型 */
	bool RegisterTypeSupport(const TSString& TypeName, TSTypeSupportPtr TypeSupport, int DataMetaTypeId);
	/* 添加主题到主题管理Map中 */
	UINT32 GenerateTopicByName(const TSString & TopicName);
	 
	void PostModulesLoaded();

private:
	TSTopicTypeManager();
	~TSTopicTypeManager();

	/* 解析配置文件中PerLoadModules节点并加载响应库 */
	void LoadPreloadModules();
	/* 解析配置文件中Modules节点并加载响应库 */
	void LoadModules();
    void LoadObjectsModules();
	/* 解析配置文件中ExtendConfigs节点配置 */
	void LoadExtendCfgs();

	void LoadModule(const TSString& ModuleFileName );

private:
	TSTopicTypeManagerPrivate * _p;
};

TOPSIMDATAINTERFACE_DECL UINT32 GenerateTopicByName(const TSString & TopicName);

TOPSIMDATAINTERFACE_DECL bool RegisterTypeSupport(const TSString& TypeName,
	TSTypeSupportPtr TypeSupport,int DataMetaTypeId);

TOPSIMDATAINTERFACE_DECL bool RegisterTopic(UINT32 Id,
	const TSString& TypeName,
	const TSString& TopicName,
	const TSString& Desc,
	TSTopicHelperPtr TopicTopicHelper);

template<typename T1>
TSTOPICHANDLE TempalteRegisterTopicType(const TSString& TopicName,
	const TSString & Desc,
	TSTopicHelperPtr TopicDescHepler,
	const TSString& TypeName,
	TSTypeSupportPtr TypeSupport)
{
	TSTOPICHANDLE TopicId  = GenerateTopicByName(TopicName);

	RegisterTopic(TopicId,
		TypeName,
		TopicName,
		Desc,
		TopicDescHepler);

	return TopicId;
}

template<typename DataType>
int CompareT(const void * left,const void * right)
{
	if(*(DataType *)left == *(DataType *)right)
	{
		return 0;
	}

	if(*(DataType *)left < *(DataType *)right)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}
template<typename DataType>
void AssignT(void * left,const void * right)
{
	*(DataType *)left = *(DataType *)right;
}
template<typename DataType>
bool HasFindSetT(TopSimDataInterface::FindSetFlag Flag)
{
	if(TESTBIT(Flag,TopSimDataInterface::kNone))
	{
		return true;
	}

	return false;
}
template<typename DataType>
TSTypeSupport::TypeFlag TypeFlagT()
{
	return TSTypeSupport::TF_DataFlag;
}


#define  DEMARSHALPREPARE(TIME)\
	UINT32 HashVersionCode4Des;\
	Demarshal<UINT32>(Ser,HashVersionCode4Des);\
	if(GetTypeVersionHashCode() != HashVersionCode4Des)\
	{\
		ThrowHashVersionTypeException(GetTypeName(),GetTypeVersionHashCode(),HashVersionCode4Des);\
		return false;\
	}\
		else\
	{\
		Demarshal<UINT64>(Ser,TIME);\
		return true;\
	}\

#define GEN_GENERIC_FUNC(SUPPORT_TYPE)\
	virtual TypeFlag       GetTypeFlag();\
	virtual const TSChar * GetTypeName();\
	virtual bool           HasFindSet(TopSimDataInterface::FindSetFlag Key);\
	virtual void *         ConstructDataObject();\
	virtual VoidPtr        ConstructSmartDataObject();\
	virtual void           DestroyDataObject(TSInterObject * obj);\
	virtual bool           Serialize(const TSInterObject * Data,TSISerializer& Ser);\
	virtual bool           Deserialize(TSInterObject *Data,TSISerializer &Ser);\
	virtual bool           IsTypeMatched(const TSInterObject * Data);\
	virtual UINT32         GetTypeVersionHashCode();\
	static TypeFlag        GetTypeFlagStatic();\
	static const TSChar *  GetTypeNameStatic();\
	static bool            HasFindSetStatic(TopSimDataInterface::FindSetFlag Key);\
	static void *          ConstructDataObjectStatic();\
	static VoidPtr         ConstructSmartDataObjectStatic();\
	static void            DestroyDataObjectStatic(TSInterObject * obj);\
	static bool            SerializeStatic(const TSInterObject * Data,TSISerializer& Ser,UINT32 MarshaledFlag);\
	static bool            DeserializeStatic(TSInterObject * Data,TSISerializer &Ser);\
	static bool            IsTypeMatchedStatic(const TSInterObject * Data);\
	static UINT32          GetTypeVersionHashCodeStatic();\


#define GEN_TYPESUPPORT_CPP_I(TYPE,SUPPORT_TYPE,SELFVERSONMACRO)\
	TSTypeSupport::TypeFlag SUPPORT_TYPE::GetTypeFlag(){return GetTypeFlagStatic();}\
	const TSChar * SUPPORT_TYPE::GetTypeName(){return GetTypeNameStatic();}\
	bool           SUPPORT_TYPE::HasFindSet(TopSimDataInterface::FindSetFlag Key){return HasFindSetStatic(Key);}\
	void *         SUPPORT_TYPE::ConstructDataObject(){return ConstructDataObjectStatic();}\
	VoidPtr        SUPPORT_TYPE::ConstructSmartDataObject(){return ConstructSmartDataObjectStatic();}\
	void           SUPPORT_TYPE::DestroyDataObject(TSInterObject * obj){DestroyDataObjectStatic(obj);}\
	bool           SUPPORT_TYPE::Serialize(const TSInterObject * Data,TSISerializer& Ser){return SerializeStatic(Data,Ser,TopSimDataInterface::kKey);}\
	bool           SUPPORT_TYPE::Deserialize(TSInterObject * Data,TSISerializer &Ser){return DeserializeStatic(Data,Ser);}\
	bool           SUPPORT_TYPE::IsTypeMatched(const TSInterObject * Data){return IsTypeMatchedStatic(Data);}\
	TSTypeSupport::TypeFlag SUPPORT_TYPE::GetTypeFlagStatic(){return TypeFlagT<TYPE>();}\
	const TSChar * SUPPORT_TYPE::GetTypeNameStatic() {return STRING(TYPE);}\
	bool           SUPPORT_TYPE::HasFindSetStatic(TopSimDataInterface::FindSetFlag Key){return HasFindSetT<TYPE>(Key);}\
	void *         SUPPORT_TYPE::ConstructDataObjectStatic(){return new TYPE();}\
	VoidPtr        SUPPORT_TYPE::ConstructSmartDataObjectStatic(){return VoidPtr(new TYPE());}\
	void		   SUPPORT_TYPE::DestroyDataObjectStatic(TSInterObject * obj){delete (TYPE *)obj;}\
	bool           SUPPORT_TYPE::SerializeStatic(const TSInterObject * Data,TSISerializer& Ser,UINT32 MarshaledFlag){Marshal<TYPE>(Ser,*(const TYPE*)Data,MarshaledFlag);return true;}\
	bool           SUPPORT_TYPE::DeserializeStatic(TSInterObject * Data,TSISerializer &Ser){Demarshal<TYPE>(Ser,*(TYPE*)Data);return true;}\
	bool           SUPPORT_TYPE::IsTypeMatchedStatic(const TSInterObject * Data){return dynamic_cast<const TYPE *>(Data);}\
	UINT32         SUPPORT_TYPE::GetTypeVersionHashCode(){return GetTypeVersionHashCodeStatic();}\
    UINT32         SUPPORT_TYPE::GetTypeVersionHashCodeStatic(){return SELFVERSONMACRO;}\

template<typename SUPPORT,typename TYPE>
boost::intrusive_ptr<SUPPORT> CreateAndRegisterSupportT()
{
	boost::intrusive_ptr<SUPPORT> Support = new SUPPORT;
	RegisterTypeSupport(Support->GetTypeName(),Support, GetMetaTypeId<TYPE>(reinterpret_cast<TYPE*>(0)));
	return Support;
}

#define GEN_TYPESUPPORT_VAL(SUPPORT_TYPE,EXPORT_DECL)\
	EXPORT_DECL boost::intrusive_ptr<SUPPORT_TYPE> CONN(Get,SUPPORT_TYPE)();\

#define GEN_TYPESUPPORT_VAL_IMPL(TYPE,SUPPORT_TYPE)\
	boost::intrusive_ptr<SUPPORT_TYPE> CONN(Get,SUPPORT_TYPE)()\
	{\
		static boost::intrusive_ptr<SUPPORT_TYPE> Support = CreateAndRegisterSupportT<SUPPORT_TYPE,TYPE>();\
		return Support;\
	}\
	 

#define GEN_TYPESUPPORT_CPP(TYPE,SUPPORT_TYPE,SELFVERSONMACRO)\
	GEN_TYPESUPPORT_CPP_I(TYPE,SUPPORT_TYPE,SELFVERSONMACRO);\
	GEN_TYPESUPPORT_VAL_IMPL(TYPE,SUPPORT_TYPE);\

#define GEN_TYPESUPPORT_I(SUPPORT_TYPE,EXPORT_DECL)\
class EXPORT_DECL SUPPORT_TYPE : public TSTypeSupport\
{\
public:\
	GEN_GENERIC_FUNC(SUPPORT_TYPE)\
};\

#define GEN_TYPESUPPORT_2(SUPPORT_TYPE,EXPORT_DECL)\
	GEN_TYPESUPPORT_I(SUPPORT_TYPE,EXPORT_DECL);\
	GEN_TYPESUPPORT_VAL(SUPPORT_TYPE,EXPORT_DECL);


#ifdef _MSC_VER
#		define GEN_TYPESUPPORT(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(GEN_TYPESUPPORT_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define GEN_TYPESUPPORT(...) BOOST_PP_OVERLOAD(GEN_TYPESUPPORT_,__VA_ARGS__)(__VA_ARGS__)
#endif

#define TYPE_VAL(NAMESPACE) CONN_STR(NAMESPACE,CONN(::Get,SUPPORT_TYPE)())

#define TEMPLATE_IMPL(DATA_TYPE,EXPORT_MACRO) template<>\
	FORCEINLINE int CompareT<DATA_TYPE>(const void * left,const void * right)\
{\
	if(*(DATA_TYPE *)left == *(DATA_TYPE *)right)\
{\
	return 0;\
}\
	if(*(DATA_TYPE *)left < *(DATA_TYPE *)right)\
{\
	return -1;\
}\
	else\
{\
	return 1;\
}\
}\
	template<>\
	FORCEINLINE void AssignT<DATA_TYPE>(void * left,const void * right)\
{\
	*(DATA_TYPE *)left = *(DATA_TYPE *)right;\
}\


#define TYPESUPPORT_IMPL\
	BEGIN_NAMESPACE\
	GEN_TYPESUPPORT_CPP(DATA_TYPE,SUPPORT_TYPE,SELFVERSONMACRO);\
	END_NAMESPACE\


#define TYPESUPPORT_DECLARE\
	BEGIN_NAMESPACE\
	GEN_TYPESUPPORT(SUPPORT_TYPE,EXPORT_MACRO);\
	END_NAMESPACE;\
	TEMPLATE_IMPL(DATA_TYPE,EXPORT_MACRO);\

template<typename TYPE>
void MarshalFiled(TSISerializer& Ser,TYPE& Filed,UINT32 KeyType,UINT32 MarshaledFlag)
{
	Marshal(Ser,Filed,MarshaledFlag == TopSimDataInterface::kNone ? MarshaledFlag : KeyType);
}

template<typename TYPE>
void DemarshalFiled(TSISerializer& Ser,TYPE& Filed,UINT32 KeyType)
{
	Demarshal(Ser,Filed);
}

#define Marshal_Filed(Filed,KeyType)\
	MarshalFiled(Ser,CONN_STR(Data.,Filed),KeyType,MarshaledFlag)

#define Marshal_Supper\
	if(SUPERVERSIONMACRO != CONN_STR(SUP_NAMESPACE,::TypeSupport)::GetTypeVersionHashCodeStatic())\
	{\
		ThrowHashVersionTypeException(CONN_STR(NAMESPACE,::SUPPORT_TYPE)::GetTypeNameStatic(),SUPERVERSIONMACRO,CONN_STR(SUP_NAMESPACE,::TypeSupport)::GetTypeVersionHashCodeStatic());\
		return Ser;\
	}\
	CONN_STR(SUP_NAMESPACE,::TypeSupport)::SerializeStatic(&Data,Ser,MarshaledFlag)

#define Demarshal_Filed(Filed,KeyType)\
	DemarshalFiled(Ser,CONN_STR(Data.,Filed),KeyType);\

#define Demarshal_Supper\
	CONN_STR(SUP_NAMESPACE,::TypeSupport)::DeserializeStatic(&Data,Ser)

#define BEGIN_MARSHAL\
	template<>\
	FORCEINLINE TSISerializer& Marshal(TSISerializer &Ser,const DATA_TYPE& Data,UINT32 MarshaledFlag)\
{\
    Ser.PushComplexType(CONN_STR(NAMESPACE,::SUPPORT_TYPE)::GetTypeNameStatic(),&Data,TSISerializer::Marshal);\

#define END_MARSHAL\
	Ser.PopComplexType();\
	return Ser;\
}\

#define BEGIN_DEMARSHAL\
	template<>\
	FORCEINLINE TSISerializer& Demarshal(TSISerializer &Ser,DATA_TYPE& Data)\
{\
	Ser.PushComplexType(CONN_STR(NAMESPACE,::SUPPORT_TYPE)::GetTypeNameStatic(),&Data,TSISerializer::Demarshal);\

#define END_DEMARSHAL\
	Ser.PopComplexType();\
	return Ser;\
}\

#define TS_TOPIC_DECLARE(VAR_NAME,EXPORT_MACRO) EXPORT_MACRO extern TSTOPICHANDLE VAR_NAME;

#define TS_TOPIC_IMPLEMENT(VAR_NAME,DESC,PARAM,SUPPORT,DESCRIPTION)\
		GLOBAL_VAR_IMPLEMENT(TSTOPICHANDLE,VAR_NAME,TempalteRegisterTopicType<PARAM>(STRING(VAR_NAME),DESC,DESCRIPTION,STRING(PARAM),SUPPORT));


#define GEN_TOPICHELPER_2_I(NAME,EXPORT_DECL)\
TS_TOPIC_DECLARE(NAME,EXPORT_DECL);\
class EXPORT_DECL CONN(NAME,TopicHelper) : public TSTopicHelper\
{\
public:\
	virtual bool CanConvert(TSTOPICHANDLE To);\
	static  bool CanConvertStatic(TSTOPICHANDLE To);\
}\

#define GEN_TOPICHELPER_VAL(NAME,EXPORT_DECL)\
	EXPORT_DECL boost::intrusive_ptr<CONN(NAME,TopicHelper)> CONN(NAME,GetHelper)();\

#define GEN_TOPICHELPER_VAL_IMPL(NAME)\
	 boost::intrusive_ptr<CONN(NAME,TopicHelper)> CONN(NAME,GetHelper)()\
	{\
		static boost::intrusive_ptr<CONN(NAME,TopicHelper)> CONN(NAME,TopicHelper_Val) = new CONN(NAME,TopicHelper)();\
		return CONN(NAME,TopicHelper_Val);\
	}\
	 

#define GEN_TOPICHELPER_2(NAME,EXPORT_DECL)\
	GEN_TOPICHELPER_2_I(NAME,EXPORT_DECL);\
	GEN_TOPICHELPER_VAL(NAME,EXPORT_DECL)\

#define GEN_TOPICHELPER_3_I(NAME,SUP_NAME,EXPORT_DECL)\
class EXPORT_DECL CONN(NAME,TopicHelper) : public CONN(SUP_NAME,TopicHelper)\
{\
public:\
	virtual bool CanConvert(TSTOPICHANDLE To);\
	static  bool CanConvertStatic(TSTOPICHANDLE To);\
}\

#define GEN_TOPICHELPER_3(NAME,SUP_NAME,EXPORT_DECL)\
GEN_TOPICHELPER_3_I(NAME,SUP_NAME,EXPORT_DECL);\
GEN_TOPICHELPER_VAL(NAME,EXPORT_DECL);\


#ifdef _MSC_VER
#		define GEN_TOPICHELPER(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(GEN_TOPICHELPER_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define GEN_TOPICHELPER(...) BOOST_PP_OVERLOAD(GEN_TOPICHELPER_,__VA_ARGS__)(__VA_ARGS__)
#endif

#define GEN_TOPICHELPER_CPP_I(NAME,DESC,PARAM,SUPPORT,HELPER)\
	TS_TOPIC_IMPLEMENT(NAME,DESC,PARAM,SUPPORT,HELPER)\
	bool CONN(NAME,TopicHelper)::CanConvert(TSTOPICHANDLE To){return CanConvertStatic(To);}\
	bool CONN(NAME,TopicHelper)::CanConvertStatic(TSTOPICHANDLE To)\
    {\
        return TSTopicConvertor<CONN(NAME,TopicHelper)>::CanConvertT(To);\
    }\

#define GEN_TOPICHELPER_CPP(NAME,DESC,PARAM,SUPPORT,DESCRIPTION)\
	GEN_TOPICHELPER_CPP_I(NAME,DESC,PARAM,SUPPORT,DESCRIPTION);\

#define HELPER_VAL(NAME) CONN(NAME,GetHelper)()

#define TOPICHELPER_IMPL\
    GEN_TOPICHELPER_CPP(NAME,DESC,DATA_TYPE,TYPE_VAL(NAMESPACE),HELPER_VAL(NAME));\
    GEN_TOPICHELPER_VAL_IMPL(NAME);\

#define CONVERT_TEMPLATE(NAME,SUPNAME)\
template<>\
struct TSTopicConvertor<CONN(NAME,TopicHelper)>\
{\
    typedef CONN(SUPNAME,TopicHelper) SuperConvertHelper_T;\
    \
    FORCEINLINE static bool CanConvertT(TSTOPICHANDLE To)\
    {\
        if(!TSTopicConvertor<SuperConvertHelper_T>::CanConvertT(To)){\
            return To == NAME;\
        }\
        return true;\
    }\
};\

#define TOPICHELPER_DECLARE TS_TOPIC_DECLARE(NAME,EXPORT_MACRO);\
	GEN_TOPICHELPER(NAME,SUPNAME,EXPORT_MACRO);\
	CONVERT_TEMPLATE(NAME,SUPNAME)\



#define HandleToSupport(a) TSTopicTypeManager::Instance()->GetTypeSupportByTopic(a)
#define HandleToName(a) TSTopicTypeManager::Instance()->GetTopicNameByTopic(a)
#define HandleToType(a) TSTopicTypeManager::Instance()->GetTypeNameByTopic(a)
#define GetCfgPath() TSTopicTypeManager::Instance()->GetConfigurePath()
#define GetWorkDirPath() TSTopicTypeManager::Instance()->GetWorkDirectoryPath()
#define GetCfgDoc() TSTopicTypeManager::Instance()->GetConfigureDoc()
#define GetTopicByName(a) TSTopicTypeManager::Instance()->GetTopicByTopicName(a)

#endif



