#include "stdafx.h"

#include <boost/foreach.hpp>

#include "TSBuiltinPlugin.h"
#include "TSIDLUtils.h"

BEGIN_METADATA(TSIBuiltinPlugin)
REG_BASE(TSIIDLPlugin);
END_METADATA()

BEGIN_METADATA(TSBuiltinPlugin)
REG_BASE(TSIBuiltinPlugin);
END_METADATA()

TSTypeDefPtr TSIBuiltinPlugin::CreateTypeDefByTypeInfoName( const char * TypeInfoName )
{
	TSTypeDefPtr TypeDef;
    
	if(IS_MATCH(TypeInfoName,TSIDLGram::PACKAGE))
	{
		TypeDef = boost::make_shared<TSPackageDef>();
		TypeDef->Type = TSTypeDef::Package;
	}
	else if(IS_MATCH(TypeInfoName,TSIDLGram::SERVICE))
	{
		TypeDef = boost::make_shared<TSServiceDef>();
		TypeDef->Type = TSTypeDef::Service;
	}
	else if(IS_MATCH(TypeInfoName,TSIDLGram::STRUCT))
	{
		TypeDef = boost::make_shared<TSStructDef>();
		TypeDef->Type = TSTypeDef::Struct;
	}
	else if(IS_MATCH(TypeInfoName,TSIDLGram::ENUM))
	{
		TypeDef = boost::make_shared<TSEnumDef>();
		TypeDef->Type = TSTypeDef::Enum;
	}
    else if(IS_MATCH(TypeInfoName,TSIDLGram::TOPIC))
    {
        TypeDef = boost::make_shared<TSTopicTypeDef>();
        TypeDef->Type = TSTypeDef::Topic;
    }
    else if (IS_MATCH(TypeInfoName,TSIDLGram::XEXCEPTION))
    {
        TypeDef = boost::make_shared<TSExceptionDef>();
        TypeDef->Type = TSTypeDef::Exception;
    }
    else if (IS_MATCH(TypeInfoName,TSIDLGram::TYPEDEFINE))
    {
        TypeDef = boost::make_shared<TSTypeDefineLink>();
        TypeDef->Type = TSTypeDef::TypeDefine;
    }

	ASSERT(TypeDef && "TypeDef");
	return TypeDef;
}

const char * TSIBuiltinPlugin::GetTypeInfoNameByTypeName( const TSString & TypeName )
{
	if(TypeName == "package")
	{
		return typeid(TSIDLGram::PACKAGE).name();
	}
	else if(TypeName == "service")
	{
		return typeid(TSIDLGram::SERVICE).name();
	}
	else if(TypeName == "struct")
	{
		return typeid(TSIDLGram::STRUCT).name();
	}
	else if(TypeName == "enum")
	{
		return typeid(TSIDLGram::ENUM).name();
	}
	else
	{
		return NULL;	
	}
}

TSTypeDefPtr TSIBuiltinPlugin::CreateBuiltinTypeDefByTypeName( const TSString & TypeName )
{
	TSBuiltinTypeDefPtr TypeDef = boost::make_shared<TSBuiltinTypeDef>();
	TypeDef->Type = TSTypeDef::Builtin;
	TypeDef->BuiltinType = GetBuiltinTypeFromTypeName(TypeName);
	TypeDef->TypeName = TypeName;

	if(TypeDef->BuiltinType == TSVariant::VT_INVALID)
	{
		TypeDef.reset();
	}

	return TypeDef;
}

TSString TSIBuiltinPlugin::GetSuitFullTypeName( TSTypeDefPtr TypeDef, bool getSpace, TypeKind Kind)
{
    TSString Cp;
    Cp = TSIDLPluginMgr::Instance()->GetCodeType();

	if(TypeDef)
	{
		TSString FullPackageName = GetFullPackageNameByPackageDef(TypeDef->SupperPkg.lock());
		std::vector<TSString> TypeNames;
		if(!FullPackageName.empty())
		{
			TypeNames.push_back(FullPackageName);
		}
        
        TSString TypeName = TypeDef->TypeName;
        if (TypeDef->Type == TSTypeDef::Builtin)
        {
            #define TSIDL_TD(TSIDLTYPE,CPPTYPE,CSHARPTYPE,JAVATYPE) \
            if(TypeName == TSIDLTYPE) \
            { \
                if (Cp == "c++")\
                {\
                    TypeName = CPPTYPE;\
                }\
                else if (Cp == "c#")\
                {\
                    TypeName = CSHARPTYPE;\
                }\
                else if (Cp == "java")\
                {\
                    TypeName = JAVATYPE;\
                }\
            }
            TSIDL_GEN_TYPES(TSIDL_TD);
            #undef TSIDL_TD   
        }
        else if (TypeDef->Type == TSTypeDef::Array)
        {
            TSVectorBuiltinTypeDefPtr vectorType = TS_CAST(TypeDef,TSVectorBuiltinTypeDefPtr);
            if (vectorType)
            {
                if (Cp == "c#")
                {
					if (vectorType->Value->Type == TSTypeDef::Builtin)
					{
						//如果是TSHANDLE类型或者是TSVector3d类型，创建List
						if (vectorType->ValueTypeName == "TSHANDLE" || vectorType->ValueTypeName == "TSVector3d")
						{
							TypeName = "List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorType->Value) + ">";
						}
						else
						{
							TypeName += TSBuiltinPlugin::GetSuitFullTypeName(vectorType->Value) + "[]";
						}
					}
					else
					{
						TypeName += "List<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorType->Value) + ">";
					}
                    
                }
                else if (Cp == "c++")
                {
                    TypeName += "std::vector<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorType->Value) + ">";
                }
                else if(Cp == "java")
                {
                    TypeName += "Vector<" + TSBuiltinPlugin::GetSuitFullTypeName(vectorType->Value) + ">";
                }
            }
            
        }
        else if ((TypeDef->Type == TSTypeDef::Enum 
            || TypeDef->Type == TSTypeDef::TypeDefine) 
            && !getSpace)
        {
            TypeNames.push_back(TypeName);
        }
        else if (TypeDef->Type == TSTypeDef::Map)        
        {
     
            TSMapBuiltinTypeDefPtr mapType = TS_CAST(TypeDef,TSMapBuiltinTypeDefPtr);
            if (Cp == "c++")
            {
            TypeName += "std::map<" + TSBuiltinPlugin::GetSuitFullTypeName(mapType->Key) + "," 
                + TSBuiltinPlugin::GetSuitFullTypeName(mapType->Value) + ">";
            }
            else if (Cp == "c#")
            {
                TypeName += "Dictionary<" + TSBuiltinPlugin::GetSuitFullTypeName(mapType->Key) + "," 
                    + TSBuiltinPlugin::GetSuitFullTypeName(mapType->Value) + ">";
            }
            else if (Cp == "java")
            {
               TypeName += "Map<" + TSBuiltinPlugin::GetSuitFullTypeName(mapType->Key) + "," 
                    + TSBuiltinPlugin::GetSuitFullTypeName(mapType->Value) + ">";    
            }

        }
        else if (TypeDef->Type == TSTypeDef::Struct && !getSpace)
        {
            TypeName += "::DataType";
        }
        else if (TypeDef->Type == TSTypeDef::HeaderStruct)
        {
        }
        
        if (TypeDef->Type != TSTypeDef::Enum && TypeDef->Type != TSTypeDef::TypeDefine)
        {
            TypeNames.push_back(TypeName);
        } 

        TSString TyNamejoinEnd = boost::algorithm::join(TypeNames,PACKAGE_SPLIT);
        if (Cp == "c#" && !getSpace)
        {
            boost::algorithm::replace_all(TyNamejoinEnd,"::",".");
        }
        return TyNamejoinEnd;
       
	}
	else
	{
		return TSString();
	}
}

TSString TSIBuiltinPlugin::GetFullPackageNameByPackageDef( TSPackageDefPtr Pkg )
{
	if(Pkg)
	{
		std::vector<TSString> PkgNames;
		PkgNames.push_back(Pkg->TypeName);
		while(TSPackageDefPtr Supper = Pkg->SupperPkg.lock())
		{
			Pkg = Supper;
			PkgNames.push_back(Supper->TypeName);
		}

		std::reverse(PkgNames.begin(),PkgNames.end());
		return boost::algorithm::join(PkgNames,PACKAGE_SPLIT);
	}

	return TSString();
}

