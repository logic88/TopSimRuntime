#ifndef __TSGRAMCOMMON_H__
#define __TSGRAMCOMMON_H__

#include <queue>

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <yard/yard_base_grammar.hpp>
#include <yard/yard_char_set.hpp>
#include <yard/yard_tree.hpp>
#include <yard/yard_text_grammar.hpp>
#include <yard/yard_parser.hpp>
#include <TopSimTypes/TopSimCommTypes.h>
#else
#include <TopSimRuntime/yard/yard_base_grammar.hpp>
#include <TopSimRuntime/yard/yard_char_set.hpp>
#include <TopSimRuntime/yard/yard_tree.hpp>
#include <TopSimRuntime/yard/yard_text_grammar.hpp>
#include <TopSimRuntime/yard/yard_parser.hpp>
#include <TopSimRuntime/TopSimCommTypes.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TopSimIDL.h"

//一般匹配规则

// Store 创建一个新的节点，用于保存符合规则的标识。
// Finao 如果规则匹配失败,抛一个异常。
// StoreFinao 保存会产生有异常的规则匹配。
// True_T 规则匹配总是返回true
// False_T 规则匹配总是返回false
// EndOfInput 当前达到匹配字符的末尾，则返回true，否则false
// At 如果规则匹配成功，则返回true，并且重置输入指针到匹配之前，否则返回false.
// Pick 满足条件返回true，但是不更改输入指针
// NotAt 如果规则匹配失败，则返回成功，并且重置输入指针到匹配之前，否则返回false。
// Or，可接受1-9个模板元参数，只要有一个成功就返回true。
// Seq，可接受1-9个模板元参数，必须所有匹配都成功才返回成功。
// Star，匹配某一规则0或者n次，总是返回成功。
// Plus，尽可能多的匹配某一规则，直到返回false，如果有一次成功返回ture，否则返回false。
// Opt，尝试匹配一次规则，即使未成功或到文件末尾也会返回true。
// Repeat，匹配一个规则N次，N作为模板元传入，如果有一次失败，则重置输入指针到匹配之前，并返回false，否则返回true。
// UntilPast，匹配某一规则，直到成功，并返回true，如果到文件末尾，则返回false，并重置指针到匹配之前。 （成功后，指针在匹配成功之后）
// UntilAt，UntilPast<At<Rule_T>>,匹配某一规则，直到成功，并返回true，并重置指针到匹配成功之前，如果到文件末尾，则返回false。
// MustStore,如果保存失败则抛出异常.
// FinaoIf，如果满足条件T，则进行匹配，如果匹配失败，则抛出异常。
// StoreIf，如果满足条件T，则保存节点，如果节点保存失败，则抛出异常。
// NoFailSeq，如果满足条件T0，则执行匹配T1-T9，如果有匹配执行失败，则抛出异常。
// RepeatWhile，循环，如果条件T满足，则执行条件U，直到T或者U失败。
// RepeatWhileNoFail，循环，如果条件T满足，则执行条件U，直到T失败，如果U失败， 则抛出异常。
// Log，调试，将匹配规则T,打印出匹配的字符行数和列数，如果成功则输入succeeded，否则输出failed。
// StarExcept，匹配规则U和T，如果规则U匹配失败，则匹配规则T，直到U成功或者T失败。
// 
// 
// 字符规则
// AnyChar,匹配任何字符，并总是返回true，将指针向前移动一个字符，如果到文件末尾返回false。
// Char，匹配一个字符，如果是模板元字符则返回true，并将指针向前移动一个字符，否则返回false。
// ExceptChar，期望某个字符，如果不是则抛出异常，否则返回true，并将指针向前移动一个字符。
// NotChar，匹配一个字符，如果不是模板元字符则返回true，并且将指针向前移动一个字符。否则返回false
// CharSetParser，字符集解析，传入一个字符集，如果当前字符在字符集中，则返回true，并将指针移动一个字符，否则返回false。
// ChineseCharSetParser，中文字符解析，如果字符是中文，则返回true，并且将字符向前移动一个字符，否则返回false。
// AnyCharExcept，匹配任何字符除了制定的字符集。
// CharRange，解析字符C0到字符C1。
// Digit，解析10进制数字字符。
// BinDigit，解析二进制字符。
// HexDigit，解析16进制字符。
// OctDigit，解析8进制字符。
// IdentFirstChar，解析标识符首字符，标识符为字母或下划线打头。
// IdentNextChar，解析标识符接下来的字符，可以是字母或数字或下划线。
// Letter，解析字母字符集。
// AlphaNum，解析字母或数字。
// LowerCastLetter，解析小写字母。
// UpperCastLetter，解析大写字母。
// Ident，c/c++标识符。
// NS,空字符串结尾标识
// CharSeq,字符序列，最多支持15个字符
// CharSeqIgnoreCase,不区分大小写的字符序列
// Word,单词，匹配规则T，并且结尾不是标识符的字符NotAt<IdentNextChar>。
// DoubleQuoted，形如""的字符串。
// SingleQuoted，形如''的字符串。
// SingleOrDoubleQuoted，形如""或''的字符串。

