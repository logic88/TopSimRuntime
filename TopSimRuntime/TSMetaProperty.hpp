#ifndef __TSMETAPROPERTY_H__
#define __TSMETAPROPERTY_H__
#include "TSVariant.h"

//TSPropertyOperatorHolder::GetFlag()
#define PROP_FLAG_VARIABLY											(1<<0)
#define PROP_FLAG_READONLY											(1<<1)
#define PROP_FLAG_DATA_ARCHIVE_HAS_VALNAME							(1<<3)
#define PROP_FLAG_PROPERTY_IS_FIELD									(1<<4)
#define PROP_FLAG_PROPERTY_HAS_FIELD_VALUE_AS_POINTER_CAPACITY		(1<<5)

TS_BEGIN_RT_NAMESPACE

template <typename FunctorType>
struct TSGetFuncPtrHelper
{
	union FunctorData
	{
		void *funtorPtr;
		FunctorType memfuncPtr;
	};
	static void *Get(FunctorType fn) 
	{
		FunctorData tw;
		tw.memfuncPtr = fn;
		return tw.funtorPtr;
	}
};

template <typename FunctorType>
void *TSGetFuncPtr(FunctorType fn)
{
	return TSGetFuncPtrHelper<FunctorType>::Get(fn);
}

template<typename T>
struct TSConvertToPointerHelper
{
	FORCEINLINE static T * Convert(T & val)
	{
		return boost::addressof(val);
	}
};

template<typename T>
struct TSConvertToPointerHelper< boost::shared_ptr<T> > 
{
	FORCEINLINE static boost::shared_ptr<T> & Convert(boost::shared_ptr<T> & val)
	{
		return val;
	}
};

template<typename T>
struct TSConvertToPointerHelper< T * > 
{
	FORCEINLINE static T * Convert(T * val)
	{
		return val;
	}
};


class TSPropertyOperatorHolder
{
public:
	enum FuncType 
	{
		FT_SET = 1,
		FT_GET = 2,
	};
	virtual TSVariant GetValue(const void * obj)=0;
	virtual TSVariant GetDummyVariant()=0;
	virtual void SetValue(void * obj,const TSVariant & Val) = 0;
	virtual UINT32 GetFlag() {return 0;}
	virtual TSString GetOriginalTypeName()=0;
	virtual void ResetValue(void * obj)=0;
	virtual bool TestFuncType(void *fn, FuncType ft) = 0;
	virtual TSVariant GetFieldValueAsPointer(const void * obj) { return TSVariant(); };
	virtual int GetValueTypeId()
	{
		return GetDummyVariant().GetType();
	}
	template  <typename FunctorType>
	bool TestFuncTypeT(FunctorType fn, FuncType ft)
	{
		return TestFuncType(TSGetFuncPtr(fn), ft);
	}
};
typedef boost::shared_ptr<TSPropertyOperatorHolder> TSPropertyOperatorHolderPtr;

template<class ClassType,class ValueType,class FieldType>
class TSFieldOperator : public TSPropertyOperatorHolder
{
public:
	TSFieldOperator(FieldType Field)
	{
		_Field.field = Field;
	}
	virtual TSVariant GetValue(const void * obj)
	{
		return TSVariant::FromValue(((ClassType *)obj)->*_Field.field);
	}
	virtual void SetValue(void * obj,const TSVariant & Val)
	{
		((ClassType *)obj)->*_Field.field = Val.Value<ValueType>();
	}
	virtual TSVariant GetDummyVariant()
	{
		return TSVariant::CreateDummy<ValueType>();
	}
	virtual UINT32 GetFlag() 
	{
		return PROP_FLAG_PROPERTY_IS_FIELD | PROP_FLAG_PROPERTY_HAS_FIELD_VALUE_AS_POINTER_CAPACITY;
	}
	virtual TSString GetOriginalTypeName()
	{
		return GetMetaTypeOriginalName<ValueType>();
	}
	virtual void ResetValue(void * obj)
	{
		((ClassType *)obj)->*_Field.field = ValueType();
	}
	virtual bool TestFuncType(void *fn, FuncType)
	{
		return _Field.funcPtr == fn;
	}
	virtual TSVariant GetFieldValueAsPointer(const void * obj) 
	{ 
		return TSVariant::FromValue(TSConvertToPointerHelper<ValueType>::Convert(((ClassType *)obj)->*_Field.field));
	}
	virtual int GetValueTypeId()
	{
		return GetMetaTypeId<ValueType>(reinterpret_cast<ValueType *>(0));
	}
private:
	union
	{
		void *funcPtr;
		FieldType field;
	} _Field;
};

