#ifndef __TSFUNCTIONMETADATA__H__
#define __TSFUNCTIONMETADATA__H__
#include <stack>
#ifndef Q_MOC_RUN
#include <boost/algorithm/string.hpp>
#endif//Q_MOC_RUN
#include "TSVariant.h"
#include "Function_Traits.h"

TS_BEGIN_RT_NAMESPACE

struct TSInvokeStackInterface 
{
	virtual TSVariant & TopStack() = 0;
	virtual TSVariant PopStack() = 0;
	virtual TSVariant & PushStack() = 0;
};
typedef TSInvokeStackInterface * TSInvokeStackInterfacePtr;

struct  TSFunctionArgTypeDesc
{
	TSFunctionArgTypeDesc()
	{
	
	}

	TSFunctionArgTypeDesc(TSVariant typeDesc)
		:TypeDesc(typeDesc)
	{
	
	}
	TSVariant TypeDesc;
};

struct TSBaseFunctionHolder 
{
	virtual ~TSBaseFunctionHolder(){};
	virtual int GetNumArgs() const = 0;
	virtual const TSString & GetArgs() const = 0;
	virtual TSString GetTypeString() const = 0;
	virtual bool IsMemberFunction() const = 0;
	virtual const TSString & GetName() const = 0;
	virtual std::vector<TSFunctionArgTypeDesc> GetArgsDesc() const = 0;
	virtual void Invoke(TSVariant &self,TSInvokeStackInterfacePtr Stack) const = 0;
};

typedef UINT32 TSFUNCHANDLE;

typedef boost::shared_ptr<TSBaseFunctionHolder> TSBaseFunctionHolderPtr;

template <class TList,class Func>
struct TSBaseFunctionHolderTempl : public TSBaseFunctionHolder
{
public:
	TSBaseFunctionHolderTempl(const TSChar * name,Func func,const TSChar *args) : name(name), func(func), args(args) { }

	enum { Length = TSTypeListLength<TList>::Value - 1 };

	typedef BOOST_DEDUCED_TYPENAME TSTypeListTypeAt<TList,0>::Type Ret;
	typedef BOOST_DEDUCED_TYPENAME TSTypeListTypeAt<TList,1>::Type OriA0;
	typedef BOOST_DEDUCED_TYPENAME TSTypeListTypeAt<TList,2>::Type OriA1;
	typedef BOOST_DEDUCED_TYPENAME TSTypeListTypeAt<TList,3>::Type OriA2;
	typedef BOOST_DEDUCED_TYPENAME TSTypeListTypeAt<TList,4>::Type OriA3;
	typedef BOOST_DEDUCED_TYPENAME TSTypeListTypeAt<TList,5>::Type OriA4;
	typedef BOOST_DEDUCED_TYPENAME TSTypeListTypeAt<TList,6>::Type OriA5;
	typedef BOOST_DEDUCED_TYPENAME TSTypeListTypeAt<TList,7>::Type OriA6;
	typedef BOOST_DEDUCED_TYPENAME TSTypeListTypeAt<TList,8>::Type OriA7;

	typedef BOOST_DEDUCED_TYPENAME TSNoConstRef<OriA0>::Type NoRefA0;
	typedef BOOST_DEDUCED_TYPENAME TSNoConstRef<OriA1>::Type NoRefA1;
	typedef BOOST_DEDUCED_TYPENAME TSNoConstRef<OriA2>::Type NoRefA2;
	typedef BOOST_DEDUCED_TYPENAME TSNoConstRef<OriA3>::Type NoRefA3;
	typedef BOOST_DEDUCED_TYPENAME TSNoConstRef<OriA4>::Type NoRefA4;
	typedef BOOST_DEDUCED_TYPENAME TSNoConstRef<OriA5>::Type NoRefA5;
	typedef BOOST_DEDUCED_TYPENAME TSNoConstRef<OriA6>::Type NoRefA6;
	typedef BOOST_DEDUCED_TYPENAME TSNoConstRef<OriA7>::Type NoRefA7;

	TSString DoGetTypeString(TSIntToType<0>) const
	{
		return TSStringUtil::Format(TS_TEXT("%s %s();"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name));
	}