TSVariant::Type TSIBuiltinPlugin::GetBuiltinTypeFromTypeName( const TSString & TypeName )
{
	if(TypeName == "bool"
		|| TypeName == "BOOL")
	{
		return TSVariant::VT_BOOL;
	}
#if !defined(XSIM3_3) && !defined(XSIM3_2)
    else if(TypeName == "char")
    {
        return TSVariant::VT_CHAR;
    }
    else if (TypeName == "wchar_t")
    {
        return TSVariant::VT_WCHAR;
    }
    else if(TypeName == "TSBattleTime")
    {
        return TSVariant::VT_BATTLE_TIME;
    }
#endif // !defined(XSIM3_3) && !defined(XSIM3_2)
	else if(TypeName == "uint8"
		|| TypeName == "UINT8"
		|| TypeName == "byte"
		|| TypeName == "Byte"
		|| TypeName == "BYTE"
		|| TypeName == "octet"
		|| TypeName == "Octet"
		|| TypeName == "OCTET")
	{
		return TSVariant::VT_UINT8;
	}
    else if (TypeName == "UINT16"
        || TypeName == "uint16"
        || TypeName == "ushort")
    {
        return TSVariant::VT_UINT16;
    }
    else if (TypeName == "uint32"
        || TypeName == "UINT32" )
    {
        return TSVariant::VT_UINT32;
    }
    else if(TypeName == "UINT64"
        || TypeName == "uint64")
    {
        return TSVariant::VT_UINT64;
    }
	else if(TypeName == "int8"
		|| TypeName == "INT8")
	{
		return TSVariant::VT_INT8;
	}
	else if(TypeName == "int16"
		|| TypeName == "INT16"
        || TypeName == "short")
	{
		return TSVariant::VT_INT16;
	}
	else if(TypeName == "int32"
		|| TypeName == "INT32"
        || TypeName == "int"
        || TypeName == "long")
	{
		return TSVariant::VT_INT32;
	}
	else if(TypeName == "int64"
		|| TypeName == "INT64")
	{
		return TSVariant::VT_INT64;
	}
	else if(TypeName == "float"
		|| TypeName == "FLOAT"
		|| TypeName == "float32"
		|| TypeName == "FLOAT32")
	{
		return TSVariant::VT_FLOAT;
	}
	else if(TypeName == "double"
		|| TypeName == "DOUBLE"
		|| TypeName == "float64"
		|| TypeName == "FLOAT64")
	{
		return TSVariant::VT_DOUBLE;
	}
	else if(TypeName == "string"
		|| TypeName == "std::string"
        || TypeName == "Wstring")
	{
		return TSVariant::VT_STRING;
	}
	else if(TypeName == "TSVector2d")
	{
		return TSVariant::VT_VECTOR2D;
	}
	else if(TypeName == "TSVector3d")
	{
		return TSVariant::VT_VECTOR3D;
	}
	else if(TypeName == "TSVector2f")
	{
		return TSVariant::VT_VECTOR2F;
	}
	else if(TypeName == "TSVector3f")
	{
		return TSVariant::VT_VECTOR3F;
	}
	else if(TypeName == "TSTime")
	{
		return TSVariant::VT_TIME;
	}
	else if(TypeName == "TSTimeDuration")
	{
		return TSVariant::VT_TIMEDURATION;
	}
	else if(TypeName == "TSHANDLE"
		|| TypeName == "handle")
	{
		return TSVariant::VT_HANDLE;
	}
    else if (TypeName == "TSTOPICHANDLE")
    {
        return TSVariant::VT_USER;
    }
	else if(TypeName == "TSModelTmplOrClassId")
	{
		return TSVariant::VT_MODELTMPLORCLASSID;
	}
	else if(TypeName == "ByteBuffer"
		|| TypeName == "TSByteBuffer"
		|| TypeName == "Binary"
		|| TypeName == "binary"
		|| TypeName == "bytebuffer"
		|| TypeName == "tsbytebuffer"
		|| TypeName == "buffer"
		|| TypeName == "Buffer")
	{
		return TSVariant::VT_BINARY;
	}
	else if (TypeName == "TSAssetTypeId")
	{
		return TSVariant::VT_ASSETTYPEID;
	}
	else if (TypeName == "TSClassificationId")
	{
		return TSVariant::VT_CLASSIFICATIONID;
	}
	else if (TypeName == "TSModelDataId")
	{
		return TSVariant::VT_MODELDATAID;
	}
	else if (TypeName == "TSTemplateId")
	{
		return TSVariant::VT_TEMPLATEID;
	}
	else if (TypeName == "TSVariant"
		|| TypeName == "TSVariantArray"
		|| TypeName == "TSVariantPair")
	{
		return TSVariant::VT_VARIANT;
	}
	else if (TypeName == "void" 
        || TypeName == "Void")
	{
        return TSVariant::VT_USER;
	}

	return TSVariant::VT_INVALID;
}


class TSBuiltinPluginPrivate
{
public:
	std::vector<TSTypeDefPtr> _TopLevelTypeDefs;
	std::vector<TSTypeDefWithFuncAndFiledPtr> _SortedTypeDefs;
	std::vector<TSTopicTypeDefPtr> _SortedTopicTypeDefs;
	std::vector<TSTypeDefPtr> _TempTopLevelTypeDefs;
};

template<typename T>
void GetPackageTypeDefs(TSPackageDefPtr  Pkg,
	std::vector<boost::shared_ptr<T> > & FlatTypeDefs)
{
    BOOST_FOREACH(TSTypeDefPtr TypeDef,Pkg->TypeDefs)
    {
        if(boost::shared_ptr<T> CastTypeDef = boost::dynamic_pointer_cast<typename boost::shared_ptr<T>::element_type >(TypeDef))
        {
            FlatTypeDefs.push_back(CastTypeDef);
        }
    }

	BOOST_FOREACH(TSPackageDefPtr TypeDef,Pkg->Subordinates)
	{
		GetPackageTypeDefs(TypeDef,FlatTypeDefs);
	}
	
	if(boost::shared_ptr<T> CastTypeDef = boost::dynamic_pointer_cast<typename boost::shared_ptr<T>::element_type >(Pkg))
	{
		FlatTypeDefs.push_back(CastTypeDef);
	}
}

template<typename T>
void GetFlatTypeDefs(const std::vector<TSTypeDefPtr> & TopLevelTypeDefs
	,std::vector<boost::shared_ptr<T> > & FlatTypeDefs)
{
	BOOST_FOREACH(TSTypeDefPtr TypeDef,TopLevelTypeDefs)
	{
		if(TSPackageDefPtr Pkg = TS_CAST(TypeDef,TSPackageDefPtr))
		{
			GetPackageTypeDefs(Pkg,FlatTypeDefs);
		}
		else
		{
			if(boost::shared_ptr<T> CastTypeDef = boost::dynamic_pointer_cast<typename boost::shared_ptr<T>::element_type >(TypeDef))
			{
				FlatTypeDefs.push_back(CastTypeDef);
			}
		}
	}
}

TSBuiltinPlugin::TSBuiltinPlugin( void )
	:_p(new TSBuiltinPluginPrivate)
{

}

TSBuiltinPlugin::~TSBuiltinPlugin( void )
{
	delete _p;
}

const std::vector<TSTypeDefPtr> & TSBuiltinPlugin::GetTopLevelTypeDefs()
{
	return _p->_TopLevelTypeDefs;
}

TSString  TSBuiltinPlugin::GetName()
{
	return GB18030ToTSString("内建类型插件");
}

bool TSBuiltinPlugin::DoParse()
{
    std::vector<TSTypeDefPtr>().swap(_p->_TopLevelTypeDefs);
	const TSIDLPluginMgr::ParseMap_T & Parsers = TSIDLPluginMgr::Instance()->GetParserUtils();
	bool Ret = false;

	TSIDLPluginMgr::ParseMap_T::const_iterator it = Parsers.begin();
	while(it != Parsers.end())
	{
		//向_TopLevelTypeDefs容器中添加元素
		DoParseFinished_p(it->second->Parse /*Parse结构体（包含xidl文件的一些操作）*/);
		Ret = true;
		++it;
	}

	if(Ret)
	{
		try
		{
            PrepareBuild();
            std::vector<TSTypeDefPtr> TopLevelTypeDefs = GetTopLevelTypeDefs();
            CheckoutPackageError(TopLevelTypeDefs);
            CheckoutTypeError();    
		}
        catch(TSException &exp)
        {
            throw exp;
        }
		catch(...)
		{
			Ret = false;
		}
	}

	return true;
}

void TSBuiltinPlugin::OnPluginLoaded()
{
	
}

void TSBuiltinPlugin::Clean()
{
   
}

void TSBuiltinPlugin::CleanTopLevelTypeDefs()
{
    std::vector<TSTypeDefPtr>().swap(_p->_TopLevelTypeDefs);
	std::vector<TSTypeDefWithFuncAndFiledPtr>().swap(_p->_SortedTypeDefs);
	std::vector<TSTopicTypeDefPtr>().swap(_p->_SortedTopicTypeDefs);
}


void TSBuiltinPlugin::BuildDefaultVal_p( TSFieldDefPtr Field,TSIDLGram::Parser::Node  * Node )
{
	if(IS_TYPE(Node,TSIDLGram::DefaultValParam))
	{
		Field->DefaultVal = NodeToStringTrim(Node);
	}
}

void TSBuiltinPlugin::BuildFlagVal_p( TSFieldDefPtr Field,TSIDLGram::Parser::Node  * Node )
{
	if(IS_TYPE(Node,TSIDLGram::FlagValParam))
	{
		Field->Flag = NodeToStringTrim(Node);
	}
}

void TSBuiltinPlugin::BuildFindSetVal_p( TSFieldDefPtr Field,TSIDLGram::Parser::Node  * Node )
{
	if(IS_TYPE(Node,TSIDLGram::HANDLE))
	{
		Field->FindSetEnumType = Field->FindSetEnumType | TSFieldDef::Handle;
        Field->FindSet = TSFieldDef::Handle;
	}
	else if(IS_TYPE(Node,TSIDLGram::MODELORTMPLID))
	{
		Field->FindSetEnumType = Field->FindSetEnumType | TSFieldDef::ModelOrTmplId;
        Field->FindSet = TSFieldDef::ModelOrTmplId;
	}
	else if(IS_TYPE(Node,TSIDLGram::SPACIALRANGE))
	{
		Field->FindSetEnumType = Field->FindSetEnumType | TSFieldDef::SpacialRange;
        Field->FindSet = TSFieldDef::SpacialRange;
	}
	else if(IS_TYPE(Node,TSIDLGram::KEY))
	{
		Field->FindSetEnumType = Field->FindSetEnumType | TSFieldDef::Key;
        Field->FindSet = TSFieldDef::Key;
        Field->HaveKey = true;
	}
}


