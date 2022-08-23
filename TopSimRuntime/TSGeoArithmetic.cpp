#include "stdafx.h"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimUtil/TSOSMisc.h>
#include <TopSimTypes/TSOrderTypeMgr.h>
#else
#include <TopSimRuntime/TSOSMisc.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TSIDLUtils.h"
#include "temp/temp.h"
#include "temp/KeyWord.h"
#include "temp/topic.temp.h"



namespace TSIDLUtils{

TSString Table(size_t x)
{
    TSString t;
    for(size_t i = 0; i < x; ++i)
    {
        t += TSIDL_TABLE;
    }
    return t;
}

void ReplaceFullNameSpaceString( const TSString & replace_begin_namespace, 
    const TSString & replace_end_namespace, 
    TSString & replace_content, 
    TSTypeDefPtr TypeDef, 
    const TSString & WithFun,
    bool SupportSpace)
{
    TSString name_space_begin;
    TSString name_space_end;
	
	TSString suit_full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef, true);

	std::vector<TSString> Names = TSMetaType4Link::Split(suit_full_type_name, PACKAGE_SPLIT, false);

    if (!WithFun.empty() && TypeDef->Type == TSTypeDef::Service)
    {
        Names.push_back(WithFun);
    }
    if (!SupportSpace)
    {
        for (unsigned int i = 0 ; i < Names.size(); i++)
        {
            TSString Name = Names[i];
            name_space_begin += "namespace ";
            name_space_begin += Name;
            name_space_begin += "{";
            if (i != Names.size() - 1)
            {
                name_space_begin += TSIDL_NEWLINE;
            }
            name_space_end += "}\n";
        }
    }
    else
    {
        BOOST_FOREACH(const TSString & Name,Names)
        {
            name_space_begin += "namespace ";
            name_space_begin += Name;
            name_space_begin += "{";
            name_space_end += "}";
        }
    }
    boost::algorithm::replace_all(replace_content,replace_begin_namespace,name_space_begin + TSIDL_NEWLINE);
    boost::algorithm::replace_all(replace_content,replace_end_namespace,name_space_end + TSIDL_NEWLINE);
}

void ReplaceInitList( const TSString & replace_init_list, 
    TSString & replace_content, 
    TSTypeDefWithFiledPtr TypeDef )
{
    std::vector<TSString> InitListStrings;
    BOOST_FOREACH(TSFieldDefPtr Filed,TypeDef->Fileds)
    {
       if (Filed->DefaultVal.empty())
        {
            continue;
        }

        TSString InitListString = Filed->ValName;
        InitListString += "(";
        InitListString += Filed->DefaultVal;
        InitListString += ")";
        InitListStrings.push_back(InitListString);
    }

    if(InitListStrings.size())
    {
        TSString FromatString = TSString(":") + boost::algorithm::join(InitListStrings,",");
        boost::algorithm::replace_all(replace_content,replace_init_list,FromatString);
    }
    else
    {
        boost::algorithm::replace_all(replace_content,replace_init_list,"");
    }
}

void ReplaceTypeFullName( const TSString & replace_type_full_name, 
    TSString & replace_content, 
    TSTypeDefWithFiledPtr TypeDef )
{
    TSString suit_full_type_name;
	TSString parament = ": public SuperType";
    if (TypeDef->Extend.lock())
    {
        suit_full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
    }
     
   
    if(!suit_full_type_name.empty())
    {
        TSString TypeFullName = "typedef " + suit_full_type_name + "::DataType SuperType;";
        boost::algorithm::replace_all(replace_content,replace_type_full_name,TypeFullName);
        boost::algorithm::replace_all(replace_content,"__PARAENT__",parament);
    }
    else
    {
        TSString TypeFullName = "";
        if (TSTypeDef::Service == TypeDef->Type)
        {
            TypeFullName = "typedef TSBasicService::DataType SuperType;";
            boost::algorithm::replace_all(replace_content,"__PARAENT__",parament);
        }
        else if (TSTypeDef::Struct == TypeDef->Type)
        {
            TypeFullName = "";
            parament = ": public TSInterObject";
            boost::algorithm::replace_all(replace_content,"__PARAENT__",parament);
        }
        else
        {
            TypeFullName = "typedef TSBasicSdo::DataType SuperType;";
            boost::algorithm::replace_all(replace_content,"__PARAENT__",parament);
        }
        boost::algorithm::replace_all(replace_content,replace_type_full_name,TypeFullName);
    }
}

void ReplaceExportMacro( const TSString & replace_export_macro, 
    TSString & replace_content,TSTypeDefPtr TypeDef)
{
    TSString macro = "";
    if(HasOption("ExportMacro"))
    {
       macro = Option<TSString>("ExportMacro");
    }

    if (macro.empty())
    {
        TSString base = GetBaseName(TypeDef->Parse.lock()->GetFullFileName());
        std::transform(base.begin(),base.end(),base.begin(),::toupper);
        macro = base + "_DECL";
    }
    boost::algorithm::replace_all(macro,"-","_");
    boost::algorithm::replace_all(replace_content,replace_export_macro,macro);
}

void ReplaceFiledsAssign( const TSString & replace_fileds_assign,
    const TSString & OtherName,
    TSString & replace_content, 
    TSTypeDefWithFiledPtr TypeDef )
{
    TSString FiledAssign;
    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();

	
    if(TypeDef->Extend.lock() && replace_fileds_assign == __FIELD_ASSIGN__)
    {
        if(Cp == "c++")
        {
           FiledAssign += Table(2) +  "SuperType::operator=(other);\n";
        }
        else if(Cp == "c#")
        {
           FiledAssign += Table(2) +  "base::operator=(other);\n";
        }
      
    }

	if (TypeDef->Fileds.size() > 0)
	{
		BOOST_FOREACH(TSFieldDefPtr Filed, TypeDef->Fileds)
		{

			if (replace_fileds_assign == __FIELD_ASSIGN__)//赋值运算符重载字段
			{
				FiledAssign += Table(2) + Filed->ValName + " = " + OtherName + "." + Filed->ValName + ";\n";
			}
			else if (replace_fileds_assign == _EXTEND_)//继承字段
			{
				if (TypeDef->Extend.lock())//是否有继承
				{
					if (Cp == "c#")
					{
						FiledAssign = "\n" + Table(2) + ":base(other)";
					}
					else if (Cp == "c++")
					{
						FiledAssign = "\n" + Table(2) + ":SuperType(other)";
					}

				}
				else
				{
					FiledAssign = " ";
				}
			}
			else
			{
				FiledAssign += Table(2) + Filed->ValName + " = " + OtherName + "." + Filed->ValName + ";\n";
			}
		}
	}
	else
	{
		if (TypeDef->Extend.lock() && replace_fileds_assign == _EXTEND_)
		{
			if (Cp == "c#")
			{
				FiledAssign = "\n" + Table(2) + ":base(other)";
			}
			else if (Cp == "c++")
			{
				FiledAssign = "\n" + Table(2) + ":SuperType(other)";
			}
		}
	}
	

	/*if (TypeDef->Extend.lock() && replace_fileds_assign == _EXTEND_)
	{
		if (Cp == "c#")
		{
			FiledAssign = "\n" + Table(2) + ":base(other)";
		}
		else if (Cp == "c++")
		{
			FiledAssign = "\n" + Table(2) + ":SuperType(other)";
		}
	}*/
	
    boost::algorithm::replace_all(replace_content,replace_fileds_assign,FiledAssign);
}

