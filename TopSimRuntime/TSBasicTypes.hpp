#ifndef __TSBASICTYPES_H__
#define __TSBASICTYPES_H__

#define _USE_LIB_WITH_SUFFIX 1

#if _WIN32 || _WIN64
#	if defined(_M_X64) || defined(__x86_64__) || defined(__aarch64__) || defined(__mips64) || defined(__sw_64) || defined(__sw_64__)
#       define __TOPSIM_arch64 1
#   elif defined(_M_IA64) || defined(__ia64__)
#		define __TOPSIM_ipf 1
#   elif defined(_M_IX86) || defined(__i386__) || defined(__aarch32__) 
#       define __TOPSIM_arch32 1
#   endif
#else
#   if !__linux__ && !__APPLE__
#       define __TOPSIM_generic_os 1
#   endif
#   if __x86_64__ || __aarch64__ || defined(__mips64) || defined(__sw_64) || defined(__sw_64__)
#       define __TOPSIM_arch64 1
#   elif __ia64__
#       define __TOPSIM_ipf 1
#   elif __i386__ || __i386 || __aarch32__
#       define __TOPSIM_arch32 1
#   else
#       define __TOPSIM_generic_arch 1
#   endif
#endif // _WIN32

#if !defined(IS_LITTLE_ENDIAN) && (defined(__BIG_ENDIAN__) || defined(__sparc) || defined(__sparc__) || defined(__hpux) || defined(__hppa__) || (defined(__MWERKS__) && !defined(__INTEL__)))
#	define IS_BIG_ENDIAN
#endif

#ifndef IS_BIG_ENDIAN
#	define IS_LITTLE_ENDIAN
#endif

#if defined(_MSC_VER)
#define TS_ALIGNOF(type)	__alignof(type)
#define TS_DECL_ALIGN(n)	__declspec(align(n))
#define FORCEINLINE			__forceinline
#define FORCENOINLINE		__declspec(noinline)
#define TS_WEAK_VAR_DECL	__declspec(selectany)
#define TS_PACKED_ALIGN_STRUCT_1	
#else
#define TS_ALIGNOF(type)	__alignof__(type)
#define TS_DECL_ALIGN(n)	__attribute__((__aligned__(n)))
#define FORCEINLINE			inline __attribute__ ((always_inline))
#define FORCENOINLINE		__attribute__((noinline))
#define TS_WEAK_VAR_DECL	__attribute__((weak))
#define TS_PACKED_ALIGN_STRUCT_1	__attribute__((packed, aligned(1)))
#endif

#ifdef _MSC_VER
#	if	 _MSC_VER == 1400
#		define _CRT_VER_NAME "80" // Visual Studio 2005
#	elif _MSC_VER == 1500
#		define _CRT_VER_NAME "90" // Visual Studio 2008
#	elif _MSC_VER == 1600
#		define _CRT_VER_NAME "100" // Visual Studio 2010
#	elif _MSC_VER == 1700
#		define _CRT_VER_NAME "110" // Visual Studio 2012
#	elif _MSC_VER == 1800
#		define _CRT_VER_NAME "120" // Visual Studio 2013
#	elif _MSC_VER == 1900
#		define _CRT_VER_NAME "140" // Visual Studio 2015
#	elif _MSC_VER >= 1910
#		define _CRT_VER_NAME "140" // Visual Studio 2017
#	elif _MSC_VER < 1400
#		error "Not Support Compiler Version!"
#	endif
#	define _COMPILER_NAME	"vc" 
#	ifndef _SHOW_ALL_WARNING
#		pragma warning(disable:4275)
#		pragma warning(disable:4251)
#		pragma warning(disable:4996)
#		pragma warning(disable:4800)
#	endif
#	ifdef _LIB
#		define TOPSIM_SYMBOL_EXPORT
#		define TOPSIM_SYMBOL_IMPORT
#	else
#		define TOPSIM_SYMBOL_EXPORT	__declspec(dllexport)
#		define TOPSIM_SYMBOL_IMPORT	__declspec(dllimport)
#	endif
#else
#	define _COMPILER_NAME	""
#	define _CRT_VER_NAME ""
#	define _VERNAME ""
#	define TOPSIM_SYMBOL_EXPORT		__attribute__((visibility("default")))
#	define TOPSIM_SYMBOL_IMPORT
#endif