void TSBuiltinPlugin::BuildFiledDef_p( TSFieldDefPtr Field,TSIDLGram::Parser::Node  * Node )
{
	//todo 暂不支持容器中套容器

	if(IS_TYPE(Node,TSIDLGram::DescVal))
	{
		Field->Desc = StringNodeToString(Node);
	}
	else if(IS_TYPE(Node,TSIDLGram::FlagVal))
	{
		TSIDLGram::Visit(Node,boost::bind(&TSBuiltinPlugin::BuildFlagVal_p,this,Field,_1));
	}
	else if(IS_TYPE(Node,TSIDLGram::DefaultVal))
	{
		TSIDLGram::Visit(Node,boost::bind(&TSBuiltinPlugin::BuildDefaultVal_p,this,Field,_1));
	}
	else if(IS_TYPE(Node,TSIDLGram::FindSetVal))
	{
		TSIDLGram::Visit(Node,boost::bind(&TSBuiltinPlugin::BuildFindSetVal_p,this,Field,_1));
	}
	else if(IS_TYPE(Node,TSIDLGram::FieldTypeName) || IS_TYPE(Node,TSIDLGram::TypeDefineIdent))
	{
		Field->DefTypeName = NodeToStringTrim(Node);
		TSIDLGram::Visit(Node,boost::bind(&TSBuiltinPlugin::BuildBuiltinTypePair_p,this,Field,_1));
	}
	else if(IS_TYPE(Node,TSIDLGram::FieldValName))
	{
		Field->ValName = NodeToStringTrim(Node);
	}
}

void TSBuiltinPlugin::BuildEnum_p( TSEnumFiledDefPtr Enum,TSIDLGram::Parser::Node *Node)
{
	if(IS_TYPE(Node,TSIDLGram::EnumSimpleVal))
	{
		Enum->ValName = NodeToStringTrim(Node);
	}
	else if(IS_TYPE(Node,TSIDLGram::EnumComplexDefaultVal))
	{
		Enum->DefaultVal = NodeToStringTrim(Node);
	}
}
//解析出主题
void TSBuiltinPlugin::BuildTypeDef_p( TSTypeDefPtr TypeDef,TSIDLGram::Parser::Node *Node )
{
	if(IS_TYPE(Node,TSIDLGram::EnumVal))
	{
		TSEnumFiledDefPtr Field = boost::make_shared<TSEnumFiledDef>();
		Field->Node = Node;
        Field->ValName = NodeToString(Node);

        std::vector<TSString> vecEnumFiled;
        boost::split(vecEnumFiled,Field->ValName,boost::is_any_of("="));
        if(vecEnumFiled.size() == 1)
        {
            Field->Name = vecEnumFiled[0];
            Field->DefaultVal = "";
        }
        else
        {
            Field->Name = vecEnumFiled[0];
            Field->DefaultVal = vecEnumFiled[1];
        }
        boost::algorithm::replace_all(Field->Name," ","");
        boost::algorithm::replace_all(Field->DefaultVal," ","");
        boost::algorithm::replace_all(Field->Name,"\n","");
        boost::algorithm::replace_all(Field->DefaultVal,"\n","");
        boost::algorithm::replace_all(Field->Name,"	","");
        boost::algorithm::replace_all(Field->DefaultVal,"	","");
		ConvertSafeTypeDef<TSEnumDef>(TypeDef)->Fileds.push_back(Field);
	}
	else if(IS_TYPE(Node,TSIDLGram::Field))
	{
		TSFieldDefPtr Field = boost::make_shared<TSFieldDef>();
		Field->Node = Node;

		TSIDLGram::VisitChild(Node,boost::bind(&TSBuiltinPlugin::BuildFiledDef_p,this,Field,_1));
		ConvertSafeTypeDef<TSTypeDefWithFiled>(TypeDef)->Fileds.push_back(Field);
	}
	else if(IS_TYPE(Node,TSIDLGram::Function))
	{
		TSFunctionDefPtr Func = boost::make_shared<TSFunctionDef>();
		Func->Node = Node;

		TSIDLGram::VisitChild(Node,boost::bind(&TSBuiltinPlugin::BuildFunction_p,this,Func,_1));
		ConvertSafeTypeDef<TSTypeDefWithFunction>(TypeDef)->Functions.push_back(Func);
	}
	else if(IS_TYPE(Node,TSIDLGram::TopicDesc))
	{
		TSIDLGram::VisitChild(Node,boost::bind(&TSBuiltinPlugin::BuildTopicDesc_p,this,ConvertSafeTypeDef<TSTopicTypeDef>(TypeDef),_1));
	}
	else if(IS_TYPE(Node,TSIDLGram::TopicTypeName))
	{
		//trim_copy的宏定义，删除字符串首部尾部空格，获得新的字符串
		TypeDef->TypeName = NodeToStringTrim(Node);
	}
	else if(IS_TYPE(Node,TSIDLGram::TopicParam))
	{
		ConvertSafeTypeDef<TSTopicTypeDef>(TypeDef)->DefTypeName = NodeToStringTrim(Node);
	}
    else if (IS_TYPE(Node,TSIDLGram::TypeDefineField))
    {
        TSFieldDefPtr field = boost::make_shared<TSFieldDef>();
        field->Node = Node;
        TSIDLGram::VisitChild(Node,boost::bind(&TSBuiltinPlugin::BuildFiledDef_p,this,field,_1));
        if (TSTypeDefineLinkPtr DefineDef = TS_CAST(TypeDef,TSTypeDefineLinkPtr))
        {
            DefineDef->Field = field;          
        }
    }
}

void TSBuiltinPlugin::BuildTypeDef( TSTypeDefPtr TypeDef,TSIDLGram::TypeDefPtr ParseTypeDef)
{
	TypeDef->TypeName = ParseTypeDef->TypeName;

    TypeDef->Node = ParseTypeDef->Node;
	if(TSExtendDefPtr Extend = TS_CAST(TypeDef,TSExtendDefPtr))
	{
		TSIDLGram::Visit(ParseTypeDef->Node,boost::bind(&TSBuiltinPlugin::BuildExtend_p,this,Extend,_1));
	}

    if (NULL == ParseTypeDef->ContentNode)
    {
        return;
    }
	//在此解析出主题
	TSIDLGram::Visit(ParseTypeDef->ContentNode, boost::bind(&TSBuiltinPlugin::BuildTypeDef_p,this,TypeDef,_1));
}

void TSBuiltinPlugin::BuildPackageDef( TSPackageDefPtr SupperPkgDef,TSPackageDefPtr TypePkgDef,TSIDLGram::TypeDefPtr ParseTypeDef )
{
	ASSERT(TypePkgDef->Type == TSTypeDef::Package);

	TypePkgDef->TypeName = ParseTypeDef->TypeName;
	TypePkgDef->Node = ParseTypeDef->Node;

	BOOST_FOREACH(TSIDLGram::TypeDefPtr Subordinate,ParseTypeDef->Subordinates)
	{
		TSTypeDefPtr SubordinateTypeDef =  CreateTypeDefByTypeInfoName(Subordinate->TypeInfoName);
		SubordinateTypeDef->Node = Subordinate->Node;
		SubordinateTypeDef->Parse =     TypePkgDef->Parse;
		SubordinateTypeDef->SupperPkg = TypePkgDef;

		if(SubordinateTypeDef->Type == TSTypeDef::Package)
		{
			TypePkgDef->Subordinates.push_back(TS_STATIC_CAST(SubordinateTypeDef,TSPackageDefPtr));
			BuildPackageDef(TypePkgDef,TS_STATIC_CAST(SubordinateTypeDef,TSPackageDefPtr),Subordinate);
		}
		else
		{
			TypePkgDef->TypeDefs.push_back(SubordinateTypeDef);
			BuildTypeDef(SubordinateTypeDef,Subordinate);
		}
	}
}

void TSBuiltinPlugin::BuildFunction_p( TSFunctionDefPtr Func,TSIDLGram::Parser::Node *Node)
{
	if(IS_TYPE(Node,TSIDLGram::FunctionReturnVal))
	{
		Func->DefTypeName = NodeToStringTrim(Node);
		TSIDLGram::VisitChild(Node,boost::bind(&TSBuiltinPlugin::BuildBuiltinTypePair_p,this,Func,_1));
	}
	else if(IS_TYPE(Node,TSIDLGram::FunctionName))
	{
		Func->ValName = NodeToStringTrim(Node);
	}
	else if(IS_TYPE(Node,TSIDLGram::Param))
	{
		TSParamDefPtr Param = boost::make_shared<TSParamDef>();
		Param->Node = Node;
		TSIDLGram::VisitChild(Node,boost::bind(&TSBuiltinPlugin::BuildFuncParam_p,this,Param,_1));
		Func->Params.push_back(Param);
	}
    else if (IS_TYPE(Node,TSIDLGram::ExceptionTypeName))
    {
        TSTypeDefPtr Exp = boost::make_shared<TSTypeDef>();       
        TSString exceptionName = NodeToStringTrim(Node);
        Exp->TypeName = exceptionName;
        Func->Exceptions.push_back(Exp);
        Func->ExceptionTypeNames.push_back(exceptionName);
    }
}

void TSBuiltinPlugin::BuildFuncParam_p( TSParamDefPtr Param,TSIDLGram::Parser::Node *Node )
{
	if(IS_TYPE(Node,TSIDLGram::XIN))
	{
		Param->Type = TSParamDef::In;
	}
	else if(IS_TYPE(Node,TSIDLGram::XINOUT))
	{
		Param->Type = TSParamDef::Inout;
	}
	else if(IS_TYPE(Node,TSIDLGram::XOUT))
	{
		Param->Type = TSParamDef::Out;
	}
	else if(IS_TYPE(Node,TSIDLGram::ParamTypeName))
	{
		Param->DefTypeName = NodeToStringTrim(Node);
		TSIDLGram::VisitChild(Node,boost::bind(&TSBuiltinPlugin::BuildBuiltinTypePair_p,this,Param,_1));
	}
	else if(IS_TYPE(Node,TSIDLGram::ParamTypeVal))
	{
		Param->ValName = NodeToStringTrim(Node);
	}
    else if (IS_TYPE(Node,TSIDLGram::XVOID))
    {
        Param->Type = TSParamDef::Void;
        Param->DefTypeName = "void";
        Param->ValName = "void";
        TSIDLGram::VisitChild(Node,boost::bind(&TSBuiltinPlugin::BuildBuiltinTypePair_p,this,Param,_1));
    }
}

