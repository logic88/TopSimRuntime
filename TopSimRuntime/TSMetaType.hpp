#ifndef __TSMETATYPE_H__
#define __TSMETATYPE_H__
#include <typeinfo>
#ifndef Q_MOC_RUN
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/detail/is_xxx.hpp>
#include <boost/type_traits.hpp>
#include "TSSmartCast.h"
#endif

#include "TSVariant.h"
#include "TSMetaProperty.h"
#include "TSFunctionMetaData.h"
#include "TSActiveSingleton.h"

#ifndef EXTERNAL_METATYPE_USED
#define EXTERNAL_METATYPE_USED	1
#endif

#define TSMETATYPESPEC_FLAG_WRAPED				(1 << 0)
#define TSMETATYPESPEC_FLAG_FORCE_REGISTER		(1 << 1)
#define TSMETATYPESPEC_FLAG_HAS_TYPE_INFO		(1 << 2)

TS_BEGIN_RT_NAMESPACE

class TSMutex;

struct TSIDataArchive;

typedef void * TSInterfaceObjTable;

typedef std::vector<TSMetaProperty *> MetaPropertyCollection;
typedef std::vector<TSBaseFunctionHolderPtr> MetaFunctionCollection;

class TSMetaTypeCaster
{
public:
	virtual void const * Downcast(void const * const t) const =0;
	virtual void const * Upcast(void const * const t) const = 0;
};
typedef boost::shared_ptr<TSMetaTypeCaster> TSMetaTypeCasterPtr;

template <class Derived, class Base>
class TSMetaTypeVirtualBaseCaster : public TSMetaTypeCaster
{
public:
	virtual void const * Downcast(void const * const t) const {
		const Derived * d = 
			dynamic_cast<const Derived *>(
			static_cast<const Base *>(t)
			);
		return d;
	}
	virtual void const * Upcast(void const * const t) const {
		const Base * b = 
			dynamic_cast<const Base *>(
			static_cast<const Derived *>(t)
			);
		return b;
	}
};

template <class Derived, class Base>
class TSMetaTypePrimitiveCaster : public TSMetaTypeCaster
{
	virtual void const * Downcast(void const * const t) const 
	{
		const Derived * d = TSSmartCast<const Derived *, const Base *>(static_cast<const Base *>(t));
		return d;
	}
	virtual void const * Upcast(void const * const t) const 
	{
		const Base * b = TSSmartCast<const Base *, const Derived *>(static_cast<const Derived *>(t));
		return b;
	}
};

template<class Derived, class Base> 
FORCEINLINE TSMetaTypeCasterPtr CreateMetaTypeCaster(Derived const * =0, Base const * =0)
{
	typedef
		BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<boost::is_virtual_base_of<Base,Derived>,
		boost::mpl::identity<
		TSMetaTypeVirtualBaseCaster<Derived, Base>
		>
		,// else
		boost::mpl::identity<
		TSMetaTypePrimitiveCaster<Derived, Base>
		>
		>::type typex;

	return boost::make_shared<typex>();
}

struct TSIDataArchive;

#define  THROW_TSEXCEPTION(a) throw TSException(a)

class TOPSIM_RT_DECL TSMetaTypeNotFoundException : public TSException
{
public:
    TSMetaTypeNotFoundException(const TSString & MetaTypeName) throw();
    virtual ~TSMetaTypeNotFoundException() throw();
	const TSString & GetMetaTypeName() const;
private:
	TSString _MetaTypeName;
};

class TSClass
{
public:
	virtual int GetTypeId() const = 0;
};

template<class Base> 
class TSSubclassOf : public TSClass
{
public:
	TSSubclassOf()
		: MetaTypeId(GetMetaTypeId<Base>(reinterpret_cast<Base *>(0)))
	{
	
	}

	template<class Derived>
	TSSubclassOf(const TSSubclassOf<Derived> & subclass)
		: MetaTypeId(GetMetaTypeId<Derived>(reinterpret_cast<Derived *>(0)))
	{
		BOOST_STATIC_ASSERT((boost::is_base_and_derived<Base,Derived>::value));
	}

	virtual int GetTypeId() const
	{
		return MetaTypeId;
	}
private:
	int MetaTypeId;
};

struct TSMetaExtentionInfo
{
	virtual const UINT8 GetType()=0;
	virtual const TSChar * GetName()=0;
};

typedef boost::shared_ptr<TSMetaExtentionInfo> TSMetaExtentionInfoPtr;

typedef boost::function<bool (int type,void * Obj,TSMetaProperty * Property)> EnterPropertyCallBackType;
typedef boost::function<bool (const TSVariant & Variant,const TSVariant & Owner,TSMetaProperty * Property,const TSString & PropPath)> EnumPropertyCallBackType;

struct TSMetaTypeSpec;

class TOPSIM_RT_DECL TSMetaType 
{
public:
	typedef void (*Destructor)(void *,int);
	typedef void *(*Constructor)(const void *,int);
	typedef void (*NewOperatorT)(void *,const void *,int);
	typedef void (*RefObjSetter)(void *,VoidPtr,int);
	typedef bool (*Serializer)(void *,TSIDataArchive &,int);
	typedef VoidPtr (*SPAssociator)(void *,int);
	typedef void (*SPSharedFromThisOperator)(VoidPtr *,int);
	typedef __MetaTypeObjectInfo * (*GetObjectInfoOperator)(void *,int);
	typedef void (*Initializer)(void *,int);