#ifdef _WIN32
#	define _EXE_EXT	".exe"
#	define _MODULE_EXT ".dll"
#elif defined(_OS_MAC)
#	define _EXE_EXT	""
#	define _MODULE_EXT ".dylib"
#else
#	define _EXE_EXT	""
#	define _MODULE_EXT ".so"
#endif

#ifdef _USE_LIB_WITH_SUFFIX
#	ifndef _VERNAME
#		ifdef __TOPSIM_arch32
#			define _VERNAME "-" _COMPILER_NAME _CRT_VER_NAME "-x32"
#		else
#			define _VERNAME "-" _COMPILER_NAME _CRT_VER_NAME "-x64"
#		endif // __TOPSIM_arch32
#	endif // _VERNAME

#	ifndef _SGINAME
#		ifdef USESTLPORT
#			define _SGINAME "-sgi"
#		else
#			define _SGINAME ""
#		endif
#	endif // _SGINAME


#	ifdef _DEBUG
#		define _DEBUG_FLAG "d"
#	else
#		define _DEBUG_FLAG ""
#	endif // _DEBUG

#	ifdef _UNICODE
#		define _UNICODE_FLAG "U"
#	else
#		define _UNICODE_FLAG ""
#	endif // _UNICODE

#	if defined(_UNICODE) || defined(_DEBUG)
#		define _DLLEXTNAME "_" _UNICODE_FLAG _DEBUG_FLAG _MODULE_EXT
#	else
#		define _DLLEXTNAME  _MODULE_EXT
#	endif
#else
#	ifndef _VERNAME
#			define _VERNAME "_" _CPER_NAME _CPER_VER_NAME
#	endif // _VERNAME

#	ifndef _SGINAME
#		ifdef USESTLPORT
#			define _SGINAME "_sgi"
#		else
#			define _SGINAME ""
#		endif
#	endif // _SGINAME


#	ifdef _DEBUG
#		define _DEBUG_FLAG "d"
#	else
#		define _DEBUG_FLAG ""
#	endif // _DEBUG

#	ifdef _UNICODE
#		define _UNICODE_FLAG "U"
#	else
#		define _UNICODE_FLAG ""
#	endif // _UNICODE

#	define _DLLEXTNAME _UNICODE_FLAG _DEBUG_FLAG _MODULE_EXT
#endif

#define DYNAMICLIBNAME(a) a _VERNAME _SGINAME _DLLEXTNAME


#ifdef TOPSIMRUNTIME_EXPORTS
#	define TOPSIM_RT_DECL TOPSIM_SYMBOL_EXPORT
#else
#	ifdef USE_TOPSIM_RT_STATIC
#		define TOPSIM_RT_DECL
#	else
#		define TOPSIM_RT_DECL TOPSIM_SYMBOL_IMPORT
#	endif
#	define __LIB_NAME__ "TopSimRuntime"
#	include<TopSimRuntime/TSAutoLink.h>
#endif

#define TS_MAX_NAME_LENGTH 64

#if _WIN32 || _WIN64
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT		0x0502
#	endif // _WIN32_WINNT
#	ifndef WINVER
#		define WINVER			_WIN32_WINNT
#	endif // WINVER
#	if _MSC_VER < 1500
#		pragma push_macro("_interlockedbittestandset")
#		pragma push_macro("_interlockedbittestandreset")
#		pragma push_macro("_interlockedbittestandset64")
#		pragma push_macro("_interlockedbittestandreset64")
#		define _interlockedbittestandset _local_interlockedbittestandset
#		define _interlockedbittestandreset _local_interlockedbittestandreset
#		define _interlockedbittestandset64 _local_interlockedbittestandset64
#		define _interlockedbittestandreset64 _local_interlockedbittestandreset64
#		include <intrin.h>
#		pragma pop_macro("_interlockedbittestandset")
#		pragma pop_macro("_interlockedbittestandreset")
#		pragma pop_macro("_interlockedbittestandset64")
#		pragma pop_macro("_interlockedbittestandreset64")
#	else
#		include <intrin.h>
#	endif
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN 
#	endif
#	include <windows.h>
#	include <tchar.h>
#endif // _WIN32

