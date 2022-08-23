#ifndef __TSGRAMMAR_H__
#define __TSGRAMMAR_H__

#include "TopSimIDL.h"
#include "TSGramCommon.h"

namespace TSIDLGram 
{
    using namespace yard;
    using namespace text_grammar;
    using namespace TSGramCommon;

    struct COMPLEX_PARAM;
    struct Array;
    //struct Map;
    struct PluginContent;
    struct EnumSimpleVal;
    struct Package;

    struct ENUM         : Keyword<CharSeq<'e','n','u','m'> > {};
    struct STRUCT       : Keyword<CharSeq<'s','t','r','u','c','t'> > {};
    struct TOPIC        : Keyword<CharSeq<'t','o','p','i','c'> > {};
    struct SERVICE      : Keyword<CharSeq<'s','e','r','v','i','c','e'> >{};
    struct PACKAGE      : Keyword<CharSeq<'p','a','c','k','a','g','e'> >{};
    struct PLUGIN       : Keyword<Seq<TokC<Char<'#'> >,CharSeq<'p','l','u','g','i','n'> > >{};
    struct PLUGINEND    : Keyword<Seq<TokC<Char<'#'> >,CharSeq<'e','n','d'> > > {};
    struct IMPORT       : Keyword<Seq<TokC<Char<'#'> >,CharSeq<'i','m','p','o','r','t'> > >{};
    struct EXTENDS      : Keyword<CharSeq<'e','x','t','e','n','d','s'> >{};
    struct XEXCEPTION   : Keyword<CharSeq<'e','x','c','e','p','t','i','o','n'> > {};
    struct RAISES       : Keyword<CharSeq<'r','a','i','s','e','s'> > {};
    struct KEY          : Keyword<CharSeq<'K','e','y'> > {};
    struct DEFAULT      : Keyword<CharSeq<'D','e','f','a','u','l','t'> > {};
    struct FLAG         : Keyword<CharSeq<'F','l','a','g'> >{};
    struct FINDSET      : Keyword<CharSeq<'F','i','n','d','S','e','t'> >{};                
    struct HANDLE       : Keyword<Seq<CharSeq<'k','H','a','n','d','l','e'>,CharSeq<'T','y','p','e'> > >{};
    struct SPACIALRANGE : Keyword<Seq<CharSeq<'k','S','p','a','c','i','a','l','R','a','n','g','e'>,CharSeq<'T','y','p','e'> > >{};
    struct MODELORTMPLID: Keyword<Seq<CharSeq<'k','M','o','d','e','l','O','r','T','m','p','l','I','d'>,CharSeq<'T','y','p','e'> > >{};
    struct TRUESTR      : Keyword<CharSeq<'t','r','u','e'> > {};
    struct FALSESTR     : Keyword<CharSeq<'f','a','l','s','e'> > {};
    struct TYPEDEFINE   : Keyword<CharSeq<'t','y','p','e','d','e','f'>>{};
    struct INCLUDE      : Keyword<CharSeq<'i','n','c','l','u','d','e'>>{};
    struct TypeNameIdent : OneOrMore<STC<Ident>,CharSeq<':',':'> >{};
    struct TypeValueIdent : TypeNameIdent{};
    template<typename CharSeq_T>
    struct STDPrev : Or<Seq<TokC<CharSeq<'s','t','d'> >,TokC<CharSeq<':',':'> >,TokC<CharSeq_T> >,
        TokC<CharSeq_T> >{};

    struct ArrayVal : TypeNameIdent{};
    struct Array : Seq<TokC<STDPrev<CharSeq<'v','e','c','t','o','r'> > >,
        TokC<LBRACKET>,
        STC<Or<STC<Array>,/*STC<Map>,*/STC<ArrayVal> > >,
        TokC<RBRACKET> >{};

    struct TypeName   : Ident{};

    struct MapTypeName : Ident{};

    struct MapValIdent : Or<STC<TypeValueIdent>,STC<MapTypeName> >{};

    struct MapKey : TypeNameIdent{};

    struct MapArrayVal : TypeNameIdent{};

