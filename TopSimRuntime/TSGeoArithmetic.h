#ifndef __BUILTINGENERATEUTILS_H__
#define __BUILTINGENERATEUTILS_H__

#include "TSIdlGram.h"
#include "TSBuiltinPlugin.h"
#include "TSSolutionMgr.h"

namespace TSIDLUtils
{
    TOPSIMIDL_DECL TSString GenHeadUUID( const TSString &FileNme);
    TOPSIMIDL_DECL TSString Table(size_t x = 1);
    TOPSIMIDL_DECL bool WriteFile(const TSString &FileName, const TSString &Content, TSString Folder = "");
    TOPSIMIDL_DECL TSString Link2Type(TSString Link);

    //TS(LinkStudio, c++, c#, java)
#define TSIDL_GEN_TYPES_SCALAR(TD) \
    TD("enum",                 "INT32",            "uint",              "") \
    TD("void",                 "void",             "void",              "void") \
    TD("char8",                "char",             "sbyte",             "byte") \
    TD("wchar_t",              "wchar_t",          "short",             "short") \
    TD("bool",                 "bool",             "bool",              "boolean") \
    TD("BOOL",                 "bool",             "bool",              "boolean") \
    TD("boolean",              "bool",             "bool",              "boolean") \
    TD("octet",                "UINT8",            "byte",              "short") \
    TD("byte",                 "UINT8",            "byte",              "byte") \
    TD("char",                 "char",             "sbyte",             "byte") \
    TD("int8",                 "INT8",             "sbyte",             "byte") \
    TD("INT8",                 "INT8",             "sbyte",             "byte") \
    TD("UINT8",                "UINT8",            "byte",              "byte") \
    TD("uint8",                "UINT8",            "byte",              "byte") \
    TD("int16",                "INT16",            "short",             "short") \
    TD("INT16",                "INT16",            "short",             "short") \
    TD("uint16",               "UINT16",           "ushort",            "short") \
    TD("UINT16",               "UINT16",           "ushort",            "short") \
    TD("int",                  "INT32",            "int",               "int") \
    TD("int32",                "INT32",            "int",               "int") \
    TD("INT32",                "INT32",            "int",               "int") \
    TD("long",                 "INT32",            "int",               "int") \
    TD("uint32",               "UINT32",           "uint",              "int") \
    TD("UINT32",               "UINT32",           "uint",              "int") \
    TD("unsigned long",        "UINT32",           "uint",              "int") \
    TD("int64",                "INT64",            "long",              "long") \
    TD("INT64",                "INT64",            "long",              "long") \
    TD("long long",            "INT64",            "long",              "long") \
    TD("uint64",               "UINT64",           "ulong",             "long") \
    TD("UINT64",               "UINT64",           "ulong",             "long") \
    TD("unsigned long long",   "UINT64",           "ulong",             "long") \
    TD("short",                "INT16",            "short",             "short") \
    TD("unsigned short",       "UINT16",           "ushort",            "short") \
    TD("ushort",               "UINT16",           "ushort",            "short") \
    TD("float",                "FLOAT",            "float",             "float") \
    TD("FLOAT",                "float",            "float",             "float") \
    TD("float32",              "FLOAT",            "float",             "float") \
    TD("float64",              "FLOAT",            "float",             "float") \
    TD("double",               "DOUBLE",           "double",            "double") \
    TD("DOUBLE",               "double",           "double",            "double") \
    TD("string",               "std::string",      "string",            "String") \
    TD("vector",               "vector",           "List",              "Vector") \
    TD("map",                  "map",              "Dictionary",        "Map") 
    
#define TSIDL_GEN_TYPES(TD) \
    TSIDL_GEN_TYPES_SCALAR(TD) \

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	替换全部命名空间字符串. </summary>
	///
	/// <remarks>	孟龙龙, 2017/11/28. </remarks>
	///
	/// <param name="replace_begin_namespace">	开始命名空间字符串 __NAMESPACE_BEGIN__. </param>
	/// <param name="replace_end_namespace">  	结束命名空间字符串__NAMESPACE_END__. </param>
	/// <param name="replace_content">		  	[in,out] 被替换的字符串. </param>
	/// <param name="suit_full_type_name">	  	全类型名 形如 xxx::xxx::xxx. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceFullNameSpaceString(const TSString & replace_begin_namespace,
		const TSString & replace_end_namespace,
		TSString & replace_content,
		TSTypeDefPtr TypeDef,
        const TSString & WithFun = "",
        bool SupportSpace = false);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	替换类型字符串为类型名（带DataType）. </summary>
	///
	/// <remarks>	孟龙龙, 2017/11/28. </remarks>
	///
	/// <param name="replace_super_type_full_name">	替换的类型字符串 __TYPE_FULLNAME__. </param>
	/// <param name="replace_content">			   	[in,out] 被替换的字符串. </param>
	/// <param name="suit_full_type_name">		   全类型名(不含DataType). </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceTypeFullName(const TSString & replace_type_full_name,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	替换导出宏. </summary>
	///
	/// <remarks>	孟龙龙, 2017/11/28. </remarks>
	///
	/// <param name="replace_export_macro">	导出宏字符串 __EXPORT_MACRO__. </param>
	/// <param name="replace_content">	   	[in,out] 被替换的字符串. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceExportMacro(const TSString & replace_export_macro,
		TSString & replace_content,TSTypeDefPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	替换初始化列表. </summary>
	///
	/// <remarks>	孟龙龙, 2017/11/28. </remarks>
	///
	/// <param name="replace_init_list">	初始化列表字符串 __CONSTRUCT_LIST__. </param>
	/// <param name="replace_content">  	[in,out] 被替换的字符串. </param>
	/// <param name="TypeDef">			类型定义. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceInitList(const TSString & replace_init_list,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	替换成员变量赋值. </summary>
	///
	/// <remarks>	孟龙龙, 2017/11/28. </remarks>
	///
	/// <param name="replace_fileds_assign">	成员变量赋值字符串. </param>
	/// <param name="OtherName">		        右元名称. </param>
	/// <param name="replace_content">			[in,out] 被替换的字符串. </param>
	/// <param name="TypeDef">			类型定义. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceFiledsAssign(const TSString & replace_fileds_assign,
		const TSString & OtherName,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	替换成员变量相等比较. </summary>
	///
	/// <remarks>	孟龙龙, 2017/11/28. </remarks>
	///
	/// <param name="replace_fileds_equal">成员变量相等字符串. </param>
	/// <param name="OtherName">		   右元名称. </param>
	/// <param name="replace_content">	   [in,out] 被替换的字符串. </param>
	/// <param name="TypeDef">			类型定义. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceFiledsEqual(const TSString & replace_fileds_equal,
		const TSString & OtherName,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	替换成员变量小于比较. </summary>
	///
	/// <remarks>	孟龙龙, 2017/11/28. </remarks>
	///
	/// <param name="replace_fileds_less_compare">	成员变量小于字符串. </param>
	/// <param name="OtherName">				  	右元名称. </param>
	/// <param name="replace_content">			  	[in,out] 被替换的字符串. </param>
	/// <param name="TypeDef">			类型定义. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceFiledsLessCompare(const TSString & replace_fileds_less_compare,
		const TSString & OtherName,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	替换成员变量定义. </summary>
	///
	/// <remarks>	孟龙龙, 2017/11/28. </remarks>
	///
	/// <param name="replace_fileds_declare">	成员变量定义字符串. </param>
	/// <param name="replace_content">		 	[in,out] 被替换的字符串. </param>
	/// <param name="TypeDef">			        类型定义. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceFiledsDeclare(const TSString & replace_fileds_declare,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

    void GenerateEnum(TSProjFilePtr ProjFile,TSEnumDefPtr TypeDef);

    TSString GetTopicName(TSTypeDefPtr TypeDef);

    TSString GetFileRelativePath(TSTypeDefPtr TypeDef);


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>  获取函数字符串. </summary>
    ///
    /// <remarks>   李子洋, 2017/12/19. </remarks>
    ///
    /// <param name="Func"> 函数定义指针. </param>
    ///
    /// <returns>   函数字符串. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    TSString GetFunctionStr(TSFunctionDefPtr Func);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   获取属性类型设置属性Get函数. </summary>
    ///
    /// <remarks>   李子洋, 2017/12/19. </remarks>
    ///
    /// <param name="Field">    属性指针. </param>
    ///
    /// <returns>   属性获取函数字符串. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    TSString GetFieldGetFuncStr(TSFieldDefPtr Field);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>    获取属性类型设置属性Set函数. </summary>
    ///
    /// <remarks>   李子洋, 2017/12/19. </remarks>
    ///
    /// <param name="Field">    属性指针. </param>
    ///
    /// <returns>   属性设置函数字符串. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Writexidl(const TSString & FileName,const TSString & WritePath);

    TSString GetFieldSetFuncStr(TSFieldDefPtr Field);

    TSString GetFuncWithoutResultStr(TSFunctionDefPtr Func,bool IsService = false);

    TSString GetFuncAsyncCallBackStr(TSFunctionDefPtr func,bool IsService = false);

    TSString GetFuncAsyncStr(TSFunctionDefPtr func,const TSString & Namespace,bool IsService = false,bool IsHead = false);

    TSString GetServiceFunAsyncStr(TSFunctionDefPtr func,bool IsServiceTime = false);

    TSString GetFuncAsyncHandlerStr(TSFunctionDefPtr func,const TSString & Namespace,bool IsService = false);

    TSString GetFuncHandlerStr(TSFunctionDefPtr func,const TSString & Namespace);

    TSString GetFuncCallStr(TSFunctionDefPtr func);

    TSString GetOutParamCode(TSTypeDefPtr Type , TSString ParamName , TSTypeDefPtr ParamType);

    TSString GetOutParamVecCode(TSTypeDefPtr Type ,TSString ParamName ,TSTypeDefPtr ParamType);

    TSString GetFunctions(TSTypeDefWithFuncAndFiledPtr TypeDef);

    TSString GetFuncWithoutParamType(TSFunctionDefPtr Func,const TSString & Response = TSString(""));

    TSString GetFuncParamWithoutFuncName(TSFunctionDefPtr Func);

    void ReplaceTypeNameWithCsharp(TSString & replace_content);

    void ReplaceFieldDeclareWithService(const TSString & replace_fileds_declare,
        TSString & replace_content,
        TSFunctionDefPtr TypeDef,const bool & Requst = true);
    
    TSString ReplaceExportFileName(TSString & FileContent);
    void ReplaceExportHeader(const TSString & replace_export_file,TSString & replace_content,const TSString & replace_dir_name);
    void ReplaceLinkerVersion(TSString & content,const TSString & replace_export_name);
    bool KeyWordSearch( const TSString & keyWord );
	TSString GenerateTypeVersionNum(TSTypeDefWithFiledPtr Typedef);
	TSString GenerateTypeVersionNum4Service(TSFunctionDefPtr FunDef,bool IsReq);
}

#endif