//将所有解析出来的服务存储到_ServiceNameVec容器中
void TSBuiltinPlugin::StoresAllServiceName()
{
	//std::vector<TSServiceDefPtr> serviceVec;
	std::vector<TSTypeDefWithFuncAndFiledPtr> serviceVec;
	
	GetFlatTypeDefs(_p->_TopLevelTypeDefs, serviceVec);

	if (!serviceVec.empty())
	{
		//_ServiceNameVec.push_back(serviceVec[0]->TypeName);

		BOOST_FOREACH(TSTypeDefWithFuncAndFiledPtr type, serviceVec)
		{
			BOOST_FOREACH(TSFunctionDefPtr Func, type->Functions)
			{
				_ServiceNameVec.push_back(Func->ValName);
			}
		}
	}

	/*if (serviceVec.size() > 0)
	{
		for (int i = 0; i < serviceVec.size(); ++i)
		{
			_ServiceNameVec.push_back(serviceVec[i]->TypeName);
		}
	}*/
}

//将所有解析出来的主题存入TopicNameMap容器中
void TSBuiltinPlugin::StoresAllTopicName()
{
	std::vector<TSTopicTypeDefPtr> topicVec;
	GetFlatTypeDefs(_p->_TopLevelTypeDefs, topicVec);
	
	if (topicVec.size() > 0)
	{
		CompositorTopic(topicVec);
		for (int i = 0; i < topicVec.size(); ++i)
		{
			_TopicNameMap.insert(std::pair<TSString, TSString>(topicVec[i]->DefTypeName, topicVec[i]->ExtendTypeName));
		}
	}
}

std::vector<TSString> TSBuiltinPlugin::TransferServiceName()
{
	return _ServiceNameVec;
}

void TSBuiltinPlugin::StoresCurrentFileTopicName()
{
	TSString currentFileName = TSIDLPluginMgr::Instance()->GetSelectFileName();

	std::vector<TSTypeDefPtr> tempTypeDefVec;

	for (int i = 0; i < _p->_TopLevelTypeDefs.size(); ++i)
	{
		TSString fileName = _p->_TopLevelTypeDefs[i]->Parse.lock()->GetFullFileName();
		if (currentFileName == fileName)
		{
			tempTypeDefVec.push_back(_p->_TopLevelTypeDefs[i]);
		}
	}

	BOOST_FOREACH(TSTypeDefPtr type, tempTypeDefVec)
	{
		if (type->Type == TSTypeDef::Topic)
		{
			TSTopicTypeDefPtr topicDef = TS_CAST(type, TSTopicTypeDefPtr);
			TSString topicName = topicDef->TypeName;
			TSString topicDefine = topicDef->DefTypeName;
			TSString extendName = topicDef->ExtendTypeName;
		
			_CurrentFileTopicNameMap.insert(std::pair<std::pair<TSString, TSString>, TSString>(std::pair<TSString, TSString>(topicName, extendName), topicDefine));
		}
	}
	

	//std::vector<TSTopicTypeDefPtr> topicVec;

	/*GetFlatTypeDefs(tempTypeDefVec, topicVec);

	if (topicVec.size() > 0)
	{
		CompositorTopic(topicVec);
	}

	if (topicVec.size() > 0)
	{
		for (int i = 0; i < topicVec.size(); ++i)
		{
			_CurrentFileTopicNameMap.insert(std::pair<TSString, TSString>(topicVec[i]->DefTypeName, topicVec[i]->ExtendTypeName));
		}
	}*/
}

bool TSBuiltinPlugin::DoBuild()
{
	return true;
}

void TSBuiltinPlugin::BuildExtend_p( TSExtendDefPtr TypeDef,TSIDLGram::Parser::Node *Node )
{
	if(IS_TYPE(Node,TSIDLGram::ExtendTypeName))
	{
		TypeDef->ExtendTypeName = NodeToStringTrim(Node);
	}
}

std::vector<TSPackageDefPtr> TSBuiltinPlugin::GetAllPackageDefs()
{
	std::vector<TSPackageDefPtr> Defs;
	GetFlatTypeDefs(_p->_TopLevelTypeDefs,Defs);
	return Defs;
}

void TSBuiltinPlugin::GetAllPackageDefs_p( TSPackageDefPtr CurPackageDef,std::vector<TSPackageDefPtr> & CurPackageDefs )
{
	TSString CurPackageFullName = GetFullPackageNameByPackageDef(CurPackageDef);
	const std::vector<TSPackageDefPtr> & PkgDefs = GetAllPackageDefs();
	BOOST_FOREACH(TSPackageDefPtr PkgDef,PkgDefs)
	{
		TSString PkgDefFullName = GetFullPackageNameByPackageDef(PkgDef);
		if(boost::starts_with(PkgDefFullName,CurPackageFullName))
		{
			CurPackageDefs.push_back(PkgDef);
		}
	}
}
//获得xidl文件中所有的enum类型，并存入到vector中
std::vector<TSEnumDefPtr> TSBuiltinPlugin::GetAllEnumDefs()
{
	std::vector<TSEnumDefPtr> Defs;

	GetFlatTypeDefs(_p->_TopLevelTypeDefs,Defs);

	return Defs;
}

std::vector<TSServiceDefPtr> TSBuiltinPlugin::GetAllServiceDefs()
{
	std::vector<TSServiceDefPtr> Defs;
	GetFlatTypeDefs(_p->_TopLevelTypeDefs,Defs);
	return Defs;
}
//获得xidl文件中所有的struct，并将所有的struct存入vector
std::vector<TSStructDefPtr> TSBuiltinPlugin::GetAllStructDefs()
{
	std::vector<TSStructDefPtr> Defs;
	
	GetFlatTypeDefs(_p->_TopLevelTypeDefs,Defs);

	return Defs;
}

std::vector<TSTypeDefWithFunctionPtr> TSBuiltinPlugin::GetAllTypeDefWithFunctions()
{
	std::vector<TSTypeDefWithFunctionPtr> Defs;
	GetFlatTypeDefs(_p->_TopLevelTypeDefs,Defs);
	return Defs;
}

std::vector<TSTypeDefWithFiledPtr> TSBuiltinPlugin::GetAllTypeDefWithFileds()
{
	std::vector<TSTypeDefWithFiledPtr> Defs;
	GetFlatTypeDefs(_p->_TopLevelTypeDefs,Defs);
	return Defs;
}

//获取xidl文件中的所有Topic，将主题存入vector中
std::vector<TSTopicTypeDefPtr> TSBuiltinPlugin::GetAllTopicTypeDefs()
{
	if (!_p->_SortedTopicTypeDefs.size() || (_p->_TopLevelTypeDefs.size() != _p->_TempTopLevelTypeDefs.size()))
	{
		GetFlatTypeDefs(_p->_TopLevelTypeDefs, _p->_SortedTopicTypeDefs);

		if (_p->_SortedTopicTypeDefs.size())
		{
			CompositorTopic(_p->_SortedTopicTypeDefs);
		}

		_p->_TempTopLevelTypeDefs = _p->_TopLevelTypeDefs;
	}

   //将主题的名字存取 DefTypeName 变量中
    return _p->_SortedTopicTypeDefs;
}

std::vector<TSTypeDefWithFuncAndFiledPtr> TSBuiltinPlugin::GetAllTypeDefWithFuncAndFiled()
{
	if (!_p->_SortedTypeDefs.size())
	{
		GetFlatTypeDefs(_p->_TopLevelTypeDefs, _p->_SortedTypeDefs);

		if (_p->_SortedTypeDefs.size())
		{
			Compositor(_p->_SortedTypeDefs);
		}
	}
     
	return _p->_SortedTypeDefs;
}

std::vector<TSExtendDefPtr> TSBuiltinPlugin::GetAllExtendDefs()
{
	std::vector<TSExtendDefPtr> Defs;
	GetFlatTypeDefs(_p->_TopLevelTypeDefs,Defs);
	return Defs;
}

std::vector<TSTypeDefPtr> TSBuiltinPlugin::GetAllTypes()
{
    std::vector<TSTypeDefPtr> Defs;
    GetFlatTypeDefs(_p->_TopLevelTypeDefs,Defs);
 
    return Defs;
}

std::vector<TSTypeDefineLinkPtr> TSBuiltinPlugin::GetAllTypeDefineDefs()
{
    std::vector<TSTypeDefineLinkPtr> Defs;
    GetFlatTypeDefs(_p->_TopLevelTypeDefs,Defs);
    return Defs;
}
//对外接口，返回所有主题名字字符串
std::map<TSString, TSString> TSBuiltinPlugin::TransferTopicName()
{
	return _TopicNameMap;
}

std::map<std::pair<TSString, TSString>, TSString> TSBuiltinPlugin::TransferCurrentFileTopicName()
{
	return _CurrentFileTopicNameMap;
}

bool CompareTopic(TSTopicTypeDefPtr Left,TSTopicTypeDefPtr Right)
{
	TSExtendDefPtr ExtendDef = Left;
	while(ExtendDef = TS_CAST(ExtendDef->Extend.lock(),TSExtendDefPtr))
	{
		if(TSBuiltinPlugin::GetSuitFullTypeName(ExtendDef,true)
			== TSBuiltinPlugin::GetSuitFullTypeName(Right,true))
		{
			return false;
		}
	}

	return true;
}

bool TSBuiltinPlugin::CompositorTopic( std::vector<TSTopicTypeDefPtr> & Defs)
{
	size_t Deep = TSIDLPluginMgr::Instance()->GetSortDeep();
	size_t maxDeep = Defs.size() >  Deep ? Deep : Defs.size() - 1;
	for (size_t m = 0; m < maxDeep; m++)
	{
		for(size_t i = 0; i < Defs.size(); ++i)
		{
			for(size_t j = i + 1; j < Defs.size(); ++j)
			{
				if(!CompareTopic(Defs[i],Defs[j]))
				{
					TSTopicTypeDefPtr Temp = Defs[i];
					Defs[i] = Defs[j];
					Defs[j] = Temp;
				}
			}
		}
	}
	return true;
}