void ReplaceFiledsEqual( const TSString & replace_fileds_equal, 
    const TSString & OtherName, 
    TSString & replace_content, 
    TSTypeDefWithFiledPtr TypeDef )
{
    TSString FiledEqual;
    if (!TypeDef->Extend.lock() && TypeDef->Type == TSTypeDef::Struct)
    {
        BOOST_FOREACH(TSFieldDefPtr Filed,TypeDef->Fileds)
        {
            FiledEqual += Table(2) + "if(!(" + Filed->ValName + " == " + OtherName + "." + Filed->ValName + "))\n";
            FiledEqual += Table(2) + "{\n";
            FiledEqual += Table(3) + "return false;\n";
            FiledEqual += Table(2) + "}\n" ;
        }
    }
    else
    {
		FiledEqual += Table(2) + "if (!SuperType::operator==(other)) \n";
		FiledEqual += Table(2) + "{ \n";
		FiledEqual += Table(3) + "return false; \n";
		FiledEqual += Table(2) + "} \n";

        FiledEqual += Table(2) + "else \n";
        FiledEqual += Table(2) + "{ \n";
        BOOST_FOREACH(TSFieldDefPtr Filed,TypeDef->Fileds)
        {
            FiledEqual += Table(3) + "if(!(" + Filed->ValName + " == " + OtherName + "." + Filed->ValName + ")) \n";
            FiledEqual += Table(3) + "{ \n";
            FiledEqual += Table(4) + "return false; \n";
            FiledEqual += Table(3) + "} \n" ;
        }
        FiledEqual += Table(2) + "} \n";
    }

    boost::algorithm::replace_all(replace_content,replace_fileds_equal,FiledEqual);
}

void ReplaceFiledsLessCompare( const TSString & replace_fileds_less_compare, 
    const TSString & OtherName, 
    TSString & replace_content, 
    TSTypeDefWithFiledPtr TypeDef )
{
    TSString FiledLess;
    if (TypeDef->Type == TSTypeDef::Struct && !TypeDef->Extend.lock())
    {
        BOOST_FOREACH(TSFieldDefPtr Filed,TypeDef->Fileds)
        {
			FiledLess += (Filed == *(TypeDef->Fileds.begin()) ? "" : Table(2)) + "if(!(" + Filed->ValName + " < " + OtherName + "." + Filed->ValName + ")) \n";
			FiledLess += Table(2) + "{ \n";
			FiledLess += Table(3) + "return false; \n";
			FiledLess += Table(2) + "} \n";

            //FiledLess += "if(!(" + Filed->ValName + " < " + OtherName + "." + Filed->ValName + ")) " + "return false;\n" + Table(2);
            //FiledLess += "else return false;\n" + Table(2);
        }
    }
    else
    {
		FiledLess += "if (!SuperType::operator<(other)) \n";
		FiledLess += Table(2) + "{ \n";
		FiledLess += Table(3) + "return false; \n";
		FiledLess += Table(2) + "} \n";

        FiledLess += Table(2) + "else \n";
		FiledLess += Table(2) + "{ \n";
        BOOST_FOREACH(TSFieldDefPtr Filed,TypeDef->Fileds)
        {
			FiledLess += Table(3) + "if(!(" + Filed->ValName + " < " + OtherName + "." + Filed->ValName + ")) \n";
			FiledLess += Table(3) + "{ \n";
			FiledLess += Table(4) + "return false; \n";
			FiledLess += Table(3) + "} \n";
            //FiledLess += "if(!(" + Filed->ValName + " < " + OtherName + "." + Filed->ValName + ")) " + "return false;\n" + Table(2);
            //FiledLess += "else return false;\n" + Table(2);
        }
		FiledLess += Table(2) + "} \n";
    }
    FiledLess += Table(2) + "return true; \n";
    boost::algorithm::replace_all(replace_content,replace_fileds_less_compare,FiledLess);
}

void ReplaceFiledsDeclare( const TSString & replace_fileds_declare, 
    TSString & replace_content, 
    TSTypeDefWithFiledPtr TypeDef )
{
    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();

    TSString FiledDeclare = "public:" + TSIDL_NEWLINE;
    if (Cp == "c#")
    {
        FiledDeclare = "";
    }
    BOOST_FOREACH(TSFieldDefPtr Filed,TypeDef->Fileds)
    {
        if (Cp == "c#")
        {
			if (!Filed->Desc.empty())
			{
				FiledDeclare += Table(2) + "[XmlElement(ElementName=\"" + Filed->Desc + "\")]\n";
			}
            FiledDeclare += Table(2) + "public "; 
        }
        else if (Cp == "c++")
        {
            FiledDeclare += Table();
        }
        
        if(Filed->Def->Type != TSTypeDef::Builtin &&
            Filed->Def->Type != TSTypeDef::Enum &&
            Filed->Def->Type != TSTypeDef::Array &&
            Filed->Def->Type != TSTypeDef::Map &&
            Filed->Def->Type != TSTypeDef::TypeDefine)
        {
            FiledDeclare += TSBuiltinPlugin::GetSuitFullTypeName(Filed->Def,true) + "::DataType";
        }
        else if (Filed->Def->Type == TSTypeDef::Array)
        {
            TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(Filed->Def,TSVectorBuiltinTypeDefPtr);
            if (vectorField)
            {
                if (Cp == "c#")
                {
                    if (vectorField->Value->Type == TSTypeDef::Builtin)
                    {
                        TSString fieldType = TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value);
                        if (fieldType == "TSVector2d" || fieldType == "TSVector3d" || fieldType == "TSVector2f" || fieldType == "TSVector3f"
                            || fieldType == "TSHANDLE" || fieldType == "handle" || fieldType == "TSModelTmplOrClassId" 
                            || fieldType == "TSTOPICHANDLE")
                        {
                            FiledDeclare += "List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">";
                        }
                        else
                        {
                            FiledDeclare += TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + "[]";
                        }
                       
                    }
                    else
                    {
                         FiledDeclare += "List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">";
                    }              
                }
                else if (Cp == "c++")
                {
                    FiledDeclare += "std::vector<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">";
                }
            }
           
         }
        else if (Filed->Def->Type == TSTypeDef::Builtin ||
            Filed->Def->Type == TSTypeDef::Enum||
             Filed->Def->Type == TSTypeDef::TypeDefine)
        {
            FiledDeclare += TSBuiltinPlugin::GetSuitFullTypeName(Filed->Def);
        }
        else if (Filed->Def->Type == TSTypeDef::Map)
        {
             TSMapBuiltinTypeDefPtr mapField = TS_CAST(Filed->Def,TSMapBuiltinTypeDefPtr);
             if(Cp == "c++")
             {
                 FiledDeclare += "std::map<" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + "," 
                     + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) +  ">";
             }
             else
             {
                 FiledDeclare += "Dictionary<" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + "," 
                 + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) +  ">";
             }
        }
     
        FiledDeclare += TSIDL_BLANK + Filed->ValName + ";\n";
    }

    boost::algorithm::replace_all(replace_content,replace_fileds_declare,FiledDeclare);
}