	TSString DoGetTypeString(TSIntToType<1>) const
	{
		std::vector< TSString > SplitVec;
		boost::split( SplitVec, args, boost::is_any_of(",") );

		if (SplitVec.size()==1)
		{
			return TSStringUtil::Format(TS_TEXT("%s %s(%s %s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
				,constchar_cast(GetMetaTypeOriginalName<OriA0>())
				,constchar_cast(SplitVec[0])
				);
		}
		return TSStringUtil::Format(TS_TEXT("%s %s(%s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
			,constchar_cast(GetMetaTypeOriginalName<OriA0>())
			);
	}

	TSString DoGetTypeString(TSIntToType<2>) const
	{
		std::vector< TSString > SplitVec;
		boost::split( SplitVec, args, boost::is_any_of(",") );

		if (SplitVec.size()==2)
		{
			return TSStringUtil::Format(TS_TEXT("%s %s(%s %s,%s %s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
				,constchar_cast(GetMetaTypeOriginalName<OriA0>())
				,constchar_cast(SplitVec[0])
				,constchar_cast(GetMetaTypeOriginalName<OriA1>())
				,constchar_cast(SplitVec[1])
				);
		}
		return TSStringUtil::Format(TS_TEXT("%s %s(%s,%s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
			,constchar_cast(GetMetaTypeOriginalName<OriA0>())
			,constchar_cast(GetMetaTypeOriginalName<OriA1>())
			);
	}

	TSString DoGetTypeString(TSIntToType<3>) const
	{
		std::vector< TSString > SplitVec;
		boost::split( SplitVec, args, boost::is_any_of(",") );

		if (SplitVec.size()==3)
		{
			return TSStringUtil::Format(TS_TEXT("%s %s(%s %s,%s %s,%s %s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
				,constchar_cast(GetMetaTypeOriginalName<OriA0>())
				,constchar_cast(SplitVec[0])
				,constchar_cast(GetMetaTypeOriginalName<OriA1>())
				,constchar_cast(SplitVec[1])
				,constchar_cast(GetMetaTypeOriginalName<OriA2>())
				,constchar_cast(SplitVec[2])
				);
		}
		return TSStringUtil::Format(TS_TEXT("%s %s(%s,%s,%s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
			,constchar_cast(GetMetaTypeOriginalName<OriA0>())
			,constchar_cast(GetMetaTypeOriginalName<OriA1>())
			,constchar_cast(GetMetaTypeOriginalName<OriA2>())
			);
	}

	TSString DoGetTypeString(TSIntToType<4>) const
	{
		std::vector< TSString > SplitVec;
		boost::split( SplitVec, args, boost::is_any_of(",") );

		if (SplitVec.size()==4)
		{
			return TSStringUtil::Format(TS_TEXT("%s %s(%s %s,%s %s,%s %s,%s %s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
				,constchar_cast(GetMetaTypeOriginalName<OriA0>())
				,constchar_cast(SplitVec[0])
				,constchar_cast(GetMetaTypeOriginalName<OriA1>())
				,constchar_cast(SplitVec[1])
				,constchar_cast(GetMetaTypeOriginalName<OriA2>())
				,constchar_cast(SplitVec[2])
				,constchar_cast(GetMetaTypeOriginalName<OriA3>())
				,constchar_cast(SplitVec[3])
				);
		}
		return TSStringUtil::Format(TS_TEXT("%s %s(%s,%s,%s,%s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
			,constchar_cast(GetMetaTypeOriginalName<OriA0>())
			,constchar_cast(GetMetaTypeOriginalName<OriA1>())
			,constchar_cast(GetMetaTypeOriginalName<OriA2>())
			,constchar_cast(GetMetaTypeOriginalName<OriA3>())
			);
	}

	TSString DoGetTypeString(TSIntToType<5>) const
	{
		std::vector< TSString > SplitVec;
		boost::split( SplitVec, args, boost::is_any_of(",") );

		if (SplitVec.size()==5)
		{
			return TSStringUtil::Format(TS_TEXT("%s %s(%s %s,%s %s,%s %s,%s %s,%s %s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
				,constchar_cast(GetMetaTypeOriginalName<OriA0>())
				,constchar_cast(SplitVec[0])
				,constchar_cast(GetMetaTypeOriginalName<OriA1>())
				,constchar_cast(SplitVec[1])
				,constchar_cast(GetMetaTypeOriginalName<OriA2>())
				,constchar_cast(SplitVec[2])
				,constchar_cast(GetMetaTypeOriginalName<OriA3>())
				,constchar_cast(SplitVec[3])
				,constchar_cast(GetMetaTypeOriginalName<OriA4>())
				,constchar_cast(SplitVec[4])
				);
		}
		return TSStringUtil::Format(TS_TEXT("%s %s(%s,%s,%s,%s,%s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
			,constchar_cast(GetMetaTypeOriginalName<OriA0>())
			,constchar_cast(GetMetaTypeOriginalName<OriA1>())
			,constchar_cast(GetMetaTypeOriginalName<OriA2>())
			,constchar_cast(GetMetaTypeOriginalName<OriA3>())
			,constchar_cast(GetMetaTypeOriginalName<OriA4>())
			);
	}

	TSString DoGetTypeString(TSIntToType<6>) const
	{
		std::vector< TSString > SplitVec;
		boost::split( SplitVec, args, boost::is_any_of(",") );

		if (SplitVec.size()==6)
		{
			return TSStringUtil::Format(TS_TEXT("%s %s(%s %s,%s %s,%s %s,%s %s,%s %s,%s %s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
				,constchar_cast(GetMetaTypeOriginalName<OriA0>())
				,constchar_cast(SplitVec[0])
				,constchar_cast(GetMetaTypeOriginalName<OriA1>())
				,constchar_cast(SplitVec[1])
				,constchar_cast(GetMetaTypeOriginalName<OriA2>())
				,constchar_cast(SplitVec[2])
				,constchar_cast(GetMetaTypeOriginalName<OriA3>())
				,constchar_cast(SplitVec[3])
				,constchar_cast(GetMetaTypeOriginalName<OriA4>())
				,constchar_cast(SplitVec[4])
				,constchar_cast(GetMetaTypeOriginalName<OriA5>())
				,constchar_cast(SplitVec[5])
				);
		}
		return TSStringUtil::Format(TS_TEXT("%s %s(%s,%s,%s,%s,%s,%s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
			,constchar_cast(GetMetaTypeOriginalName<OriA0>())
			,constchar_cast(GetMetaTypeOriginalName<OriA1>())
			,constchar_cast(GetMetaTypeOriginalName<OriA2>())
			,constchar_cast(GetMetaTypeOriginalName<OriA3>())
			,constchar_cast(GetMetaTypeOriginalName<OriA4>())
			,constchar_cast(GetMetaTypeOriginalName<OriA5>())
			);
	}

	TSString DoGetTypeString(TSIntToType<7>) const
	{
		std::vector< TSString > SplitVec;
		boost::split( SplitVec, args, boost::is_any_of(",") );

		if (SplitVec.size()==7)
		{
			return TSStringUtil::Format(TS_TEXT("%s %s(%s %s,%s %s,%s %s,%s %s,%s %s,%s %s,%s %s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
				,constchar_cast(GetMetaTypeOriginalName<OriA0>())
				,constchar_cast(SplitVec[0])
				,constchar_cast(GetMetaTypeOriginalName<OriA1>())
				,constchar_cast(SplitVec[1])
				,constchar_cast(GetMetaTypeOriginalName<OriA2>())
				,constchar_cast(SplitVec[2])
				,constchar_cast(GetMetaTypeOriginalName<OriA3>())
				,constchar_cast(SplitVec[3])
				,constchar_cast(GetMetaTypeOriginalName<OriA4>())
				,constchar_cast(SplitVec[4])
				,constchar_cast(GetMetaTypeOriginalName<OriA5>())
				,constchar_cast(SplitVec[5])
				,constchar_cast(GetMetaTypeOriginalName<OriA6>())
				,constchar_cast(SplitVec[6])
				);
		}
		return TSStringUtil::Format(TS_TEXT("%s %s(%s,%s,%s,%s,%s,%s,%s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
			,constchar_cast(GetMetaTypeOriginalName<OriA0>())
			,constchar_cast(GetMetaTypeOriginalName<OriA1>())
			,constchar_cast(GetMetaTypeOriginalName<OriA2>())
			,constchar_cast(GetMetaTypeOriginalName<OriA3>())
			,constchar_cast(GetMetaTypeOriginalName<OriA4>())
			,constchar_cast(GetMetaTypeOriginalName<OriA5>())
			,constchar_cast(GetMetaTypeOriginalName<OriA6>())
			);
	}

	TSString DoGetTypeString(TSIntToType<8>) const
	{
		std::vector< TSString > SplitVec;
		boost::split( SplitVec, args, boost::is_any_of(",") );

		if (SplitVec.size()==8)
		{
			return TSStringUtil::Format(TS_TEXT("%s %s(%s %s,%s %s,%s %s,%s %s,%s %s,%s %s,%s %s,%s %s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
				,constchar_cast(GetMetaTypeOriginalName<OriA0>())
				,constchar_cast(SplitVec[0])
				,constchar_cast(GetMetaTypeOriginalName<OriA1>())
				,constchar_cast(SplitVec[1])
				,constchar_cast(GetMetaTypeOriginalName<OriA2>())
				,constchar_cast(SplitVec[2])
				,constchar_cast(GetMetaTypeOriginalName<OriA3>())
				,constchar_cast(SplitVec[3])
				,constchar_cast(GetMetaTypeOriginalName<OriA4>())
				,constchar_cast(SplitVec[4])
				,constchar_cast(GetMetaTypeOriginalName<OriA5>())
				,constchar_cast(SplitVec[5])
				,constchar_cast(GetMetaTypeOriginalName<OriA6>())
				,constchar_cast(SplitVec[6])
				,constchar_cast(GetMetaTypeOriginalName<OriA7>())
				,constchar_cast(SplitVec[7])
				);
		}
		return TSStringUtil::Format(TS_TEXT("%s %s(%s,%s,%s,%s,%s,%s,%s,%s);"),constchar_cast(GetMetaTypeOriginalName<Ret>()),constchar_cast(name)
			,constchar_cast(GetMetaTypeOriginalName<OriA0>())
			,constchar_cast(GetMetaTypeOriginalName<OriA1>())
			,constchar_cast(GetMetaTypeOriginalName<OriA2>())
			,constchar_cast(GetMetaTypeOriginalName<OriA3>())
			,constchar_cast(GetMetaTypeOriginalName<OriA4>())
			,constchar_cast(GetMetaTypeOriginalName<OriA5>())
			,constchar_cast(GetMetaTypeOriginalName<OriA6>())
			,constchar_cast(GetMetaTypeOriginalName<OriA7>())
			);
	}

	std::vector<TSFunctionArgTypeDesc> DoGetArgsDesc(TSIntToType<0>) const
	{
		return std::vector<TSFunctionArgTypeDesc>();
	}

	std::vector<TSFunctionArgTypeDesc> DoGetArgsDesc(TSIntToType<1>) const
	{
		std::vector<TSFunctionArgTypeDesc> DescArray;

		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA0>()));

		return DescArray;
	}

	std::vector<TSFunctionArgTypeDesc> DoGetArgsDesc(TSIntToType<2>) const
	{
		std::vector<TSFunctionArgTypeDesc> DescArray;

		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA0>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA1>()));

		return DescArray;
	}

	std::vector<TSFunctionArgTypeDesc> DoGetArgsDesc(TSIntToType<3>) const
	{
		std::vector<TSFunctionArgTypeDesc> DescArray;

		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA0>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA1>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA2>()));

		return DescArray;
	}

	std::vector<TSFunctionArgTypeDesc> DoGetArgsDesc(TSIntToType<4>) const
	{
		std::vector<TSFunctionArgTypeDesc> DescArray;

		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA0>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA1>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA2>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA3>()));

		return DescArray;
	}

	std::vector<TSFunctionArgTypeDesc> DoGetArgsDesc(TSIntToType<5>) const
	{
		std::vector<TSFunctionArgTypeDesc> DescArray;

		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA0>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA1>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA2>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA3>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA4>()));

		return DescArray;
	}

	std::vector<TSFunctionArgTypeDesc> DoGetArgsDesc(TSIntToType<6>) const
	{
		std::vector<TSFunctionArgTypeDesc> DescArray;

		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA0>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA1>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA2>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA3>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA4>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA5>()));

		return DescArray;
	}

	std::vector<TSFunctionArgTypeDesc> DoGetArgsDesc(TSIntToType<7>) const
	{
		std::vector<TSFunctionArgTypeDesc> DescArray;

		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA0>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA1>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA2>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA3>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA4>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA5>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA6>()));

		return DescArray;
	}

	std::vector<TSFunctionArgTypeDesc> DoGetArgsDesc(TSIntToType<8>) const
	{
		std::vector<TSFunctionArgTypeDesc> DescArray;

		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA0>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA1>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA2>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA3>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA4>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA5>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA6>()));
		DescArray.push_back(TSFunctionArgTypeDesc(TSVariant::CreateDummy<NoRefA7>()));