bool CompareTypeDefs(TSTypeDefWithFuncAndFiledPtr Left,TSTypeDefWithFuncAndFiledPtr Right)
{
	BOOST_FOREACH(TSFunctionDefPtr Func,Left->Functions)
	{
		if(TSBuiltinPlugin::GetSuitFullTypeName(Func->Def,true)
			== TSBuiltinPlugin::GetSuitFullTypeName(Right,true))
		{
			return false;
		}

		BOOST_FOREACH(TSParamDefPtr Param,Func->Params)
		{
			if(Param->Def->Type == TSTypeDef::Array)
			{
				TSVectorBuiltinTypeDefPtr Vec = TS_CAST(Param->Def,TSVectorBuiltinTypeDefPtr);

				if(TSBuiltinPlugin::GetSuitFullTypeName(Vec->Value,true)
					== TSBuiltinPlugin::GetSuitFullTypeName(Right,true))
				{
					return false;
				}
			}
			else
			{
				if(TSBuiltinPlugin::GetSuitFullTypeName(Param->Def,true)
					== TSBuiltinPlugin::GetSuitFullTypeName(Right,true))
				{
					return false;
				}
			}
		}
	}

	BOOST_FOREACH(TSFieldDefPtr Filed,Left->Fileds)
	{
		if(Filed->Def->Type == TSTypeDef::Array)
		{
			TSVectorBuiltinTypeDefPtr Vec = TS_CAST(Filed->Def,TSVectorBuiltinTypeDefPtr);
			
			if(TSBuiltinPlugin::GetSuitFullTypeName(Vec->Value,true)
				== TSBuiltinPlugin::GetSuitFullTypeName(Right,true))
			{
				return false;
			}
		}
		else if(TSBuiltinPlugin::GetSuitFullTypeName(Filed->Def,true)
			== TSBuiltinPlugin::GetSuitFullTypeName(Right,true))
		{
			return false;
		}
	}

	TSExtendDefPtr ExtendDef = Left;
	while(ExtendDef = TS_CAST(ExtendDef->Extend.lock(),TSExtendDefPtr))
	{
		if(TSBuiltinPlugin::GetSuitFullTypeName(ExtendDef,true)
			== TSBuiltinPlugin::GetSuitFullTypeName(Right,true))
		{
			return false;
		}
	}

	return true;
}

bool TSBuiltinPlugin::Compositor( std::vector<TSTypeDefWithFuncAndFiledPtr> & Defs)
{
	size_t Deep = TSIDLPluginMgr::Instance()->GetSortDeep();
	size_t maxDeep = Defs.size() >  Deep ? Deep : Defs.size() - 1;
	for (size_t m = 0; m < maxDeep; m++)
	{
		for(size_t i = 0; i < Defs.size(); ++i)
		{
			for(size_t j = i + 1; j < Defs.size(); ++j)
			{
				if(!CompareTypeDefs(Defs[i],Defs[j]))
				{
					TSTypeDefWithFuncAndFiledPtr Temp = Defs[i];
					Defs[i] = Defs[j];
					Defs[j] = Temp;
				}
			}
		}
	}

	return true;
}


void TSBuiltinPlugin::DoParseFinished_p( TSIDLGram::ParserPtr Parse /*xidl文件相关变量，函数*/)
{
	const std::vector<TSIDLGram::TypeDefPtr> &ParseDefs = Parse->TopLevelTypeDefs;

	BOOST_FOREACH(TSIDLGram::TypeDefPtr Def,ParseDefs)
	{
		//确定类型：Package or Service or Struct or Topic
		TSTypeDefPtr TopLevelTypeDef = CreateTypeDefByTypeInfoName(Def->TypeInfoName);
		TopLevelTypeDef->Parse = Parse;

		if(TopLevelTypeDef->Type == TSTypeDef::Package)
		{
			BuildPackageDef(TSPackageDefPtr(),TS_STATIC_CAST(TopLevelTypeDef,TSPackageDefPtr),Def);
		}
		else
		{
			BuildTypeDef(TopLevelTypeDef, Def);
		}
		//package 名，topic 名， 存入_TopLevelTypeDefs
		_p->_TopLevelTypeDefs.push_back(TopLevelTypeDef);
	}

	std::cout << "=============" << std::endl;
	std::cout << "FileName: " << Parse->GetFullFileName() << std::endl;

	if (TSIDLPluginMgr::Instance()->IsLinkCodeGenTool())
	{
		//将所有的主题单独存储到一个容器中
		StoresAllTopicName();
		StoresCurrentFileTopicName();
		StoresAllServiceName();

		TSIDLPluginMgr::Instance()->SetTopicNameMap(TransferTopicName());
		TSIDLPluginMgr::Instance()->SetCurrentFileTopicNameMap(TransferCurrentFileTopicName());
		TSIDLPluginMgr::Instance()->SetServiceNameVec(TransferServiceName());
	}
}

bool TSBuiltinPlugin::IsImportTypeDef( TSTypeDefPtr TypeDef )
{
	ASSERT(TypeDef && "TypeDef");
	ASSERT(TypeDef->Parse.lock() && "TypeDef->Parse");
	return TSIDLPluginMgr::Instance()->IsImportParse(TypeDef->Parse.lock());
}

void TSBuiltinPlugin::PrepareBuild()
{
    const std::vector<TSTypeDefPtr> & TopLevelTypeDefs = GetTopLevelTypeDefs();

    BOOST_FOREACH(TSTypeDefPtr TypeDef,TopLevelTypeDefs)
    {
        try
        {
            if(TSPackageDefPtr Pkg = TS_CAST(TypeDef,TSPackageDefPtr))
            {
                PrepareBuildPackage_p(Pkg);
            }
            else
            {
                PrepareBuildTypeDef_p(TypeDef);
            }
        }
        catch (TSException &exp)
        {
            throw exp;
        }
        catch(...)
        {
            throw TSException(GB18030ToTSString("未知异常"));
        }
    }
}