    struct MapArray : Seq<TokC<STDPrev<CharSeq<'v','e','c','t','o','r'> > >,
        TokC<LBRACKET>,
        STC<Or<STC<MapArray>,STC<MapArrayVal> > >,
        TokC<RBRACKET> >{};
    struct MapVal : Or<STC<MapArray>,/*STC<Map>,*/STC<MapValIdent>>{};
	/*
    struct Map : Seq<TokC<STDPrev<CharSeq<'m','a','p'> > >,
        TokC<LBRACKET>,
        STC<MapKey>,
        TokC<Char<','> >,
        STC<MapVal>,
        TokC<RBRACKET> >{};
	*/
    struct ComplexTypeNameIdent : TypeNameIdent{};
    struct TypeIdent  : Or<STC<Array>,/*STC<Map>,*/STC<ComplexTypeNameIdent> >{};

    struct ExtendIdent : Or<STC<EXTENDS>,Seq<Tok<Char<':'> >,STC<EXTENDS> > >{};
    struct ExtendTypeName : TypeNameIdent{};
    struct Extend : Seq<TokC<ExtendIdent>,STC<ExtendTypeName> >{};

    template<typename TypeKeyword,typename TypeContent>
    struct Type : Or<
        STC<Seq<STC<TypeKeyword>,STC<TypeName>,Or<NotAt<ExtendIdent>,STC<Extend> >,TokC<LBRACE>,TokC<RBRACE>,TokC<Char<';'> > > >,
        STC<Seq<STC<TypeKeyword>,STC<TypeName>,Or<NotAt<ExtendIdent>,STC<Extend> >,TokC<LBRACE>,STC<TypeContent>,TokC<RBRACE>,TokC<Char<';'> > > >
    >{};

    struct BitOp : Or<STC<LSHIFT>,STC<RSHIFT>,STC<XOR>,STC<AND> >{};
    struct Int :  Seq<Digit,Star<Digit> >{};

    struct EnumSimpleDefaultVal : Or<STC<EnumSimpleVal>,STC<Int> >{};
    struct EnumComplexDefaultVal : OneOrMore<STC<EnumSimpleDefaultVal>,STC<BitOp> >{};
    struct EnumSimpleVal : Ident{};
    struct EnumDefaultVal : Seq<STC<EnumSimpleVal>,TokC<Char<'='> >,Or<STC<EnumComplexDefaultVal>, STC<EnumSimpleDefaultVal> > >{};
    struct EnumVal : Or<STC<EnumDefaultVal>,STC<EnumSimpleVal> >{};

    struct EnumContent : 
        Seq<
        STC<EnumVal>,
        Star<Seq<TokC<Char<','> >,STC<EnumVal> > >,
        Opt<TokC<Char<','> > >, 
        Pick<Char<'}'> >
        >{}; 

    struct Number : Seq<Opt<Char<'-'> >,Digit, Star<Digit>,Opt<Char<'.'> >,Star<Digit> >{};
    struct DEFIDENTPARAM  : Ident{};
    struct SIMPLE_PARAM   : Or<STC<String>,STC<SChar>,STC<Number>,STC<TRUESTR>,STC<FALSESTR>,STC<DEFIDENTPARAM> >{};

    struct COMPLEX_PARAM  : Or<
		Seq<
		TokC<Ident>,
		Or< 
		OneOrMore<Seq<CharSeq<':',':'>,Ident > >,
		Seq< TokC<LPAREN>,STC<OneOrMore<Or<STC<COMPLEX_PARAM>,STC<SIMPLE_PARAM> > >  >,TokC<RPAREN> > > >,
		SIMPLE_PARAM>{};
    struct DefaultValParam : OneOrMore<STC<COMPLEX_PARAM> >{};
    struct DefaultVal     : Seq<TokC<DEFAULT >,TokC<Char<'='> >,STC<DefaultValParam> >{};
    struct DescVal        : String{};
    struct FlagValSimpleParam   : Ident{};
    struct FlagValParam   : OneOrMore<STC<FlagValSimpleParam>,TokC<Char<'|'> > >{};
    struct FlagVal        : Seq<TokC<FLAG>,TokC<Char<'='> >,STC<FlagValParam> >{};

