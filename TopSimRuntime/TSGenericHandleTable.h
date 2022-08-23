#ifndef __TSIDLPLUGINMGR_H__
#define __TSIDLPLUGINMGR_H__

#include <boost/program_options.hpp>

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimTypes/TSSingleton.h>
#include <TopSimUtil/pugixml.hpp>
#else
#include <TopSimRuntime/TSSingleton.h>
#include <TopSimRuntime/pugixml.hpp>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TopSimIDL.h"
#include "TSIdlGram.h"

#define FileKey         TS_TEXT("File")
#define DebugKey        TS_TEXT("Debug")
#define HelpKey         TS_TEXT("Help")
#define IncludePathKey  TS_TEXT("IncludePath")
#define GImportKey      TS_TEXT("GImport")
#define GExe            TS_TEXT("GExe")
#define Gstdafx         TS_TEXT("Gstdafx")
#define VSKey           TS_TEXT("VS")
#define LibPathKey      TS_TEXT("LibPath")
#define ExportFileKey   TS_TEXT("ExportFile")
#define ExportMacroKey  TS_TEXT("ExportMacro")
#define GListEntityKey  TS_TEXT("GListEntity")
#define GFileOnlyKey    TS_TEXT("GFileOnly")
#define CompileKey      TS_TEXT("Compile")
#define GSqlKey         TS_TEXT("GSql")
#define GstdafxKey      TS_TEXT("Gstdafx")
#define GExeKey         TS_TEXT("GExe")
#define SlnKey          TS_TEXT("SlnType")
#define SavePathKey     TS_TEXT("SavePath")
#define CodeTypeKey     TS_TEXT("CodeType")
#define NodeFileKey     TS_TEXT("NodeFile")
#define LorisKey        TS_TEXT("Loris")
#define LinuxKey        TS_TEXT("Linux")
#define GXmlKey         TS_TEXT("GXml")
#define SetSDKKey       TS_TEXT("SetSDK")
#define VerSionKey      TS_TEXT("Version")
#define FileNameWithXidlKey      TS_TEXT("FileNameWithXidl")
#define GeneratePyKey   TS_TEXT("GeneratePy")
#define GenerateXsimfileKey TS_TEXT("GenerateXsimfile")
#define GenPython       TS_TEXT("Python")

struct TOPSIMIDL_DECL TSIIDLPlugin : public boost::enable_shared_from_this<TSIIDLPlugin>
{
    TS_MetaType(TSIIDLPlugin);
public:
    virtual TSString         GetName() = 0;
    virtual void             OnPluginLoaded() = 0;
    virtual bool             DoParse() = 0;
    virtual bool             DoBuild() = 0;
    virtual void             Clean() = 0;
};
STRUCT_PTR_DECLARE(TSIIDLPlugin);

typedef TSConUnorderedMap<int,TSIIDLPluginPtr> PluginMap_T;
typedef boost::shared_ptr<pugi::xml_document> DocumentPtr;

struct TSIDLPluginMgrPrivate;
class TOPSIMIDL_DECL TSIDLPluginMgr : public TSSingleton<TSIDLPluginMgr>
{
    SINGLETON_DECLARE(TSIDLPluginMgr);

public:
    struct ParserUtil
    {
        ParserUtil():IsImportFile(false)
        {
            
        }
        TSIDLGram::ParserPtr  Parse;
        TSString              SourceName;
        bool                  IsImportFile;
    };