		return DescArray;
	}
protected:
	TSString name;
	Func func;
	TSString args;
};

#define COV_ARG(i) TSToCallArg<BOOST_DEDUCED_TYPENAME SuperType::OriA##i>(v_a##i)

template <class TList,class Func,bool IsMember = false>
class TSFunction : public TSBaseFunctionHolderTempl<TList,Func>
{
	typedef TSBaseFunctionHolderTempl<TList,Func> SuperType;
public:
	TSFunction(const TSChar * name,Func func,const TSChar *args) 
		: SuperType(name, func, args) 
	{ }

	virtual int GetNumArgs() const 
	{
        return SuperType:: Length;
	}
	virtual const TSString & GetArgs() const 
	{
        return SuperType::args;
	}
	virtual bool IsMemberFunction() const
	{
		return IsMember;
	}
	virtual const TSString & GetName() const 
	{
        return  SuperType::name;
	}
	virtual TSString GetTypeString() const
	{
        return SuperType::DoGetTypeString(TSIntToType<SuperType::Length>());
	}
	virtual void Invoke(TSVariant &self,TSInvokeStackInterfacePtr Stack) const
	{
        DoInvoke(self, Stack, TSTypeToType< BOOST_DEDUCED_TYPENAME SuperType::Ret>(),TSIntToType< SuperType::Length>());
	}
	virtual std::vector<TSFunctionArgTypeDesc> GetArgsDesc() const
	{
        return SuperType::DoGetArgsDesc(TSIntToType<SuperType::Length>());
	}
protected:
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//0个参数带返回值
	template <class Type> void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<0>) const
	{
        Stack->PushStack() = TSVariant::FromValue(SuperType::func(self));
	}
	//0个参数不带返回值
	void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<0>) const
	{
        SuperType::func(self);
		Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//1个参数带返回值
	template <class Type> void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<1>) const
	{
		TSVariant v_a0 = Stack->PopStack();

        Stack->PushStack() = TSVariant::FromValue(SuperType::func(self,COV_ARG(0)) );
	}
	//1个参数不带返回值
	void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<1>) const
	{
		TSVariant v_a0 = Stack->PopStack();

		SuperType::func(self,COV_ARG(0));

		Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<2>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();

		Stack->PushStack() = TSVariant::FromValue( SuperType::func(self,COV_ARG(0),COV_ARG(1)) );
	}

	void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<2>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();

		SuperType::func(self,COV_ARG(0),COV_ARG(1));

        Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<3>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();

		Stack->PushStack() = TSVariant::FromValue( SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2)) );
	}

	void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<3>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();


		SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2));

        Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<4>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();


		Stack->PushStack() = TSVariant::FromValue( SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3)) );
	}

	void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<4>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();


		SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3));

        Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<5>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();

		Stack->PushStack() = TSVariant::FromValue( SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4)) );
	}

	void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<5>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();

		SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4));

        Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<6>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();


		Stack->PushStack() = TSVariant::FromValue( SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5)) );
	}

	void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<6>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();


		SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5));

        Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<7>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();
		TSVariant v_a6 = Stack->PopStack();


		Stack->PushStack() = TSVariant::FromValue( SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5),COV_ARG(6)) );
	}

	void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<7>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();
		TSVariant v_a6 = Stack->PopStack();


		SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5),COV_ARG(6));

        Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<8>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();
		TSVariant v_a6 = Stack->PopStack();
		TSVariant v_a7 = Stack->PopStack();

		Stack->PushStack() = TSVariant::FromValue( SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5),COV_ARG(6),COV_ARG(7)) );
	}

	void DoInvoke(TSVariant &self,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<8>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();
		TSVariant v_a6 = Stack->PopStack();
		TSVariant v_a7 = Stack->PopStack();

		SuperType::func(self,COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5),COV_ARG(6),COV_ARG(7));

        Stack->PushStack();//压栈空值作为返回值
	}
};