void TSBuiltinPlugin::PrepareBuildTypeDef_p( TSTypeDefPtr TypeDef )
{
	if(TSExtendDefPtr ExtendTypeDef = TS_CAST(TypeDef,TSExtendDefPtr))
	{
		if(!ExtendTypeDef->ExtendTypeName.empty())
		{
			if(!ExtendTypeDef->Extend.lock())
			{
				ExtendTypeDef->Extend = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),ExtendTypeDef->ExtendTypeName);
                if (ExtendTypeDef->Extend.lock())
                {
                    if(ExtendTypeDef->Extend.lock()->Type == TSTypeDef::Enum ||
                        ExtendTypeDef->Extend.lock()->Type == TSTypeDef::Service)
                    {
                        TSString thr = GB18030ToTSString("不合法的继承对像:") + ExtendTypeDef->ExtendTypeName;
                        throw TSException(thr);
                    }
                    if (ExtendTypeDef->Type != TSTypeDef::Exception && 
                        ExtendTypeDef->Extend.lock()->Type == TSTypeDef::Exception)
                    {
                        TSString thr = GB18030ToTSString("不合法的继承对像:") + ExtendTypeDef->ExtendTypeName;
                        throw TSException(thr);
                    }
                }
			}
			
			if(!ExtendTypeDef->Extend.lock())
			{
                TSString exp = GB18030ToTSString("未定义的类型:") + ExtendTypeDef->ExtendTypeName,ExtendTypeDef;
				throw TSException(exp);
			}
		}
	}
    //typedefine类型
    if (TSTypeDefineLinkPtr DefineType = TS_CAST(TypeDef,TSTypeDefineLinkPtr))
    {
        if(!DefineType->Field->Def)
        {
            DefineType->Field->Def = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),DefineType->Field->DefTypeName,
                DefineType->Field->Node);

        }
        if (!DefineType->Field->Def)
        {
            TSString exp = GB18030ToTSString("未定义的类型:") + DefineType->TypeName;
            throw TSException(exp);
        }
    }

	if(TSTypeDefWithFiledPtr FiledTypeDef = TS_CAST(TypeDef,TSTypeDefWithFiledPtr))
	{
		const std::vector<TSFieldDefPtr> & Fileds = FiledTypeDef->Fileds;
		BOOST_FOREACH(TSFieldDefPtr Field,Fileds)
		{
			if(!Field->Def)
			{
				Field->Def = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),Field->DefTypeName,Field->Node);
			}
            
            if(!Field->Def)
            {
                TSString exp = GB18030ToTSString("未定义的类型:") + Field->DefTypeName;
				throw TSException(exp);
            }
            else if(TSMapBuiltinTypeDefPtr MapBuiltinTypeDef = TS_CAST(Field->Def,TSMapBuiltinTypeDefPtr))
            {
                MapBuiltinTypeDef->Key = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),MapBuiltinTypeDef->KeyTypeName,Field->Node);
                if (!MapBuiltinTypeDef->Value)
                {
                    MapBuiltinTypeDef->Value = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),MapBuiltinTypeDef->ValueTypeName,Field->Node);
                }
                else
                {
                    if (TSVectorBuiltinTypeDefPtr VectorBuiltinTypeDef = TS_CAST(MapBuiltinTypeDef->Value,TSVectorBuiltinTypeDefPtr))
                    {
                        VectorBuiltinTypeDef->Value = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),VectorBuiltinTypeDef->ValueTypeName,Field->Node);
                    }
                }            

                if (!MapBuiltinTypeDef->Value)
                {
                    TSString exp = GB18030ToTSString("未定义的类型:") + MapBuiltinTypeDef->ValueTypeName;
                    throw TSException(exp);
                }

                if (!MapBuiltinTypeDef->Key)
                {
                    TSString exp = GB18030ToTSString("未定义的类型:") + MapBuiltinTypeDef->KeyTypeName;
                    throw TSException(exp);
                }
            }
            else if(TSVectorBuiltinTypeDefPtr VectorBuiltinTypeDef = TS_CAST(Field->Def,TSVectorBuiltinTypeDefPtr))
            {
                VectorBuiltinTypeDef->Value = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),VectorBuiltinTypeDef->ValueTypeName,Field->Node);
                if (!VectorBuiltinTypeDef->Value)
                {
                    TSString exp = GB18030ToTSString("未定义的类型:") + VectorBuiltinTypeDef->ValueTypeName;
                    throw TSException(exp);
                }
            }
		}
       
	}
	
	if(TSTypeDefWithFunctionPtr FuncTypeDef = TS_CAST(TypeDef,TSTypeDefWithFunctionPtr))
	{
		const std::vector<TSFunctionDefPtr> & Funcs = FuncTypeDef->Functions;
		BOOST_FOREACH(TSFunctionDefPtr Func,Funcs)
		{  
            if (!Func->Def)
            {
                Func->Def = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),Func->DefTypeName,Func->Node);
            }
            if(!Func->Def)
            {
                ERROR_LOG_THROW(GB18030ToTSString("未定义的类型:") + Func->DefTypeName,Func);
            }
            else if (TSVectorBuiltinTypeDefPtr VectorBuiltinTypeDef = TS_CAST(Func->Def,TSVectorBuiltinTypeDefPtr))
            {
                VectorBuiltinTypeDef->Value = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),VectorBuiltinTypeDef->ValueTypeName,Func->Node);
                if (!VectorBuiltinTypeDef->Value)
                {
                    TSString exp = GB18030ToTSString("未定义的类型:") + VectorBuiltinTypeDef->ValueTypeName;
                    throw TSException(exp);
                }
            }
            else if (TSMapBuiltinTypeDefPtr MapBuiltinTypeDef = TS_CAST(Func->Def,TSMapBuiltinTypeDefPtr))
            {
                MapBuiltinTypeDef->Key = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),MapBuiltinTypeDef->KeyTypeName,Func->Node);
                MapBuiltinTypeDef->Value = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),MapBuiltinTypeDef->ValueTypeName,Func->Node);
                if (!MapBuiltinTypeDef->Value)
                {
                    TSString exp = GB18030ToTSString("未定义的类型:") + MapBuiltinTypeDef->ValueTypeName;
                    throw TSException(exp);
                }
                if (!MapBuiltinTypeDef->Key)
                {
                    TSString exp = GB18030ToTSString("未定义的类型:") + MapBuiltinTypeDef->KeyTypeName;
                    throw TSException(exp);
                }
            }

            BOOST_FOREACH(TSTypeDefPtr ExceptionDef,Func->Exceptions)
            {
                 ExceptionDef = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),ExceptionDef->TypeName,Func->Node);
  
                if (ExceptionDef->Type != TSTypeDef::Exception)
                {
                    TSString exp = GB18030ToTSString("未定义异常类型:") + ExceptionDef->TypeName;
                    throw TSException(exp);
                }
            }

            BOOST_FOREACH(TSParamDefPtr Param, Func->Params)
            {
                if (!Param->Def)
                {
                    Param->Def = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),Param->DefTypeName,Param->Node);
                }
                if(!Param->Def)
                {
                    TSString exp = GB18030ToTSString("未定义的类型:") + Param->DefTypeName;
                    throw TSException(exp);
                }
                else if(TSVectorBuiltinTypeDefPtr VectorBuiltinTypeDef = TS_CAST(Param->Def,TSVectorBuiltinTypeDefPtr))
                {
                    VectorBuiltinTypeDef->Value = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),VectorBuiltinTypeDef->ValueTypeName,Param->Node);
                    if (!VectorBuiltinTypeDef->Value)
                    {
                        TSString exp = GB18030ToTSString("未定义的类型:") + VectorBuiltinTypeDef->ValueTypeName;
                        throw TSException(exp);
                    }
                }
                else if (TSMapBuiltinTypeDefPtr MapBuiltinTypeDef = TS_CAST(Param->Def,TSMapBuiltinTypeDefPtr))
                {
                    MapBuiltinTypeDef->Key = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),MapBuiltinTypeDef->KeyTypeName,Param->Node);
                    MapBuiltinTypeDef->Value = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),MapBuiltinTypeDef->ValueTypeName,Param->Node);
                    if (!MapBuiltinTypeDef->Value)
                    {
                        TSString exp = GB18030ToTSString("未定义的类型:") + MapBuiltinTypeDef->ValueTypeName;
                        throw TSException(exp);
                    }
                    if (!MapBuiltinTypeDef->Key)
                    {
                        TSString exp = GB18030ToTSString("未定义的类型:") + MapBuiltinTypeDef->KeyTypeName;
                        throw TSException(exp);
                    }
                }
            }
		}
	}
   
    if (TSTopicTypeDefPtr TopicTypeDef = TS_CAST(TypeDef,TSTopicTypeDefPtr))
    {
        if (!TopicTypeDef->Def.lock())
        {
            TopicTypeDef->Def = FindTypeDefByTypeName(TypeDef->SupperPkg.lock(),TopicTypeDef->DefTypeName,TopicTypeDef->Node);
        }
        if (!TopicTypeDef->Def.lock())
        {
            TSString exp = GB18030ToTSString("未定义的类型:") + TopicTypeDef->DefTypeName;
            throw TSException(exp);
        }
    }
}

void TSBuiltinPlugin::PrepareBuildPackage_p( TSPackageDefPtr Pkg )
{
	BOOST_FOREACH(TSPackageDefPtr Subordinate,Pkg->Subordinates)
	{
		PrepareBuildPackage_p(Subordinate);
	}

	BOOST_FOREACH(TSTypeDefPtr TypeDef,Pkg->TypeDefs)
	{
		PrepareBuildTypeDef_p(TypeDef);
	}
}

TSTypeDefPtr TSBuiltinPlugin::FindTypeDefByTypeName( TSPackageDefPtr CurPackageDef,const TSString & TypeName ,TSIDLGram::Parser::Node * OwnerNode)
{
	TSTypeDefPtr RetTypeDef = CreateBuiltinTypeDefByTypeName(TypeName);
	bool IsFindCurrentPackage = false;
	if(!RetTypeDef)
	{
		if(CurPackageDef)
		{
			if(TSPackageDefPtr SupperPkg = FindPackageByTypeName(CurPackageDef,TypeName))
			{
				BOOST_FOREACH(TSTypeDefPtr TypeDef,SupperPkg->TypeDefs)
				{
					if(TypeDef->TypeName == GetShortTypeName(TypeName))
					{
                        if (TypeDef->Type == TSTypeDef::TypeDefine)
                        {
                            if (TSTypeDefineLinkPtr typeDefine = TS_CAST(TypeDef,TSTypeDefineLinkPtr))
                            {
                                if (TSIDLPluginMgr::Instance()->GetCodeType() == "c++")
                                {
                                    RetTypeDef = typeDefine;
                                }
                                else
                                {
                                    RetTypeDef = typeDefine->Field->Def;
                                }
                                
                            }
                        }
                        else
                        {
                            RetTypeDef = TypeDef;
                        }
                        IsFindCurrentPackage = true;
						break;
					}
				}
			}
		}

		if(!IsFindCurrentPackage)
		{
			//顶级节点
			const std::vector<TSTypeDefPtr> & TopLevelTypeDefs = GetTopLevelTypeDefs();
			BOOST_FOREACH(TSTypeDefPtr TypeDef,TopLevelTypeDefs)
			{
				if(RetTypeDef)
				{
					break;
				}

				if(TSPackageDefPtr PackageDef = TS_CAST(TypeDef,TSPackageDefPtr))
				{
                    std::vector<TSString> TypeNamePackageNames = TSMetaType4Link::Split(TypeName,PACKAGE_SPLIT,false);
                    TypeNamePackageNames.pop_back();
                    if (!TypeNamePackageNames.size())
                    {
                        continue;
                    }
					if(TSPackageDefPtr TypeDefPackageDef = FindPackageByTypeName(PackageDef,TypeName))
					{
						BOOST_FOREACH(TSTypeDefPtr PkgTypeDef,TypeDefPackageDef->TypeDefs)
						{
							if(PkgTypeDef->TypeName == GetShortTypeName(TypeName))
							{
                                if (PkgTypeDef->Type == TSTypeDef::TypeDefine)
                                {
                                    if (TSTypeDefineLinkPtr typeDefine = TS_CAST(PkgTypeDef,TSTypeDefineLinkPtr))
                                    {
                                        if (TSIDLPluginMgr::Instance()->GetCodeType() == "c++")
                                        {
                                            RetTypeDef = typeDefine;
                                        }
                                        else
                                        {
                                            RetTypeDef = typeDefine->Field->Def;
                                        }
                                    }
                                }
                                else
                                {
                                    RetTypeDef = PkgTypeDef;
                                }
                                break;
							}
						}
					}

					if(!RetTypeDef)
					{
						BOOST_FOREACH(TSTypeDefPtr PkgTypeDef,PackageDef->TypeDefs)
						{
							if(PkgTypeDef->TypeName == GetShortTypeName(TypeName))
							{
                                if (PkgTypeDef->Type == TSTypeDef::TypeDefine)
                                {
                                    if (TSTypeDefineLinkPtr typeDefine = TS_CAST(PkgTypeDef,TSTypeDefineLinkPtr))
                                    {
                                        if (TSIDLPluginMgr::Instance()->GetCodeType() == "c++")
                                        {
                                            RetTypeDef = typeDefine;
                                        }
                                        else
                                        {
                                            RetTypeDef = typeDefine->Field->Def;
                                        }
                                    }
                                }
                                else
                                {
                                    RetTypeDef = PkgTypeDef;
                                }
                                break;
							}
						}
					}
				}
				else
				{
					if(TypeDef->TypeName == GetShortTypeName(TypeName))
					{
                        if (TypeDef->Type == TSTypeDef::TypeDefine)
                        {
                            if (TSTypeDefineLinkPtr typeDefine = TS_CAST(TypeDef,TSTypeDefineLinkPtr))
                            {
                                if (TSIDLPluginMgr::Instance()->GetCodeType() == "c++")
                                {
                                    RetTypeDef = typeDefine;
                                }
                                else
                                {
                                    RetTypeDef = typeDefine->Field->Def;
                                }
                            }
                        }
                        else
                        {
                            RetTypeDef = TypeDef;
                        }
                        break;
					}
				}
			}
		}
	}
	else 
	{
		RetTypeDef->Node = OwnerNode;
	}

	if(!RetTypeDef)
	{
		int a = 0;
	}

	return RetTypeDef;
}