    typedef boost::shared_ptr<ParserUtil> ParserUtilPtr;
    typedef std::vector<std::pair<TSString,TSIDLPluginMgr::ParserUtilPtr>> ParseMap_T;

public:
    void                                               SetConfigFileName(const TSString & FileName);
	/* 初始化配置 */
    bool                                               InitOption(int argc,char * argv[]);
    bool                                               HasOption(const TSString & Key);
    void                                               PrintHelps();
    const std::vector<TSString> &                 UnrecognizedOpts();
    const boost::program_options::variable_value &     GetOptionVal(const TSString & Key);
    bool                                               LoadPlugins();
    DocumentPtr                                        GetConfigureDocument();
    bool                                               IsImportParse(ParserUtilPtr ParseUtil);
    bool                                               IsImportParse(TSIDLGram::ParserPtr Parse);
    bool                                               ParseFile(const TSString & FileName);
	/* 获取所有xidl文件路径 */
    const std::vector<TSString> &                      GetOptionFiles();
    const std::vector<TSString> &                      GetIncludePaths();
    const std::vector<TSString> &                      GetLibraryPaths();
    const TSString &                                   GetCurParseFileFullPath();
    void                                               SetCurParseFileFullPath(const TSString & FilePath);
	const TSString&                            GetImplFullPath();
	void                                               SetImplFullPath(const TSString& FilePath);
	void                                               SetGenericFullPath(const TSString& FilePath);
	const TSString&                            GetGenericFullPath();
    ParserUtilPtr                                      GetParserUtil(const TSString & FileFullName);
    const ParseMap_T &                                 GetParserUtils();
    void                                               CleanParse();
    UINT32                                             GetLineCount(TSIDLGram::Parser::Node * AstNode);
    TSString                                           GetLineMsg(UINT32 nLine);
    UINT32                                             GetMaxLineCount();
    void                                               LogErrorMsg(const TSString & Msg,TSIDLGram::Parser::Node * AstNode);
    void                                               CleanPlugins();
    void                                               SetPipe(TSGramCommon::OutputPipe * Pipe);
    TSGramCommon::OutputPipe *                         GetPipe();
    void                                               DoParse();
    void                                               DoBuild();
    TSIIDLPluginPtr                                    GetPlugin(int InterfaceMetaTypeId) const;
    const PluginMap_T &                                GetPlugins() const;
    const std::vector<TSIIDLPluginPtr> &               GetFlatPlugins();
    void                                               SetTopicOrService(bool IsTopicOrService);
    bool                                               GetTopicOrService();
    void                                               SetService(bool IsService);
    bool                                               GetService();
    void                                               SetIsPlugin(bool IsPlugin);
    void                                               SetHaveXimPlugin(bool IsPlugin);
    bool                                               GetHaveXimPlugin();
    bool                                               GetIsPlugin();
    TSString                                         GetCodeType();
    bool                                           GetFileNameWithXidl();
    void                                           SetFileNameWithXidl(bool IsXidl);
    bool                                           GetGeneratePy();
    bool                                           GetGenerateXsimfile();
    bool                                           GetIsGenerateImportFile();
    void                                           SetIsGenerateImportFile(bool IsImport);
    bool                                           GetIsIncludePath();
    void                                           SetIsIncludePath(bool IncludePath); 
    void                                           SetPareseMap(TSString Key,ParserUtilPtr Util);
    TSString									  GetPravatFilePath();
    TSString                                    GetFullIdlPath(const TSString & FileName);
	TSString                                    GetSDKPath();
	UINT32                                     GetSortDeep();
	void											 SetSortDeep(const UINT32 & Deep);
	void											 SetIncludePaths(std::vector<TSString> Ipaths);
	void											 setXidlBegin(TSString FileName, UINT32 nLine);
	bool											 IsDefualtXidlFile(TSString FileName);
	std::map<TSString, UINT32>	 GetXidlBegin();
	void                                          SetTopicNameMap(std::map<TSString, TSString> topicMap);
	std::map<TSString, TSString> GetTopicNameMap();
	void                                          TransferSelectTopic(std::vector<TSString>& vecTopic);
	std::vector<TSString>             GetSelectTopicVector();
	void                                         TranferSelectFileName(TSString fileName);
	TSString                                   GetSelectFileName();
	bool IsLinkCodeGenTool();
	void SetLinkCodeGenToolFlag(bool isTool);
	
	//设置主题名字的Map
	void SetCurrentFileTopicNameMap(std::map<std::pair<TSString, TSString>, TSString> topicVec);
	//获取主题名字Map
	std::map<std::pair<TSString, TSString>, TSString> GetCurrentFileTopicNameMap();
	void                                         SetServiceNameVec(std::vector<TSString> serviceVec);
	std::vector<TSString>             GetServiceNameVec();
	void                                         TransferSelectService(std::vector<TSString>& vecService);
	std::vector<TSString>             GetSelectServiceVector();

private:
    void ProcessPreCompiles(TSIDLGram::ParserPtr Parse);
    void VisitPreCompile_p(TSIDLGram::Parser::Node * Node,TSIDLGram::ParserPtr Parse);
    TSIDLGram::ParserPtr ParseFile_p(const TSString & FileFullPath);
    TSString GetFileFullPath(const TSString & FileName);
    TSIDLPluginMgr(void);
    ~TSIDLPluginMgr(void);
    bool _IsTopicOrService;
    bool _IsPlugin;
    bool _HaveXsim;
    TSIDLPluginMgrPrivate * _p;
	std::map<TSString, TSString> _topicMap;
	std::vector<TSString>             _selectTopicVector;
	TSString                                  _selectFileName;
	std::map<std::pair<TSString, TSString>, TSString> _currentFileTopicVec;
	std::vector<TSString>            _serviceVec;
	std::vector<TSString>            _selectServiceVector;
	bool _isTool;
};

TOPSIMIDL_DECL TSIDLPluginMgr * PluginMgr();

TOPSIMIDL_DECL bool     HasOption(const TSString & Key);
TOPSIMIDL_DECL TSString GetBaseName(const TSString & FileFullPath);
TOPSIMIDL_DECL TSString GetAbsoluteDir(const TSString & FileFullPath);
TOPSIMIDL_DECL TSString GetPluginContent(const TSString & PluginName);

template<typename T>
FORCEINLINE T  Option(const TSString & Key)
{
    if(HasOption(Key))
    {
        return TSIDLPluginMgr::Instance()->GetOptionVal(Key).as<T>();
    }

    return T();
}

template<typename T>
boost::shared_ptr<T> GetPluginT()
{
    return boost::static_pointer_cast<T>(TSIDLPluginMgr::Instance()->GetPlugin(T::GetMetaTypeIdStatic()));
}

#define NODE_ERROR_LOG(ErrorMsg,Node) TSIDLPluginMgr::Instance()->LogErrorMsg(ErrorMsg,Node)

#endif


