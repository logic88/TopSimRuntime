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

//һ��ƥ�����

// Store ����һ���µĽڵ㣬���ڱ�����Ϲ���ı�ʶ��
// Finao �������ƥ��ʧ��,��һ���쳣��
// StoreFinao �����������쳣�Ĺ���ƥ�䡣
// True_T ����ƥ�����Ƿ���true
// False_T ����ƥ�����Ƿ���false
// EndOfInput ��ǰ�ﵽƥ���ַ���ĩβ���򷵻�true������false
// At �������ƥ��ɹ����򷵻�true��������������ָ�뵽ƥ��֮ǰ�����򷵻�false.
// Pick ������������true�����ǲ���������ָ��
// NotAt �������ƥ��ʧ�ܣ��򷵻سɹ���������������ָ�뵽ƥ��֮ǰ�����򷵻�false��
// Or���ɽ���1-9��ģ��Ԫ������ֻҪ��һ���ɹ��ͷ���true��
// Seq���ɽ���1-9��ģ��Ԫ��������������ƥ�䶼�ɹ��ŷ��سɹ���
// Star��ƥ��ĳһ����0����n�Σ����Ƿ��سɹ���
// Plus�������ܶ��ƥ��ĳһ����ֱ������false�������һ�γɹ�����ture�����򷵻�false��
// Opt������ƥ��һ�ι��򣬼�ʹδ�ɹ����ļ�ĩβҲ�᷵��true��
// Repeat��ƥ��һ������N�Σ�N��Ϊģ��Ԫ���룬�����һ��ʧ�ܣ�����������ָ�뵽ƥ��֮ǰ��������false�����򷵻�true��
// UntilPast��ƥ��ĳһ����ֱ���ɹ���������true��������ļ�ĩβ���򷵻�false��������ָ�뵽ƥ��֮ǰ�� ���ɹ���ָ����ƥ��ɹ�֮��
// UntilAt��UntilPast<At<Rule_T>>,ƥ��ĳһ����ֱ���ɹ���������true��������ָ�뵽ƥ��ɹ�֮ǰ��������ļ�ĩβ���򷵻�false��
// MustStore,�������ʧ�����׳��쳣.
// FinaoIf�������������T�������ƥ�䣬���ƥ��ʧ�ܣ����׳��쳣��
// StoreIf�������������T���򱣴�ڵ㣬����ڵ㱣��ʧ�ܣ����׳��쳣��
// NoFailSeq�������������T0����ִ��ƥ��T1-T9�������ƥ��ִ��ʧ�ܣ����׳��쳣��
// RepeatWhile��ѭ�����������T���㣬��ִ������U��ֱ��T����Uʧ�ܡ�
// RepeatWhileNoFail��ѭ�����������T���㣬��ִ������U��ֱ��Tʧ�ܣ����Uʧ�ܣ� ���׳��쳣��
// Log�����ԣ���ƥ�����T,��ӡ��ƥ����ַ�����������������ɹ�������succeeded���������failed��
// StarExcept��ƥ�����U��T���������Uƥ��ʧ�ܣ���ƥ�����T��ֱ��U�ɹ�����Tʧ�ܡ�
// 
// 
// �ַ�����
// AnyChar,ƥ���κ��ַ��������Ƿ���true����ָ����ǰ�ƶ�һ���ַ���������ļ�ĩβ����false��
// Char��ƥ��һ���ַ��������ģ��Ԫ�ַ��򷵻�true������ָ����ǰ�ƶ�һ���ַ������򷵻�false��
// ExceptChar������ĳ���ַ�������������׳��쳣�����򷵻�true������ָ����ǰ�ƶ�һ���ַ���
// NotChar��ƥ��һ���ַ����������ģ��Ԫ�ַ��򷵻�true�����ҽ�ָ����ǰ�ƶ�һ���ַ������򷵻�false
// CharSetParser���ַ�������������һ���ַ����������ǰ�ַ����ַ����У��򷵻�true������ָ���ƶ�һ���ַ������򷵻�false��
// ChineseCharSetParser�������ַ�����������ַ������ģ��򷵻�true�����ҽ��ַ���ǰ�ƶ�һ���ַ������򷵻�false��
// AnyCharExcept��ƥ���κ��ַ������ƶ����ַ�����
// CharRange�������ַ�C0���ַ�C1��
// Digit������10���������ַ���
// BinDigit�������������ַ���
// HexDigit������16�����ַ���
// OctDigit������8�����ַ���
// IdentFirstChar��������ʶ�����ַ�����ʶ��Ϊ��ĸ���»��ߴ�ͷ��
// IdentNextChar��������ʶ�����������ַ�����������ĸ�����ֻ��»��ߡ�
// Letter��������ĸ�ַ�����
// AlphaNum��������ĸ�����֡�
// LowerCastLetter������Сд��ĸ��
// UpperCastLetter��������д��ĸ��
// Ident��c/c++��ʶ����
// NS,���ַ�����β��ʶ
// CharSeq,�ַ����У����֧��15���ַ�
// CharSeqIgnoreCase,�����ִ�Сд���ַ�����
// Word,���ʣ�ƥ�����T�����ҽ�β���Ǳ�ʶ�����ַ�NotAt<IdentNextChar>��
// DoubleQuoted������""���ַ�����
// SingleQuoted������''���ַ�����
// SingleOrDoubleQuoted������""��''���ַ�����

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

    //�հ��ַ���,����' '��\t,\n,\r
    struct WS : CharSetParser<WhiteSpaceCharSet> {}; 

    //��ʶ��ƥ�����T�����T�ɹ�������ת����һ���ǿհ��ַ�
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

	//ƥ��ע��
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

    //�ؼ��֣�ƥ�����T�����T�ɹ���
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
		//���xidl�ļ�����λ��
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

