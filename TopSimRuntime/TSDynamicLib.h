#ifndef __TSBUILTINPLUGIN_H__
#define __TSBUILTINPLUGIN_H__

#include "TSIDLPluginMgr.h"

#define PACKAGE_SPLIT TS_TEXT("::")
#define PACKAGE_SPLIT_4CS TS_TEXT(".")
#define FILEDIRCPP TS_TEXT("../..")

#define IS_MATCH(a,b) strcmp(a,typeid(b).name()) == 0

CLASS_PTR_DECLARE(TSIBuiltinPlugin);
CLASS_PTR_DECLARE(TSPackageDef);
CLASS_PTR_DECLARE(TSTypeDef);
CLASS_PTR_DECLARE(TSTypePair);
CLASS_PTR_DECLARE(TSTypeDefWithFiled);
CLASS_PTR_DECLARE(TSTypeDefWithFunction);
CLASS_PTR_DECLARE(TSTypeDefWithFuncAndFiled);
CLASS_PTR_DECLARE(TSFunctionDef);
CLASS_PTR_DECLARE(TSFieldDef);
CLASS_PTR_DECLARE(TSParamDef);
CLASS_PTR_DECLARE(TSStructDef);
CLASS_PTR_DECLARE(TSServiceDef);
CLASS_PTR_DECLARE(TSEnumFiledDef);
CLASS_PTR_DECLARE(TSEnumDef);
CLASS_PTR_DECLARE(TSExtendDef);
CLASS_PTR_DECLARE(TSVectorBuiltinTypeDef);
CLASS_PTR_DECLARE(TSMapBuiltinTypeDef);
CLASS_PTR_DECLARE(TSBuiltinTypeDef);
CLASS_PTR_DECLARE(TSLogNodeDef);
CLASS_PTR_DECLARE(TSTopicTypeDef);
CLASS_PTR_DECLARE(TSExceptionDef);
CLASS_PTR_DECLARE(TSTypeDefineLink);

enum TypeKind
{
    InterfaceType = 1 << 1,
    ClassIdentity = 1 << 2,
    ParameterType = 1 << 3,
};

struct TopStruct
{
	//主题名称
	TSString topic;
	//主题对应的父主题
	TSString fatherTopic;
	//主题定义
	TSString topicDefine;
};

class TOPSIMIDL_DECL TSLogNodeDef
{
public:
	TSLogNodeDef();
	TSIDLGram::Parser::Node * Node;
};

class TOPSIMIDL_DECL TSTypeDef : public TSLogNodeDef
{
public:
	enum EnumTypeDef
	{
		Enum,
		Struct,
		Service,
		Package,
		Topic,
		Map,
		Array,
		Builtin,
        Exception,
        Control,
        Component,
        Condition,
        Action,
        BBkeydelcear,
        TypeDefine,
		HeaderStruct,
		Assertdeclear,
		Resourcedeclear,
		FSMComponent
	};

	TSPackageDefWPtr           SupperPkg;
	EnumTypeDef                Type;
	TSString                   TypeName;
	TSString				   TypeDescName;
	TSIDLGram::ParserWPtr     Parse;

	virtual void Print();
};

class TOPSIMIDL_DECL TSBuiltinTypeDef : public TSTypeDef
{
public:
	TSVariant::Type BuiltinType;

	virtual void Print();
};

class TOPSIMIDL_DECL TSMapBuiltinTypeDef : public TSTypeDef
{
public:
	TSTypeDefPtr Key;
	TSString     KeyTypeName;
	TSTypeDefPtr Value;
	TSString     ValueTypeName;

	virtual void Print();
};

class TOPSIMIDL_DECL TSVectorBuiltinTypeDef: public TSTypeDef
{
public:
	TSTypeDefPtr Value;
	TSString     ValueTypeName;
	virtual void Print();
};

class TOPSIMIDL_DECL TSPackageDef : public TSTypeDef
{
public:
	typedef TSTypeDef SupperType;

	std::vector<TSPackageDefPtr> Subordinates;
	std::vector<TSTypeDefPtr>    TypeDefs;

	virtual void Print();
};
//主题的父类
class TOPSIMIDL_DECL TSExtendDef : public TSTypeDef
{
public:
	typedef TSTypeDef SupperType;

