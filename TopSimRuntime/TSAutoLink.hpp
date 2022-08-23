#ifndef _USE_LIB_WITH_SUFFIX
#define _USE_LIB_WITH_SUFFIX 1
#endif

#ifdef _MSC_VER
#	if  _USE_LIB_WITH_SUFFIX == 1
#		ifndef _LIBEXTNAME
#			if defined(_UNICODE) || defined(_DEBUG)
#				define _LIBEXTNAME "_" _UNICODE_FLAG _DEBUG_FLAG ".lib"
#			else
#				define _LIBEXTNAME  ".lib"
#			endif
#		endif // _LIBEXTNAME
#	else
#		ifndef _LIBEXTNAME
#			ifdef _DEBUG
#				define _LIBEXTNAME _UNICODE_FLAG "d.lib"
#			else 
#				define _LIBEXTNAME _UNICODE_FLAG ".lib"
#			endif // _DEBUG
#		endif
#	endif

#	ifdef __LIB_NAME__
#		ifndef _NO_AUTO_LINK
#			pragma comment(lib, __LIB_NAME__ _VERNAME _SGINAME _LIBEXTNAME)
#			pragma message ("Linking to lib file: " __LIB_NAME__ _VERNAME _SGINAME _LIBEXTNAME)
#		endif // _NO_AUTO_LINK
#		undef __LIB_NAME__
#	else
#		error "undefine __LIB_NAME__"
#	endif // __LIB_NAME__
#else
#	undef __LIB_NAME__
#endif // _MSC_VER