template <class Ret>
TSBaseFunctionHolderPtr TSMakeFunctionData(const TSChar * name,Ret (*func)(TSVariant &),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret>::Type,Ret (*)(TSVariant &)>(name,func,args));
}

template <class Ret,class OriA0>
TSBaseFunctionHolderPtr TSMakeFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0>::Type,Ret (*)(TSVariant &,OriA0)>(name,func,args));
}

template <class Ret,class OriA0,class OriA1>
TSBaseFunctionHolderPtr TSMakeFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1>::Type,Ret (*)(TSVariant &,OriA0,OriA1)>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2>
TSBaseFunctionHolderPtr TSMakeFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2)>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2,class OriA3>
TSBaseFunctionHolderPtr TSMakeFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2,OriA3),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2,OriA3)>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4>
TSBaseFunctionHolderPtr TSMakeFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4)>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5>
TSBaseFunctionHolderPtr TSMakeFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5)>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5,class OriA6>
TSBaseFunctionHolderPtr TSMakeFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6)>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5,class OriA6,class OriA7>
TSBaseFunctionHolderPtr TSMakeFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7)>(name,func,args));
}

//member wrapper functions
template <class Ret>
TSBaseFunctionHolderPtr TSMakeMemWrapperFunctionData(const TSChar * name,Ret (*func)(TSVariant &),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret>::Type,Ret (*)(TSVariant &),true>(name,func,args));
}