	TSString       ExtendTypeName;
	TSTypeDefWPtr  Extend;

	virtual void Print();
};

class TOPSIMIDL_DECL TSTypePair : public TSLogNodeDef
{
public:
	TSTypeDefPtr               Def;
	TSString                   DefTypeName;
	TSString                   ValName;

	virtual void Print();
};

class TOPSIMIDL_DECL TSEnumFiledDef
{
public:
	TSEnumFiledDef();
    TSString              ValName;//整个字段
	TSString              DefaultVal;//默认值
    TSString              Name;//枚举名字
	std::vector<TSString> Comments;
	TSIDLGram::Parser::Node * Node;

	virtual void Print();
};

class TOPSIMIDL_DECL TSFieldDef : public TSTypePair
{
public:
	typedef TSTypePair SupperType;


	FORCEINLINE TSFieldDef()
		:FindSetEnumType(0),FindSet(None),HaveKey(false)
	{
	    
	}

	enum EnumFindSet
	{
		None             = 0,
		Key              = 1 << 1,
		SpacialRange     = 1 << 2,
		ModelOrTmplId    = 1 << 3,
        Handle           = 1 << 4,
	};
    bool        HaveKey;
	EnumFindSet FindSet;
	TSString    Desc;
	TSString    DefaultVal;
	TSString    Flag;
	int         FindSetEnumType;
	virtual void Print();
};


class TOPSIMIDL_DECL TSParamDef : public TSTypePair
{
public:
	typedef TSTypePair SupperType;
    TSParamDef():Type(In)
    {
    
    }

	enum EnumTypeDef
	{
		In,
		Inout,
		Out,
        Void,
	};

	EnumTypeDef Type;

	virtual void Print();
};


class TOPSIMIDL_DECL TSFunctionDef : public TSTypePair
{
public:
	typedef TSTypePair SupperType;

    std::vector<TSTypeDefPtr>            Exceptions;
	std::vector<TSParamDefPtr>           Params;
	std::vector<TSString>                ExceptionTypeNames;

	virtual void Print();
};

class TOPSIMIDL_DECL TSTypeDefWithFiled : virtual public TSExtendDef
{
public:
	typedef TSExtendDef SupperType;

	std::vector<TSFieldDefPtr> Fileds;

	virtual void Print();
};

class TOPSIMIDL_DECL TSTypeDefWithFunction : virtual public TSExtendDef
{
public:
	typedef TSExtendDef SupperType;

	std::vector<TSFunctionDefPtr> Functions;

	virtual void Print();
};

class TOPSIMIDL_DECL TSTypeDefWithFuncAndFiled : virtual public TSTypeDefWithFunction,
    virtual public TSTypeDefWithFiled
{
public:
    virtual void Print();
};

class TOPSIMIDL_DECL TSServiceDef : virtual public TSTypeDefWithFuncAndFiled
{
public:
	virtual void Print();
};

class TOPSIMIDL_DECL TSStructDef : virtual public TSTypeDefWithFuncAndFiled
{
public:
	virtual void Print();
};

class TOPSIMIDL_DECL TSEnumDef : virtual public TSTypeDef
{
public:
	typedef TSTypeDef SupperType;
	std::vector<TSEnumFiledDefPtr> Fileds;

	virtual void Print();
};

class TOPSIMIDL_DECL TSExceptionDef : virtual public TSTypeDefWithFiled
{
    virtual void Print();
};

class TOPSIMIDL_DECL TSTopicTypeDef : public TSExtendDef
{
public:
	typedef TSTypeDef TSExtendDef;

	TSTypeDefWPtr Def;
	TSString      Desc;
	TSString      DefTypeName;
};

class TOPSIMIDL_DECL TSTypeDefineLink : virtual public TSTypeDef
{
public:
    TSFieldDefPtr Field;
    virtual void Print();
};