#define IS_TYPE(Node,T) Node->TypeMatches<T>()
#define IS_BEGIN(Node,T) Node->TypeMatches<TSGramCommon::TB<T> >()
#define IS_END(Node,T) Node->TypeMatches<TSGramCommon::TE<T> >()
#define NodeToString(Node) TSString(Node->GetFirstToken(),Node->GetLastToken()).c_str()
#define NodeToStringTrim(Node) boost::algorithm::trim_copy(TSString(Node->GetFirstToken(),Node->GetLastToken()))
#define StringNodeToString(Node) TSString(Node->GetFirstToken() + 1,Node->GetLastToken() - 1).c_str()

namespace TSGramCommon 
{
    using namespace yard;
    using namespace text_grammar;

    //空白字符集,包括' '，\t,\n,\r
    struct WS : CharSetParser<WhiteSpaceCharSet> {}; 

    //标识，匹配规则T，如果T成功，则跳转到下一个非空白字符
    template<typename T> 
    struct Tok 
	    : Seq<
	        T, 
			Star<WS> 
		>{}; 

    // "Store-Tokenize"
    template<typename T> 
    struct ST 
		: Tok<
		Store<T> 
		>{}; 

    typedef CharSetUnion<CharSet<'*'>, CharSet<'/'> >
        CommentPostfixCharSet;
    
    struct Grule : Seq<Star<AnyCharExcept<CharSet<'*'> > >,Char<'*'>,AnyCharExcept<CharSet<'/'> > >{};

	//匹配注释
    struct Comment : 
        Or<
            ST<Seq<CharSeq<'/','/'>,Star<AnyCharExcept<CharSet<'\n'> > >,Char<'\n'> > >,
            ST<Seq<CharSeq<'/','*'>,Star<Grule>,Star<AnyCharExcept<CharSet<'*'> > >,Char<'*'>,Char<'/'> > >
            //ST<Seq<CharSeq<'/','*'>,Star<AnyCharExcept<CharSet<'*','/'> > >,CharSeq<'*','/'> > >
        >{};

    template<typename T> 
    struct TokC : 
        Seq<T, Star<Or<WS,ST<Comment> > > > {};

    template<typename T> 
    struct STC 
	    : TokC<
			Store<T>
		> {};

    //关键字，匹配规则T，如果T成功则
    template<typename T> 
    struct Keyword : 
        Tok<Seq<T, NotAlphaNum> > 
    {};

    template<typename Rule_T>
    struct While
    {
        template<typename ParserState_T>
        static bool Match(ParserState_T& p)
        {
            while(true)
            {
                typename ParserState_T::Iterator pos = p.GetPos();
                if(Rule_T::Match(p))
                {
                    continue;
                }
                else
                {
                    if(!p.AtEnd())
                    {
                        p.SetPos(pos);
                    }
                    break;
                }
            }

            return p.AtEnd();
        }
    };