template <class Ret,class OriA0>
TSBaseFunctionHolderPtr TSMakeMemWrapperFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0>::Type,Ret (*)(TSVariant &,OriA0),true>(name,func,args));
}

template <class Ret,class OriA0,class OriA1>
TSBaseFunctionHolderPtr TSMakeMemWrapperFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1>::Type,Ret (*)(TSVariant &,OriA0,OriA1),true>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2>
TSBaseFunctionHolderPtr TSMakeMemWrapperFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2),true>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2,class OriA3>
TSBaseFunctionHolderPtr TSMakeMemWrapperFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2,OriA3),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2,OriA3),true>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4>
TSBaseFunctionHolderPtr TSMakeMemWrapperFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4),true>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5>
TSBaseFunctionHolderPtr TSMakeMemWrapperFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5),true>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5,class OriA6>
TSBaseFunctionHolderPtr TSMakeMemWrapperFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6),true>(name,func,args));
}

template <class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5,class OriA6,class OriA7>
TSBaseFunctionHolderPtr TSMakeMemWrapperFunctionData(const TSChar * name,Ret (*func)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7),const TSChar *args = TS_TEXT("")) {
	return TSBaseFunctionHolderPtr(new TSFunction<typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7>::Type,Ret (*)(TSVariant &,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7),true>(name,func,args));
}