class TOPSIMIDL_DECL TSIBuiltinPlugin : public TSIIDLPlugin
{
	TS_MetaType(TSIBuiltinPlugin,TSIIDLPlugin);
	
public:
	virtual TSPackageDefPtr                                             FindPackageByTypeName(TSPackageDefPtr CurPackageDef,const TSString & TypeName) = 0;
	virtual TSTypeDefPtr                                                   FindTypeDefByTypeName(TSPackageDefPtr CurPackageDef,const TSString & TypeName,TSIDLGram::Parser::Node * OwnerNode = NULL) = 0;
	virtual bool                                                                  IsImportTypeDef(TSTypeDefPtr TypeDef) = 0;
	virtual TSString                                                           GetShortTypeName(const TSString & TypeName) = 0;
	virtual const std::vector<TSTypeDefPtr> &               GetTopLevelTypeDefs() = 0;
	virtual std::vector<TSPackageDefPtr>                       GetAllPackageDefs() = 0;
	virtual std::vector<TSEnumDefPtr>                            GetAllEnumDefs() = 0;
	virtual std::vector<TSServiceDefPtr>                          GetAllServiceDefs() = 0;
	virtual std::vector<TSStructDefPtr>                            GetAllStructDefs() = 0;
	virtual std::vector<TSTypeDefWithFunctionPtr>        GetAllTypeDefWithFunctions() = 0;
	virtual std::vector<TSTypeDefWithFiledPtr>               GetAllTypeDefWithFileds() = 0;
    virtual std::vector<TSTypeDefWithFuncAndFiledPtr> GetAllTypeDefWithFuncAndFiled() = 0;
	virtual std::vector<TSExtendDefPtr>                           GetAllExtendDefs() = 0;
	virtual std::vector<TSTopicTypeDefPtr>                     GetAllTopicTypeDefs() = 0;
    virtual std::vector<TSTypeDefPtr>                              GetAllTypes() = 0;
    virtual std::vector<TSTypeDefineLinkPtr>                  GetAllTypeDefineDefs() = 0;
    

	static TSString             GetSuitFullTypeName(TSTypeDefPtr TypeDef,bool getSpace = false,TypeKind Kind = InterfaceType);
	static TSString             GetFullPackageNameByPackageDef(TSPackageDefPtr Pkg);
	static TSTypeDefPtr    CreateTypeDefByTypeInfoName(const char * TypeInfoName);
	static TSTypeDefPtr    CreateBuiltinTypeDefByTypeName(const TSString & BuiltinTypeName);
	static const char *       GetTypeInfoNameByTypeName(const TSString & TypeName);
	static TSVariant::Type GetBuiltinTypeFromTypeName(const TSString & TypeName);
};

class TSBuiltinPluginPrivate;
class TOPSIMIDL_DECL TSBuiltinPlugin : public TSIBuiltinPlugin
{
	TS_MetaType(TSBuiltinPlugin,TSIBuiltinPlugin);
public:
	TSBuiltinPlugin(void);
	~TSBuiltinPlugin(void);

public:
	virtual TSPackageDefPtr                                             FindPackageByTypeName(TSPackageDefPtr CurPackageDef,const TSString & TypeName);
	virtual TSTypeDefPtr                                                   FindTypeDefByTypeName(TSPackageDefPtr CurPackageDef,const TSString & TypeName,TSIDLGram::Parser::Node * OwnerNode = NULL);
	virtual TSString                                                           GetShortTypeName(const TSString & TypeName);
	virtual bool                                                                  IsImportTypeDef(TSTypeDefPtr TypeDef);
	virtual const std::vector<TSTypeDefPtr> &                GetTopLevelTypeDefs();
	virtual std::vector<TSPackageDefPtr>                        GetAllPackageDefs();
	virtual std::vector<TSEnumDefPtr>                            GetAllEnumDefs();
	virtual std::vector<TSServiceDefPtr>                          GetAllServiceDefs();
	virtual std::vector<TSStructDefPtr>                            GetAllStructDefs();
	virtual std::vector<TSTypeDefWithFunctionPtr>        GetAllTypeDefWithFunctions();
	virtual std::vector<TSTypeDefWithFiledPtr>               GetAllTypeDefWithFileds();
    virtual std::vector<TSTypeDefWithFuncAndFiledPtr> GetAllTypeDefWithFuncAndFiled();
	virtual std::vector<TSExtendDefPtr>                           GetAllExtendDefs();
	virtual std::vector<TSTopicTypeDefPtr>                     GetAllTopicTypeDefs();
    virtual std::vector<TSTypeDefPtr>                              GetAllTypes();
    virtual void                                                                   CheckoutPackageError(std::vector<TSTypeDefPtr> TopLevelTypeDefs);
    virtual void                                                                   CheckoutTypeError();
    virtual void                                                                   CleanTopLevelTypeDefs();
    virtual std::vector<TSTypeDefineLinkPtr>                   GetAllTypeDefineDefs();
	virtual std::map<TSString, TSString>                          TransferTopicName();
	std::map<std::pair<TSString, TSString>, TSString>	TransferCurrentFileTopicName();
	virtual std::vector<TSString>                                       TransferServiceName();
	