	static void RegisterParent(int type,int ParentType);
	static void RegisterInterface(int type,int InterfaceTypeId,int ImplementTypeId,TSMetaTypeCasterPtr Caster,RefObjSetter refObjSetter);
	static TSMetaProperty & RegisterProperty(int type,const TSMetaProperty & Property);
	static bool RegisterFunction(int type,TSBaseFunctionHolderPtr FunctionHolder);
	static bool RegisterExtentionInfo(int type,TSMetaExtentionInfoPtr ExtentionInfo);
	static TSMetaExtentionInfoPtr TryRegisterExtentionInfo(int type,UINT8 ExtentionType,int TypeIfNoExist);
	static TSMetaExtentionInfoPtr GetExtentionInfo(int type,UINT8 ExtentionType);
	static bool ExistProperty(int type,const TSChar * propertyName);
	static bool ExistFunction(int type,const TSChar * FunctionName);
	static bool CanConvert(int type,int toType);
	static TSInterfaceObjTable CreateInterfaceObjTable(int type);
	static void DestroyInterfaceObjTable(TSInterfaceObjTable iTable);
	static VoidPtr GetOrCreateInterfaceObj(int type,int InterfaceTypeId,TSInterfaceObjTable iTable,VoidPtr pRefObj,TSMutex & TableLock);
	static const MetaPropertyCollection & GetProperties(int type,bool IncludeInherited=true);
	static const MetaFunctionCollection & GetFunctions(int type,bool IncludeInherited=true);
	static TSATTRHANDLE GetPropertyH(int type,const TSString & PropertyName);
	static TSATTRHANDLE GetPropertyHByVarName(int type,const TSString & VarName);
	static TSFUNCHANDLE GetFunctionH(int type,const TSString & FunctionName);
	static TSFUNCHANDLE GetFunctionH(int type,const TSString & FunctionName,int NumArgs);
	static TSMetaProperty * GetProperty(int type,const TSATTRHANDLE & AttributeHandle);
	static TSBaseFunctionHolderPtr GetFunction(int type,const TSFUNCHANDLE & FuncHandle);
	static int RegisterType(const TSChar *typeName,const TSMetaTypeSpec & MetaTypeSpec);
	static int RegisterTypeForce(const TSChar *typeName,const TSMetaTypeSpec & MetaTypeSpec, bool bForce);
	static int GetType(const TSChar *typeName);
	static std::size_t GetTypeSize(int type);
	static int GetParentType(int type);
	static void GetAllParentTypes(int type,std::vector<int> & ParentTypes);
	static void GetDerivedTypes(int type,std::vector<int> & DerivedTypes);
	static void GetAllDerivedTypes(int type,std::vector<int> & AllDerivedTypes);
	static bool EnterProperty(int type,void * obj,const TSChar * path,EnterPropertyCallBackType EnterCallBack,bool ResizeCollection=false);
	static TSVariant GetObjPropertyValue(int type,void * obj,const TSChar * path);
	static bool SetObjPropertyValue(int type,void * obj,const TSChar * path,const TSVariant & Val,bool ResizeCollection=false);
	static const TSChar *GetTypeName(int type);
	static void * GetTypeMemAddress(int type);
	static TSString GetTypeModuleFileName(int type);
	static const TSChar * GetInheritedPath(int type);
	static void RegisterSerializer(int type,Serializer ser);
	static void RegisterInitializer(int type,Initializer Init);
	static bool DefaultSerialize(int type,void * obj,TSIDataArchive & archive);
	static bool Serialize(int type,void * obj,TSIDataArchive & archive);
	static bool IsRegistered(int type);
	static bool IsAbstract(int type);
	static void * Construct(int type, const void *copy = 0,bool DefaultInit = true);
	static VoidPtr ConstructSmart(int type,const void * copy = 0,bool DefaultInit = true);
	static void NewOperator(int type,void * pObj, const void *pObjToCopy = 0);
	static void Destroy(int type, void *data);
	static void * Cast(int fromtype,int totype, void *fromdata);
	static void UnregisterType(const TSChar *typeName);
	static void UnregisterType(int TypeId);
	static TSVariant DeepCopy(TSVariant & FromVal,bool CopyHandle);
	static int CompareVariant(const TSVariant & FirstVariant,const TSVariant & SecondVariant,UINT16 ExcludeFlag=0);
	static bool FindNewVariantDiff(const TSVariant & OriVariant,const TSVariant & NewVariant,TSVariantHashMap & DiffPaths,UINT16 ExcludeFlag=0);
	static void ApplyVariantDiff(TSVariant & OriVariant,const TSVariantHashMap & DiffPaths);
	static void EnumWholePropertyValue(const TSVariant & Variant,EnumPropertyCallBackType CallBack);
	static void SetPropertiesFromObject(TSVariant & Object,const TSVariant & DataObject);
	static const TSMetaTypeSpec * GetTypeSpec(int type);
	static int GetMaxTypeId();
	static TSMetaProperty & AppendProperty(int type,const TSMetaProperty & Property);
	static bool AppendFunction( int type,TSBaseFunctionHolderPtr FunctionHolder );
	static int GetTypeIdByTypeInfo(const std::type_info & info);
};


template<typename T>
struct TSTypeInfoHelper
{
    static FORCEINLINE TSString TypeInfo()
    {
        return TSMetaType::GetTypeName(GetMetaTypeId<T>(reinterpret_cast<T *>(0)));
    }
};

template<class InterfaceType,class InterfaceImplClass,class RefClass>
void TSMetaTypeRegisterInterfaceHelper(InterfaceType const * =0, InterfaceImplClass const * =0, RefClass const * =0);

template <typename T>
struct TSRegisterMetaTypeHelper;

template<class Derived, class Base> 
void TSMetaTypeRegisterParentHelper(Derived const * =0, Base const * =0)
{
	BOOST_STATIC_ASSERT((boost::is_base_and_derived<Base,Derived>::value));

	const UINT32 BaseType = GetMetaTypeId<Base>(reinterpret_cast<Base *>(0));
	const UINT32 DerivedType = GetMetaTypeId<Derived>(reinterpret_cast<Derived *>(0));

	TSMetaType::RegisterParent(DerivedType,BaseType);
}

template<class Derived> 
void TSMetaTypeRegisterParentHelper(Derived const * =0, TSNullClassTag const * =0)
{
}

template<class InterfaceType,class InterfaceImplClass,class RefClass>
void RegisterInterface()
{
	TSMetaTypeRegisterInterfaceHelper<InterfaceType,InterfaceImplClass,RefClass>();
}

template<class InterfaceImplClass,class RefClass>
void TSInterfaceSetRefObjHelper(InterfaceImplClass *interObj,boost::shared_ptr<RefClass> refObj,int)
{
	interObj->SetRefObj(refObj);
}

template<class T>
FORCEINLINE T DuplicateMetaObject(T & obj,bool CopyHandle)
{
    TSVariant Var = TSVariant::FromValue(obj);
    TSVariant V = TSMetaType::DeepCopy(Var,CopyHandle);
    return V.Value<T>();
}

template<class T>
FORCEINLINE boost::shared_ptr<T> ConstructMetaObject(int MetaTypeId,const void * copy=0 )
{
    return boost::static_pointer_cast<T>(TSMetaType::ConstructSmart(MetaTypeId,copy));
}


template <typename T>
bool Serialize(TSIDataArchive & archive, T & t,int)
{
	return t.Serialize(archive);
}

template <typename T>
bool Serialize2(T * t,TSIDataArchive & archive,int type)
{
	return Serialize(archive,*t,type);
}


template<class T> 
void TSMetaTypeRegisterSerializerHelper(T const * =0)
{
    int MetaTypeId = GetMetaTypeId<T>(reinterpret_cast<T *>(0));
    typedef bool (*Serialize2_T)(T *,TSIDataArchive &,int);
    Serialize2_T Ser_T = Serialize2<T>;
    TSMetaType::Serializer Ser = reinterpret_cast<BOOST_DEDUCED_TYPENAME TSMetaType::Serializer>(Ser_T);
    TSMetaType::RegisterSerializer(MetaTypeId,Ser);
}


template <typename T>
void InitializeTmpl(T & t,int)
{
	t.Initialize();
}