template <class Class,class TList,class Func>
class TSClassMemberFunction : public TSBaseFunctionHolderTempl<TList,Func>
{
public:
	typedef TSBaseFunctionHolderTempl<TList,Func> SuperType;

	TSClassMemberFunction(const TSChar * name,Func func,const TSChar *args)
		: SuperType(name, func, args) 
	{

	}

	virtual int GetNumArgs() const 
	{
        return  SuperType::Length;
	}

	virtual bool IsMemberFunction() const
	{
		return true;
	}

	virtual const TSString & GetName() const 
	{
        return  SuperType::name;
	}

	virtual const TSString & GetArgs() const 
	{
        return  SuperType::args;
	}

	virtual TSString GetTypeString() const
	{
        return SuperType::DoGetTypeString(TSIntToType< SuperType::Length>());
	}

	virtual std::vector<TSFunctionArgTypeDesc> GetArgsDesc() const
	{
        return SuperType::DoGetArgsDesc(TSIntToType< SuperType::Length>());
	}

	virtual void Invoke(TSVariant &self,TSInvokeStackInterfacePtr Stack) const
	{
        DoInvoke(self.Value<Class *>(),Stack,TSTypeToType<BOOST_DEDUCED_TYPENAME SuperType::Ret>(),TSIntToType< SuperType::Length>());
	}
protected:
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//0个参数带返回值
	template <class Type> void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<0>) const
	{
        Stack->PushStack() = TSVariant::FromValue( (Obj->*SuperType::func)() );
	}

	//0个参数不带返回值
	void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<0>) const
	{
        (Obj->*SuperType::func)();

		Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<1>) const
	{
		TSVariant v_a0 = Stack->PopStack();

        Stack->PushStack() = TSVariant::FromValue( (Obj->*SuperType::func)(COV_ARG(0)) );
	}

	void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<1>) const
	{
		TSVariant v_a0 = Stack->PopStack();

        (Obj->*SuperType::func)(COV_ARG(0));

		Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<2>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();


        Stack->PushStack() = TSVariant::FromValue( (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1)) );
	}

	void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<2>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();


        (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1));

		Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<3>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();


        Stack->PushStack() = TSVariant::FromValue( (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2)) );
	}

	void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<3>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();


        (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2));

        Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<4>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();

        Stack->PushStack() = TSVariant::FromValue( (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3)) );
	}

	void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<4>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();


        (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3));

		Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<5>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();


        Stack->PushStack() = TSVariant::FromValue( (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4)) );
	}

	void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<5>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();


        (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4));

        Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<6>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();


        Stack->PushStack() = TSVariant::FromValue( (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5)) );
	}

	void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<6>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();


        (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5));

        Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<7>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();
		TSVariant v_a6 = Stack->PopStack();


        Stack->PushStack() = TSVariant::FromValue( (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5),COV_ARG(6)) );
	}

	void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<7>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();
		TSVariant v_a6 = Stack->PopStack();


        (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5),COV_ARG(6));

        Stack->PushStack();//压栈空值作为返回值
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class Type> void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,Type,TSIntToType<8>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();
		TSVariant v_a6 = Stack->PopStack();
		TSVariant v_a7 = Stack->PopStack();


        Stack->PushStack() = TSVariant::FromValue( (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5),COV_ARG(6),COV_ARG(7)) );
	}

	void DoInvoke(Class * Obj,TSInvokeStackInterfacePtr Stack,TSTypeToType<void>,TSIntToType<8>) const
	{
		TSVariant v_a0 = Stack->PopStack();
		TSVariant v_a1 = Stack->PopStack();
		TSVariant v_a2 = Stack->PopStack();
		TSVariant v_a3 = Stack->PopStack();
		TSVariant v_a4 = Stack->PopStack();
		TSVariant v_a5 = Stack->PopStack();
		TSVariant v_a6 = Stack->PopStack();
		TSVariant v_a7 = Stack->PopStack();


        (Obj->*SuperType::func)(COV_ARG(0),COV_ARG(1),COV_ARG(2),COV_ARG(3),COV_ARG(4),COV_ARG(5),COV_ARG(6),COV_ARG(7));

        Stack->PushStack();//压栈空值作为返回值
	}
};