void GenerateEnum( TSProjFilePtr ProjFile,TSEnumDefPtr TypeDef)
{
    TSString code = 
        "__NAMESPACE_BEGIN__" + TSIDL_NEWLINE + 
        "__ENUM_DEF__" + TSIDL_NEWLINE + 
        "__NAMESPACE_END__" + TSIDL_NEWLINE;

    ReplaceFullNameSpaceString(__NAMESPACE_BEGIN__,
        __NAMESPACE_END__,
        code,
        TypeDef);

    TSString enumStr;

    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
    if(Cp == "c++"
        || Cp == "cplusplus")
    {
        enumStr += "enum ";
    }
    else if(Cp == "c#"
        || Cp == "csharp")
    {
        enumStr += "public enum ";
    }
    
    else
    {
        enumStr += "enum ";
    }

    enumStr += TypeDef->TypeName;

    enumStr += TSIDL_NEWLINE + "{" + TSIDL_NEWLINE;
    for(size_t i = 0; i < TypeDef->Fileds.size(); ++i)
    {
        if(TSEnumFiledDefPtr enumVal = TypeDef->Fileds[i])
        {
            if(i != 0)
            {
                enumStr += TSIDL_NEWLINE;
            }
            enumStr += TSIDL_BLANK + enumVal->ValName;
            /*if(!enumVal->DefaultVal.empty())
            {
                enumStr += " = ";
                enumStr += enumVal->DefaultVal;
            }*/
            enumStr += ",";
        }
    }
    enumStr += TSIDL_NEWLINE + "};" + TSIDL_2NEWLINE;

    boost::algorithm::replace_all(code, "__ENUM_DEF__", enumStr);

    ProjFile->AppendLast(code);
}

TSString GenHeadUUID( const TSString &FileNme = "")
{
    TSString upperName = FileNme + "_" + TSOSMisc::GenerateUUID();

    std::transform(upperName.begin(),upperName.end(),upperName.begin(),::toupper);

    if(upperName.find_last_of('.') != TSString::npos)
    {
        upperName.replace(upperName.find_last_of('.'),1,"_");   
    }

    boost::algorithm::replace_all(upperName,"-","_");

    TSString condition = "__" + upperName + "__";

    TSString frontCondition;
    frontCondition += "#ifndef" + TSIDL_BLANK + condition + TSIDL_NEWLINE;
    frontCondition += "#define" + TSIDL_BLANK + condition + TSIDL_2NEWLINE;

    return frontCondition;
}

TSString GetTopicName( TSTypeDefPtr TypeDef )
{
    TSString result;
    if (TSTopicTypeDefPtr topicDef = TS_CAST(TypeDef,TSTopicTypeDefPtr))
    {
        result += topicDef->TypeName;
    }
    else
    {
        if (TypeDef->Type != TSTypeDef::Service)
        {
            result = TSIDL_TOPIC_PREFIX;
        }
		else 
		{
			result = TSIDL_SERVER_PREFIX;
		}
        result += TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
        boost::algorithm::replace_all(result,PACKAGE_SPLIT,"_");
    }
    return result;
}

bool WriteFile( const TSString &FileName, const TSString &Content, TSString Folder )
{
    TSString SavePath;
    if (Folder.empty())
    {
        SavePath = boost::filesystem::current_path().string();
    }
    else
    {
        boost::filesystem::path p(Folder);
        if(!p.is_absolute())
        {
            p = boost::filesystem::current_path()/Folder;
            if(!p.is_absolute())
            {
                return false;
            }
            SavePath = p.string();
        }
        else
        {
            SavePath = Folder;
        }
    }

    if(!boost::filesystem::exists(SavePath))
    {
        boost::system::error_code err;
        boost::filesystem::create_directories(SavePath,err);
        if(err)
        {
            return false;
        }
    }

    TSString saveFile = SavePath + "/" + FileName;

    boost::filesystem::path parentpath(saveFile);
    TSString parentPath = parentpath.parent_path().string();
    if(!boost::filesystem::exists(parentPath))
    {
        boost::system::error_code err;
        boost::filesystem::create_directories(parentPath,err);
        if(err)
        {
            return false;
        }
    }
    
    std::ofstream ofs(saveFile.c_str(),std::ofstream::out);
    if(!ofs.is_open())
    {
        return false;
    }

    ofs.write(Content.c_str(), Content.length());
    ofs.close();

    return true;
}

TSString GetFileRelativePath( TSTypeDefPtr TypeDef )
{
    TSString result;
    TSString suit_full_type_name = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);

    std::vector<TSString> Names = TSMetaType4Link::Split(suit_full_type_name,PACKAGE_SPLIT,false);
    result = boost::algorithm::join(Names,PACKAGE_SPLIT_4CS);
    return result;
}

TSString Link2Type( TSString Link )
{
    TSString TypeName;
    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
#define TSIDL_TD(TSIDLTYPE,CPPTYPE,CSHARPTYPE,JAVATYPE) \
    if(Link == TSIDLTYPE) \
    { \
        if(Cp == "c++") \
        { \
            TypeName = CPPTYPE; \
        } \
        else if (Cp == "c#") \
        { \
            TypeName = CSHARPTYPE; \
        } \
         else if (Cp == "java") \
        { \
            TypeName = JAVATYPE; \
        } \
    }
    TSIDL_GEN_TYPES(TSIDL_TD);
#undef TSIDL_TD

    return TypeName;
}

TSString GetFunctionStr( TSFunctionDefPtr Func)
{
    TSString code;
    TSString TypeName;
    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
    bool Init = true;

    TSString TypeNameFunc;
    if(Func->Def->Type == TSTypeDef::Array)
    {
        TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(Func->Def,TSVectorBuiltinTypeDefPtr);

        if (vectorField->Value->Type != TSTypeDef::Builtin)
        {
            TypeNameFunc = "List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">";
        }
        else
        {
			//如果是TSHANDLE类型或者是TSVector3d类型，创建List
			if (vectorField->ValueTypeName == "TSHANDLE" || vectorField->ValueTypeName == "TSVector3d")
			{
				TypeNameFunc = "List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">";
			}
			else
			{
				TypeNameFunc = TSBuiltinPlugin::GetSuitFullTypeName(Func->Def);
			}
        }
    }
    else
    {
        TypeNameFunc = TSBuiltinPlugin::GetSuitFullTypeName(Func->Def);
    }

    code += TypeNameFunc + TSIDL_BLANK + Func->ValName + "(";

    for (unsigned int i = 0; i < Func->Params.size(); i++)
    {
        TSParamDefPtr Param = Func->Params[i];
        if (Param->Def->Type != TSTypeDef::Builtin ||
            Param->Def->Type != TSTypeDef::Enum)
        {
            if (Cp == "c#")
            {
                if(Param->Def->Type == TSTypeDef::Array)
                {
                    TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(Param->Def,TSVectorBuiltinTypeDefPtr);
                    if (vectorField->Value->Type != TSTypeDef::Builtin)
                    {
                        TypeName = "List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">";
                    }
                    else
                    {
						//如果是TSHANDLE类型或者是TSVector3d类型，创建List
						if (vectorField->ValueTypeName == "TSHANDLE" || vectorField->ValueTypeName == "TSVector3d")
						{
							TypeName = "List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">";
						}
						else
						{
							TypeName = TSBuiltinPlugin::GetSuitFullTypeName(Param->Def);
						}
                    }
                }
                else
                {
                    TypeName = TSBuiltinPlugin::GetSuitFullTypeName(Param->Def);
                }
            }
        }
        else
        {
            TypeName = Link2Type(Param->DefTypeName);
        }
        if (Cp == "c#")
        {
            if (Param->Type == TSParamDef::Inout)
            {
                code += "ref ";
            }
            else if (Param->Type == TSParamDef::Out)
            {
                code += "out ";
            }
        }
        if(Cp == "java")
        {
            TSString TypeName = TSIBuiltinPlugin::GetSuitFullTypeName(Param->Def);

            if (Param->Type == TSParamDef::In)
            {  
                code += TypeName + TSIDL_BLANK + Param->ValName;
                
                if (Func->Params.size() > (i + 3))
                {
                    code += ",";
                }
                else if (Func->Params.size()<3)
                {
                    code += ",";
                }
            }
            else if(Init)
            {    
                TSString filenameReponse = TSIBuiltinPlugin::GetSuitFullTypeName(Func->Def,true)+ "_" + Func->ValName + "_" + Param->ValName;
                code += filenameReponse + " " + Param->ValName;
                Init = false;
                if (Func->Params.size() != i + 1)
                {
                    code += ",";
                }
            }
        }
        else
        {
            code += TypeName + TSIDL_BLANK + Param->ValName;
            if (Func->Params.size() != i + 1)
            {
                code += ",";
            }
        }
    
       
    }

    code += ")";
    return code;
}