template <typename T>
void InitializeTmpl2(T * t,int type)
{
	InitializeTmpl(*t,type);
}

template<class T> 
void TSMetaTypeRegisterInitializerHelper(T const * =0)
{
	int MetaTypeId = GetMetaTypeId<T>(reinterpret_cast<T *>(0));
	typedef void (*InitializerTmpl2_T)(T *,int);
	InitializerTmpl2_T Init_T = InitializeTmpl2<T>;
	TSMetaType::Initializer Init = reinterpret_cast<BOOST_DEDUCED_TYPENAME TSMetaType::Initializer>(Init_T);
	TSMetaType::RegisterInitializer(MetaTypeId,Init);
}

template <typename T,bool has_copy_construct>
struct  TSMetaNoCopyableTypeConstructor;

template <typename T>
struct  TSMetaNoCopyableTypeConstructor<T,true>
{
	static void * Construct(const T *t)
	{
		void * ret = NULL;
		if (!t)
		{
			ret = new T;
		}
		else
		{
			ret = new T(*static_cast<const T*>(t));	
		}
		return ret;	
	}

	static void NewOperator(void * p,const T *t)
	{
		if (!t)
		{
			::new(p) T();
		}
		else
		{
			::new(p) T(*t);
		}
	}
};

template <typename T>
struct  TSMetaNoCopyableTypeConstructor<T,false>
{
	static void * Construct(const T *t)
	{
		UNUSED(t);

		return new T;
	}

	static void NewOperator(void * p,const T *t)
	{
		UNUSED(t);

		::new(p) T();
	}
};


template <typename T>
FORCEINLINE void * TSMetaNoCopyableTypeConstruct(const T *t,const BOOST_DEDUCED_TYPENAME T::CopyableType * = 0)
{
	return TSMetaNoCopyableTypeConstructor< T,boost::is_base_of<BOOST_DEDUCED_TYPENAME T::NoCopyableType,BOOST_DEDUCED_TYPENAME T::CopyableType>::value >::Construct(t);
}


template <typename T>
FORCEINLINE void * TSMetaNoCopyableTypeConstruct(const T *t,const void * = 0)
{
	UNUSED(t);

	return new T;
}

template <typename T>
FORCEINLINE void * TSMetaTypeConstruct(const T *t,const BOOST_DEDUCED_TYPENAME T::NoCopyableType * = 0)
{
	return TSMetaNoCopyableTypeConstruct(t,t);
}

template <typename T>
FORCEINLINE void * TSMetaTypeConstruct(const T *t,const void * = 0)
{
	void * ret = NULL;
	if (!t)
	{
		ret = new T;
	}
	else
	{
		ret = new T(*static_cast<const T*>(t));	
	}
	return ret;	
}


template <typename T>
FORCEINLINE void TSMetaNoCopyableTypeNewOperator(void * p,const T *t,const BOOST_DEDUCED_TYPENAME T::CopyableType * = 0)
{
	return TSMetaNoCopyableTypeConstructor< T,boost::is_base_of<BOOST_DEDUCED_TYPENAME T::NoCopyableType,BOOST_DEDUCED_TYPENAME T::CopyableType>::value >::NewOperator(p,t);
}


template <typename T>
FORCEINLINE void TSMetaNoCopyableTypeNewOperator(void * p,const T *t,const void * = 0)
{
	UNUSED(t);

	::new(p) T();
}

template <typename T>
FORCEINLINE void TSMetaTypeNewOperator(void * p,const T *t,const BOOST_DEDUCED_TYPENAME T::NoCopyableType * = 0)
{
	TSMetaNoCopyableTypeNewOperator(p,t,t);
}

template <typename T>
FORCEINLINE void TSMetaTypeNewOperator(void * p,const T *t,const void * = 0)
{
	if (!t)
	{
		::new(p) T();
	}
	else
	{
		::new(p) T(*t);
	}
}

template<class T>
FORCEINLINE bool DefaultSerialize(T * obj,TSIDataArchive & archive)
{
	return TSMetaType::DefaultSerialize(GetMetaTypeId<T>(reinterpret_cast<T *>(0)),obj,archive);
}

template<class T>
FORCEINLINE VoidPtr AssociateSPHelper(void * p)
{
	return boost::shared_ptr<T>(static_cast<T *>(p));
}

template <typename T>
struct TSObtainObjectInfoOp
{
	static FORCEINLINE __MetaTypeObjectInfo * GetObjectInfo(T * t,int)
	{
		return t->__get__obj_info();
	}
};

template <typename T>
FORCEINLINE __MetaTypeObjectInfo * TSGetObjectInfo(T * t,int type,BOOST_DEDUCED_TYPENAME T::MetaTypeIdDefined * = 0)
{
	return TSObtainObjectInfoOp<T>::GetObjectInfo(t,type);
}

template <typename T>
FORCEINLINE __MetaTypeObjectInfo * TSGetObjectInfo(T * t,int type,const void * = 0)
{
	return NULL;
}

template <typename T>
FORCEINLINE bool TSGetIsForceRegisterType(T * t, BOOST_DEDUCED_TYPENAME T::ForceRegisterType * = 0)
{
	return boost::is_same<T,BOOST_DEDUCED_TYPENAME T::ForceRegisterType>::value;
}

template <typename T>
FORCEINLINE bool TSGetIsForceRegisterType(T * t,const void * = 0)
{
	return false;
}

template <typename T>
struct TSMetaTypeOp
{
	static FORCEINLINE void * Construct(const T *t,int)
	{
		return TSMetaTypeConstruct<T>(t,t);	
	}
	static FORCEINLINE void Delete(T *t,int)
	{
		delete t;
	}
	static FORCEINLINE void NewOperator(void *p,const T *t,int)
	{
		TSMetaTypeNewOperator<T>(p,t,t);
	}

	static FORCEINLINE bool Serialize(T * t,TSIDataArchive & archive,int)
	{
		return DefaultSerialize<T>(t,archive);
	}
	static FORCEINLINE VoidPtr AssociateSP(void * p,int)
	{
		return AssociateSPHelper<T>(p);
	}
	static FORCEINLINE void SharedFromThis(VoidPtr * pObjPtr,int)
	{
		boost::shared_ptr<T> TypedObjPtr = boost::static_pointer_cast<T>(*pObjPtr);

		boost::detail::sp_enable_shared_from_this(&TypedObjPtr,TypedObjPtr.get(),TypedObjPtr.get());
	}

	static FORCEINLINE __MetaTypeObjectInfo * GetObjectInfo(T * t,int type)
	{
		return TSGetObjectInfo(t,type,t);
	}

	static FORCEINLINE int GetFlag()
	{
		return TSGetIsForceRegisterType(reinterpret_cast<T *>(0), reinterpret_cast<T *>(0)) ? TSMETATYPESPEC_FLAG_FORCE_REGISTER : 0;
	}
};

