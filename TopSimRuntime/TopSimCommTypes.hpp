#ifndef __TOPSIMCOMMTYPES__H__
#define __TOPSIMCOMMTYPES__H__
#ifdef USESTLPORT
#	ifdef _DEBUG
#		ifndef _STLP_DEBUG
#			define _STLP_DEBUG 1
#		endif // _STLP_DEBUG
#	endif // _DEBUG
#	include <cstddef>
#	ifndef __SGI_STL_PORT
#		error "Unknown STLPort library - please configure"
#	endif
#endif // USESTLPORT
#include <map>
#undef __DEPRECATED
#include <set>
#include <typeinfo>
#include "TSBasicTypes.h"
#include "TSConstVal.h"

#ifndef Q_MOC_RUN
#	include <boost/functional/hash.hpp>
#	include <boost/scoped_ptr.hpp>
#	include <boost/shared_ptr.hpp>
#	include <boost/weak_ptr.hpp>
#	include <boost/bind.hpp>
#	include <boost/function.hpp>
#	include <boost/static_assert.hpp>
#	include <boost/date_time.hpp>
#	include <boost/make_shared.hpp>
#	include <boost/unordered_map.hpp>
#	include <boost/unordered_set.hpp>
#	if BOOST_VERSION != 105600
//#		error "Don't support boost version!"
#	endif
#endif//Q_MOC_RUN



#include <tbb/concurrent_unordered_map.h>

namespace tbb
{
	template<typename T>
	class tbb_hash< boost::shared_ptr<T> >
	{
	public:
		std::size_t operator()(const boost::shared_ptr<T> & key) const
		{
			return (std::size_t)key.get();
		}
	};
}


TS_BEGIN_RT_NAMESPACE

struct null_shared_ptr_deleter 
{
	void operator() (void const *) const 
	{
	}
};

#ifndef TSCAST_DEDUCED_TYPENAME
#define TSCAST_DEDUCED_TYPENAME	BOOST_DEDUCED_TYPENAME
#endif // TSCAST_DEDUCED_TYPENAME

#ifndef TSREMOVECV_DEDUCED_TYPENAME
#define TSREMOVECV_DEDUCED_TYPENAME	BOOST_DEDUCED_TYPENAME
#endif // TSREMOVECV_DEDUCED_TYPENAME

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
///
/// <param name="from">	. </param>
/// <param name="to">  	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ToType,class FromType>
FORCEINLINE ToType TSStaticCastHelper(const FromType& from)
{
	return boost::static_pointer_cast< TSCAST_DEDUCED_TYPENAME ToType::element_type >(from);
}

#define TS_STATIC_CAST(from,to) TSStaticCastHelper< to >(from)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	this. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TS_THISPTR  boost::static_pointer_cast<ThisType>(shared_from_this())