TSString GetFieldGetFuncStr( TSFieldDefPtr Field )
{
    TSString code;
    if (Field)
    {
         code += TSBuiltinPlugin::GetSuitFullTypeName(Field->Def) + " Get" + Field->ValName + "()";
         boost::algorithm::replace_all(code,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);
    }
    return code;
}

TSString GetFieldSetFuncStr( TSFieldDefPtr Field )
{
    TSString code;
    if (Field)
    {
        code += "void Set" + Field->ValName + "(" + TSBuiltinPlugin::GetSuitFullTypeName(Field->Def) +
            TSIDL_BLANK + Field->ValName + ")";
    }
    return code;
}

TSString GetFuncWithoutResultStr(TSFunctionDefPtr Func, bool IsService)
{
    TSString code;
    TSString TypeName;
    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
    code += Func->ValName + "(";
    for (unsigned int i = 0; i < Func->Params.size(); i++)
    {
        TSParamDefPtr Param = Func->Params[i];
        if (Param->Def->Type != TSTypeDef::Builtin ||
            Param->Def->Type != TSTypeDef::Enum)
        {
            if (Cp == "c#")
            {
                if(Param->Def->Type == TSTypeDef::Array)
                {
                    TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(Param->Def,TSVectorBuiltinTypeDefPtr);
                    if (vectorField->Value->Type != TSTypeDef::Builtin)
                    {
                        TypeName = "List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">";
                    }
                    else
                    {
                        TypeName = TSBuiltinPlugin::GetSuitFullTypeName(Param->Def);
                    }
                }
                else
                {
                    TypeName = TSBuiltinPlugin::GetSuitFullTypeName(Param->Def);
                }
           }
           else
           {
               TypeName = TSBuiltinPlugin::GetSuitFullTypeName(Param->Def);
           }
        }
        else
        {
            TypeName = Link2Type(Param->DefTypeName);
        }
        if (Cp == "c#")
        {
            if (Param->Type == TSParamDef::Inout)
            {
                code += "ref ";
            }
            else if (Param->Type == TSParamDef::Out)
            {
                code += "out ";
            }

            code += TypeName + TSIDL_BLANK + Param->ValName;
        }
        else if (Cp == "c++")
        {
            if (Param->Type == TSParamDef::Inout || Param->Type == TSParamDef::Out)
            {
                code += TypeName + TSIDL_BLANK + "& " + Param->ValName;
            }
            else
            {
                code += "const " + TypeName + TSIDL_BLANK + "& " + Param->ValName;
            }    
        }
        if (Func->Params.size() != i + 1)
        {
            code += ",";
        }
    }
    if (IsService)
    {
        if (Cp == "c++")
        {
            code += "";
        }      
    }
    code += ")";
    return code;
}

TSString GetFuncAsyncCallBackStr( TSFunctionDefPtr func,bool IsService)
{
    TSString code;
    if (func)
    {
        code = "boost::function<void(const TSAsyncCall::TSAsyncError&";
        if (IsService)
        {
            code = "boost::function<void(";
        }
        if (func->DefTypeName != "void" && !IsService)
        {
            code += "," + TSBuiltinPlugin::GetSuitFullTypeName(func->Def);
        }
        if(func->DefTypeName != "void" && IsService)
        {
            code += TSBuiltinPlugin::GetSuitFullTypeName(func->Def);
        }
        for(size_t i = 0; i < func->Params.size(); ++i)
        {
            if (TSParamDefPtr param = func->Params[i])
            {
                if (param->Type == TSParamDef::Out || param->Type == TSParamDef::Inout)
                {   
                    if (func->DefTypeName == "void")
                    {
                        code += "const " + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + "&";
                        if (i != func->Params.size() - 1)
                        {
                            code += ",";
                        }
                    }
                    else
                    {
                         code += ",";
                         code += "const " + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + "&";
                    } 
                }
            }
        }
        code += ")>";
    }
	 boost::algorithm::replace_all(code,",)",")");
    return code;
}

TSString GetFuncAsyncStr( TSFunctionDefPtr func, const TSString & Namespace,bool IsService,bool IsHead)
{
    TSString result;
    if (func)
    {
		if(IsHead)
		{
			Namespace.empty()
				? result += "void " + func->ValName + "Async("
				: result += "void " + func->ValName + "Async(";
		}
		else
		{
			Namespace.empty()
				? result += "void " + Namespace + "::" + func->ValName + "Async("
				: result += "void " + Namespace + "::" + func->ValName + "Async(";
		}
        
        for(size_t i = 0; i < func->Params.size() ;++i)
        {
            if(TSParamDefPtr param = func->Params[i])
            {
                if(i != 0)
                {
                    result += ",";
                }

                result += "const " + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + " &" + param->ValName;
            }
        }
        if(func->Params.size())
        {
            result += ",";
        }

        result += GetFuncAsyncCallBackStr(func,IsService) + " func";
        
        if (IsHead)
        {
            result += ")";
        }
        else
        {
            if(IsService)
            {   
                 result += ")";
            }
            else
            {
                Namespace.empty()
                    ? result += ",UINT32 Timeout = -1)"
                    : result += ",UINT32 Timeout)";
            }
          
        }
    }
    return result;
}

TSString GetFuncAsyncHandlerStr( TSFunctionDefPtr func,const TSString & Namespace,bool IsService )
{
    TSString code;
    if(func)
    {
        code += "void ";
        if(Namespace.empty())
        {
            code += func->ValName;
        }
        else
        {
            code += Namespace + "::" + func->ValName;
        }
        if (IsService)
        {
            code += "AsyncHandler(TSBasicService::DataTypePtr Data,void * Invoker)";
        }
        else
        {
            code += "AsyncHandler(TSRpcResponse::DataTypePtr Response,void * Invoker,const TSAsyncCall::TSAsyncError& Error)";
        }
        
    }

    return code;
}

TSString GetFuncHandlerStr( TSFunctionDefPtr func,const TSString & Namespace )
{
    TSString code;
    if(func)
    {
        code += "TSRpcResponse::DataTypePtr ";
        if(Namespace.empty())
        {
            code += func->ValName;
        }
        else
        {
            code += Namespace + "::" + func->ValName;
        }

        code += "Handler(TSRpcRequest::DataTypePtr Request)";
    }

    return code;
}

TSString GetFuncCallStr( TSFunctionDefPtr func )
{
    TSString code;
    if (func)
    {
        code += func->ValName;
        code += "(";

        for(size_t i = 0; i < func->Params.size() ;++i)
        {
            if(TSParamDefPtr param = func->Params[i])
            {
                if(i != 0)
                {
                    code += ",";
                }
                code += param->ValName;
            }
        }
        code += ")";
    }
    return code;
}

TSString GetOutParamCode( TSTypeDefPtr TypeDef , TSString ParamName , TSTypeDefPtr ParamType )
{
    TSString code;
    if (TypeDef->Type == TSTypeDef::Builtin)
    {
        code += Table() + "{\n";
        code += Table(2) + "Item.Arg = TSVariant::FromValue(" + ParamName + ");\n";
        code += Table(2) + "OutputArchive.Archive(Ser,Item);\n";
        code += Table() + "}\n";
    }
    else if (TypeDef->Type == TSTypeDef::Array)
    {
        code += GetOutParamVecCode(TypeDef , ParamName , ParamType);
    }
    return code;
}