FORCEINLINE TSMetaType::RefObjSetter GetInterfaceRefObjSetter(void *,void *)
{
	return 0;
}

template<class InterfaceImplClass,class RefClass>
FORCEINLINE  TSMetaType::RefObjSetter GetInterfaceRefObjSetter(InterfaceImplClass *,RefClass *,BOOST_DEDUCED_TYPENAME InterfaceImplClass::RefObjType * = 0)
{
        typedef void (* TSInterfaceSetRefObjHelper_T)(InterfaceImplClass *,boost::shared_ptr<RefClass>,int);

        TSInterfaceSetRefObjHelper_T t = TSInterfaceSetRefObjHelper<InterfaceImplClass,RefClass>;

        return reinterpret_cast<BOOST_DEDUCED_TYPENAME TSMetaType::RefObjSetter>(t);
}

template<class InterfaceType,class InterfaceImplClass,class RefClass> 
void TSMetaTypeRegisterInterfaceHelper(InterfaceType const *, InterfaceImplClass const *, RefClass const *)
{
	const UINT32 RefClassId  = GetMetaTypeId<RefClass>(reinterpret_cast<RefClass *>(0));
	const UINT32 InterfaceTypeId = GetMetaTypeId<InterfaceType>(reinterpret_cast<InterfaceType *>(0));
	const UINT32 ImplementTypeId = GetMetaTypeId<InterfaceImplClass>(reinterpret_cast<InterfaceImplClass *>(0));

	//"interface implement class is abstract!"
	BOOST_STATIC_ASSERT(!(boost::is_abstract<InterfaceImplClass>::value));
	//"interface implement cannot  derived from ref class!"
	BOOST_STATIC_ASSERT(!(boost::is_base_and_derived<RefClass,InterfaceImplClass>::value));
	//"class not derived from interface!"
	BOOST_STATIC_ASSERT((boost::is_base_and_derived<InterfaceType,InterfaceImplClass>::value));

    TSMetaType::RefObjSetter Setter = GetInterfaceRefObjSetter(reinterpret_cast<InterfaceImplClass *>(0),reinterpret_cast<RefClass *>(0));

	TSMetaType::RegisterInterface(RefClassId,InterfaceTypeId,ImplementTypeId,CreateMetaTypeCaster<InterfaceImplClass,InterfaceType>(),Setter);
}


struct TOPSIM_RT_DECL TSMetaTypeSpec
{
	int TypeId;
	TSMetaType::Constructor Constr;
	TSMetaType::Destructor Destr;
	TSMetaType::NewOperatorT NewOp;
	TSMetaType::SPAssociator Ass;
	TSMetaType::Serializer Ser;
	TSMetaType::Initializer Init;
	TSMetaType::SPSharedFromThisOperator SharedFromThisOp;
	TSMetaType::GetObjectInfoOperator GetObjectInfoOp;
	std::size_t Size;
	UINT8 Flag;
	const std::type_info * TypeInfo;
};

template <typename T>
struct TSRegisterAbstractMetaTypeHelper
{
    static FORCEINLINE int Register(const TSChar *typeName, T * = 0)
	{
		TSMetaTypeSpec MetaTypeSpec;

		MetaTypeSpec.TypeId = 0;
		MetaTypeSpec.Constr = reinterpret_cast<TSMetaType::Constructor>(0);
		MetaTypeSpec.Destr = reinterpret_cast<TSMetaType::Destructor>(TSMetaTypeOp<T>::Delete);
		MetaTypeSpec.NewOp = reinterpret_cast<TSMetaType::NewOperatorT>(0);
		MetaTypeSpec.Ass = reinterpret_cast<TSMetaType::SPAssociator>(TSMetaTypeOp<T>::AssociateSP);
		MetaTypeSpec.Ser = reinterpret_cast<TSMetaType::Serializer>(TSMetaTypeOp<T>::Serialize);
		MetaTypeSpec.Init = 0;
		MetaTypeSpec.SharedFromThisOp = reinterpret_cast<TSMetaType::SPSharedFromThisOperator>(TSMetaTypeOp<T>::SharedFromThis);
		MetaTypeSpec.GetObjectInfoOp = reinterpret_cast<TSMetaType::GetObjectInfoOperator>(TSMetaTypeOp<T>::GetObjectInfo);
		MetaTypeSpec.Size = sizeof(T);
		MetaTypeSpec.Flag = TSMetaTypeOp<T>::GetFlag() | TSMETATYPESPEC_FLAG_HAS_TYPE_INFO;
		MetaTypeSpec.TypeInfo = &typeid(T);
        return TSMetaType::RegisterType(typeName,MetaTypeSpec);
	}
};

template <typename T>
struct TSRegisterMetaTypeHelper
{
	static FORCEINLINE int Register(const TSChar *typeName, T * = 0)
	{
		TSMetaTypeSpec MetaTypeSpec;

		MetaTypeSpec.TypeId = 0;
		MetaTypeSpec.Constr = reinterpret_cast<TSMetaType::Constructor>(TSMetaTypeOp<T>::Construct);
		MetaTypeSpec.Destr = reinterpret_cast<TSMetaType::Destructor>(TSMetaTypeOp<T>::Delete);
		MetaTypeSpec.NewOp = reinterpret_cast<TSMetaType::NewOperatorT>(TSMetaTypeOp<T>::NewOperator);
		MetaTypeSpec.Ass = reinterpret_cast<TSMetaType::SPAssociator>(TSMetaTypeOp<T>::AssociateSP);
		MetaTypeSpec.Ser = reinterpret_cast<TSMetaType::Serializer>(TSMetaTypeOp<T>::Serialize);
		MetaTypeSpec.Init = 0;
		MetaTypeSpec.SharedFromThisOp = reinterpret_cast<TSMetaType::SPSharedFromThisOperator>(TSMetaTypeOp<T>::SharedFromThis);
		MetaTypeSpec.GetObjectInfoOp = reinterpret_cast<TSMetaType::GetObjectInfoOperator>(TSMetaTypeOp<T>::GetObjectInfo);
		MetaTypeSpec.Size = sizeof(T);
		MetaTypeSpec.Flag = TSMetaTypeOp<T>::GetFlag() | TSMETATYPESPEC_FLAG_HAS_TYPE_INFO;
		MetaTypeSpec.TypeInfo = &typeid(T);

        return TSMetaType::RegisterType(typeName,MetaTypeSpec);
	}
};

template <typename T>
int TSRegisterMetaType(const TSChar *typeName, T * dummy = 0)
{
	typedef 
		//if
        BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<boost::is_abstract<T>,boost::mpl::identity< TSRegisterAbstractMetaTypeHelper<T> >,
		//else
        boost::mpl::identity< TSRegisterMetaTypeHelper<T> >
		>::type typex;

	return typex::Register(typeName,dummy);
}


template <typename T>
FORCEINLINE int TSRegisterMetaType(T * dummy = 0)
{
	return GetMetaTypeId(dummy);
}