TSPackageDefPtr TSBuiltinPlugin::FindPackageByTypeName( TSPackageDefPtr CurPackageDef,const TSString & TypeName )
{
	TSPackageDefPtr RetPackageDef;
	std::vector<TSString> TypeNamePackageNames = TSMetaType4Link::Split(TypeName,PACKAGE_SPLIT,false);
	if(TypeNamePackageNames.size())
	{
		TSString ShortTypeName = TypeNamePackageNames.back();
		TypeNamePackageNames.pop_back();

		if(!TypeNamePackageNames.size())
		{
			//没带命名空间
			BOOST_FOREACH(TSTypeDefPtr TypeDef,CurPackageDef->TypeDefs)
			{
				if(TypeDef->TypeName == TypeName)
				{
					RetPackageDef = CurPackageDef;
					break;
				}
			}
		}
		
        if(!RetPackageDef)
		{
			TSString TypeNamePkgFullName = boost::algorithm::join(TypeNamePackageNames,PACKAGE_SPLIT);

			//1、找到所有本命名空间的定义
			std::vector<TSPackageDefPtr> CurPackageDefs;
			GetAllPackageDefs_p(CurPackageDef,CurPackageDefs);

			//2 在本命名空间内找
			BOOST_FOREACH(TSPackageDefPtr CurDef,CurPackageDefs)
			{
				TSConUnorderedMap<TSString,TSPackageDefPtr> PkgMaps;
				BuildPackageTypeMap(CurDef,PkgMaps);
				TSConUnorderedMap<TSString,TSPackageDefPtr>::iterator it = PkgMaps.find(TypeName);
				if(it != PkgMaps.end())
				{
					RetPackageDef = it->second;
					break;
				}
			}

            if(!RetPackageDef)
            {
                //3 在父命名空间内找
                TSPackageDefPtr TempCurPackage = CurPackageDef;
                while(TSPackageDefPtr SupperPkg = TempCurPackage->SupperPkg.lock())
                {
                    std::vector<TSPackageDefPtr> SupperPackageDefs;
                    GetAllPackageDefs_p(SupperPkg,SupperPackageDefs);

                    //在父类命名空间内找
                    BOOST_FOREACH(TSPackageDefPtr SupperDef,SupperPackageDefs)
                    {
                        TSConUnorderedMap<TSString,TSPackageDefPtr> PkgMaps;
                        BuildPackageTypeMap(SupperDef,PkgMaps);
                        TSConUnorderedMap<TSString,TSPackageDefPtr>::iterator it = PkgMaps.find(TypeName);
                        if(it != PkgMaps.end())
                        {
                            RetPackageDef = it->second;
                            break;
                        }
                    }

                    if(RetPackageDef)
                    {
                        break;
                    }

                    TempCurPackage = SupperPkg;
                }

                //3 找不到在其它命名空间内找
                if(!RetPackageDef)
                {
                    const std::vector<TSPackageDefPtr> & PkgDefs = GetAllPackageDefs();
                    BOOST_FOREACH(TSPackageDefPtr Pkg,PkgDefs)
                    {
                        TSString FullPackageName = GetFullPackageNameByPackageDef(Pkg);
                        if(FullPackageName == TypeNamePkgFullName)
                        {
                            RetPackageDef = Pkg;
                            break;
                        }
                    }
                }
            }
		}
	}

	return RetPackageDef;	
}

TSString TSBuiltinPlugin::GetShortTypeName( const TSString & TypeName )
{
	std::vector<TSString> Names = TSMetaType4Link::Split(TypeName,PACKAGE_SPLIT,false);
	if(Names.size())
	{
		return Names.back();
	}	

	return TypeName;
}

void TSBuiltinPlugin::BuildPackageTypeMap( TSPackageDefPtr PkgDef,
	TSConUnorderedMap<TSString,TSPackageDefPtr> & BuildMap )
{
	if(PkgDef)
	{
		TSString FullPkgName = GetFullPackageNameByPackageDef(PkgDef);
		std::vector<TSString> PkgNames = TSMetaType4Link::Split(FullPkgName,PACKAGE_SPLIT,false);

		//倒序
		std::reverse(PkgNames.begin(),PkgNames.end());

		for(size_t i = 0; i <PkgNames.size(); ++i)
		{
			BOOST_FOREACH(TSTypeDefPtr TypeDef,PkgDef->TypeDefs)
			{
				std::vector<TSString> TempPkgNames;
				for(size_t j = 0; j <= i; ++j)
				{
					TempPkgNames.push_back(PkgNames[j]);
				}

				//倒过来
				std::reverse(TempPkgNames.begin(),TempPkgNames.end());

				TempPkgNames.push_back(TypeDef->TypeName);
				TSString TempFullPkgName = boost::algorithm::join(TempPkgNames,PACKAGE_SPLIT);
				BuildMap.insert(std::make_pair(TempFullPkgName,PkgDef));
			}
		}

		BOOST_FOREACH(TSTypeDefPtr TypeDef,PkgDef->TypeDefs)
		{
			BuildMap.insert(std::make_pair(TypeDef->TypeName,PkgDef));
		}
	}
}

void TSBuiltinPlugin::BuildBuiltinTypePair_p( TSTypePairPtr TypePair,TSIDLGram::Parser::Node *Node)
{
	if(IS_TYPE(Node,TSIDLGram::Array))
	{
		TypePair->Def = boost::make_shared<TSVectorBuiltinTypeDef>();
		TypePair->Def->Type = TSTypeDef::Array;
	}
    else if (IS_TYPE(Node,TSIDLGram::MapArray))
    {
        ConvertSafeTypeDef<TSMapBuiltinTypeDef>(TypePair->Def)->Value = boost::make_shared<TSVectorBuiltinTypeDef>();
        ConvertSafeTypeDef<TSMapBuiltinTypeDef>(TypePair->Def)->Value->Type = TSTypeDef::Array;
    }
    else if(IS_TYPE(Node,TSIDLGram::MapArrayVal))
    {
        ConvertSafeTypeDef<TSVectorBuiltinTypeDef>(ConvertSafeTypeDef<TSMapBuiltinTypeDef>(TypePair->Def)->Value)->ValueTypeName = 
            NodeToStringTrim(Node);
    }
	/*else if(IS_TYPE(Node,TSIDLGram::Map))
	{
		TypePair->Def = boost::make_shared<TSMapBuiltinTypeDef>();
		TypePair->Def->Type = TSTypeDef::Map;
	}*/
	else if(IS_TYPE(Node,TSIDLGram::ArrayVal))
	{
		ConvertSafeTypeDef<TSVectorBuiltinTypeDef>(TypePair->Def)->ValueTypeName = NodeToStringTrim(Node);
	}
	else if(IS_TYPE(Node,TSIDLGram::MapKey))
	{
		ConvertSafeTypeDef<TSMapBuiltinTypeDef>(TypePair->Def)->KeyTypeName = NodeToStringTrim(Node);
	}
	else if(IS_TYPE(Node,TSIDLGram::MapVal))
	{
		ConvertSafeTypeDef<TSMapBuiltinTypeDef>(TypePair->Def)->ValueTypeName = NodeToStringTrim(Node);
	}
	else if(IS_TYPE(Node,TSIDLGram::ComplexTypeNameIdent) || IS_TYPE(Node,TSIDLGram::XVOID))
	{
		//非容器
        if(TSFieldDefPtr field = TS_CAST(TypePair,TSFieldDefPtr))
        {
            if (NodeToStringTrim(Node) == "void"|| NodeToString(Node) == "Void")      
            {
                ERROR_LOG_THROW(GB18030ToTSString("类型错误") ,field);
            }
        }
		TypePair->Def = CreateBuiltinTypeDefByTypeName(NodeToStringTrim(Node));
	}
}

void TSBuiltinPlugin::BuildTopicDesc_p( TSTopicTypeDefPtr TypeDef,TSIDLGram::Parser::Node *Node )
{
	if(IS_TYPE(Node,TSIDLGram::String))
	{
		TypeDef->Desc = NodeToString(Node);
	}
}