#include <stdlib.h>
#include <string>
#include <assert.h>
#include <vector>
#include <iostream>
#include <deque>
#include <list>

#ifndef ASSERT
#define ASSERT assert
#endif // ASSERT

#ifdef _DEBUG
#define VERIFY(f)          ASSERT(f)
#else
#define VERIFY(f)           ((void)(f))
#endif // _DEBUG

#define BOOST_PP_VARIADICS 1

#include <tbb/concurrent_vector.h>

#if (defined __hpux || defined __linux)
#define BOOST_THREAD_DONT_USE_CHRONO 1
#endif

#ifndef _WIN32
#ifndef Q_MOC_RUN
#include <boost/cstdint.hpp>
#endif//Q_MOC_RUN

#ifndef __VXWORKS__
typedef boost::uint8_t UINT8;
typedef boost::uint16_t UINT16;
typedef boost::uint32_t UINT32;
typedef boost::uint64_t UINT64;
typedef boost::int8_t INT8;
typedef boost::int16_t INT16;
typedef boost::int32_t INT32;
typedef boost::int64_t INT64;
#elif defined(_WRS_KERNEL)
#define _TSRUNTIME_NO_WCHAR_T
#endif // __VXWORKS__

#endif // _WIN32
//
typedef float FLOAT;
typedef double DOUBLE;
typedef UINT64 TrackNumberType;

#ifdef _tstoui64
#undef _tstoui64
#endif // _tstoui64

#ifdef _tstoui
#undef _tstoui
#endif // _tstoui

#if defined(_UNICODE) && !defined(_TSRUNTIME_NO_WCHAR_T)
#	define  TS_TEXT(quote) L##quote 
	typedef wchar_t TSChar;
#	ifndef _TCHAR_DEFINED
		typedef wchar_t     TCHAR;
#		define _TCHAR_DEFINED
#	endif
#	define  STD_COUT std::wcout
#	define  STD_CERR std::wcerr
#	define _tstoui(a)	wcstoul(a,NULL,10)
#	ifndef _WIN32
#		define _tcslen		wcslen
#		define _tcscpy		wcscpy
#		define _tcscat		wcscat
#		define _tcscmp		wcscmp
#		define _tcsicmp		wcsicmp
#		define _tcsupr		wcsupr
#		define _tcslwr		wcslwr
#		define _tcsncpy		wcsncpy
#		define _tcsncmp		wcsncmp
#		define _stscanf		swscanf
#		define _tprintf		wprintf 
#		define _tcsstr		wcsstr 
#		define _tstoi(a)	wcstol(a,NULL,10)
#		define _tstoi64(a)	wcstoll(a,NULL,10)
#		define _tstoui64(a) wcstoull(a,NULL,10)
#		define _tstof(a)	wcstod(a,NULL)
#		define _stprintf	swprintf
#	else
#		define _tstoui64(a) _wcstoui64(a,NULL,10)
#	endif
#else
#	define  TS_TEXT(quote) quote
		typedef char TSChar;
#	ifndef _TCHAR_DEFINED
		typedef char     TCHAR;
#		define _TCHAR_DEFINED
#	endif
#	define STD_COUT std::cout
#	define STD_CERR std::cerr
#	define _tstoui(a)	strtoul(a,NULL,10)
#	ifndef _WIN32
#		define _tcslen		strlen
#		define _tcscpy		strcpy
#		define _tcscat		strcat
#		define _tcscmp		strcmp
#		define _tcsicmp		stricmp
#		define _tcsupr		strupr
#		define _tcslwr		strlwr
#		define _tcsncpy		strncpy
#		define _tcsncmp		strncmp
#		define _stscanf		sscanf
#		define _tprintf		printf 
#		define _tcsstr		strstr 
#		if (defined __hpux || defined __linux || defined __VXWORKS__)
#			define _tstoi(a)	atoi(a)
#			define _tstoi64(a)	atoll(a)
#			define _tstof(a)	atof(a)
#		else
#			define _tstoi(a)	atoi(a,NULL,10)
#			define _tstoi64(a)	atoll(a,NULL,10)
#			define _tstof(a)	atof(a,NULL)
#		endif
#		define _tstoui64(a) strtoull(a,NULL,10)
#		define _stprintf	sprintf
#	else
#		define _tstoui64(a) _strtoui64(a,NULL,10)
#	endif
#endif