    struct FindSetKey     : Seq<TokC<FINDSET>,TokC<Char<'='> >,STC<Or<STC<HANDLE>,STC<SPACIALRANGE>,STC<MODELORTMPLID> > > >{};
    struct FindSetVal     : Or<STC<FindSetKey>,STC<KEY> >{};
    struct FieldDescVal   : Or<STC<DefaultVal>,STC<FindSetVal>,STC<DescVal>,STC<FlagVal> >{};
    struct FieldDesc      : Seq<TokC<LSQUARE>,STC<OneOrMore<STC<FieldDescVal> > >,TokC<RSQUARE> > {};
    struct FieldTypeName  : TypeIdent{};
    struct FieldValName   : Ident{};
    struct SimpleField    : Seq<STC<FieldTypeName>,Or<Seq< STC<FieldValName>, NotAt<TokC<Char<':'> > > >, Seq<STC<FieldValName>, TokC<Char<':'> >, TokC<Digit > > >,TokC<Char<';'> > >{};
    struct Field          : Or<STC<SimpleField>,STC<Seq<STC<FieldDesc>,STC<SimpleField> > > >{};

    struct ExceptionTypeName : TypeIdent{};
    struct ExceptionDecl  : Seq<TokC<RAISES>,Opt<TokC<Char<'('> > >,STC<ExceptionTypeName>,Opt<TokC<Char<')'> > > >{};

    struct ParamDesc      : Or<STC<XINOUT>,STC<XOUT>,STC<XIN> >{};
    struct ParamTypeName  : TypeIdent{};
    struct ParamTypeVal   : Ident{};
    struct Param 
        : Or<
        STC<Seq<STC<ParamDesc>,STC<ParamTypeName>,STC<ParamTypeVal> > >,
        STC<Seq<STC<ParamTypeName>,STC<ParamTypeVal> > >,
        STC<XVOID>
        >{};

    struct FunctionName   : Ident{};
    struct FunctionReturnVal : TypeIdent{};

    struct Function 
        : Or<
        STC<Seq<STC<FunctionReturnVal>,STC<FunctionName>,TokC<LPAREN>,STC<OneOrMore<STC<Param> > >,TokC<RPAREN>,STC<Star<STC<ExceptionDecl> > >,TokC<Char<';'> > > >,
        STC<Seq<STC<FunctionReturnVal>,STC<FunctionName>,TokC<LPAREN>,TokC<RPAREN>,STC<Star<STC<ExceptionDecl> > >,TokC<Char<';'> > > >,
        STC<Seq<STC<FunctionName>,TokC<LPAREN>,STC<OneOrMore<STC<Param> > >,TokC<RPAREN>,STC<Star<STC<ExceptionDecl> > >,TokC<Char<';'> > > > //构造函数
        >{};

    struct EnumContentStar : EnumContent{};
    struct Enum : Type<STC<ENUM>,STC<EnumContentStar> >{};

    struct StructContentStar : Star<STC<Field> >{};
    struct Struct : Type<STC<STRUCT>,STC<StructContentStar> >{};

    struct ServiceContentStar : Star<STC<Function> >{};
    struct Service : Type<STC<SERVICE>,STC<ServiceContentStar> >{};

    struct ExceptionContentStar : Star<STC<Field> >{};
    struct Exception : Type<STC<XEXCEPTION>,STC<ExceptionContentStar> >{};

    struct TypeDefineIdent        : TypeIdent{};
    struct TypeDefineField        : Seq<STC<TypeDefineIdent>,STC<TypeName> >{};
    struct TypeDefineFieldStar    : Seq<STC<TypeDefineField>,TokC<Char<';'> > >{};
    struct TypeDefineDefine : Seq<STC<TYPEDEFINE>,STC<TypeDefineFieldStar> >{};

    struct PreDeclare : Seq<
        STC<Or<
        STC<SERVICE>,
        STC<ENUM>,
        STC<STRUCT>,
        STC<XEXCEPTION>
        > >,
        STC<TypeIdent>,
        TokC<Char<';'> > >{};