#undef COV_ARG

////////////////////////////////////////////////////////////////////////////////////////////////////
template <class InvokerType,class Class,class Ret>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(),const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret>::Type,Ret (Class::*)()>(name,func,args));
}

template <class InvokerType, class Class, class Ret>
TSBaseFunctionHolderPtr TSMakePropGetFunctionData(const TSChar *name, Ret(Class::*func)(), const TSChar *args = TS_TEXT(""))
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType, typename TSMakeTypeList<Ret>::Type, Ret(Class::*)()>(name, func, args));
}

template <class InvokerType,class Class,class Ret>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)() const,const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret>::Type,Ret (Class::*)() const>(name,func,args));
}

template <class InvokerType, class Class, class Ret>
TSBaseFunctionHolderPtr TSMakePropGetFunctionData(const TSChar *name, Ret(Class::*func)() const, const TSChar *args = TS_TEXT(""))
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType, typename TSMakeTypeList<Ret>::Type, Ret(Class::*)() const>(name, func, args));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <class InvokerType,class Class,class Ret,class OriA0>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0),const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0>::Type,Ret (Class::*)(OriA0)>(name,func,args));
}

template <class InvokerType, class Class, class Ret, class OriA0>
TSBaseFunctionHolderPtr TSMakePropSetFunctionData(const TSChar *name, Ret(Class::*func)(OriA0), const TSChar *args = TS_TEXT(""))
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType, typename TSMakeTypeList<Ret, OriA0>::Type, Ret(Class::*)(OriA0)>(name, func, args));
}