TSString GetOutParamVecCode( TSTypeDefPtr Type ,TSString ParamName ,TSTypeDefPtr ParamType )
{
    TSString code;
    TSVectorBuiltinTypeDefPtr vectorType = TS_CAST(Type,TSVectorBuiltinTypeDefPtr);
    if (vectorType)
    {
        if (vectorType->Value->Type == TSTypeDef::Builtin)
        {
            code += Table(2) + "Item.Arg = TSVariant::FromValue(" + ParamName + ");\n";
            code += Table(2) + "OutputArchive.Archive(Ser,Item);\n";
        }
        else
        {
            code += GetOutParamCode(vectorType->Value , ParamName,ParamType);
        }

    }

    return code;
}

TSString ReplaceExportFileName(TSString & FileContent)
{
    TSString fileName = "";
    if (HasOption("ExportFile"))
    {
        TSString fileFullName = Option<TSString>("ExportFile");
        TSString::size_type separatorPos1 = fileFullName.find_last_of("/");
        TSString::size_type separatorPos2 = fileFullName.find_last_of("\\");
        TSString::size_type separatorPos = 0;
        if(separatorPos1 != TSString::npos)
        {
            separatorPos = separatorPos1;
        }
        else if(separatorPos2 != TSString::npos)
        {
            if(separatorPos < separatorPos2)
            {
                separatorPos = separatorPos2;
            }
        }
        TSString::size_type startPose;
        if (!separatorPos)
        {
            startPose = 0;
        }
        else
        {
            startPose = separatorPos+1;
        }
        fileName = fileFullName.substr(startPose,fileFullName.length() - startPose);

        if(!boost::filesystem::exists(fileFullName))
        {
            throw TSException(fileFullName + GB18030ToTSString("文件不存在"));
        }

        std::ifstream ifs(fileFullName.c_str(),std::ifstream::in);
        if(!ifs.is_open())
        {
            return "";
        }
        std::ostringstream oss;
        oss << ifs.rdbuf();
        FileContent = oss.str();
        ifs.close();
    }
    if (fileName.empty())
    {
        fileName = "Export.h";
    }
    return fileName;
}

void ReplaceExportHeader( const TSString & replace_export_file, TSString & replace_content,const TSString & replace_dir_name)
{
    TSString fileName = "";
    if (HasOption("ExportFile"))
    {
        TSString fileop = Option<TSString>("ExportFile");
        if (!fileop.empty())
        {
            TSString::size_type separatorPos1 = fileop.find_last_of("/");
            TSString::size_type separatorPos2 = fileop.find_last_of("\\");
            TSString::size_type separatorPos = 0;
            if(separatorPos1 != TSString::npos)
            {
                separatorPos = separatorPos1;
            }
            else if(separatorPos2 != TSString::npos)
            {
                if(separatorPos < separatorPos2)
                {
                    separatorPos = separatorPos2;
                }
            }
            TSString::size_type startPose;

            if (!separatorPos)
            {
                startPose = 0;
            }
            else
            {
                startPose = separatorPos+1;
         
            }
            TSString filedshortName = fileop.substr(startPose,fileop.length() - startPose);
            fileName += "<" + replace_dir_name + "_xidl/cpp/" + filedshortName + ">"; 
        }
    }
    if (fileName.empty())
    {
		bool     IsXidl   = TSIDLPluginMgr::Instance()->GetFileNameWithXidl();
		if (IsXidl)
		{
			fileName = "<" + replace_dir_name + "_xidl/cpp/Export.h>";
		}
		else
		{
			fileName = "<" + replace_dir_name + "/cpp/Export.h>";
		}
    }
    boost::algorithm::replace_all(replace_content,replace_export_file,fileName);
}

bool Writexidl(const TSString & FileName,const TSString & WritePath )
{
    TSString SavePath;
    if (WritePath.empty())
    {
        SavePath = boost::filesystem::current_path().string();
    }
    else
    {
        boost::filesystem::path p(WritePath);
        if(!p.is_absolute())
        {
            p = boost::filesystem::current_path()/WritePath;
            if(!p.is_absolute())
            {
                return false;
            }
            SavePath = p.string();
        }
        else
        {
            SavePath = WritePath;
        }
    }

    if(!boost::filesystem::exists(SavePath))
    {
        boost::system::error_code err;
        boost::filesystem::create_directories(SavePath,err);
        if(err)
        {
            return false;
        }
    }
    TSString saveFile = SavePath + "/" + FileName + ".xidl";

    boost::filesystem::path parentpath(saveFile);
    TSString parentPath = parentpath.parent_path().string();
    if(!boost::filesystem::exists(parentPath))
    {
        boost::system::error_code err;
        boost::filesystem::create_directories(parentPath,err);
        if(err)
        {
            return false;
        }
    }
    TSString filePath = TSIDLPluginMgr::Instance()->GetFullIdlPath(FileName + ".xidl");
    std::ifstream ifs(filePath.c_str(),std::ifstream::in);
    if (!ifs.is_open())
    {
        std::cout << filePath << GB18030ToTSString("文件打开失败:") <<std::endl;
        return false; 
    }
    else
    {
        std::ostringstream oss;
        oss << ifs.rdbuf();
        TSString FileContent = oss.str();
        std::ofstream ofs(saveFile.c_str(),std::ofstream::out);
        if(!ofs.is_open())
        {
            return false;
        }
        ofs.write(FileContent.c_str(),FileContent.length());
        ofs.close();
        return true;
    }
}

void ReplaceLinkerVersion(TSString & contentconst,const TSString & replace_export_name)
{
    TSString version;
    if (HasOption(VerSionKey))
    {
        version = Option<TSString>(VerSionKey);
    }
    else
    {
        version = "XSIM3_2";
    }
    boost::algorithm::replace_all(contentconst,replace_export_name,version);
}

bool KeyWordSearch( const TSString & keyWord )
{
    for (size_t i = 0; i < (sizeof(_KeyWord)/sizeof(_KeyWord[0]));i++)
    {
        if (_KeyWord[i] == keyWord)
        {
            return false;
        }
    }

    return true;
}