    struct TopicTypeName : TypeName{};
    struct TopicParam : TypeIdent{};
    struct TopicDesc : Seq<TokC<LSQUARE>,STC<String>,TokC<RSQUARE> >{};
    struct TopicContent 
        : Seq<
        STC<TOPIC>,
        STC<TopicTypeName>,
        TokC<Char<'='> >,
        STC<TopicParam>,
        STC<Or<NotAt<ExtendIdent>,
        STC<Extend> > >,TokC<Char<';'> >
        >{};

    struct Topic 
        : Or<
        STC<TopicContent>,
        STC<Seq<STC<TopicDesc>,STC<TopicContent> > >
        > {};

    struct TypeContent 
        :Or<
        STE<Package>,
        STE<Enum>,
        STE<Struct>,
        STE<Service>,
        STE<Exception>,
        STE<PreDeclare>,
        STE<TypeDefineDefine>
        >{};

    struct PackageContentStar : Star<STC<TypeContent> > {};
    struct Package : Type<STC<PACKAGE>,STC<PackageContentStar> >{};

    struct PluginName : Ident{};

    struct PluginContent : Star<AnyCharExcept<CharSet<'#'> > >{};

    struct Plugin : 
        Or<
        STC<Seq<STC<PLUGIN>,STC<PluginName>,STC<PluginContent>,STC<PLUGINEND> > >,
        STC<Seq<STC<PLUGIN>,STC<PluginName>,STC<PLUGINEND> > >
        >
    {};

    struct PathSplit : Or<Char<'/'>,Char<'\\'> >{};
    struct PathChar : Or<Or<AlphaNum,Char<':'> >,Char<'_'>,PathSplit>{};
    template<typename Ext>
    struct Path : Seq<Star<PathChar>,Ext>{};
    struct ImportPath : Path<Or<CharSeq<'.','x','i','d','l'>,CharSeq<'.','X','I','D','L'> > >{};
    struct Import : Seq<STC<IMPORT>,TokC<LBRACKET>,STC<ImportPath>,TokC<RBRACKET> >{};

    struct LinkStudioRule : 
        Seq<
        TokC<True_T>,
        Star<
        STC<Import> >,
        While<Or<
        STE<Package>,
        STE<TypeContent>,
        STE<Topic>,
        STC<Plugin>,
        STE<PreDeclare> >
        >
        >{};

    STRUCT_PTR_DECLARE(PluginDef);
    STRUCT_PTR_DECLARE(Parser);

    struct TOPSIMIDL_DECL PluginDef
    {
        TSString    Name;
        TSString    ContentNode;
    };

    struct TOPSIMIDL_DECL Parser
    {
        typedef ExceptionTextParser::Node Node;
        typedef ExceptionTextParser TextParser;
        typedef boost::shared_ptr<TextParser> TextParserPtr;

        std::vector<TypeDefPtr>         TopLevelTypeDefs;
        TextParserPtr                   TextPars;
        TSString                        FileContent;			//xidl文件内容
        std::map<TSString,TSString>     PluginDefs;
        std::vector<TSString>           ImportFileNames;

        UINT32   GetLineCount(TSIDLGram::Parser::Node * AstNode);
        TSString GetLineMsg(UINT32 nLine);
        UINT32   GetMaxLine();
        const TSString & GetFullFileName();
        TSString GetPluginContent(const TSString & PluginName );
    };

    TOPSIMIDL_DECL ParserPtr BuildFile(const TSString & FileFullPath,OutputPipe * Pipe = NULL);

    template<typename Rule_T>
    Parser::TextParserPtr BuildAst(const char * begin,const char * end,VisitProc Proc,const TSString & FileName = TSString(),OutputPipe * Pipe = NULL)
    {
        Parser::TextParserPtr Pars = boost::make_shared<Parser::TextParser>(begin,end,FileName);
        bool Ret = Pars->Parse<Rule_T>();
        if(Ret)
        {
            Parser::Node * AstNode = Pars->GetAstRoot();
            Visit(AstNode,Proc);
        }
        else
        {
			Pars->OutputLocation(Pipe);
			Pars.reset();
        }

        return Pars;
    }

}    //end of namespace

#endif

