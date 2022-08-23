#include "stdafx.h"

#include <boost/foreach.hpp>

#include <tbb/concurrent_queue.h>

#include <TSIDLPluginMgr.h>

#include "TSIdlGram.h"



namespace TSIDLGram
{
	struct TypeVisit
	{
		std::vector<TypeDefPtr>                  _TypeDefs;
		std::stack<TypeDefPtr>                   _CurTypeDefs;
		TypeDefPtr                               _CurTypeDef;
		std::map<TSString,TSString>              _Plugins;
		PluginDefPtr                             _CurPlugin;

		TypeDefPtr & getDef()
		{
			ASSERT(_CurTypeDefs.size() && "!_CurTypeDefs.size()");
			return _CurTypeDefs.top();
		}
	};

	typedef boost::shared_ptr<TypeVisit> TypeVisitPtr;

	void TypeVisit_p(TypeVisitPtr aVisit,TSIDLGram::ExceptionTextParser::Node * AstNode)
	{
		if(IS_BEGIN(AstNode,Enum) || IS_BEGIN(AstNode,Service) || IS_BEGIN(AstNode,Package) || 
            IS_BEGIN(AstNode,Struct) || IS_BEGIN(AstNode,Topic) || IS_BEGIN(AstNode,Exception)
            ||IS_BEGIN(AstNode,TypeDefineDefine))
		{
			TypeDefPtr CurTypeDef = boost::make_shared<TypeDef>();
			aVisit->_CurTypeDefs.push(CurTypeDef);
            if(IS_BEGIN(AstNode, Enum) || IS_BEGIN(AstNode,Service) || IS_BEGIN(AstNode,Struct)
               || IS_BEGIN(AstNode,Topic))
            {
                TSIDLPluginMgr::Instance()->SetTopicOrService(true);
            }
            if (IS_BEGIN(AstNode,Service))
            {
                TSIDLPluginMgr::Instance()->SetService(true);
            }
		}
		else if(IS_TYPE(AstNode,Enum) ||IS_TYPE(AstNode,Service) || IS_TYPE(AstNode,Package) || 
            IS_TYPE(AstNode,Struct) || IS_TYPE(AstNode,Topic) || IS_TYPE(AstNode,Exception)
            || IS_TYPE(AstNode,TypeDefineDefine))
		{
			aVisit->getDef()->Node = AstNode;

			if(IS_TYPE(AstNode,TSIDLGram::Topic))
			{
				aVisit->getDef()->ContentNode = AstNode;
			}
		}
		else if(IS_END(AstNode,Enum) || IS_END(AstNode,Service) || IS_END(AstNode,Package) || 
            IS_END(AstNode,Struct) || IS_END(AstNode,Topic) || IS_END(AstNode,Exception)
            ||IS_END(AstNode,TypeDefineDefine))
		{
			TypeDefPtr CurTypeDef = aVisit->getDef();
			aVisit->_CurTypeDefs.pop();

			if(aVisit->_CurTypeDefs.size())
			{
				TypeDefPtr Supper = aVisit->_CurTypeDefs.top();
				Supper->Subordinates.push_back(CurTypeDef);
				CurTypeDef->Supper = Supper;
			}
			else
			{
				aVisit->_TypeDefs.push_back(CurTypeDef);
			}
		}
		else if(IS_TYPE(AstNode,ENUM) || IS_TYPE(AstNode,SERVICE) || IS_TYPE(AstNode,PACKAGE) || 
            IS_TYPE(AstNode,STRUCT) || IS_TYPE(AstNode,TOPIC) || IS_TYPE(AstNode,XEXCEPTION)
            || IS_TYPE(AstNode,TYPEDEFINE))
		{
			aVisit->getDef()->TypeInfoName = AstNode->GetRuleTypeInfo().name();
		}
		else if(IS_TYPE(AstNode,EnumContentStar) || IS_TYPE(AstNode,ServiceContentStar) || 
            IS_TYPE(AstNode,PackageContentStar) || IS_TYPE(AstNode,StructContentStar) ||
            IS_TYPE(AstNode,ExceptionContentStar) || IS_TYPE(AstNode,TypeDefineFieldStar))
		{
			aVisit->getDef()->ContentNode = AstNode;
		}
		else if(IS_TYPE(AstNode,TSIDLGram::TypeName))
		{
			aVisit->getDef()->TypeName = NodeToString(AstNode);
		}
		else if(IS_TYPE(AstNode,TSIDLGram::Plugin))
		{
			aVisit->_CurPlugin = boost::make_shared<PluginDef>();
                        
		}
        else if (IS_TYPE(AstNode,TSIDLGram::PluginName))
        {
            aVisit->_CurPlugin->Name = NodeToString(AstNode);
            if (aVisit->_CurPlugin->Name == "Loris" )
            {
                TSIDLPluginMgr::Instance()->SetIsPlugin(true);
            }
            else if (aVisit->_CurPlugin->Name == "XSIM")
            {
                TSIDLPluginMgr::Instance()->SetHaveXimPlugin(true);
            }
        }
        else if (IS_TYPE(AstNode,TSIDLGram::PluginContent))
        {
             aVisit->_CurPlugin->ContentNode = NodeToString(AstNode);
        }
		else if(IS_TYPE(AstNode,TSIDLGram::PLUGINEND))
		{
			ASSERT(aVisit->_CurPlugin && "!_CurPlugin");
			ASSERT(!aVisit->_CurPlugin->Name.empty() && "_CurPlugin->Name.empty()");
			aVisit->_Plugins[aVisit->_CurPlugin->Name] = aVisit->_CurPlugin->ContentNode;
		}
	}