template<class _Ty1,class _Ty2>
boost::shared_ptr<_Ty1> TSSafeStaticPointerCast(const boost::shared_ptr<_Ty2>& _Other)
{
#ifdef _DEBUG
	_Ty1 * p = dynamic_cast< _Ty1 * >( _Other.get() );

	ASSERT(!_Other || p);

	return boost::shared_ptr<_Ty1>( _Other, p );
#else
	return  boost::shared_ptr<_Ty1>( _Other, static_cast< _Ty1 * >( _Other.get() ) );
#endif // _DEBUG
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	this. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
///
/// <param name="to">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TS_SAFE_THISPTR(to)  TSSafeStaticPointerCast<to>(shared_from_this())

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	shared_ptr+Ptr. </summary>
///				weak_ptr+WPtr)
///				CLASS_PTR_DECLARE(TSSensor)
///				TSSensorPtr Sensor;
///				TSSensorWPtr OwnerSensor;()
/// 
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef SWIG
#define CLASS_PTR_DECLARE(a) typedef boost::shared_ptr<class a> a##Ptr;\
	typedef boost::weak_ptr<class a> a##WPtr;\
	typedef boost::shared_ptr<const class a> a##ConstPtr;
#endif // SWIG
////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	shared_ptr+Ptr. </summary>
///				\n weak_ptr+WPtr)
/// 
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SWIG
#define STRUCT_PTR_DECLARE(a) typedef boost::shared_ptr<struct a> a##Ptr;\
	typedef boost::weak_ptr<struct a> a##WPtr;\
	typedef boost::shared_ptr<const struct a> a##ConstPtr;	
#endif // SWIG

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	shared_ptr+Ptr. </summary>
///				weak_ptr+WPtr)
/// 
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef SWIG
#define INTERFACE_PTR_DECLARE(a) typedef boost::shared_ptr<struct a> a##Ptr;\
	typedef boost::weak_ptr<struct a> a##WPtr
#endif // SWIG


template<boost::posix_time::special_values special_val>
class TSTimeT : public boost::posix_time::ptime
{
	typedef boost::posix_time::ptime base_time_type;
public:
	TSTimeT(boost::gregorian::date d,base_time_type::time_duration_type td) : base_time_type(d,td)
	{}
	TSTimeT(const base_time_type & d) : base_time_type(d)
	{}
	explicit TSTimeT(boost::gregorian::date d) : base_time_type(d)
	{}
	TSTimeT(const base_time_type::time_rep_type& rhs):base_time_type(rhs)
	{}
	TSTimeT(const boost::posix_time::special_values sv) : base_time_type(sv)
	{}
	TSTimeT() : base_time_type(special_val)
	{}

	void SetAsInvalid()
	{
		operator=(boost::posix_time::not_a_date_time);
	}

	void SetAsMin()
	{
		operator=(boost::posix_time::min_date_time);
	}

	void SetAsMax()
	{
		operator=(boost::posix_time::max_date_time);
	}

	bool IsSpecial() const
	{
		return is_special();
	}

	bool IsInvalid() const
	{
		return is_not_a_date_time();
	}

	bool IsValid() const
	{
		return !is_not_a_date_time();
	}

	bool IsMax() const
	{
		return operator==(boost::posix_time::max_date_time);
	}

	bool IsMin() const
	{
		return operator==(boost::posix_time::min_date_time);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	boost. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef TSTimeT<boost::posix_time::not_a_date_time>  TSTime;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	</summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef boost::posix_time::time_duration TSTimeDuration;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	 </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef TSTimeT<boost::posix_time::min_date_time>  TSBattleTime;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	boost. </summary>
///				PostEvent(TSMicroseconds(2),XXX);
///				
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef boost::posix_time::microseconds TSMicroseconds;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	->boost. </summary>
///				PostEvent(TSSeconds(2),XXX);
///				TSTimeDuration dt = TSSeconds(1);
///				
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TSSeconds(a)		(boost::posix_time::millisec((INT64)((a) * 1000.0)))

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	->boost. </summary>
///				TSTimeDuration dt = TSMilliseconds(1);
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TSMilliseconds(a)  (boost::posix_time::microseconds((INT64)((a) * 1000.0)))

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTimeDuration. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TD2SECS(a) ((a).total_microseconds()/1000000.0)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTimeDuration. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TD2MILSECS(a) ((a).total_microseconds()/1000.0)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTimeDuration. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TD2MICSECS(a) (a).total_microseconds()


const boost::posix_time::ptime TSEpochTimeValue(boost::gregorian::date(1970,1,1));
const boost::posix_time::ptime TSMinTimeValue(boost::posix_time::min_date_time);
const boost::posix_time::ptime TSMaxTimeValue(boost::posix_time::max_date_time);
const boost::posix_time::ptime TSInvalidTimeValue(boost::posix_time::not_a_date_time);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIME2SECS(a) (a.is_special() ? 0 : (a - TSEpochTimeValue).total_microseconds()/1000000.0)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIME2MILSECS(a) (a.is_special() ? 0 : (a - TSEpochTimeValue).total_microseconds()/1000.0)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIME2MICSECS(a) (a.is_special() ? 0 : (a - TSEpochTimeValue).total_microseconds())

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTimeDuration. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2016/4/21. </remarks>
///
/// <param name="a">	 </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TD2TIME(a) (TSEpochTimeValue + a)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTimeDuration. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2016/4/21. </remarks>
///
/// <param name="a">	 </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIME2TD(a) (a - TSEpochTimeValue)


FORCEINLINE TSTime TSTimeFromString(const TSString & Str)
{
	TSTime result;

	if (!Str.empty() && Str!=TS_TEXT("not-a-date-time"))
	{
		try 
		{ 
			result = boost::posix_time::time_from_string(TSString2Ascii(Str));
		} 
		catch(...){}
	}

	return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	 TSTime. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define STR2TIME(a) TSTimeFromString(a)


FORCEINLINE TSTimeDuration TSTimeDurationFromString(const TSString & Str)
{
	TSTimeDuration result;

	try 
	{ 
		result = boost::posix_time::duration_from_string(TSString2Ascii(Str));
	} 
	catch(const std::exception &){}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTimeDuration. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define STR2TD(a)  TSTimeDurationFromString(a)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTimeC </summary>
///				YYYY-mmm-DD HH:MM::SS.ffffff :2011-Apr-23 10:18:00.234375.
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	TSTime. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////
#define TIME2CSTR(a) Ascii2TSString(boost::posix_time::to_simple_string(a)).c_str()	

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTimeDurationC. </summary>
///				HH:MM::SS.ffffff : 10:18:00.234375
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////
#define TD2CSTR(a) Ascii2TSString(boost::posix_time::to_simple_string(a)).c_str()	
////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTime. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_DATE_TIME TSTime(boost::posix_time::pos_infin)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTime. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define MIN_DATE_TIME   TSTime(boost::posix_time::neg_infin)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	TSTime. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="a">	TSTime. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define IS_INVALID_DATE_TIME(a)  a.is_not_a_date_time()

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TS_INVALID_TIME TSTime(boost::posix_time::not_a_date_time)

typedef boost::shared_ptr<class TSIUnknown> TSIUnknownPtr;
typedef boost::weak_ptr<class TSIUnknown> TSIUnknownWPtr;
typedef boost::shared_ptr<void> VoidPtr;
typedef boost::weak_ptr<void> VoidWPtr;
typedef boost::function<void()> CallbackRoutine_T;

#ifndef interface
#define interface struct
#endif // interface

class TSSharedPtrHash
{
public:
	enum 
	{
		bucket_size = 4,
		min_buckets = 8
	};

	FORCEINLINE std::size_t operator()( VoidPtr Ptr ) const
	{
		std::size_t a = (std::size_t)Ptr.get();
		a = (a+0x7ed55d16) + (a<<12);
		a = (a^0xc761c23c) ^ (a>>19);
		a = (a+0x165667b1) + (a<<5);
		a = (a+0xd3a2646c) ^ (a<<9);
		a = (a+0xfd7046c5) + (a<<3);
		a = (a^0xb55a4f09) ^ (a>>16);
		return a;
	};
	FORCEINLINE bool operator()( VoidPtr key1,VoidPtr key2 ) const
	{
		return key1 < key2;
	}
};

template<class Target,class Source>
struct TSValueCastHelper
{
	static FORCEINLINE Target Cast(const Source & arg)
	{
		try
		{
			return boost::lexical_cast<Target>(arg);
		}
		catch(const boost::bad_lexical_cast &)
		{
			return Target();
		}
	}
};

template<>
struct TSValueCastHelper<TSAsciiString,bool>
{
	static FORCEINLINE TSAsciiString Cast(const bool & arg)
	{
		return arg ? "1" : "0";
	}
};

#if !defined(_TSRUNTIME_NO_WCHAR_T)
template<>
struct TSValueCastHelper<TSWideString,bool>
{
	static FORCEINLINE TSWideString Cast(const bool & arg)
	{
		return arg ? L"1" : L"0";
	}
};
#endif

template<>
struct TSValueCastHelper<TSAsciiString,char>
{
	static FORCEINLINE TSAsciiString Cast(const char & arg)
	{
		return TSAsciiString(1,arg);
	}
};

#if !defined(_TSRUNTIME_NO_WCHAR_T)
template<>
struct TSValueCastHelper<TSWideString,char>
{
	static FORCEINLINE TSWideString Cast(const char & arg)
	{
		return Ascii2WideString(TSAsciiString(1,arg));
	}
};
#endif

template<>
struct TSValueCastHelper<char,TSAsciiString>
{
	static FORCEINLINE char Cast(const TSAsciiString & arg)
	{
		return arg.empty() ? '\0' : arg[0];
	}
};

template<>
struct TSValueCastHelper<char,const char *>
{
	static FORCEINLINE char Cast(const char * const & arg)
	{
		return arg ? arg[0] : '\0';
	}
};

template<int NUM>
struct TSValueCastHelper<char,const char[NUM]>
{
	static FORCEINLINE char Cast(const char (&arg)[NUM])
	{
		return arg ? arg[0] : '\0';
	}
};

#if !defined(_TSRUNTIME_NO_WCHAR_T)
template<>
struct TSValueCastHelper<char,TSWideString>
{
	static FORCEINLINE char Cast(const TSWideString & arg)
	{
		return arg.empty() ? '\0' : WideString2Ascii(arg)[0];
	}
};
#endif

#if !defined(_TSRUNTIME_NO_WCHAR_T)
#if !defined(_MSC_VER) || _NATIVE_WCHAR_T_DEFINED

template<>
struct TSValueCastHelper<TSAsciiString,wchar_t>
{
	static FORCEINLINE TSAsciiString Cast(const wchar_t & arg)
	{
		return WideString2Ascii(TSWideString(1,arg));
	}
};

template<>
struct TSValueCastHelper<TSWideString,wchar_t>
{
	static FORCEINLINE TSWideString Cast(const wchar_t & arg)
	{
		return TSWideString(1,arg);
	}
};

template<>
struct TSValueCastHelper<wchar_t,TSAsciiString>
{
	static FORCEINLINE wchar_t Cast(const TSAsciiString & arg)
	{
		return arg.empty() ? L'\0' : Ascii2WideString(arg)[0];
	}
};

template<>
struct TSValueCastHelper<wchar_t,const char *>
{
	static FORCEINLINE wchar_t Cast(const char * const & arg)
	{
		return arg ? Ascii2WideString(arg)[0] : L'\0';
	}
};

template<>
struct TSValueCastHelper<wchar_t,char *>
{
	static FORCEINLINE wchar_t Cast(char * const & arg)
	{
		return arg ? Ascii2WideString(arg)[0] : L'\0';
	}
};

template<int NUM>
struct TSValueCastHelper<wchar_t,const char[NUM]>
{
	static FORCEINLINE wchar_t Cast(const char (&arg)[NUM])
	{
		return arg ? Ascii2WideString(arg)[0] : L'\0';
	}
};

template<>
struct TSValueCastHelper<wchar_t,TSWideString>
{
	static FORCEINLINE wchar_t Cast(const TSWideString & arg)
	{
		return arg.empty() ? L'\0' : arg[0];
	}
};
#endif

template<>
struct TSValueCastHelper<wchar_t,const wchar_t *>
{
	static FORCEINLINE wchar_t Cast(const wchar_t * const & arg)
	{
		return arg ? arg[0] : L'\0';
	}
};

template<>
struct TSValueCastHelper<wchar_t,wchar_t *>
{
	static FORCEINLINE wchar_t Cast(wchar_t * const & arg)
	{
		return arg ? arg[0] : L'\0';
	}
};

template<int NUM>
struct TSValueCastHelper<wchar_t,const wchar_t[NUM]>
{
	static FORCEINLINE wchar_t Cast(const wchar_t (&arg)[NUM])
	{
		return arg ? arg[0] : '\0';
	}
};

#if !defined(_TSRUNTIME_NO_WCHAR_T)
template<>
struct TSValueCastHelper<char,const wchar_t *>
{
	static FORCEINLINE char Cast(const wchar_t * const & arg)
	{
		return arg ? WideString2Ascii(arg)[0] : '\0';
	}
};

template<int NUM>
struct TSValueCastHelper<char,const wchar_t[NUM]>
{
	static FORCEINLINE char Cast(const wchar_t (&arg)[NUM])
	{
		return arg ? WideString2Ascii(arg)[0] : '\0';
	}
};
#endif

#endif

template<>
struct TSValueCastHelper<bool,TSAsciiString>
{
	static FORCEINLINE bool Cast(const TSAsciiString & arg)
	{
		if (arg.empty())
		{
			return false;
		}
		return (arg[0] == '1' || arg[0] == 't' || arg[0] == 'T' || arg[0] == 'y' || arg[0] == 'Y');
	}
};

#if !defined(_TSRUNTIME_NO_WCHAR_T)
template<>
struct TSValueCastHelper<bool,TSWideString>
{
	static FORCEINLINE bool Cast(const TSWideString & arg)
	{
		if (arg.empty())
		{
			return false;
		}
		return (arg[0] == L'1' || arg[0] == L't' || arg[0] == L'T' || arg[0] == L'y' || arg[0] == L'Y');
	}
};
#endif

template<>
struct TSValueCastHelper<bool,const char *>
{
	static FORCEINLINE bool Cast(const char * const & arg)
	{
		if (arg)
		{
			return (arg[0] == '1' || arg[0] == 't' || arg[0] == 'T' || arg[0] == 'y' || arg[0] == 'Y');
		}
		return false;
	}
};

template<>
struct TSValueCastHelper<bool,char *>
{
	static FORCEINLINE bool Cast(char * const & arg)
	{
		if (arg)
		{
			return (arg[0] == '1' || arg[0] == 't' || arg[0] == 'T' || arg[0] == 'y' || arg[0] == 'Y');
		}
		return false;
	}
};

template<>
struct TSValueCastHelper<bool,const wchar_t *>
{
	static FORCEINLINE bool Cast(const wchar_t * const & arg)
	{
		if (arg)
		{
			return (arg[0] == L'1' || arg[0] == L't' || arg[0] == L'T' || arg[0] == L'y' || arg[0] == L'Y');
		}
		return false;
	}
};

template<>
struct TSValueCastHelper<bool,wchar_t *>
{
	static FORCEINLINE bool Cast(wchar_t * const & arg)
	{
		if (arg)
		{
			return (arg[0] == L'1' || arg[0] == L't' || arg[0] == L'T' || arg[0] == L'y' || arg[0] == L'Y');
		}
		return false;
	}
};

template<int NUM>
struct TSValueCastHelper<bool,const char[NUM]>
{
	static FORCEINLINE bool Cast(const char (&arg)[NUM])
	{
		if (arg)
		{
			return (arg[0] == '1' || arg[0] == 't' || arg[0] == 'T' || arg[0] == 'y' || arg[0] == 'Y');
		}
		return false;
	}
};

template<int NUM>
struct TSValueCastHelper<bool,const wchar_t[NUM]>
{
	static FORCEINLINE bool Cast(const wchar_t (&arg)[NUM])
	{
		if (arg)
		{
			return (arg[0] == L'1' || arg[0] == L't' || arg[0] == L'T' || arg[0] == L'y' || arg[0] == L'Y');
		}
		return false;
	}
};

#if !defined(_TSRUNTIME_NO_WCHAR_T)
#define DEF_NUMBERTOSTR__TEMPLATE(TYPE,AsciiFormat,WideFormat)\
	template<>\
	struct TSValueCastHelper<TSAsciiString,TYPE>\
	{\
		static FORCEINLINE TSAsciiString Cast(const TYPE & arg)\
		{\
			return TSStringUtil::FormatA(AsciiFormat,arg);\
		}\
	};\
	template<>\
	struct TSValueCastHelper<TSWideString,TYPE>\
	{\
		static FORCEINLINE TSWideString Cast(const TYPE & arg)\
		{\
			return TSStringUtil::FormatW(WideFormat,arg);\
		}\
	};
#else
#define DEF_NUMBERTOSTR__TEMPLATE(TYPE,AsciiFormat,WideFormat)\
	template<>\
	struct TSValueCastHelper<TSAsciiString,TYPE>\
	{\
		static FORCEINLINE TSAsciiString Cast(const TYPE & arg)\
		{\
			return TSStringUtil::FormatA(AsciiFormat,arg);\
		}\
	};
#endif


DEF_NUMBERTOSTR__TEMPLATE(INT8,"%d",L"%d")
DEF_NUMBERTOSTR__TEMPLATE(UINT8,"%u",L"%u")
DEF_NUMBERTOSTR__TEMPLATE(INT16,"%d",L"%d")
DEF_NUMBERTOSTR__TEMPLATE(UINT16,"%u",L"%u")
DEF_NUMBERTOSTR__TEMPLATE(INT32,"%d",L"%d")
DEF_NUMBERTOSTR__TEMPLATE(UINT32,"%u",L"%u")
DEF_NUMBERTOSTR__TEMPLATE(INT64,"%lld",L"%lld")
DEF_NUMBERTOSTR__TEMPLATE(UINT64,"%llu",L"%llu")

#define DEF_STRTONUMBER__TEMPLATE(TYPE,FUN)\
template<>\
struct TSValueCastHelper<TYPE,TSString>\
{\
	static FORCEINLINE TYPE Cast(const TSString & arg)\
	{\
		return (TYPE)FUN(arg.c_str());\
	}\
};\
template<>\
struct TSValueCastHelper<TYPE,const TSChar *>\
{\
	static FORCEINLINE TYPE Cast(const TSChar * const & arg)\
	{\
		return (TYPE)FUN(arg);\
	}\
};\
template<int NUM>\
struct TSValueCastHelper<TYPE,const TSChar[NUM]>\
{\
	static FORCEINLINE TYPE Cast(const TSChar (&arg)[NUM])\
	{\
		return (TYPE)FUN(arg);\
	}\
};


DEF_STRTONUMBER__TEMPLATE(INT8,_tstoi)
DEF_STRTONUMBER__TEMPLATE(UINT8,_tstoi)
DEF_STRTONUMBER__TEMPLATE(INT16,_tstoi)
DEF_STRTONUMBER__TEMPLATE(UINT16,_tstoi)
DEF_STRTONUMBER__TEMPLATE(INT32,_tstoi)
DEF_STRTONUMBER__TEMPLATE(UINT32,_tstoui)
DEF_STRTONUMBER__TEMPLATE(INT64,_tstoi64)
DEF_STRTONUMBER__TEMPLATE(UINT64,_tstoui64)
DEF_STRTONUMBER__TEMPLATE(FLOAT,_tstof)
DEF_STRTONUMBER__TEMPLATE(DOUBLE,_tstof)


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	. </summary>
///				
/// <remarks>	Huaru Infrastructure Team, 2011/8/4. </remarks>
///
/// <param name="arg">	. </param>
///
/// <returns>	. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Target,class Source>
FORCEINLINE Target TSValue_Cast(const Source & arg)
{
	return TSValueCastHelper<Target,Source>::Cast(arg);
}

template<typename T,int BaseIndex>
struct TSOrderedKeyHash 
{
	std::size_t operator()(T const& key) const
	{
		return (std::size_t)(key - BaseIndex);
	}
};

template<class T>
struct TSSkipAssign : public T
{
	TSSkipAssign(){}
	TSSkipAssign(const TSSkipAssign &){}
	TSSkipAssign & operator=(const TSSkipAssign & __that){ return *this;}
};


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	. </summary>
///
/// <remarks>	Huaru Infrastructure Team, 2011/8/3. </remarks>
///
/// <param name="from">	. </param>
/// <param name="to">  	. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ToType,class FromType>
FORCEINLINE ToType TSCastHelper(const FromType& from)
{
	return boost::dynamic_pointer_cast<TSCAST_DEDUCED_TYPENAME ToType::element_type>(from);
}

#define TS_CAST(from,to) TSCastHelper<to>(from)

template<class ToType,class FromType>
FORCEINLINE ToType TSSSCastHelper(const FromType& from)
{
	return TSSafeStaticPointerCast<TSCAST_DEDUCED_TYPENAME ToType::element_type>(from);
}

class TOPSIM_RT_DECL TSException : public std::exception
{
public:
	TSException(const TSString & What) throw();
	TSException(const TSChar *  What) throw();
	virtual ~TSException() throw();
	virtual const char* what() const throw();
private:
	TSAsciiString _What;
};

#define TS_SS_CAST(from,to) TSSSCastHelper<to>(from)

struct __MetaTypeObjectInfo 
{
	__MetaTypeObjectInfo()
		:__metatype_ext(0)
	{

	}
	struct ExtInfo{
		UINT32 MetaTypeId;
		int ModelDataTypeId;
	} * __metatype_ext;
};

struct TSNullClassTag
{

};

#define TSNullClass TSNullClassTag,0

#define TS_MetaType_MACRO_1(cls)\
	public:\
	typedef cls   ThisType;\
	typedef TSNullClassTag   SuperType;\
	typedef ThisType MetaTypeIdDefined;\
	friend struct MetaDataCollector<ThisType>;\
	friend struct TSMetaTypeOp<ThisType>;\
	friend struct TSObtainObjectInfoOp<ThisType>;\
	virtual const TSChar * GetMetaTypeName() const;\
	virtual UINT32 GetMetaTypeId() const;\
	protected:\
	FORCEINLINE __MetaTypeObjectInfo * __get__obj_info()  const\
	{\
		return const_cast<__MetaTypeObjectInfo *>(&__obj_info);\
	}\
	private:\
	__MetaTypeObjectInfo __obj_info;\
	public:\
	static UINT32 GetMetaTypeIdStatic();\
	static const TSChar * GetMetaTypeNameStatic();\
	static TSSubclassOf<ThisType> GetClassStatic()\
	{\
		return TSSubclassOf<ThisType>();\
	}\
	typedef boost::shared_ptr<ThisType>   ThisPtrType;\
	typedef boost::weak_ptr<ThisType>   ThisWPtrType;

#define TS_MetaType_MACRO_2(cls,sup)\
	public:\
	typedef cls   ThisType;\
	typedef sup   SuperType;\
	typedef ThisType MetaTypeIdDefined;\
	friend struct MetaDataCollector<ThisType>;\
	friend struct TSMetaTypeOp<ThisType>;\
	friend struct TSObtainObjectInfoOp<ThisType>;\
	virtual const TSChar * GetMetaTypeName() const;\
	virtual UINT32 GetMetaTypeId() const;\
	protected:\
	FORCEINLINE __MetaTypeObjectInfo * __get__obj_info() const\
	{\
		return SuperType::__get__obj_info();\
	}\
		public:\
		static UINT32 GetMetaTypeIdStatic();\
		static const TSChar * GetMetaTypeNameStatic();\
		static TSSubclassOf<ThisType> GetClassStatic()\
	{\
		return TSSubclassOf<ThisType>();\
	}\
	typedef boost::shared_ptr<ThisType>   ThisPtrType;\
	typedef boost::weak_ptr<ThisType>   ThisWPtrType;


#define TS_MetaType_External_Parent(cls,sup)\
	public:\
	typedef cls   ThisType;\
	typedef sup   SuperType;\
	typedef ThisType MetaTypeIdDefined;\
	friend struct MetaDataCollector<ThisType>;\
	friend struct TSMetaTypeOp<ThisType>;\
	friend struct TSObtainObjectInfoOp<ThisType>;\
	virtual const TSChar * GetMetaTypeName() const;\
	virtual UINT32 GetMetaTypeId() const;\
	protected:\
	FORCEINLINE __MetaTypeObjectInfo * __get__obj_info() const\
	{\
		return const_cast<__MetaTypeObjectInfo *>(&__obj_info);\
	}\
	private:\
	__MetaTypeObjectInfo __obj_info;\
	public:\
	static UINT32 GetMetaTypeIdStatic();\
	static const TSChar * GetMetaTypeNameStatic();\
	static TSSubclassOf<ThisType> GetClassStatic()\
	{\
		return TSSubclassOf<ThisType>();\
	}\
	typedef boost::shared_ptr<ThisType>   ThisPtrType;\
	typedef boost::weak_ptr<ThisType>   ThisWPtrType;


#define TS_MetaType_MACRO_3(cls,sup,dummy) TS_MetaType_MACRO_1(cls)

#ifdef _MSC_VER
#		define TS_MetaType(...) BOOST_PP_CAT(BOOST_PP_OVERLOAD(TS_MetaType_MACRO_,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#	else
#		define TS_MetaType(...) BOOST_PP_OVERLOAD(TS_MetaType_MACRO_,__VA_ARGS__)(__VA_ARGS__)
#endif // _MSC_VER


#define TS_USE_REF_OBJ(a)	\
public:\
	typedef a RefObjType;\
	void SetRefObj(boost::shared_ptr<RefObjType> Obj)\
{\
	__RefObj = Obj;\
}\
	boost::shared_ptr<RefObjType> GetRefObj()\
{\
	return __RefObj.lock();\
}\
private:\
	boost::weak_ptr<RefObjType> __RefObj

#define TS_NOCOPYABLE(type)\
public:\
	typedef type NoCopyableType;\
private: \
	type( const type& );\
	const type& operator=( const type& )

#define USE_MODEL_DATA(type) \
public:\
	typedef type DataType;\
	typedef boost::shared_ptr<DataType> DataPtrT;\
	typedef boost::weak_ptr<DataType> DataWPtrT;\
	typedef boost::shared_ptr<const DataType> DataConstPtrT;\
	typedef boost::weak_ptr<const DataType> DataConstWPtrT;\
public:\
	virtual int GetDataMetaTypeId() const\
{\
	if (__get__obj_info()->__metatype_ext)\
{\
	return __get__obj_info()->__metatype_ext->ModelDataTypeId;\
}\
	return GetDataMetaTypeIdStatic();\
}\
	FORCEINLINE int GetDataMetaTypeIdStatic() const\
{\
	return DataType::GetMetaTypeIdStatic();\
}\
	FORCEINLINE DataConstPtrT Data() const\
{\
	BOOST_STATIC_ASSERT((boost::is_base_and_derived<SuperType::DataType,DataType>::value));\
	\
	return TS_STATIC_CAST(DataPointer(),DataConstPtrT);\
}


#define TS_DECLARE_PRIVATE_COPY(Class) \
protected:\
	friend struct Class##Private;\
	inline Class##Private* GetPP() const { return reinterpret_cast<Class##Private *>(_Private_Ptr); } \
public:\
	typedef Class CopyableType;\
protected:\
	typedef Class##Private PrivateType;\



#define TS_DECLARE_PRIVATE(Class) \
	TS_NOCOPYABLE(Class);\
protected:\
	friend struct Class##Private;\
	inline Class##Private* GetPP() const { return reinterpret_cast<Class##Private *>(_Private_Ptr); } \
protected:\
	typedef Class##Private PrivateType;\

#define T_D() PrivateType * const d = GetPP()



#ifdef CHECK_PURE_VIRTUAL
#	define PURE_VIRTUAL0() = 0
#	define PURE_VIRTUAL(RET_TYPE) = 0
#	define PURE_VIRTUAL_REF(RET_TYPE) = 0
#	define PURE_VIRTUAL_DEF(RET_OP) = 0
#else
#	define PURE_VIRTUAL0() { ASSERT("Pure virtual not implemented." && 0); }
#	define PURE_VIRTUAL(RET_TYPE) { ASSERT("Pure virtual not implemented." && 0); return RET_TYPE(); }
#	define PURE_VIRTUAL_REF(RET_TYPE) { static RET_TYPE a; ASSERT("Pure virtual not implemented." && 0); return a; }
#	define PURE_VIRTUAL_DEF(RET_OP) { ASSERT("Pure virtual not implemented." && 0); RET_OP; }
#endif // CHECK_PURE_VIRTUAL



template<typename T>
struct TSNoConstRef
{
	typedef TSREMOVECV_DEDUCED_TYPENAME boost::remove_const<T>::type NoConstType;
	typedef TSREMOVECV_DEDUCED_TYPENAME boost::remove_reference<NoConstType>::type Type;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	. </summary>
///
/// <param name="StrMessage">	[in] . </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

TOPSIM_RT_DECL void TSOutDbgStringA( const TSAsciiString & StrMessage);
#if !defined(_TSRUNTIME_NO_WCHAR_T)
TOPSIM_RT_DECL void TSOutDbgStringW( const TSWideString & StrMessage);
#endif
TOPSIM_RT_DECL void TSOutDbgString( const TSString & StrMessage);
////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	. </summary>
///
/// <param name="pFormat">	[in] . </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

TOPSIM_RT_DECL void TSOutDbgPrintfA(const char * pFormat,...);
#if !defined(_TSRUNTIME_NO_WCHAR_T)
TOPSIM_RT_DECL void TSOutDbgPrintfW(const wchar_t * pFormat,...);
#endif
TOPSIM_RT_DECL void TSOutDbgPrintf(const TSChar * pFormat,...);
TOPSIM_RT_DECL void TSSchedYield(std::size_t i);

TS_END_RT_NAMESPACE

#endif // __TOPSIMCOMMTYPES__H__