template<class ClassType,class FieldType>
class TSFieldOperator<ClassType,TSVariant,FieldType> : public TSPropertyOperatorHolder
{
public:
	TSFieldOperator(FieldType Field)
	{
		_Field.field = Field;
	}
	virtual TSVariant GetValue(const void * obj)
	{
		return ((ClassType *)obj)->*_Field.field;
	}
	virtual void SetValue(void * obj,const TSVariant & Val)
	{
		((ClassType *)obj)->*_Field.field = Val;
	}
	virtual TSVariant GetDummyVariant()
	{
		return TSVariant();
	}
	virtual UINT32 GetFlag() 
	{
		return PROP_FLAG_VARIABLY | PROP_FLAG_PROPERTY_IS_FIELD | PROP_FLAG_PROPERTY_HAS_FIELD_VALUE_AS_POINTER_CAPACITY;
	}
	virtual TSString GetOriginalTypeName()
	{
		return TS_TEXT("TSVariant");
	}
	virtual void ResetValue(void * obj)
	{
		(((ClassType *)obj)->*_Field.field).Clear();
	}
	virtual bool TestFuncType(void *fn, FuncType)
	{
		return _Field.funcPtr == fn;
	}
	virtual TSVariant GetFieldValueAsPointer(const void * obj) 
	{ 
		return TSVariant::FromValue(TSConvertToPointerHelper<TSVariant>::Convert(((ClassType *)obj)->*_Field.field));
	}
	virtual int GetValueTypeId()
	{
		return TSVariant::VT_VARIANT;
	}
private:
	union 
	{
		void *funcPtr;
		FieldType field;
	} _Field;
};

template<class ClassType,class ValueType,class GetterType,class SetterType>
class TSPropertyOperator : public TSPropertyOperatorHolder
{
public:
	TSPropertyOperator(GetterType Getter,SetterType Setter)
	{
		_Getter.memfuncPtr = Getter;
		_Setter.memfuncPtr = Setter;
	}
	virtual TSVariant GetValue(const void * obj)
	{
		return TSVariant::FromValue((((ClassType *)obj)->*_Getter.memfuncPtr)());
	}
	virtual void SetValue(void * obj,const TSVariant & Val)
	{
		(((ClassType *)obj)->*_Setter.memfuncPtr)(Val.Value<ValueType>());
	}
	virtual TSVariant GetDummyVariant()
	{
		return TSVariant::CreateDummy<ValueType>();
	}
	virtual TSString GetOriginalTypeName()
	{
		return GetMetaTypeOriginalName<ValueType>();
	}
	virtual void ResetValue(void * obj)
	{
		(((ClassType *)obj)->*_Setter.memfuncPtr)(ValueType());
	}
	virtual bool TestFuncType(void *fn, FuncType ft)
	{
		if (FT_GET == ft)
		{
			return _Getter.funcPtr == fn;
		}
		else if (FT_SET == ft)
		{
			return _Setter.funcPtr == fn;
		}
		else 
		{
			return false;
		}
	}
	virtual int GetValueTypeId()
	{
		return GetMetaTypeId<ValueType>(reinterpret_cast<ValueType *>(0));
	}
private:
	union
	{
		void *funcPtr;
		GetterType memfuncPtr;
	} _Getter;
	
	union
	{
		void *funcPtr;
		SetterType memfuncPtr;
	} _Setter;
};

template<class ClassType,class GetterType,class SetterType>
class TSPropertyOperator<ClassType,TSVariant,GetterType,SetterType> : public TSPropertyOperatorHolder
{
public:
	TSPropertyOperator(GetterType Getter,SetterType Setter)
	{
		_Getter.memfuncPtr = Getter;
		_Setter.memfuncPtr = Setter;
	}
	virtual TSVariant GetValue(const void * obj)
	{
		return (((ClassType *)obj)->*_Getter.memfuncPtr)();
	}
	virtual void SetValue(void * obj,const TSVariant & Val)
	{
		(((ClassType *)obj)->*_Setter.memfuncPtr)(Val);
	}
	virtual TSVariant GetDummyVariant()
	{
		return TSVariant();
	}
	virtual UINT32 GetFlag() 
	{
		return PROP_FLAG_VARIABLY;
	}
	virtual TSString GetOriginalTypeName()
	{
		return TS_TEXT("TSVariant");
	}
	virtual void ResetValue(void * obj)
	{
		(((ClassType *)obj)->*_Setter.memfuncPtr)(TSVariant());
	}
	virtual bool TestFuncType(void *fn, FuncType ft)
	{
		if (FT_GET == ft)
		{
			return _Getter.funcPtr == fn;
		}
		else if (FT_SET == ft)
		{
			return _Setter.funcPtr == fn;
		}
		else 
		{
			return false;
		}
	}
	virtual int GetValueTypeId()
	{
		return TSVariant::VT_VARIANT;
	}
private:
	union
	{
		void *funcPtr;
		GetterType memfuncPtr;
	} _Getter;

