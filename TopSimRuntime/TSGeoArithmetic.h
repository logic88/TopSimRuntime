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
	/// <summary>	�滻ȫ�������ռ��ַ���. </summary>
	///
	/// <remarks>	������, 2017/11/28. </remarks>
	///
	/// <param name="replace_begin_namespace">	��ʼ�����ռ��ַ��� __NAMESPACE_BEGIN__. </param>
	/// <param name="replace_end_namespace">  	���������ռ��ַ���__NAMESPACE_END__. </param>
	/// <param name="replace_content">		  	[in,out] ���滻���ַ���. </param>
	/// <param name="suit_full_type_name">	  	ȫ������ ���� xxx::xxx::xxx. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceFullNameSpaceString(const TSString & replace_begin_namespace,
		const TSString & replace_end_namespace,
		TSString & replace_content,
		TSTypeDefPtr TypeDef,
        const TSString & WithFun = "",
        bool SupportSpace = false);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�滻�����ַ���Ϊ����������DataType��. </summary>
	///
	/// <remarks>	������, 2017/11/28. </remarks>
	///
	/// <param name="replace_super_type_full_name">	�滻�������ַ��� __TYPE_FULLNAME__. </param>
	/// <param name="replace_content">			   	[in,out] ���滻���ַ���. </param>
	/// <param name="suit_full_type_name">		   ȫ������(����DataType). </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceTypeFullName(const TSString & replace_type_full_name,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�滻������. </summary>
	///
	/// <remarks>	������, 2017/11/28. </remarks>
	///
	/// <param name="replace_export_macro">	�������ַ��� __EXPORT_MACRO__. </param>
	/// <param name="replace_content">	   	[in,out] ���滻���ַ���. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceExportMacro(const TSString & replace_export_macro,
		TSString & replace_content,TSTypeDefPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�滻��ʼ���б�. </summary>
	///
	/// <remarks>	������, 2017/11/28. </remarks>
	///
	/// <param name="replace_init_list">	��ʼ���б��ַ��� __CONSTRUCT_LIST__. </param>
	/// <param name="replace_content">  	[in,out] ���滻���ַ���. </param>
	/// <param name="TypeDef">			���Ͷ���. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceInitList(const TSString & replace_init_list,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�滻��Ա������ֵ. </summary>
	///
	/// <remarks>	������, 2017/11/28. </remarks>
	///
	/// <param name="replace_fileds_assign">	��Ա������ֵ�ַ���. </param>
	/// <param name="OtherName">		        ��Ԫ����. </param>
	/// <param name="replace_content">			[in,out] ���滻���ַ���. </param>
	/// <param name="TypeDef">			���Ͷ���. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceFiledsAssign(const TSString & replace_fileds_assign,
		const TSString & OtherName,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�滻��Ա������ȱȽ�. </summary>
	///
	/// <remarks>	������, 2017/11/28. </remarks>
	///
	/// <param name="replace_fileds_equal">��Ա��������ַ���. </param>
	/// <param name="OtherName">		   ��Ԫ����. </param>
	/// <param name="replace_content">	   [in,out] ���滻���ַ���. </param>
	/// <param name="TypeDef">			���Ͷ���. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceFiledsEqual(const TSString & replace_fileds_equal,
		const TSString & OtherName,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�滻��Ա����С�ڱȽ�. </summary>
	///
	/// <remarks>	������, 2017/11/28. </remarks>
	///
	/// <param name="replace_fileds_less_compare">	��Ա����С���ַ���. </param>
	/// <param name="OtherName">				  	��Ԫ����. </param>
	/// <param name="replace_content">			  	[in,out] ���滻���ַ���. </param>
	/// <param name="TypeDef">			���Ͷ���. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceFiledsLessCompare(const TSString & replace_fileds_less_compare,
		const TSString & OtherName,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�滻��Ա��������. </summary>
	///
	/// <remarks>	������, 2017/11/28. </remarks>
	///
	/// <param name="replace_fileds_declare">	��Ա���������ַ���. </param>
	/// <param name="replace_content">		 	[in,out] ���滻���ַ���. </param>
	/// <param name="TypeDef">			        ���Ͷ���. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void ReplaceFiledsDeclare(const TSString & replace_fileds_declare,
		TSString & replace_content,
		TSTypeDefWithFiledPtr TypeDef);

    void GenerateEnum(TSProjFilePtr ProjFile,TSEnumDefPtr TypeDef);

    TSString GetTopicName(TSTypeDefPtr TypeDef);

    TSString GetFileRelativePath(TSTypeDefPtr TypeDef);


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>  ��ȡ�����ַ���. </summary>
    ///
    /// <remarks>   ������, 2017/12/19. </remarks>
    ///
    /// <param name="Func"> ��������ָ��. </param>
    ///
    /// <returns>   �����ַ���. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    TSString GetFunctionStr(TSFunctionDefPtr Func);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   ��ȡ����������������Get����. </summary>
    ///
    /// <remarks>   ������, 2017/12/19. </remarks>
    ///
    /// <param name="Field">    ����ָ��. </param>
    ///
    /// <returns>   ���Ի�ȡ�����ַ���. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    TSString GetFieldGetFuncStr(TSFieldDefPtr Field);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>    ��ȡ����������������Set����. </summary>
    ///
    /// <remarks>   ������, 2017/12/19. </remarks>
    ///
    /// <param name="Field">    ����ָ��. </param>
    ///
    /// <returns>   �������ú����ַ���. </returns>
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