template <class InvokerType,class Class,class Ret,class OriA0>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0) const,const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0>::Type,Ret (Class::*)(OriA0) const>(name,func,args));
}

template <class InvokerType, class Class, class Ret, class OriA0>
TSBaseFunctionHolderPtr TSMakePropSetFunctionData(const TSChar *name, Ret(Class::*func)(OriA0) const, const TSChar *args = TS_TEXT(""))
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType, typename TSMakeTypeList<Ret, OriA0>::Type, Ret(Class::*)(OriA0) const>(name, func, args));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <class InvokerType,class Class,class Ret,class OriA0,class OriA1>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1),const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1>::Type,Ret (Class::*)(OriA0,OriA1)>(name,func,args));
}

template <class InvokerType,class Class,class Ret,class OriA0,class OriA1>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1) const,const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1>::Type,Ret (Class::*)(OriA0,OriA1) const>(name,func,args));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2),const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2>::Type,Ret (Class::*)(OriA0,OriA1,OriA2)>(name,func,args));
}

template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2) const,const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2>::Type,Ret (Class::*)(OriA0,OriA1,OriA2) const>(name,func,args));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2,class OriA3>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2,OriA3),const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3>::Type,Ret (Class::*)(OriA0,OriA1,OriA2,OriA3)>(name,func,args));
}

template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2,class OriA3>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2,OriA3) const,const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3>::Type,Ret (Class::*)(OriA0,OriA1,OriA2,OriA3) const>(name,func,args));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2,OriA3,OriA4),const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4>::Type,Ret (Class::*)(OriA0,OriA1,OriA2,OriA3,OriA4)>(name,func,args));
}

template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2,OriA3,OriA4) const,const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4>::Type,Ret (Class::*)(OriA0,OriA1,OriA2,OriA3,OriA4) const>(name,func,args));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5),const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5>::Type,Ret (Class::*)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5)>(name,func,args));
}

template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5) const,const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5>::Type,Ret (Class::*)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5) const>(name,func,args));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5,class OriA6>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6),const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6>::Type,Ret (Class::*)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6)>(name,func,args));
}

template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5,class OriA6>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6) const,const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6>::Type,Ret (Class::*)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6) const>(name,func,args));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5,class OriA6,class OriA7>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7),const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7>::Type,Ret (Class::*)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7)>(name,func,args));
}

template <class InvokerType,class Class,class Ret,class OriA0,class OriA1,class OriA2,class OriA3,class OriA4,class OriA5,class OriA6,class OriA7>
TSBaseFunctionHolderPtr TSMakeMemFunctionData(const TSChar *name,Ret (Class::*func)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7) const,const TSChar *args = TS_TEXT("")) 
{
	return TSBaseFunctionHolderPtr(new TSClassMemberFunction<InvokerType,typename TSMakeTypeList<Ret,OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7>::Type,Ret (Class::*)(OriA0,OriA1,OriA2,OriA3,OriA4,OriA5,OriA6,OriA7) const>(name,func,args));
}

TS_END_RT_NAMESPACE

#endif // __TSFUNCTIONMETADATA__H__