#ifndef MBS_ENCODING_IS_NOT_UTF8
#	ifndef _MSC_VER
#		define MBS_ENCODING_IS_UTF8
#	endif
#endif
#define UTF8_ENCODING_NAME					"UTF-8"

#if defined(MBS_ENCODING_IS_UTF8)
#	define MBS_ENCODING_NAME				UTF8_ENCODING_NAME
#else
#	define MBS_ENCODING_NAME				"GB18030"
#endif


#ifdef _WIN32
#	define LINE_TERMINATOR				TS_TEXT("\r\n")
#	define LINE_TERMINATOR_ANSI			"\r\n"
#else
#	define LINE_TERMINATOR				TS_TEXT("\n")
#	define LINE_TERMINATOR_ANSI			"\n"
#endif // _WIN32


#ifndef _WIN32
typedef TSChar * LPTSTR;
typedef const TSChar * LPCTSTR;
#endif

#define TS_NO_RT_NAMESPACE

#ifndef TS_NO_RT_NAMESPACE
#	ifndef TS_RT_NAMESPACE
#		define TS_RT_NAMESPACE TopSimRuntime_v5
#	endif // TS_RT_NAMESPACE
#	define TS_BEGIN_RT_NAMESPACE namespace TS_RT_NAMESPACE {
#	define TS_END_RT_NAMESPACE }
#else
#	ifndef TS_RT_NAMESPACE
#		define TS_RT_NAMESPACE
#	endif // TS_RT_NAMESPACE
#	define TS_BEGIN_RT_NAMESPACE
#	define TS_END_RT_NAMESPACE
#   define TS_NO_USE_RT_NAMESPACE
#endif // TS_NO_RT_NAMESPACE

#include "TSString.h"

#ifndef TS_NO_USE_RT_NAMESPACE
using namespace TS_RT_NAMESPACE;
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef UNUSED
#define UNUSED(EXP)		(void)EXP
#endif

#ifdef _WIN32
#define TSRGB(r,g,b) ((UINT32)((((UINT16)0xFF)<<24) | (((UINT16)b)<<16) | (((UINT16)g)<<8) | ((UINT16)r)))
#define TSRGBA(r,g,b,a) ((UINT32)((((UINT16)a)<<24) | (((UINT16)b)<<16) | (((UINT16)g)<<8) | ((UINT16)r)))
#else
#define TSRGB(r,g,b) ((UINT32)((0xffu << 24) | (((INT32)r & 0xff) << 16) | (((INT32)g & 0xff) << 8) | ((INT32)b & 0xff)))
#define TSRGBA(r,g,b,a) ((UINT32)((((INT32)a & 0xff) << 24) | (((INT32)r & 0xff) << 16) | (((INT32)g & 0xff) << 8) | ((INT32)b & 0xff)))
#endif

#define TESTBIT(a,b)  (a & (b))
#define CLRBIT(a,b)   a &= ~(b)
#define SETBIT(a,b)   a |= (b)

#define MAKEUINT16(hi,lo)	((UINT16)(((UINT8)(((UINT32)(lo)) & 0xff)) | ((UINT16)((UINT8)(((UINT32)(hi)) & 0xff))) << 8))
#define MAKEUINT32(hi,lo)	((UINT32)(((UINT16)(((UINT32)(lo)) & 0xffff)) | ((UINT32)((UINT16)(((UINT32)(hi)) & 0xffff))) << 16))
#define MAKEUINT64(hi,lo)	((UINT64)(((UINT32)(((UINT64)(lo)) & 0xffffffff)) | ((UINT64)((UINT32)(((UINT64)(hi)) & 0xffffffff))) << 32))
#define LOUINT32(i)           ((UINT32)(((UINT64)(i)) & 0xffffffff))
#define LOUINT16(i)           ((UINT16)(((UINT32)(i)) & 0xffff))
#define HIUINT32(i)           ((UINT32)((((UINT64)(i)) >> 32) & 0xffffffff))
#define HIUINT16(i)           ((UINT16)((((UINT32)(i)) >> 16) & 0xffff))