void TSBuiltinPlugin::CheckoutPackageError(std::vector<TSTypeDefPtr> TopLevelTypeDefs)
{
    std::vector<TSString> packageNames;
    std::vector<TSString> typeNames;
    std::vector<TSString>::iterator iter_package;
    std::vector<TSPackageDefPtr> packagTypes;
    std::vector<TSTypeDefPtr> TypeEnum;
    BOOST_FOREACH(TSTypeDefPtr TypeDef,TopLevelTypeDefs)
    {   
        if(!TSIDLUtils::KeyWordSearch(TypeDef->TypeName))
        {
            throw TSException(GB18030ToTSString("命名非法，是保留的关键字:") + TypeDef->TypeName);
        }
        if(TSPackageDefPtr Pkg = TS_CAST(TypeDef,TSPackageDefPtr))
        {
            if (!packageNames.size())
            {
                packageNames.push_back(Pkg->TypeName);
                packagTypes.push_back(Pkg);
                std::vector<TSTypeDefPtr> PkgallType;
                if (Pkg->TypeDefs.size())
                {
                    PkgallType = Pkg->TypeDefs;
                }
                if (Pkg->Subordinates.size())
                {
                    PkgallType.insert(PkgallType.end(),Pkg->Subordinates.begin(),Pkg->Subordinates.end());
                }
                if (PkgallType.size())
                {
                    CheckoutPackageError(PkgallType);
                }              
            }
            else
            {
                iter_package = find(packageNames.begin(),packageNames.end(),Pkg->TypeName);
                if(iter_package == packageNames.end())
                {
                    packageNames.push_back(Pkg->TypeName);
                    packagTypes.push_back(Pkg);
                    std::vector<TSTypeDefPtr> PkgallType;
                    if (Pkg->TypeDefs.size())
                    {
                        PkgallType = Pkg->TypeDefs;
                    }
                    if (Pkg->Subordinates.size())
                    {
                        PkgallType.insert(PkgallType.end(),Pkg->Subordinates.begin(),Pkg->Subordinates.end());
                    }
                    if (PkgallType.size())
                    {
                        CheckoutPackageError(PkgallType);
                    }  
                }
                else
                {
                    BOOST_FOREACH(TSPackageDefPtr package,packagTypes)
                    {
                        if (package->TypeName == *iter_package)
                        {   
                            std::vector<TSTypeDefPtr> PkgallType;
                            if (Pkg->TypeDefs.size())
                            {
                                PkgallType.insert(PkgallType.end(),Pkg->TypeDefs.begin(),Pkg->TypeDefs.end());
                                PkgallType.insert(PkgallType.end(),package->TypeDefs.begin(),package->TypeDefs.end());
                            }
                            if (Pkg->Subordinates.size())
                            {
                                PkgallType.insert(PkgallType.end(),Pkg->Subordinates.begin(),Pkg->Subordinates.end());
                                PkgallType.insert(PkgallType.end(),package->Subordinates.begin(),package->Subordinates.end());
                            }
                            if (PkgallType.size())
                            {
                                CheckoutPackageError(PkgallType);
                            }                             
                        }
                    }
                }
            }
        }
        else
        {
            if (!typeNames.size())
            {
                typeNames.push_back(TypeDef->TypeName);
                TypeEnum.push_back(TypeDef);
            }
            else
            {
                BOOST_FOREACH(TSTypeDefPtr Def,TypeEnum)
                {
                    if ((Def->Type == TypeDef->Type) &&
                        (TypeDef->TypeName == Def->TypeName))
                    {
                        ERROR_LOG_THROW(GB18030ToTSString("重复的类型名:") + TypeDef->TypeName,TypeDef);
                    }
                    else
                    {
                        continue;
                    }
                }
                TypeEnum.push_back(TypeDef);
            }
        }
    
    }
}

void TSBuiltinPlugin::CheckoutTypeError()
{
    std::vector<TSEnumDefPtr> EnumTypeDefs = GetAllEnumDefs();
    BOOST_FOREACH(TSEnumDefPtr EnumDef,EnumTypeDefs)
    {
        std::vector<TSString> typeNames;
        std::vector<TSString>::iterator ite_Type;
        BOOST_FOREACH(TSEnumFiledDefPtr field,EnumDef->Fileds)
        {
            if(!TSIDLUtils::KeyWordSearch(field->ValName))
            {
                throw TSException(GB18030ToTSString("命名非法，是保留的关键字:") + field->ValName);
            }
            if (!typeNames.size())
            {
                typeNames.push_back(field->ValName);
            }
            else
            {
                ite_Type = find(typeNames.begin(),typeNames.end(),field->ValName);
                if (ite_Type == typeNames.end())
                {
                    typeNames.push_back(field->ValName);
                }
                else
                {
                    ERROR_LOG_THROW(GB18030ToTSString("重复的枚举属性名:") + field->ValName,field);
                }
            }
        }
    }


    std::vector<TSTypeDefWithFiledPtr> TypeDefs = GetAllTypeDefWithFileds();
    BOOST_FOREACH(TSTypeDefWithFiledPtr TypeDef,TypeDefs)
    {
        std::vector<TSString> typeNames;
        std::vector<TSString>::iterator ite_Type;
        BOOST_FOREACH(TSFieldDefPtr field,TypeDef->Fileds)
        {
            if(!TSIDLUtils::KeyWordSearch(field->ValName))
            {
                throw TSException(GB18030ToTSString("命名非法，是保留的关键字:") + field->ValName);
            }
            if (!typeNames.size())
            {
                typeNames.push_back(field->ValName);
            }
            else
            {
                ite_Type = find(typeNames.begin(),typeNames.end(),field->ValName);
                if (ite_Type == typeNames.end())
                {
                    typeNames.push_back(field->ValName);
                }
                else
                {
                    ERROR_LOG_THROW(GB18030ToTSString("重复的属性类型名:") + field->ValName,field);
                }
            }
        }
    }

    std::vector<TSTypeDefWithFunctionPtr> funDefs = GetAllTypeDefWithFunctions();
    BOOST_FOREACH(TSTypeDefWithFunctionPtr TypeDef,funDefs)
    {
        std::vector<TSString> typeNames;
        std::vector<TSString>::iterator ite_Type;
        BOOST_FOREACH(TSFunctionDefPtr func,TypeDef->Functions)
        {
            if(!TSIDLUtils::KeyWordSearch(func->ValName))
            {
                throw TSException(GB18030ToTSString("命名非法，是保留的关键字:") + func->ValName);
            }
            if (!typeNames.size())
            {
                typeNames.push_back(func->ValName);
            }
            else
            {
                ite_Type = find(typeNames.begin(),typeNames.end(),func->ValName);
                if (ite_Type == typeNames.end())
                {
                    typeNames.push_back(func->ValName);
                }
                else
                {
                    ERROR_LOG_THROW(GB18030ToTSString("重复的函数名:") + func->ValName,func);
                }
            }

            BOOST_FOREACH(TSParamDefPtr param,func->Params)
            {
                if (!TSIDLUtils::KeyWordSearch(param->ValName))
                {
                    throw TSException(GB18030ToTSString("命名非法，是保留的关键字:") + param->ValName);
                }
            }
        }

        if(TypeDef->Type == TSTypeDef::Service)
        {
            if (TypeDef->Extend.lock())
            {
                throw TSException(TypeDef->TypeName + GB18030ToTSString("服务不支持继承"));
            }
        }
    }
}

void TSTypeDef::Print()
{
	TSString PrintMsg;
	PrintMsg += GB18030ToTSString("类型:") + TypeName + GB18030ToTSString("\n");

	TSIDLPluginMgr::Instance()->GetPipe()->Output(PrintMsg.c_str());
}

void TSPackageDef::Print()
{
	SupperType::Print();

	BOOST_FOREACH(TSPackageDefPtr Pkg,Subordinates)
	{
		Pkg->Print();
	}

	BOOST_FOREACH(TSTypeDefPtr TypeDef,TypeDefs)
	{
		TypeDef->Print();
	}
}

void TSExtendDef::Print()
{
	SupperType::Print();

	TSString PrintMsg;
	PrintMsg += GB18030ToTSString("继承自:") + ExtendTypeName + GB18030ToTSString("\n");

	TSIDLPluginMgr::Instance()->GetPipe()->Output(PrintMsg.c_str());
}

void TSTypePair::Print()
{
	TSString PrintMsg;
	PrintMsg += GB18030ToTSString("\n类型名:") + DefTypeName + GB18030ToTSString("\n");
	PrintMsg += GB18030ToTSString("\n变量名:") + ValName + GB18030ToTSString("\n");

	TSIDLPluginMgr::Instance()->GetPipe()->Output(PrintMsg.c_str());
}

void TSEnumFiledDef::Print()
{
	TSString PrintMsg;
	PrintMsg += GB18030ToTSString("变量名:") + ValName + GB18030ToTSString("\n");
	PrintMsg += GB18030ToTSString("默认值:") + DefaultVal + GB18030ToTSString("\n");

	TSIDLPluginMgr::Instance()->GetPipe()->Output(PrintMsg.c_str());
}

TSEnumFiledDef::TSEnumFiledDef()
	:Node(NULL)
{

}

void TSFieldDef::Print()
{
	SupperType::Print();

	TSString PrintMsg;
	PrintMsg += GB18030ToTSString("描述:")   + Desc + GB18030ToTSString("\n");
	PrintMsg += GB18030ToTSString("默认值:") + DefaultVal + GB18030ToTSString("\n");

	TSIDLPluginMgr::Instance()->GetPipe()->Output(PrintMsg.c_str());
}

void TSParamDef::Print()
{
	SupperType::Print();
}

void TSFunctionDef::Print()
{
	SupperType::Print();

	BOOST_FOREACH(TSParamDefPtr Param,Params)
	{
		Param->Print();
	}
}

void TSTypeDefWithFiled::Print()
{
	SupperType::Print();

	BOOST_FOREACH(TSFieldDefPtr Field,Fileds)
	{
		Field->Print();
	}
}

void TSTypeDefWithFunction::Print()
{
	SupperType::Print();

	BOOST_FOREACH(TSFunctionDefPtr Func,Functions)
	{
		Func->Print();
	}
}

void TSTypeDefWithFuncAndFiled::Print()
{
    
}

void TSServiceDef::Print()
{
	
}

void TSStructDef::Print()
{
	
}

void TSEnumDef::Print()
{
	


}

void TSMapBuiltinTypeDef::Print()
{

}

void TSVectorBuiltinTypeDef::Print()
{

}

void TSBuiltinTypeDef::Print()
{

}

void TSExceptionDef::Print()
{

}

void TSTypeDefineLink::Print()
{
    
}

TSLogNodeDef::TSLogNodeDef()
	:Node(NULL)
{

}