template<class T>
void ReplaceBufferWithService(TSString & custon ,TSString & writeBuffer, TSString & readBuffer,T param)
{ 
    TSString paramValue; 
    TS_CAST(param,TSParamDefPtr)? paramValue = param->ValName : paramValue = "rnt";

    if (param->Def->Type == TSTypeDef::Array)
    {
        TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(param->Def,TSVectorBuiltinTypeDefPtr);
        if (vectorField->Value->Type != TSTypeDef::Builtin)
        {
            custon += Table(3) + paramValue +  " = new List<";      
            custon += TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">();" + TSIDL_NEWLINE;
            writeBuffer += Table(3) + "buffer.WriteInt32(" + paramValue + ".Count);" + TSIDL_NEWLINE;
            writeBuffer += Table(3) + "foreach (var item in " + paramValue + ")" + TSIDL_NEWLINE;
            writeBuffer += Table(3) + "{" + TSIDL_NEWLINE;
            if (vectorField->Value->Type == TSTypeDef::Enum)
            {
                writeBuffer += Table(4) + "buffer.WriteInt32((int)item);" + TSIDL_NEWLINE;
            }
            else
            {
                writeBuffer += Table(4) + "item.Serialize(buffer,MarshaledFlag);" + TSIDL_NEWLINE;
            }

            writeBuffer += Table(3) + "}" + TSIDL_NEWLINE;
            readBuffer += Table(3) + "uint " + paramValue + "Capacity = buffer.ReadUInt32();" + TSIDL_NEWLINE;
            readBuffer += Table(3) + "for(uint i = 0; i < " + paramValue + "Capacity; i++)" + TSIDL_NEWLINE;
            readBuffer += Table(3) + "{" + TSIDL_NEWLINE;
            if (vectorField->Value->Type == TSTypeDef::Enum)
            {
                readBuffer += Table(4) + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + " topic = (" +
                    TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ")buffer.ReadInt32();\n";
            }
            else
            {
                readBuffer += Table(4) + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + " topic = " + 
                    "new " + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + "();\n"; 
                readBuffer += Table(4) + "topic.Deserialize(buffer);\n";
            }

            readBuffer += Table(4) + paramValue + ".Add(topic);\n";
            readBuffer += Table(3) + "}" + TSIDL_NEWLINE;
        }
        else
        {   
            TSString fiedType = TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value);
            if (fiedType == "TSVector2d" || fiedType == "TSVector3d" || fiedType == "TSVector2f" || fiedType == "TSVector3f"
                || fiedType == "TSHANDLE" || fiedType == "handle" || fiedType == "TSModelTmplOrClassId" || fiedType == "TSTOPICHANDLE")
            {
                custon += Table(3) + paramValue +  " = new List<";      
                custon += TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">();" + TSIDL_NEWLINE;
                writeBuffer += Table(3) + "buffer.WriteInt32(" + paramValue + ".Count);" + TSIDL_NEWLINE;
                writeBuffer += Table(3) + "foreach (" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + " item in " + 
                    paramValue + ")" + TSIDL_NEWLINE;
                writeBuffer += Table(3) + "{" + TSIDL_NEWLINE;
                writeBuffer += Table(4) + "buffer.Write" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + "(item);" + TSIDL_NEWLINE;
                writeBuffer += Table(3) + "}" + TSIDL_NEWLINE;
                readBuffer += Table(3) +paramValue + ".Capacity = buffer.ReadInt32();" + TSIDL_NEWLINE;
                readBuffer += Table(3) + "for(uint i = 0; i < " + paramValue + ".Capacity; i++)" + TSIDL_NEWLINE;
                readBuffer += Table(3) + "{" + TSIDL_NEWLINE;
                readBuffer += Table(4) + paramValue + ".Add(buffer.Read" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + 
                    "());" + TSIDL_NEWLINE;
                readBuffer += Table(3) + "}" + TSIDL_NEWLINE;

            }
            else
            {
                custon += Table(3) + paramValue +  " = new " + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + "[1];\n";      

                writeBuffer += Table(3) + "buffer.WriteArray(" + paramValue +");\n";

                if (fiedType == "sbyte")
                {
                    readBuffer += Table(3) + "this." + paramValue + " = buffer.ReadSByteForArray();\n";
                }
                else if (fiedType == "byte")
                {
                    readBuffer += Table(3) + "this." + paramValue + " = buffer.ReadByteForArray();\n";
                }
                else
                {
                    readBuffer += Table(3) + "this." + paramValue + " = buffer.Read" +  fiedType +"ForArray();\n";
                }                 
            }
        }                             
    }
    else if (param->Def->Type == TSTypeDef::Map)
    {
        custon += Table(3) + paramValue +  " = new " + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + "();\n";
        TSMapBuiltinTypeDefPtr mapField = TS_CAST(param->Def,TSMapBuiltinTypeDefPtr);
        writeBuffer += Table(3) + "int count = " + paramValue + ".Count;\n";
        writeBuffer += Table(3) + "buffer.WriteInt32(count);" + TSIDL_NEWLINE;
        writeBuffer += Table(3) + "foreach (KeyValuePair<" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + 
            ", " + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + "> kvp in " + 
            paramValue + ")" + TSIDL_NEWLINE;
        writeBuffer += Table(3) + "{\n";
        writeBuffer += Table(4) + "buffer.Write" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + "(kvp.Key);" + TSIDL_NEWLINE;
        writeBuffer += Table(4) + "buffer.Write" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + "(kvp.Value);" + TSIDL_NEWLINE;
        writeBuffer += Table(3) + "}\n";

        readBuffer += Table(3) + "int count = buffer.ReadInt32();" + TSIDL_NEWLINE;
        readBuffer += Table(3) + "for (int i = 0; i < count; i++)\n";
        readBuffer += Table(3) + "{\n";
        readBuffer += Table(4) + paramValue + ".Add(buffer.Read" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Key) + 
            "(), buffer.Read" + TSBuiltinPlugin::GetSuitFullTypeName(mapField->Value) + "());" + TSIDL_NEWLINE;
        readBuffer += Table(3) + "}\n";
    }
    else if (param->Def->Type !=  TSTypeDef::Builtin &&
        param->Def->Type != TSTypeDef::Enum )
    {
        custon += Table(3) + paramValue + " = new " + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + "();\n";
        writeBuffer += Table(3) + paramValue + ".Serialize(buffer,MarshaledFlag);" + TSIDL_NEWLINE;
        readBuffer += Table(3) + paramValue + ".Deserialize(buffer);" + TSIDL_NEWLINE;
    }
    else if (param->Def->Type == TSTypeDef::Enum)
    {

        writeBuffer += Table(3) + "buffer.WriteINT32((int)" + paramValue + ");" + TSIDL_NEWLINE;

        readBuffer += Table(3) + "this." + paramValue + " = (" + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + 
            ")buffer.ReadINT32();" + TSIDL_NEWLINE;
    }
    else
    {

        writeBuffer += Table(3) + "buffer.Write" + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + "(" + paramValue + ");" + TSIDL_NEWLINE;
        readBuffer += Table(3) + "this." + paramValue + " = buffer.Read" + 
            TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + "();" + TSIDL_NEWLINE;
    }
}