#define TS_SUCCESS        0
#define TS_EXIST                1
#define TS_NOTEXIST        2
#define TS_INVALID           ((UINT32)-1)
#define TS_INVALID_HANDLE_VALUE(a) (a==TSHANDLE(-1) || a==TSHANDLE(0))

#define TS_HIGHESTPRIORITY  0
#define TS_LOWESTPRIORITY   255
#define TS_DEFAULTPRIORITY  127

typedef UINT8 TSPriority;
typedef TSString TSMetaTypeName;
typedef TSString TSIdentification;

#define TSUnorderedMap		boost::unordered_map
#define TSUnorderedSet		boost::unordered_set
#define TSConUnorderedMap   tbb::interface5::concurrent_unordered_map
#define TSConUnorderedSet   tbb::interface5::concurrent_unordered_set
#define TSConArray			tbb::concurrent_vector


typedef std::vector<TSString> TSStringArray;

template<typename ID_T, int UNIQUE_ID>
struct TSIDWrapper
{
	ID_T _value;

	TSIDWrapper(const ID_T val = 0)
		: _value(val)
	{

	}

	template<typename OTHER_ID_T, int OTHER_UNIQUE_ID>
	TSIDWrapper(const TSIDWrapper<OTHER_ID_T,OTHER_UNIQUE_ID>& other)
		: _value((ID_T)other._value)
	{

	}

	template<typename OTHER_ID_T>
	TSIDWrapper(const TSIDWrapper<OTHER_ID_T,UNIQUE_ID>& other)
		: _value((ID_T)other._value)
	{

	}

	FORCEINLINE TSIDWrapper & operator= (const TSIDWrapper& other)
	{
		_value= other._value;

		return *this;
	}

	FORCEINLINE bool operator!() const
	{
		return _value == 0 || _value==TS_INVALID;
	}

	FORCEINLINE operator ID_T() const
	{
		return _value;
	}

	FORCEINLINE ID_T & operator *()
	{
		return _value;
	}
};

typedef	TSIDWrapper<UINT32,0> TSHANDLE;
typedef	TSIDWrapper<UINT32,1> TSModelTmplOrClassId;
typedef	TSIDWrapper<UINT32,2> TSTemplateId;
typedef	TSIDWrapper<UINT32,3> TSModelDataId;
typedef	TSIDWrapper<UINT32,4> TSAssetTypeId;
typedef	TSIDWrapper<UINT32,5> TSMissionId;
typedef	TSIDWrapper<UINT32,6> TSClassificationId;

typedef std::vector<TSMissionId> TSMissionIdArray;
typedef UINT32 TSRESULT;
typedef std::vector<TSHANDLE> HandleCollection;
typedef std::vector<TSHANDLE> HandleArray;

template <class Type>
FORCEINLINE const TSChar* constchar_cast( const Type &type )
{
	return type;
}

template <>
FORCEINLINE const TSChar* constchar_cast( const TSString &type )
{
	return type.c_str();
}

template <typename Map>
FORCEINLINE typename Map::mapped_type & MapInsertOrGet(Map& mapKeyToValue, const typename Map::key_type& key, const typename Map::mapped_type& defValue)
{
	std::pair<typename Map::iterator, bool> iresult = mapKeyToValue.insert(typename Map::value_type(key, defValue));

	return iresult.first->second;
}

template <typename Map>
FORCEINLINE typename Map::mapped_type FindInMap(const Map& mapKeyToValue, const typename Map::key_type& key, typename Map::mapped_type valueDefault)
{
	typename Map::const_iterator it = mapKeyToValue.find (key);
	if (it == mapKeyToValue.end())
	{
		return valueDefault;
	}
	else
	{
		return it->second;
	}
}

#endif // __TSBASICTYPES_H__