    struct ESC    : Seq<Char<'\\'>,AnyChar> {};
    struct String : Seq<Char<'"'>,Star<Or<ESC,AnyCharExcept<CharSet<'"'> > > >,Char<'"'> > {};
    struct SChar : Seq<Char<'\''>,Star<Or<ESC,AnyCharExcept<CharSet<'\''> > > >,Char<'\''> > {};
    template
    <
        typename Rule_T,
        typename SplitRule_T = Char<','>
    >
    struct OneOrMore  
        : Seq<
        STC<Rule_T>,
        Star<Seq<TokC<SplitRule_T>,
        STC<Rule_T> > >
        >{};

    template<typename Rule_T>
    struct TB : True_T{};

    template<typename Rule_T>
    struct TE : True_T{};

    template<typename Rule_T>
    struct STE : 
        STC<Seq<
            STC<TB<Rule_T> >,
            STC<Rule_T>,
            STC<TE<Rule_T> >
		> 
	    >{};

    struct LPAREN       : Char<'('>{};
    struct RPAREN       : Char<')'> {};
    struct LSQUARE      : Char<'['> {};
    struct RSQUARE      : Char<']'> {};
    struct LBRACKET     : Char<'<'> {};
    struct RBRACKET     : Char<'>'> {};
    struct LBRACE       : Char<'{'> {};
    struct RBRACE       : Char<'}'> {};
    struct COMMA        : Char<','>{};
    struct SEMICOLON    : Char<';'>{};
    struct LSHIFT       : CharSeq<'<','<'>{};
    struct RSHIFT       : CharSeq<'>','>'>{};
    struct XOR          : Char<'|'>{};
    struct AND          : Char<'&'>{};

    struct XIN          : Keyword<CharSeq<'i','n'> >{};
    struct XOUT         : Keyword<CharSeq<'o','u','t'> >{};
    struct XINOUT       : Keyword<CharSeq<'i','n','o','u','t'> >{};
    struct XVOID        : Keyword<CharSeqIgnoreCase<'v','o','i','d'> >{};

    struct OutputPipe
    {
        virtual void Output(const char * Msg) = 0;
    };

    struct TOPSIMIDL_DECL ExceptionTextParser : public SimpleTextParser
    {
        FORCEINLINE ExceptionTextParser(SimpleTextParser::Iterator first,
            SimpleTextParser::Iterator last,const TSString & FileName = TSString())
            :SimpleTextParser(first,last)
        {
            mFileName = FileName;
            mCurIter = first;
			mBeginCount = 0;
        }

        FORCEINLINE ~ExceptionTextParser()
        {

        }

        // Parse function
        template<typename StartRule_T>
        bool Parse() {
            return StartRule_T::Match(*this);
        }

        FORCEINLINE void GotoNext() {
            assert(mIter < End()); 

            ++mIter;
			SimpleTextParser Parser(mIter,mIter+1);
            if(!WS::Match(Parser))
            {
                if(mCurIter < mIter)
                {
                    mCurIter=mIter;
                }
            }
        }

        UINT32           GetLineCount(Iterator Pos);
        TSString         GetlineMsg(UINT32 Msg);
        UINT32           GetMaxLine();
        const TSString & GetFullFileName();
		//输出xidl文件错误位置
        void OutputLocation(TSGramCommon::OutputPipe * Pipe, UINT32 nLine = 0);

    protected:
        SimpleTextParser::Iterator mCurIter;
        TSString                   mFileName;
		UINT32					   mBeginCount;
    };

    STRUCT_PTR_DECLARE(TypeDef);
    struct TOPSIMIDL_DECL TypeDef
    {
        TypeDef()
            :Node(NULL),ContentNode(NULL),
            TypeInfoName(NULL)
        {};
        TypeDefWPtr                              Supper;
        std::vector<TypeDefPtr>                  Subordinates;
        const char *                             TypeInfoName;
        TSString                                 TypeName;
		TSString								 TypeDesc;
        ExceptionTextParser::Node *   Node;
        ExceptionTextParser::Node *   ContentNode;
    };

    typedef boost::function<void(ExceptionTextParser::Node *)>          VisitProc;

    TOPSIMIDL_DECL void Visit(ExceptionTextParser::Node * AstNode,VisitProc Proc);
    TOPSIMIDL_DECL void VisitChild(ExceptionTextParser::Node * AstNode,VisitProc Proc);
}    //end of namespace

#endif

