#ifndef __TSSOLUTIONMGR_H__
#define __TSSOLUTIONMGR_H__

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimTypes/TSSingleton.h>
#include <TopSimUtil/pugixml.hpp>
#else
#include <TopSimRuntime/TSSingleton.h>
#include <TopSimRuntime/pugixml.hpp>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TopSimIDL.h"

#include "TSIdlGram.h"

#define FILE_SPLITE "/"

using namespace  TSIDLGram;

STRUCT_PTR_DECLARE(TSProjFile);
STRUCT_PTR_DECLARE(TSProject);
STRUCT_PTR_DECLARE(TSSolution);
STRUCT_PTR_DECLARE(TSProjFileBasic);

typedef std::vector<std::pair<TSString,TSProjFilePtr>> ProjFileMap_T;
typedef std::vector<std::pair<TSString,TSProjectPtr>>  ProjMap_T;

enum FileType
{
    None   ,        
    Generic,
    Impl
};
struct TSProjFileBasicPrivate;
struct TOPSIMIDL_DECL TSProjFileBasic : public boost::enable_shared_from_this<TSProjFileBasic>
{
public:
	TSProjFileBasic();
	virtual ~TSProjFileBasic();
public:
	virtual TSProjFileBasicPtr GetSupperProj();
	virtual void               SetSupperProj(TSProjFileBasicPtr BasicProjDesc);
	virtual TSString           GetShortName();
	virtual void               SetShortName(const TSString & FileShortName);
	virtual void               SetBasicDir(const TSString & BasicDir);
	virtual TSString           GetBasicDir();
	virtual TSString           GetAbsoluteName();
    

private:
	TSProjFileBasicPrivate * _p;
    
};

struct TSProjFilePrivate;
struct TOPSIMIDL_DECL TSProjFile : public TSProjFileBasic
{

public:
    TSProjFile();
    ~TSProjFile();

	const TSString & GetContent();
	void             SetContent(const TSString & Content);
	void             AppendFirst(const TSString & FirstContent);
	void             AppendLast(const TSString & LastContent);
	const TSString & GetFileExt();
	void             SetFileExt(const TSString &  FileExt);  
    void             SetFileType(const FileType & Filetype);
    const FileType & GetFileType();
	virtual void     SetShortName(const TSString & FileShortName);
	virtual TSString GetAbsoluteName();

private:
	TSProjFilePrivate * _p;
    FileType _FileType;
    
};

struct TSProjectPrivate;
struct TOPSIMIDL_DECL TSProject : public TSProjFileBasic
{
public:
	TSProject();
	~TSProject();
public:
    TSProjFilePtr         GetOrCreateProjFile(const TSString & FileShortName,const TSString & Ext);
    const ProjFileMap_T & GetProjFiles();
    const bool     & GetNodeFilePro();;
    void             SetNodeFilePro(const bool & IsNodeFile); 
    const FileType & GetProFileType();
    void             SetProFileType(const FileType & ProFileType);
    virtual TSString      GetBasicDir();
    bool             GetIsXismPro();
    void             SetIsXismPro(const bool & IsXsimPro);
private:
	TSProjectPrivate * _p;
    bool _IsNodeFilePro;
    FileType _ProFileType;
    bool _IsXsimPro;
};

struct TSSolutionPrivate;
struct TOPSIMIDL_DECL TSSolution : public TSProject
{
	TS_MetaType(TSSolution);
public:
	TSSolution();
	~TSSolution();
	virtual void             SetFullPath(const TSString & SlnFullPath);
	virtual const TSString & GetFullPath();
    TSProjectPtr             GetOrCreateProj(const TSString &ProjName);
    const ProjMap_T &        GetProjects();
	virtual bool             GeneratorSolution() = 0;
    void                     SetSolutionName(const TSString & SolutionName);
    const TSString &         GetSolutionName();
private:
	TSSolutionPrivate * _p;
    TSString _SolutionName;
};

struct TSSolutionMgrPrivate;
class TOPSIMIDL_DECL TSSolutionMgr : public TSSingleton<TSSolutionMgr>
{
	SINGLETON_DECLARE(TSSolutionMgr);
public:
	TSSolutionMgr(void);
	~TSSolutionMgr(void);
	
	TSSolutionPtr GetOrCreateSolution(const TSString & SolutionName);
	bool          GenSolutions();
	bool          BuildSolutions();
    void          BuildJarFile(TSString wordDir,TSString solutionName);
    void          CleanSolution();
    void          SetSolutionPath( const TSString& solutionpathName);
    std::vector<TSSolutionPtr> GetSolutions();

private:
	TSString      GetSolutionFullPathByName(const TSString & SolutionName);
    TSString      _savepath; 
	TSSolutionMgrPrivate * _p;
    std::vector<TSString> _allDirName;
};

TOPSIMIDL_DECL TSSolutionMgr * SolutionMgr();

#endif