	union
	{
		void *funcPtr;
		SetterType memfuncPtr;
	} _Setter;
};

//
template<class ClassType,class ValueType,class GetterType>
class TSPropertyOperatorReadOnly : public TSPropertyOperatorHolder
{
public:
	TSPropertyOperatorReadOnly(GetterType Getter)
	{
		_Getter.memfuncPtr = Getter;
	}
	virtual TSVariant GetValue(const void * obj)
	{
		return TSVariant::FromValue((((ClassType *)obj)->*_Getter.memfuncPtr)());
	}
	virtual void SetValue(void * obj,const TSVariant & Val)
	{

	}
	virtual TSVariant GetDummyVariant()
	{
		return TSVariant::CreateDummy<ValueType>();
	}
	virtual UINT32 GetFlag() 
	{
		return PROP_FLAG_READONLY;
	}
	virtual TSString GetOriginalTypeName()
	{
		return GetMetaTypeOriginalName<ValueType>();
	}
	virtual void ResetValue(void * obj)
	{

	}
	virtual bool TestFuncType(void *fn, FuncType ft)
	{
		if (FT_GET == ft)
		{
			return _Getter.funcPtr == fn;
		}
		else
		{
			return false;
		}
	}

	virtual int GetValueTypeId()
	{
		return GetMetaTypeId<ValueType>(reinterpret_cast<ValueType *>(0));
	}
private:
	union
	{
		void *funcPtr;
		GetterType memfuncPtr;
	} _Getter;
};

template<class ClassType,class GetterType>
class TSPropertyOperatorReadOnly<ClassType,TSVariant,GetterType> : public TSPropertyOperatorHolder
{
public:
	TSPropertyOperatorReadOnly(GetterType Getter)
	{
		_Getter.memfuncPtr = Getter;
	}
	virtual TSVariant GetValue(const void * obj)
	{
		return (((ClassType *)obj)->*_Getter.memfuncPtr)();
	}
	virtual void SetValue(void * obj,const TSVariant & Val)
	{

	}
	virtual TSVariant GetDummyVariant()
	{
		return TSVariant();
	}
	virtual UINT32 GetFlag() 
	{
		return PROP_FLAG_VARIABLY|PROP_FLAG_READONLY;
	}
	virtual TSString GetOriginalTypeName()
	{
		return TS_TEXT("TSVariant");
	}
	virtual void ResetValue(void * obj)
	{

	}
	virtual bool TestFuncType(void *fn, FuncType ft)
	{
		if (FT_GET == ft)
		{
			return _Getter.funcPtr == fn;
		}
		else 
		{
			return false;
		}
	}

	virtual int GetValueTypeId()
	{
		return TSVariant::VT_VARIANT;
	}
private:
	union
	{
		void *funcPtr;
		GetterType memfuncPtr;
	} _Getter;
};
//
class TSPropertyOperatorHolder;
typedef boost::shared_ptr<TSPropertyOperatorHolder> TSPropertyOperatorHolderPtr;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	����������.  </summary>
///
/// <remarks>	��������ģ�����Ե���ϸ���ݡ�
/// 			Huaru Infrastructure Team, 2014/10/24. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

class TSMetaProperty
{
public:
	TSMetaProperty(const TSChar *varName,const TSChar * name,const TSPropertyOperatorHolderPtr propertyOperator,const UINT32 owningType)
		:VarName(varName),Name(name),Flag(0),PropertyOperator(propertyOperator),OwningType(owningType)
	{

	}