template<class T>
struct MetaDataCollector;

template<class T>
struct MetaDataCollectorRegister;

TS_END_RT_NAMESPACE

#if EXTERNAL_METATYPE_USED
#define TS_DECLARE_METATYPE(TYPE)                                        \
	TS_BEGIN_RT_NAMESPACE \
	template <>                                                         \
	struct TSMetaTypeId< TYPE >                                          \
	{                                                                   \
		static int GetMetaTypeId(TYPE * obj)                                     \
		{                                                           \
			static INT32 metatype_id = 0; \
			if (!metatype_id)                                       \
			{\
				metatype_id = TSRegisterMetaType< TYPE >(#TYPE, obj); \
			}\
			return obj ? TSMetaType::GetTypeIdByTypeInfo(typeid(*obj)) : metatype_id;\
		}\
	}; \
	TS_END_RT_NAMESPACE
#endif

#define TS_DECLARE_BUILTIN_METATYPE(TYPE, NAME) \
	TS_BEGIN_RT_NAMESPACE \
	template<> struct TSMetaTypeId2<TYPE> \
	{ \
        static FORCEINLINE int GetMetaTypeId(TYPE *) { return TSVariant::NAME; } \
	}; \
	TS_END_RT_NAMESPACE

#define TS_META_PROP(GETTER,SETTER,VALNAME,NAME,OWNINGTYPE) TSMetaProperty(VALNAME,NAME,TSCreatePropertyOperator(GETTER,SETTER),OWNINGTYPE)
#define TS_META_PROP_READONLY(GETTER,VALNAME,NAME,OWNINGTYPE) TSMetaProperty(VALNAME,NAME,TSCreatePropertyOperator(GETTER),OWNINGTYPE)
#define TS_META_FIELD(FIELD,VALNAME,NAME,OWNINGTYPE) TSMetaProperty(VALNAME,NAME,TSCreateFieldOperator(FIELD),OWNINGTYPE)


#define VERIFY_MODEL_IMPL(m) \
	BOOST_MPL_ASSERT_MSG(boost::is_abstract<m>::value == false, class_##m##_is_abstract, (m))


#define BEGIN_METADATA(CLASS) \
	UINT32 CLASS::GetMetaTypeIdStatic()\
	{\
		static UINT32 __metatype_id_static = 0;\
		if (!__metatype_id_static)\
		{\
			__metatype_id_static = TSRegisterMetaType(TS_TEXT(#CLASS),reinterpret_cast< ThisType *>(0));\
			TSMetaTypeRegisterParentHelper(reinterpret_cast< ThisType *>(0),reinterpret_cast< SuperType *>(0));\
		}\
		return __metatype_id_static;\
	}\
	const TSChar * CLASS::GetMetaTypeNameStatic()\
	{\
		return TS_TEXT(#CLASS);\
	}\
	UINT32 CLASS::GetMetaTypeId() const\
	{\
		if (__get__obj_info()->__metatype_ext)\
		{\
			return __get__obj_info()->__metatype_ext->MetaTypeId;\
		}\
		return GetMetaTypeIdStatic();\
	}\
	const TSChar * CLASS::GetMetaTypeName() const\
	{\
		if (__get__obj_info()->__metatype_ext)\
		{\
			return TSMetaType::GetTypeName(__get__obj_info()->__metatype_ext->MetaTypeId);\
		}\
		return GetMetaTypeNameStatic();\
	}\
	TS_BEGIN_RT_NAMESPACE	\
	template<>\
	struct MetaDataCollector<CLASS>\
	{\
		typedef CLASS ThisType;\
		MetaDataCollector();\
		static void * Construct();\
	};\
	template<>\
	struct MetaDataCollectorRegister<CLASS>\
	{ \
		MetaDataCollectorRegister() \
		{ \
			TSActiveSingletonRegistry::Instance()->Register(reinterpret_cast<void *>(MetaDataCollector<CLASS>::Construct), MetaDataCollector<CLASS>::Construct, "MetaDataCollector<" #CLASS ">"); \
		}\
		static MetaDataCollectorRegister INITIALIZER;\
	};\
	TS_END_RT_NAMESPACE \
	MetaDataCollectorRegister<CLASS> MetaDataCollectorRegister<CLASS>::INITIALIZER;\
	void * MetaDataCollector<CLASS>::Construct() { static MetaDataCollector<CLASS> Obj; return &Obj; }\
	MetaDataCollector<CLASS>::MetaDataCollector()\
	{\
		TSRegisterMetaType<ThisType>(reinterpret_cast<ThisType *>(0));

#define BEGIN_METADATA_NS(NS,CLASS) \
	UINT32 NS::CLASS::GetMetaTypeIdStatic()\
	{\
		static UINT32 __metatype_id_static = 0;\
		if (!__metatype_id_static)\
		{\
			__metatype_id_static = TSRegisterMetaType(TS_TEXT(#CLASS),reinterpret_cast< ThisType *>(0));\
			TSMetaTypeRegisterParentHelper(reinterpret_cast< ThisType *>(0),reinterpret_cast< SuperType *>(0));\
		}\
		return __metatype_id_static;\
	}\
	const TSChar * NS::CLASS::GetMetaTypeNameStatic()\
	{\
		return TS_TEXT(#CLASS);\
	}\
	UINT32 NS::CLASS::GetMetaTypeId() const\
	{\
		if (__get__obj_info()->__metatype_ext)\
		{\
			return __get__obj_info()->__metatype_ext->MetaTypeId;\
		}\
		return GetMetaTypeIdStatic();\
	}\
	const TSChar * NS::CLASS::GetMetaTypeName() const\
	{\
		if (__get__obj_info()->__metatype_ext)\
		{\
			return TSMetaType::GetTypeName(__get__obj_info()->__metatype_ext->MetaTypeId);\
		}\
		return GetMetaTypeNameStatic();\
	}\
	TS_BEGIN_RT_NAMESPACE	\
	template<>\
	struct MetaDataCollector<NS::CLASS>\
	{\
		typedef NS::CLASS ThisType;\
		MetaDataCollector();\
		static void * Construct();\
	};\
	template<>\
	struct MetaDataCollectorRegister<NS::CLASS>\
	{ \
		MetaDataCollectorRegister() \
		{ \
			TSActiveSingletonRegistry::Instance()->Register(reinterpret_cast<void *>(MetaDataCollector<NS::CLASS>::Construct), MetaDataCollector<NS::CLASS>::Construct, "MetaDataCollector<" #CLASS ">"); \
		}\
		static MetaDataCollectorRegister INITIALIZER;\
	};\
	TS_END_RT_NAMESPACE \
	MetaDataCollectorRegister<NS::CLASS> MetaDataCollectorRegister<NS::CLASS>::INITIALIZER;\
	void * MetaDataCollector<NS::CLASS>::Construct() { static MetaDataCollector<NS::CLASS> Obj; return &Obj; }\
	MetaDataCollector<NS::CLASS>::MetaDataCollector()\
	{\
		TSRegisterMetaType<ThisType>(reinterpret_cast<ThisType *>(0));

#define REG_BASE_MACRO_1(SUPERTYPE)  \
	typedef SUPERTYPE SuperType;\
	TSMetaTypeRegisterParentHelper(reinterpret_cast<ThisType *>(0),reinterpret_cast<SuperType *>(0))

#define REG_BASE_MACRO_2(SUPERTYPE,dummy)

#ifdef _MSC_VER
#		define REG_BASE(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(REG_BASE_MACRO_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define REG_BASE(...) BOOST_PP_OVERLOAD(REG_BASE_MACRO_,__VA_ARGS__)(__VA_ARGS__)
#endif // _MSC_VER


#define REG_INTERFACE(INTERFACETYPE)  \
	TSMetaTypeRegisterInterfaceHelper(reinterpret_cast<INTERFACETYPE *>(0),reinterpret_cast<ThisType *>(0),reinterpret_cast<ThisType *>(0))
#ifdef _MSC_VER
#define REG_PROP_DETAIL(GETVALNAME,SETVALNAME,VALNAME,NAME)  \
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakePropGetFunctionData<ThisType>(TS_TEXT(#GETVALNAME),&ThisType::##GETVALNAME));\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakePropSetFunctionData<ThisType>(TS_TEXT(#SETVALNAME),&ThisType::##SETVALNAME));\
	TSMetaType::RegisterProperty(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TS_META_PROP(&ThisType::##GETVALNAME,&ThisType::##SETVALNAME,VALNAME,NAME,GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0))))

#define REG_PROP_READONLY_DETAIL(GETVALNAME,VALNAME,NAME)  \
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakePropGetFunctionData<ThisType>(TS_TEXT(#GETVALNAME),&ThisType::##GETVALNAME));\
	TSMetaType::RegisterProperty(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TS_META_PROP_READONLY(&ThisType::##GETVALNAME,VALNAME,NAME,GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0))))
#else
#define REG_PROP_DETAIL(GETVALNAME,SETVALNAME,VALNAME,NAME)  \
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakePropGetFunctionData<ThisType>(#GETVALNAME,&ThisType::GETVALNAME));\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakePropSetFunctionData<ThisType>(#SETVALNAME,&ThisType::SETVALNAME));\
	TSMetaType::RegisterProperty(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TS_META_PROP(&ThisType::GETVALNAME,&ThisType::SETVALNAME,VALNAME,NAME,GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0))))

#define REG_PROP_READONLY_DETAIL(GETVALNAME,VALNAME,NAME)  \
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakePropGetFunctionData<ThisType>(#GETVALNAME,&ThisType::GETVALNAME));\
	TSMetaType::RegisterProperty(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TS_META_PROP_READONLY(&ThisType::GETVALNAME,VALNAME,NAME,GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0))))
#endif
#define REG_PROP_MACRO_2(VALNAME,NAME)  \
	REG_PROP_DETAIL(Get##VALNAME,Set##VALNAME,TS_TEXT(#VALNAME),TS_TEXT(NAME))

#define REG_PROP_MACRO_1(VALNAME)  \
	REG_PROP_DETAIL(Get##VALNAME,Set##VALNAME,TS_TEXT(#VALNAME),TS_TEXT(#VALNAME))

#define REG_PROP2(VALNAME) REG_PROP_MACRO_1(VALNAME)

#ifdef _MSC_VER
#		define REG_PROP(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(REG_PROP_MACRO_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define REG_PROP(...) BOOST_PP_OVERLOAD(REG_PROP_MACRO_,__VA_ARGS__)(__VA_ARGS__)
#endif // _MSC_VER

#define REG_PROP_READONLY_MACRO_2(VALNAME,NAME)  \
	REG_PROP_READONLY_DETAIL(Get##VALNAME,TS_TEXT(#VALNAME),TS_TEXT(NAME))

#define REG_PROP_READONLY_MACRO_1(VALNAME)  \
	REG_PROP_READONLY_DETAIL(Get##VALNAME,TS_TEXT(#VALNAME),TS_TEXT(#VALNAME))

#ifdef _MSC_VER
#		define REG_PROP_READONLY(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(REG_PROP_READONLY_MACRO_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define REG_PROP_READONLY(...) BOOST_PP_OVERLOAD(REG_PROP_READONLY_MACRO_,__VA_ARGS__)(__VA_ARGS__)
#endif // _MSC_VER

#define REG_PROP_READONLY_SPEC_MACRO_2(GETVALNAME,NAME)  \
	REG_PROP_READONLY_DETAIL(GETVALNAME,TS_TEXT(#GETVALNAME),TS_TEXT(NAME))

#define REG_PROP_READONLY_SPEC_MACRO_1(GETVALNAME)  \
	REG_PROP_READONLY_DETAIL(GETVALNAME,TS_TEXT(#GETVALNAME),TS_TEXT(#GETVALNAME))

#ifdef _MSC_VER
#		define REG_PROP_READONLY_SPEC(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(REG_PROP_READONLY_SPEC_MACRO_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define REG_PROP_READONLY_SPEC(...) BOOST_PP_OVERLOAD(REG_PROP_READONLY_SPEC_MACRO_,__VA_ARGS__)(__VA_ARGS__)
#endif // _MSC_VER

#define REG_PROP_FIELD_MACRO_2(VALNAME,NAME)  \
	TSMetaType::RegisterProperty(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TS_META_FIELD(&ThisType::VALNAME,TS_TEXT(#VALNAME),TS_TEXT(NAME),GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0))))

#define REG_PROP_FIELD_MACRO_1(VALNAME)  \
	TSMetaType::RegisterProperty(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TS_META_FIELD(&ThisType::VALNAME,TS_TEXT(#VALNAME),TS_TEXT(#VALNAME),GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0))))

#define REG_PROP_FIELD2(VALNAME) REG_PROP_FIELD_MACRO_1(VALNAME)

#ifdef _MSC_VER
#		define REG_PROP_FIELD(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(REG_PROP_FIELD_MACRO_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define REG_PROP_FIELD(...) BOOST_PP_OVERLOAD(REG_PROP_FIELD_MACRO_,__VA_ARGS__)(__VA_ARGS__)
#endif // _MSC_VER


#define REG_FUNC_MACRO_3(FUNCNAME,NAME,ARGS)  \
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemFunctionData<ThisType>(TS_TEXT(NAME),&ThisType::FUNCNAME,TS_TEXT(ARGS)))

#define REG_FUNC_MACRO_2(FUNCNAME,NAME)  \
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemFunctionData<ThisType>(TS_TEXT(NAME),&ThisType::FUNCNAME))

#define REG_FUNC_MACRO_1(FUNCNAME)  \
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemFunctionData<ThisType>(TS_TEXT(#FUNCNAME),&ThisType::FUNCNAME))

#ifdef _MSC_VER
#		define REG_FUNC(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(REG_FUNC_MACRO_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define REG_FUNC(...) BOOST_PP_OVERLOAD(REG_FUNC_MACRO_,__VA_ARGS__)(__VA_ARGS__)
#endif // _MSC_VER

#define REG_FUNC_OVERLOAD_RET_MACRO_10(FUNCNAME,RET_T,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8)  \
struct FUNCNAME##_Overload_Ret_8 {static RET_T FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3,ARG4 a4,ARG5 a5,ARG6 a6,ARG7 a7,ARG8 a8) { return obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3,a4,a5,a6,a7,a8); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_Ret_8::FUNCNAME))

#define REG_FUNC_OVERLOAD_RET_MACRO_9(FUNCNAME,RET_T,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7)  \
struct FUNCNAME##_Overload_Ret_7 {static RET_T FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3,ARG4 a4,ARG5 a5,ARG6 a6,ARG7 a7) { return obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3,a4,a5,a6,a7); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_Ret_7::FUNCNAME))

#define REG_FUNC_OVERLOAD_RET_MACRO_8(FUNCNAME,RET_T,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6)  \
struct FUNCNAME##_Overload_Ret_6 {static RET_T FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3,ARG4 a4,ARG5 a5,ARG6 a6) { return obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3,a4,a5,a6); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_Ret_6::FUNCNAME))


#define REG_FUNC_OVERLOAD_RET_MACRO_7(FUNCNAME,RET_T,ARG1,ARG2,ARG3,ARG4,ARG5)  \
struct FUNCNAME##_Overload_Ret_5 {static RET_T FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3,ARG4 a4,ARG5 a5) { return obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3,a4,a5); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_Ret_5::FUNCNAME))


#define REG_FUNC_OVERLOAD_RET_MACRO_6(FUNCNAME,RET_T,ARG1,ARG2,ARG3,ARG4)  \
struct FUNCNAME##_Overload_Ret_4 {static RET_T FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3,ARG4 a4) { return obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3,a4); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_Ret_4::FUNCNAME))


#define REG_FUNC_OVERLOAD_RET_MACRO_5(FUNCNAME,RET_T,ARG1,ARG2,ARG3)  \
struct FUNCNAME##_Overload_Ret_3 {static RET_T FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3) { return obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_Ret_3::FUNCNAME))

#define REG_FUNC_OVERLOAD_RET_MACRO_4(FUNCNAME,RET_T,ARG1,ARG2)  \
struct FUNCNAME##_Overload_Ret_2 {static RET_T FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2) { return obj.Value<ThisType *>()->FUNCNAME(a1,a2); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_Ret_2::FUNCNAME))

#define REG_FUNC_OVERLOAD_RET_MACRO_3(FUNCNAME,RET_T,ARG1)  \
struct FUNCNAME##_Overload_Ret_1 {static RET_T FUNCNAME(TSVariant & obj,ARG1 a1) { return obj.Value<ThisType *>()->FUNCNAME(a1); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_Ret_1::FUNCNAME))

#define REG_FUNC_OVERLOAD_RET_MACRO_2(FUNCNAME,RET_T)  \
struct FUNCNAME##_Overload_Ret_0 {static RET_T FUNCNAME(TSVariant & obj) { return obj.Value<ThisType *>()->FUNCNAME(); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_Ret_0::FUNCNAME))

#ifdef _MSC_VER
#		define REG_FUNC_OVERLOAD_RET(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(REG_FUNC_OVERLOAD_RET_MACRO_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define REG_FUNC_OVERLOAD_RET(...) BOOST_PP_OVERLOAD(REG_FUNC_OVERLOAD_RET_MACRO_,__VA_ARGS__)(__VA_ARGS__)
#endif // _MSC_VER

#define REG_FUNC_OVERLOAD_VOID_MACRO_9(FUNCNAME,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8)  \
struct FUNCNAME##_Overload_8 {static void FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3,ARG4 a4,ARG5 a5,ARG6 a6,ARG7 a7,ARG8 a8) { obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3,a4,a5,a6,a7,a8); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_8::FUNCNAME))

#define REG_FUNC_OVERLOAD_VOID_MACRO_8(FUNCNAME,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7)  \
struct FUNCNAME##_Overload_7 {static void FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3,ARG4 a4,ARG5 a5,ARG6 a6,ARG7 a7) { obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3,a4,a5,a6,a7); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_7::FUNCNAME))

#define REG_FUNC_OVERLOAD_VOID_MACRO_7(FUNCNAME,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6)  \
struct FUNCNAME##_Overload_6 {static void FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3,ARG4 a4,ARG5 a5,ARG6 a6) { obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3,a4,a5,a6); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_6::FUNCNAME))


#define REG_FUNC_OVERLOAD_VOID_MACRO_6(FUNCNAME,ARG1,ARG2,ARG3,ARG4,ARG5)  \
struct FUNCNAME##_Overload_5 {static void FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3,ARG4 a4,ARG5 a5) { obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3,a4,a5); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_5::FUNCNAME))


#define REG_FUNC_OVERLOAD_VOID_MACRO_5(FUNCNAME,ARG1,ARG2,ARG3,ARG4)  \
struct FUNCNAME##_Overload_4 {static void FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3,ARG4 a4) { obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3,a4); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_4::FUNCNAME))


#define REG_FUNC_OVERLOAD_VOID_MACRO_4(FUNCNAME,ARG1,ARG2,ARG3)  \
struct FUNCNAME##_Overload_3 {static void FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2,ARG3 a3) { obj.Value<ThisType *>()->FUNCNAME(a1,a2,a3); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_3::FUNCNAME))

#define REG_FUNC_OVERLOAD_VOID_MACRO_3(FUNCNAME,ARG1,ARG2)  \
struct FUNCNAME##_Overload_2 {static void FUNCNAME(TSVariant & obj,ARG1 a1,ARG2 a2) { obj.Value<ThisType *>()->FUNCNAME(a1,a2); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_2::FUNCNAME))

#define REG_FUNC_OVERLOAD_VOID_MACRO_2(FUNCNAME,ARG1)  \
struct FUNCNAME##_Overload_1 {static void FUNCNAME(TSVariant & obj,ARG1 a1) { obj.Value<ThisType *>()->FUNCNAME(a1); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_1::FUNCNAME))

#define REG_FUNC_OVERLOAD_VOID_MACRO_1(FUNCNAME)  \
struct FUNCNAME##_Overload_0 {static void FUNCNAME(TSVariant & obj) { obj.Value<ThisType *>()->FUNCNAME(); }};\
	TSMetaType::RegisterFunction(GetMetaTypeId<ThisType>(reinterpret_cast<ThisType *>(0)),TSMakeMemWrapperFunctionData(TS_TEXT(#FUNCNAME),FUNCNAME##_Overload_0::FUNCNAME))

#ifdef _MSC_VER
#		define REG_FUNC_OVERLOAD_VOID(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(REG_FUNC_OVERLOAD_VOID_MACRO_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define REG_FUNC_OVERLOAD_VOID(...) BOOST_PP_OVERLOAD(REG_FUNC_OVERLOAD_VOID_MACRO_,__VA_ARGS__)(__VA_ARGS__)
#endif // _MSC_VER


#define REG_SER()\
	TSMetaTypeRegisterSerializerHelper(reinterpret_cast<ThisType *>(0));

#define REG_INIT()\
	TSMetaTypeRegisterInitializerHelper(reinterpret_cast<ThisType *>(0));


#define END_METADATA()	}

#define BEGIN_EXTERNAL_METADATA(CLASS) \
	TS_BEGIN_RT_NAMESPACE	\
	template<>\
	struct MetaDataCollector<CLASS>\
	{\
		typedef CLASS ThisType;\
		MetaDataCollector();\
		static void * Construct();\
	};\
	template<>\
	struct MetaDataCollectorRegister<CLASS>\
	{ \
		MetaDataCollectorRegister() \
		{ \
			TSActiveSingletonRegistry::Instance()->Register(reinterpret_cast<void *>(MetaDataCollector<CLASS>::Construct), MetaDataCollector<CLASS>::Construct, "MetaDataCollector<" #CLASS ">"); \
		}\
		static MetaDataCollectorRegister INITIALIZER;\
	};\
	TS_END_RT_NAMESPACE \
	MetaDataCollectorRegister<CLASS> MetaDataCollectorRegister<CLASS>::INITIALIZER;\
	void * MetaDataCollector<CLASS>::Construct() { static MetaDataCollector<CLASS> Obj; return &Obj; }\
	MetaDataCollector<CLASS>::MetaDataCollector()\
	{\
		TSRegisterMetaType<ThisType>(reinterpret_cast<ThisType *>(0));

#define END_EXTERNAL_METADATA() END_METADATA()


#define IMPLEMENT_METADATA(CLASS) BEGIN_METADATA(CLASS);END_METADATA();

#define TSFORCE_REGISTER(CLASS)\
	typedef CLASS ForceRegisterType

#define GLOBAL_VAR_DECLARE(TYPE,NAME) extern TYPE NAME;
#define GLOBAL_VAR_IMPLEMENT(TYPE,NAME,INITIALIZER) TYPE NAME; \
	struct __##NAME##_DelayInitializer__ { __##NAME##_DelayInitializer__(){ NAME = INITIALIZER; } \
	static void * Construct() { static __##NAME##_DelayInitializer__ Obj; return &Obj; } };\
	struct __##NAME##_DelayInitializerRegister__ { __##NAME##_DelayInitializerRegister__() \
	{ TSActiveSingletonRegistry::Instance()->Register(reinterpret_cast<void *>\
	(__##NAME##_DelayInitializer__::Construct), __##NAME##_DelayInitializer__::Construct,"Global variable[" #TYPE " " #NAME "]"); } }; \
	static __##NAME##_DelayInitializerRegister__  __##NAME##_DelayInitializerRegisterInitializer__;


TS_DECLARE_BUILTIN_METATYPE(void,VT_INVALID);
TS_DECLARE_BUILTIN_METATYPE(bool,VT_BOOL);
TS_DECLARE_BUILTIN_METATYPE(UINT8,VT_UINT8);
TS_DECLARE_BUILTIN_METATYPE(UINT16,VT_UINT16);
TS_DECLARE_BUILTIN_METATYPE(UINT32,VT_UINT32);
TS_DECLARE_BUILTIN_METATYPE(TSHANDLE,VT_HANDLE);
TS_DECLARE_BUILTIN_METATYPE(TSTemplateId,VT_TEMPLATEID);
TS_DECLARE_BUILTIN_METATYPE(TSModelDataId,VT_MODELDATAID);
TS_DECLARE_BUILTIN_METATYPE(TSAssetTypeId,VT_ASSETTYPEID);
TS_DECLARE_BUILTIN_METATYPE(TSMissionId,VT_MISSIONID);
TS_DECLARE_BUILTIN_METATYPE(TSClassificationId,VT_CLASSIFICATIONID);
TS_DECLARE_BUILTIN_METATYPE(TSModelTmplOrClassId,VT_MODELTMPLORCLASSID);
TS_DECLARE_BUILTIN_METATYPE(UINT64,VT_UINT64);
TS_DECLARE_BUILTIN_METATYPE(INT8,VT_INT8);
TS_DECLARE_BUILTIN_METATYPE(INT16,VT_INT16);
TS_DECLARE_BUILTIN_METATYPE(INT32,VT_INT32);
TS_DECLARE_BUILTIN_METATYPE(INT64,VT_INT64);
TS_DECLARE_BUILTIN_METATYPE(FLOAT,VT_FLOAT);
TS_DECLARE_BUILTIN_METATYPE(DOUBLE,VT_DOUBLE);
TS_DECLARE_BUILTIN_METATYPE(char,VT_CHAR);
#if !defined(_MSC_VER) || _NATIVE_WCHAR_T_DEFINED
TS_DECLARE_BUILTIN_METATYPE(wchar_t,VT_WCHAR);
#endif
TS_DECLARE_BUILTIN_METATYPE(TSString,VT_STRING);
TS_DECLARE_BUILTIN_METATYPE(TSVector2f,VT_VECTOR2F);
TS_DECLARE_BUILTIN_METATYPE(TSVector3f,VT_VECTOR3F);
TS_DECLARE_BUILTIN_METATYPE(TSVector2d,VT_VECTOR2D);
TS_DECLARE_BUILTIN_METATYPE(TSVector3d,VT_VECTOR3D);
TS_DECLARE_BUILTIN_METATYPE(TSByteBuffer,VT_BINARY);
TS_DECLARE_BUILTIN_METATYPE(TSTime,VT_TIME);
TS_DECLARE_BUILTIN_METATYPE(TSBattleTime,VT_BATTLE_TIME);
TS_DECLARE_BUILTIN_METATYPE(TSTimeDuration,VT_TIMEDURATION);
TS_DECLARE_BUILTIN_METATYPE(TSVariantArray,VT_ARRAY);
TS_DECLARE_BUILTIN_METATYPE(TSVariantPair,VT_PAIR);
TS_DECLARE_BUILTIN_METATYPE(TSVariant,VT_VARIANT);

TS_DECLARE_METATYPE(TSBasicReadStream);
TS_DECLARE_METATYPE(TSBasicWriteStream);
TS_DECLARE_METATYPE(TSBasicIOStream);

#endif // __TSMETATYPE_H__