	UINT32 Parser::GetLineCount( Node * AstNode )
	{
		return TextPars->GetLineCount(AstNode->GetFirstToken());
	}

	TSString Parser::GetLineMsg( UINT32 nLine )
	{
		return TextPars->GetlineMsg(nLine);
	}

	UINT32 Parser::GetMaxLine()
	{
		return TextPars->GetMaxLine();
	}

	const TSString & Parser::GetFullFileName()
	{
		return TextPars->GetFullFileName();
	}


    TSString Parser::GetPluginContent(const TSString & PluginName )
    {
        std::map<TSString,TSString>::iterator plugindef = PluginDefs.find(PluginName);

        if (plugindef != PluginDefs.end())
        {
            return plugindef->second;
        }

        return "";
    }

	//对xidl文件进行解析，将xidl文件解析的内容存入Parser结构体中
	ParserPtr BuildFile(const TSString & FileFullPath, OutputPipe * Pipe /*= NULL*/)
	{
		std::ifstream ifs(FileFullPath.c_str(),std::ifstream::in|std::ifstream::binary);

		if (!ifs.is_open())
		{
			if(Pipe)
			{
				Pipe->Output((GB18030ToTSString("文件打开失败:") + FileFullPath + ".").c_str());
			}
			else
			{
				STD_COUT << GB18030ToTSString("文件打开失败:") << FileFullPath << ".";
			}

			return ParserPtr();
		}

		ParserPtr Pars = boost::make_shared<Parser>();

		std::ostringstream oss;
		oss << ifs.rdbuf();
		
		Pars->FileContent = oss.str();
		
		//xidl文件内容开始指针
		const char * begin = Pars->FileContent.c_str();

		//xidl文件内容结束指针
		const char * end = Pars->FileContent.c_str() + Pars->FileContent.size();
		//获取xidl文件名称(不带扩展名)
		std::vector<TSString> fields;
		boost::split(fields, FileFullPath, boost::is_any_of("/"));
		std::vector<TSString>::reverse_iterator it = fields.rbegin();
		TSString FileName = *it;
		std::vector<TSString> fileNames;
		boost::split(fileNames, FileName, boost::is_any_of("."));
		std::vector<TSString>::iterator itr = fileNames.begin();
		FileName = *itr;

		//#plugin XSIM之前的行数
		bool IsFound = false;
		UINT32 nLine = 0;
		TSString FileContent = Pars->FileContent.c_str();

		std::vector<TSString> FileContents = TSStringUtil::Split(FileContent, "#plugin XSIM", false);
		if (FileContents.size() > 1)
		{
			IsFound = true;
			++nLine;

			TSString BeforStr = FileContents[0];

			for (int i = 0; i < BeforStr.length(); ++i)
			{
				if (BeforStr[i] == '\n')
				{
					++nLine;
				}
			}
		}

		//如果找到了#plugin关键字，则将xidl文件内容有效开始位置添加进去
		if (IsFound)
		{
			PluginMgr()->setXidlBegin(FileName, nLine);
		}

		TypeVisitPtr Vis = boost::make_shared<TypeVisit>();

		//为xdil文件构建Ast树（用来获取xidl文件中的主题等）
		if (Parser::TextParserPtr TextPars =
			BuildAst<LinkStudioRule>(begin, end, boost::bind(&TypeVisit_p, Vis, _1), FileFullPath, Pipe))
		{
			Pars->TextPars = TextPars;
			Pars->PluginDefs = Vis->_Plugins;
			Pars->TopLevelTypeDefs = Vis->_TypeDefs;

			return Pars;
		}

		return ParserPtr();
	}
}