	enum OpType
	{
		NoDesignWrite	= 1 <<0,							//���ɱ�UI�޸�
		NoDesignable	= 1 << 1 | NoDesignWrite,			//���ɱ�UI��ʾ���޸�
		NoSave			= 1 << 2 | NoDesignWrite,			//���ɱ����浽�κδ浵
		NoSaveAndDesign = NoSave | NoDesignable,			//���ɱ����浽�κδ浵���Ҳ��ɱ�UI��ʾ���޸�
		NoDump			= 1 << 3,							//���ɱ�����
		Mutable			= 1 << 6,							//�ױ��,�����˱�������������ʱ�仯,�˱�־ֻ�Լ����ͼ���������Ч
		MutableLeaf		= Mutable | 1 << 7,					//����Ҷ������ȫ�����ױ��(�����ӽڵ���ʽָ��Immutable��־),�˱�־ֻ�Ը������ͼ���������Ч
		Immutable		= 1 << 8,							//���ױ��,�����˱����������ڼ䲻������ı�
		DesignOnly		= NoSave | NoDump,
		RunTime			= NoSave | NoDump | NoDesignable,	//����������
		Input			= 1 << 10,							//�������Բ���
		Output			= 1 << 11,							//������Բ���
		InOut			= Input | Output,					//����/������Բ���
		RInput			= NoDesignable | Input,				//����ʱ�������Բ���
		ROutput			= NoDesignable | Output,			//����ʱ������Բ���
		RInOut			= RInput | ROutput,					//����ʱ����/������Բ���
	};	

	
	TSMetaProperty & SetFlag(UINT16 flag)
	{
		Flag = flag;

		return *this;
	}
	
	TSString VarName;
	TSString Name;
	UINT16 Flag;	
	TSPropertyOperatorHolderPtr PropertyOperator;
	UINT32 OwningType;
};

#ifndef SWIG

template<class ClassType,class ValueType,class GetterType,class SetterType>
TSPropertyOperatorHolderPtr TSCreatePropertyOperator2(GetterType Getter,SetterType Setter)
{
	typedef TSPropertyOperator<ClassType,ValueType,GetterType,SetterType> PropertyOperatorType;

	return boost::make_shared<PropertyOperatorType>(Getter,Setter);
}

template<typename ClassType,typename R,typename Arg>
TSPropertyOperatorHolderPtr TSCreatePropertyOperator(R (ClassType::*Getter)() const,void (ClassType::*Setter)(Arg))
{
    typedef typename boost::remove_const<typename boost::remove_reference<R>::type>::type ValueType1;
    typedef typename boost::remove_const<typename boost::remove_reference<Arg>::type>::type ValueType2;

	BOOST_STATIC_ASSERT((boost::is_same<ValueType1,ValueType2>::value));

	return TSCreatePropertyOperator2<ClassType,ValueType1>(Getter,Setter);
}

template<typename ClassType,typename R,typename Arg>
TSPropertyOperatorHolderPtr TSCreatePropertyOperator(R (ClassType::*Getter)(),void (ClassType::*Setter)(Arg))
{
    typedef typename boost::remove_const<typename boost::remove_reference<R>::type>::type ValueType1;
    typedef typename boost::remove_const<typename boost::remove_reference<Arg>::type>::type ValueType2;

	BOOST_STATIC_ASSERT((boost::is_same<ValueType1,ValueType2>::value));

	return TSCreatePropertyOperator2<ClassType,ValueType1>(Getter,Setter);
}

//
template<class ClassType,class ValueType,class GetterType>
TSPropertyOperatorHolderPtr TSCreatePropertyOperator2(GetterType Getter)
{
	typedef TSPropertyOperatorReadOnly<ClassType,ValueType,GetterType> PropertyOperatorType;

	return boost::make_shared<PropertyOperatorType>(Getter);
}

template<typename ClassType,typename R>
TSPropertyOperatorHolderPtr TSCreatePropertyOperator(R (ClassType::*Getter)() const)
{
	typedef typename boost::remove_const<typename boost::remove_reference<R>::type>::type ValueType1;

	return TSCreatePropertyOperator2<ClassType,ValueType1>(Getter);
}

template<typename ClassType,typename R>
TSPropertyOperatorHolderPtr TSCreatePropertyOperator(R (ClassType::*Getter)())
{
	typedef typename boost::remove_const<typename boost::remove_reference<R>::type>::type ValueType1;

	return TSCreatePropertyOperator2<ClassType,ValueType1>(Getter);
}
//
template<typename ClassType,class ValueType,class FieldType>
TSPropertyOperatorHolderPtr TSCreateFieldOperator2(FieldType Field)
{
    return boost::make_shared< TSFieldOperator<ClassType,ValueType,FieldType> >(Field);
}

template<typename ClassType,class ValueType>
TSPropertyOperatorHolderPtr TSCreateFieldOperator(ValueType ClassType::*Field)
{
	return TSCreateFieldOperator2<ClassType,ValueType>(Field);
}
#endif // SWIG

typedef UINT32 TSATTRHANDLE;


TS_END_RT_NAMESPACE

#endif // __TSMETAPROPERTY_H__