	//存储所有的主题名字及其父类主题(父类主题不存在则父类主题为空)
	std::map<TSString, TSString> _TopicNameMap;
	std::map<std::pair<TSString, TSString>, TSString> _CurrentFileTopicNameMap;
	std::vector<TSString>             _ServiceNameVec;

protected:
	virtual TSString       GetName();
	virtual bool             DoParse();
	virtual bool             DoBuild();
	virtual void             OnPluginLoaded();
	virtual void             Clean();
	template<typename T>
	boost::shared_ptr<T> ConvertSafeTypeDef(TSTypeDefPtr TypeDef)
	{
		ASSERT(TypeDef);
		return boost::dynamic_pointer_cast< typename boost::shared_ptr<T>::element_type >(TypeDef);
	}

private:
	TSBuiltinPluginPrivate * _p;

private:
	void PrepareBuild();
	void GetAllPackageDefs_p(TSPackageDefPtr CurPackageDef,std::vector<TSPackageDefPtr> & CurPackageDefs);
	void BuildPackageTypeMap(TSPackageDefPtr PkgDef,TSConUnorderedMap<TSString,TSPackageDefPtr> & BuildMap);
	void PrepareBuildTypeDef_p(TSTypeDefPtr TypeDef);
	void PrepareBuildPackage_p(TSPackageDefPtr Pkg);
	void DoParseFinished_p(TSIDLGram::ParserPtr Parse);
	void BuildTopicDesc_p(TSTopicTypeDefPtr TypeDef,TSIDLGram::Parser::Node *Node);
	void BuildBuiltinTypePair_p(TSTypePairPtr TypePair,TSIDLGram::Parser::Node *Node);
	void BuildPackageDef(TSPackageDefPtr SupperTypeDef,TSPackageDefPtr TypeDef,TSIDLGram::TypeDefPtr ParseTypeDef);
	void BuildTypeDef(TSTypeDefPtr TypeDef,TSIDLGram::TypeDefPtr ParseTypeDef);
	void BuildExtend_p(TSExtendDefPtr TypeDef,TSIDLGram::Parser::Node *Node);
	void BuildEnum_p(TSEnumFiledDefPtr Enum,TSIDLGram::Parser::Node *Node);
	void BuildDefaultVal_p(TSFieldDefPtr Field,TSIDLGram::Parser::Node *Node);
	void BuildFlagVal_p(TSFieldDefPtr Field,TSIDLGram::Parser::Node *Node);
	void BuildFindSetVal_p(TSFieldDefPtr Field,TSIDLGram::Parser::Node *Node);
	void BuildTypeDef_p(TSTypeDefPtr TypeDef,TSIDLGram::Parser::Node *Node);
	void BuildFiledDef_p(TSFieldDefPtr Field,TSIDLGram::Parser::Node *Node);
	void BuildFunction_p(TSFunctionDefPtr Func,TSIDLGram::Parser::Node *Node);
	void BuildFuncParam_p(TSParamDefPtr Param,TSIDLGram::Parser::Node *Node);
    bool Compositor( std::vector<TSTypeDefWithFuncAndFiledPtr> & Defs);
    bool CompositorTopic( std::vector<TSTopicTypeDefPtr> & Def);
	void Compositor1(std::vector<TSTypeDefWithFuncAndFiledPtr> & Defs,std::vector<TSTypeDefWithFuncAndFiledPtr> TempDefs,TSTypeDefWithFuncAndFiledPtr Def);
	void StoresAllTopicName();
	void StoresCurrentFileTopicName();
	void StoresAllServiceName();
};

#define ERROR_LOG(Msg,LogDef) NODE_ERROR_LOG(Msg,LogDef->Node)
#define ERROR_LOG_THROW(Msg,LogDef) NODE_ERROR_LOG(Msg,LogDef->Node);throw TSException(Msg);

#endif