void ReplaceFieldDeclareWithService( const TSString & replace_fileds_declare, TSString & replace_content, TSFunctionDefPtr Func,const bool & Requst)
{
    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
    if (Cp == "c#")
    {
        TSString defineInAttribute,defineOutAttribute,parmsInit,paramsOutInit,custon,writeBuffer,readBuffer;
        BOOST_FOREACH(TSParamDefPtr param,Func->Params)
        {
            if (param->Type != TSParamDef::Out)
            {
                parmsInit += Table(2) + param->ValName + " = other." + param->ValName +  ";" + TSIDL_NEWLINE;
                if(param->Def->Type == TSTypeDef::Array)
                {
                    TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(param->Def,TSVectorBuiltinTypeDefPtr);
                    defineInAttribute += Table() + "public " + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + TSIDL_BLANK + 
						param->ValName + ";" + TSIDL_NEWLINE;
                }
                else
                {
                    defineInAttribute += Table() + "public " + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + TSIDL_BLANK + 
                        param->ValName + ";" + TSIDL_NEWLINE;
                }
               
                if (Requst)
                {
                    ReplaceBufferWithService<TSParamDefPtr>(custon,writeBuffer,readBuffer,param);
                }          
            }

           if(param->Type != TSParamDef::In)
           {
               paramsOutInit += Table(2) + param->ValName + " = other." + param->ValName +  ";" + TSIDL_NEWLINE;
               if(param->Def->Type == TSTypeDef::Array)
               {
                   TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(param->Def,TSVectorBuiltinTypeDefPtr);
   
                   defineOutAttribute += Table() + "public " + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + TSIDL_BLANK + 
						param->ValName + ";" + TSIDL_NEWLINE;

               }
               else
               {
                   defineOutAttribute += Table(2) + "public " + TSBuiltinPlugin::GetSuitFullTypeName(param->Def) + TSIDL_BLANK + 
                       param->ValName + ";" + TSIDL_NEWLINE;
               }

               if (!Requst)
               {
                   ReplaceBufferWithService<TSParamDefPtr>(custon,writeBuffer,readBuffer,param);
               }
           }        
       }

       if (Func->DefTypeName != "void" && 
           Func->DefTypeName != "Void")
       {
           if(Func->Def->Type == TSTypeDef::Array)
           {
               TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(Func->Def,TSVectorBuiltinTypeDefPtr);
            
               defineOutAttribute += Table() + "public " + TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + TSIDL_BLANK + 
				"rnt;" + TSIDL_NEWLINE;
         
           }
           else
           {
                  defineOutAttribute += Table(2) + "public " + TSBuiltinPlugin::GetSuitFullTypeName(Func->Def) + TSIDL_BLANK + "rnt;\n";
           }
          
           paramsOutInit += Table(2) + "rnt = other.rnt;" + TSIDL_NEWLINE;
           if (!Requst)
           {
               ReplaceBufferWithService<TSFunctionDefPtr>(custon,writeBuffer,readBuffer,Func);
           }         
       }

       TSString suit_full_type_name = ": TSBasicService.DataType";
       boost::algorithm::replace_all(replace_content, "__SUPER_TYPE__", suit_full_type_name);
       boost::algorithm::replace_all(replace_content, "_NEW_LIZE_", "new");

       TSString baseSer = "base.Serialize(buffer,MarshaledFlag);" + TSIDL_NEWLINE;
       TSString baseDeSer = "base.Deserialize(buffer);" + TSIDL_NEWLINE;

       boost::algorithm::replace_all(replace_content, "__BASE_SERIALIZE__", baseSer);
       boost::algorithm::replace_all(replace_content, "__BASE_DESERIALIZE__",baseDeSer);

       boost::algorithm::replace_all(replace_content,"_CUSTON_",custon);
       boost::algorithm::replace_all(replace_content, "_BUFFER_WRITE_", writeBuffer);     
       boost::algorithm::replace_all(replace_content, "_BUFFER_READ_", readBuffer);

       if (Requst)
       {
           boost::algorithm::replace_all(replace_content, __FIELD_DECLARE__, defineInAttribute);
           boost::algorithm::replace_all(replace_content, __FIELD_ASSIGN__, parmsInit);
       }
       else
       {
           boost::algorithm::replace_all(replace_content, __FIELD_DECLARE__, defineOutAttribute);
           boost::algorithm::replace_all(replace_content, __FIELD_ASSIGN__, paramsOutInit);
       }

       ReplaceTypeNameWithCsharp(replace_content);
       
    }
}
    
    TSString GetFuncWithoutParamType( TSFunctionDefPtr Func, const TSString & Response)
    {
        TSString code;
        TSString TypeName;
        TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
        code += Func->ValName + "(";
        for (unsigned int i = 0; i < Func->Params.size(); i++)
        {
            TSParamDefPtr Param = Func->Params[i];

            if (Cp == "c#")
            {
                if (Param->Type == TSParamDef::Inout)
                {
                    code += "ref ";
                }
                else if (Param->Type == TSParamDef::Out)
                {
                    code += "out ";
                }
            }
            code += TypeName + TSIDL_BLANK;
            if (Cp == "c#")
            {
                if (Param->Type != TSParamDef::In)
                {
                    code += Response;
                }
                else
                {
                    code += "Req.";
                }
                
            }
            code += Param->ValName;
            if (Func->Params.size() != i + 1)
            {
                code += ",";
            }
        }
        code += ")";
        return code;
    }

    void ReplaceTypeNameWithCsharp( TSString & replace_content )
    {
        boost::algorithm::replace_all(replace_content,PACKAGE_SPLIT,PACKAGE_SPLIT_4CS);

        boost::algorithm::replace_all(replace_content, "Readbool",   "ReadBool");
        boost::algorithm::replace_all(replace_content, "Readsbyte",  "ReadInt8");
        boost::algorithm::replace_all(replace_content, "Readbyte",   "ReadUInt8");
        boost::algorithm::replace_all(replace_content, "Readshort",  "ReadInt16");
        boost::algorithm::replace_all(replace_content, "Readushort", "ReadUInt16");
        boost::algorithm::replace_all(replace_content, "Readint64",  "ReadInt64");
        boost::algorithm::replace_all(replace_content, "Readint16",  "ReadInt16");
        boost::algorithm::replace_all(replace_content, "Readint8",   "ReadInt8");
        boost::algorithm::replace_all(replace_content, "Readint32",  "ReadInt32");
        boost::algorithm::replace_all(replace_content, "Readint",    "ReadInt32");
        boost::algorithm::replace_all(replace_content, "Readlong",   "ReadInt64");
        boost::algorithm::replace_all(replace_content, "Readulong",  "ReadUInt64");
        boost::algorithm::replace_all(replace_content, "Readfloat64", "ReadFloat");
        boost::algorithm::replace_all(replace_content, "Readfloat32", "ReadFloat");
        boost::algorithm::replace_all(replace_content, "Readfloat",   "ReadFloat");

        boost::algorithm::replace_all(replace_content, "Readdouble", "ReadDouble");
        boost::algorithm::replace_all(replace_content, "ReadSBYTE",  "ReadInt8");
        boost::algorithm::replace_all(replace_content, "ReadBYTE",   "ReadUInt8");
        boost::algorithm::replace_all(replace_content, "ReadSHORT",  "ReadInt16");
        boost::algorithm::replace_all(replace_content, "ReadUSHORT", "ReadUInt16");
        boost::algorithm::replace_all(replace_content, "ReadINT",  "ReadInt");

        boost::algorithm::replace_all(replace_content, "ReadLONG",   "ReadInt64");
        boost::algorithm::replace_all(replace_content, "ReadULONG",  "ReadUInt64");

        boost::algorithm::replace_all(replace_content, "ReadUINT", "ReadUInt");
        boost::algorithm::replace_all(replace_content, "ReadFLOAT",  "ReadFloat");
        boost::algorithm::replace_all(replace_content, "ReadDOUBLE", "ReadDouble");
        boost::algorithm::replace_all(replace_content, "Readstring", "ReadString");
        boost::algorithm::replace_all(replace_content, "Readuint64", "ReadUInt64");
        boost::algorithm::replace_all(replace_content, "Readuint32", "ReadUInt32");
        boost::algorithm::replace_all(replace_content, "Readuint16", "ReadUInt16");
        boost::algorithm::replace_all(replace_content, "Readuint8",  "ReadUInt8");
        boost::algorithm::replace_all(replace_content, "Readuint",   "ReadUInt32");
        boost::algorithm::replace_all(replace_content, "Writebool",   "WriteBool"); 
        boost::algorithm::replace_all(replace_content, "WriteBYTE",   "WriteUInt8"); 
        boost::algorithm::replace_all(replace_content, "WriteSBYTE",  "WriteInt8");
        boost::algorithm::replace_all(replace_content, "WriteSHORT",  "WriteInt32");
        boost::algorithm::replace_all(replace_content, "WriteUSHORT", "WriteUInt32");

        boost::algorithm::replace_all(replace_content, "WriteLONG",   "WriteInt64");
        boost::algorithm::replace_all(replace_content, "WriteULONG",  "WriteUInt64");
        boost::algorithm::replace_all(replace_content, "WriteUINT",   "WriteUInt");
        boost::algorithm::replace_all(replace_content, "WriteFLOAT",  "WriteFloat");
        boost::algorithm::replace_all(replace_content, "WriteDOUBLE", "WriteDouble");
        boost::algorithm::replace_all(replace_content, "Writebyte",   "WriteUInt8"); 
        boost::algorithm::replace_all(replace_content, "Writesbyte",  "WriteInt8");
        boost::algorithm::replace_all(replace_content, "Writeshort",  "WriteInt16");
        boost::algorithm::replace_all(replace_content, "WriteINT",    "WriteInt");
        boost::algorithm::replace_all(replace_content, "Writeushort", "WriteUInt16");
        boost::algorithm::replace_all(replace_content, "Writestring", "WriteString");
        boost::algorithm::replace_all(replace_content, "Writeuint64", "WriteUInt64");
        boost::algorithm::replace_all(replace_content, "Writeuint32", "WriteUInt32");
        boost::algorithm::replace_all(replace_content, "Writeuint16", "WriteUInt16");
        boost::algorithm::replace_all(replace_content, "Writeuint8",  "WriteUInt8");
        boost::algorithm::replace_all(replace_content, "Writeuint",   "WriteUInt32");
        boost::algorithm::replace_all(replace_content, "Writeint64",  "WriteInt64");
        boost::algorithm::replace_all(replace_content, "Writeint32",  "WriteInt32");
        boost::algorithm::replace_all(replace_content, "Writeint16",  "WriteInt16");
        boost::algorithm::replace_all(replace_content, "Writeint8",   "WriteInt8");
        boost::algorithm::replace_all(replace_content, "Writeint",    "WriteInt32");
        boost::algorithm::replace_all(replace_content, "Writelong",   "WriteInt64");
        boost::algorithm::replace_all(replace_content, "Writeulong",  "WriteUInt64");

        boost::algorithm::replace_all(replace_content, "Writefloat64",  "WriteFloat");
        boost::algorithm::replace_all(replace_content, "Writefloat32",  "WriteFloat");
        boost::algorithm::replace_all(replace_content, "Writefloat",    "WriteFloat");

        boost::algorithm::replace_all(replace_content, "Writedouble", "WriteDouble");


        boost::algorithm::replace_all(replace_content, "ReadTSHANDLE",  "ReadTSTOPICHANDLE");
        boost::algorithm::replace_all(replace_content, "ReadTSBasicSdo", "ReadTSObjectHandle");
        boost::algorithm::replace_all(replace_content, "WriteTSHANDLE", "WriteTSTOPICHANDLE");
        boost::algorithm::replace_all(replace_content, "WriteTSBasicSdo", "WriteTSObjectHandle");
    }

    TSString GetServiceFunAsyncStr( TSFunctionDefPtr func,bool IsServiceTime)
    {
        TSString code;
        TSString TypeName;
        TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
        code += "void" + TSIDL_BLANK + func->ValName + "Async(";
        for (unsigned int i = 0; i < func->Params.size(); i++)
        {
            TSParamDefPtr Param = func->Params[i];
            if (Param->Def->Type != TSTypeDef::Builtin ||
                Param->Def->Type != TSTypeDef::Enum)
            {
                if (Cp == "c#")
                {
                   if(Param->Def->Type == TSTypeDef::Array)
                   {
                    TSVectorBuiltinTypeDefPtr vectorField = TS_CAST(Param->Def,TSVectorBuiltinTypeDefPtr);
                    if (vectorField->Value->Type != TSTypeDef::Builtin)
                    {
                        TypeName = "List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorField->Value) + ">";
                    }
                    else
                    {
                        TypeName = TSBuiltinPlugin::GetSuitFullTypeName(Param->Def);
                    }
                   }
                   else
                   {
                      TypeName = TSBuiltinPlugin::GetSuitFullTypeName(Param->Def);
                   }
                }
              
            }
            else
            {
                TypeName = Link2Type(Param->DefTypeName);
            }
            if (Cp == "c#")
            {
                if (Param->Type == TSParamDef::Inout)
                {
                    code += "ref ";
                }
                else if (Param->Type == TSParamDef::Out)
                {
                    code += "out ";
                }
            }
            code += TypeName + TSIDL_BLANK + Param->ValName;
            if (func->Params.size() != i + 1)
            {
                code += ",";
            }
        }
        if (func->Params.size())
        {
            if(Cp == "java")
            {
                code += ",proxyInter func";
            }
            else
            {
                code += ",Method." + func->ValName + "Async func";
            }
        }
        else
        {
            if(Cp == "java")
            {
                code += "proxyInter func";
            }
            else
            {
                code += "Method." + func->ValName + "Async func";
            }
           
        }
       
        if (IsServiceTime)
        {
            code += ",UInt32 Timeout = 0";
        }
        code += ")";
        return code;
    }

    TSString GetFuncParamWithoutFuncName( TSFunctionDefPtr Func )
    {
        TSString code;
        TSString TypeName;
        bool HaveReturn = false;
        std::vector<TSParamDefPtr> vecParams;
        TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
        code = "(";
        if (Func->DefTypeName != "void" &&
            Func->DefTypeName != "Void")
        {
            code += "rnt";
            HaveReturn = true;
        }
        for (unsigned int i = 0; i < Func->Params.size(); i++)
        {
            TSParamDefPtr Param = Func->Params[i];
            if (Cp == "c#")
            {
                if (Param->Type == TSParamDef::Inout)
                {
                    vecParams.push_back(Param);         
                }
                else if (Param->Type == TSParamDef::Out)
                {
                    vecParams.push_back(Param);    
                }
            }
        }
        
        for(unsigned int i = 0; i < vecParams.size(); i++)
        {

            TSParamDefPtr Param = vecParams[i];
            if (Param->Type == TSParamDef::Inout || TSParamDef::Out)
            {
                if (HaveReturn)
                {
                    code += ",ref ";
                }
                else
                {
                    code += "ref ";
                }
            }

            code += Param->ValName;
            if (vecParams.size() != i + 1)
            {
                if (!HaveReturn)
                {
                    code += ",";
                }               
            }
            
        }
        code += ")";
        return code;
    }

	TSString GenerateTypeVersionNum( TSTypeDefWithFiledPtr Typedef )
	{
		UINT32 IDVersion = 0;
		if (Typedef->Extend.lock())
		{
			if(TSTypeDefWithFiledPtr filedDef = TS_CAST(Typedef->Extend.lock(),TSTypeDefWithFiledPtr))
			{
				BOOST_FOREACH(TSFieldDefPtr filed,filedDef->Fileds)
				{
					IDVersion += GetCRC32(filed->DefTypeName);
				}
			}	
		}

		BOOST_FOREACH(TSFieldDefPtr filed,Typedef->Fileds)
		{
			IDVersion += GetCRC32(filed->DefTypeName);
		}

		TSString IDVersionStr = TSValue_Cast<TSString,UINT32>(IDVersion);
		return IDVersionStr;
	}

	TSString GenerateTypeVersionNum4Service( TSFunctionDefPtr FunDef,bool IsReq)
	{
		UINT32 ReqVer = 0;
		UINT32 ResVer = 0;
		BOOST_FOREACH(TSParamDefPtr Param,FunDef->Params)
		{
			if (Param->Type != TSParamDef::In)
			{
				ResVer += GetCRC32(Param->DefTypeName);
			}

			if(Param->Type != TSParamDef::Out)
			{
				ReqVer += GetCRC32(Param->DefTypeName);
			}
		}

		if (FunDef->DefTypeName != "void" && FunDef->DefTypeName != "Void")
		{
			ResVer += GetCRC32(FunDef->DefTypeName);
		}

		if (IsReq)
		{
			return TSValue_Cast<TSString,UINT32>(ReqVer);
		}
		else
		{
			return TSValue_Cast<TSString,UINT32>(ResVer);
		}
	}
}
