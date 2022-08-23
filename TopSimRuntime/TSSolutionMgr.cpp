#include "stdafx.h"

#include <boost/filesystem.hpp>

#if defined(_MSC_VER)&&(BOOST_VERSION >= 106700)
#include <boost/process.hpp>
#endif

#if defined(XSIM3_2) || defined(XSIM3_3)
#else
#include <TopSimRuntime/TSOSMisc.h>
#endif

#include "TSSolutionMgr.h"
#include "TSIDLPluginMgr.h"
#include "TSIDLUtils.h"

SINGLETON_IMPLEMENT(TSSolutionMgr);

BEGIN_METADATA(TSSolution)
END_METADATA()

#if defined(_MSC_VER)&&(BOOST_VERSION >= 106700)
namespace bp = ::boost::process;
#endif

template<typename Proj> 
boost::shared_ptr<Proj> GetOrCreateProj_T(TSProjFileBasicPtr Basic,const TSString & ProjName,
	std::vector<std::pair<TSString,boost::shared_ptr<Proj> > > & ProjMap)
{
	typename std::vector<std::pair<TSString,boost::shared_ptr<Proj> > >::iterator it = ProjMap.begin(); 
    for (it;it != ProjMap.end();it++)
    {
        if (it->first == ProjName)
        {
            return it->second;
        }
    }

	boost::shared_ptr<Proj> ProjInstance = boost::make_shared<Proj>();
	ProjInstance->SetShortName(ProjName);
	ProjInstance->SetBasicDir(Basic->GetBasicDir());
	ProjInstance->SetSupperProj(Basic);
	ProjMap.push_back(std::pair<TSString,boost::shared_ptr<Proj> >(ProjName,ProjInstance));
	return ProjInstance;
	
}

struct TSSolutionMgrPrivate
{

	std::vector<TSSolutionPtr> _Solutions;
};

TSSolutionMgr::TSSolutionMgr(void)
	:_p(new TSSolutionMgrPrivate)
{
}


TSSolutionMgr::~TSSolutionMgr(void)
{
	delete _p;
}

bool TSSolutionMgr::GenSolutions()
{
	bool Ret = true;
    if (!HasOption(GXmlKey))
    {
        BOOST_FOREACH(TSSolutionPtr it,_p->_Solutions)
        {
            if(!it->GeneratorSolution())
            {
                Ret = false;
                break;
            }
        }
    }
	return Ret;
}

TSSolutionPtr TSSolutionMgr::GetOrCreateSolution( const TSString & SolutionName )
{
	TSSolutionPtr Solution;
    TSString codeType = TSIDLPluginMgr::Instance()->GetCodeType();
    bool     IsXidl   = TSIDLPluginMgr::Instance()->GetFileNameWithXidl();
    TSString AbsSlnPath = GetSolutionFullPathByName(SolutionName);

	//bool IsXsimBuiltIn = false;

	//if( SolutionName == "TopSimBrain_Inter" || SolutionName == "TopSimCore_Inter" || SolutionName == "TopSimMMF_Inter" || SolutionName == "TopSimModels_Inter")
	//{
	//	IsXsimBuiltIn = true;
	//}

    if (codeType == "c++")
    {
        if(IsXidl)//是true则加后缀_xidl
        {
            AbsSlnPath = GetSolutionFullPathByName(SolutionName + "_xidl/cpp");
        }
        else
        {
            AbsSlnPath = GetSolutionFullPathByName(SolutionName + "/cpp");
        }
       
    }
    else if (codeType == "c#")
    {
        AbsSlnPath = GetSolutionFullPathByName(SolutionName + "_xidl/c#");
    }
    else if (codeType == "java")
    {
        AbsSlnPath = GetSolutionFullPathByName(SolutionName + "_xidl/java");
    }

    BOOST_FOREACH(TSSolutionPtr it,_p->_Solutions)
    {
        if(it->GetFullPath() == AbsSlnPath)
        {
            Solution = it;
        }
    }

	if(!Solution)
	{
		TSString SlnOption = Option<TSString>(SlnKey);
		TSString SlnOpCongirue = TSStringUtil::Format("Config/Solutions/Solution[@Name='%s']",SlnOption.c_str());

		TSString SlnTypeName;
		if(pugi::xml_node Node = 
			TSIDLPluginMgr::Instance()->GetConfigureDocument()->select_single_node(SlnOpCongirue.c_str()).node())
		{
			SlnTypeName = Node.attribute("Realized").as_string();
		}

		if(SlnTypeName.empty())
		{
#ifdef _WIN32
			//使用默认解决方案.
			SlnTypeName = "TSVisualStudioSolution";
#else
			SlnTypeName = "TSLinuxSolution";
#endif
		}

        int TypeId = TSMetaType::GetType(SlnTypeName.c_str());
		Solution = TS_STATIC_CAST(TSMetaType4Link::ConstructSmart(TypeId),TSSolutionPtr);

		if(Solution)
		{
			Solution->SetFullPath(AbsSlnPath);
            Solution->SetSolutionName(SolutionName);
			_p->_Solutions.push_back(Solution);
		}
	}

	return Solution;
}

#if defined(_MSC_VER)&&(BOOST_VERSION >= 106700)
bp::child start_child(const TSString & cmdStr)
{
    bp::ipstream pipe_stream;
	bp::child cl(cmdStr.c_str());
	//bp::std_out > pipe_stream;
	//TSString line;
	//while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
	//{

		//std::cerr << line << std::endl;
	//}

	cl.wait();

	return cl;
}
#endif



bool TSSolutionMgr::BuildSolutions()
{
	bool Ret = true;
    if (!HasOption(GXmlKey))
    {
        BOOST_FOREACH(TSSolutionPtr allSolutionPtr,_p->_Solutions)
        {
            if(!allSolutionPtr->GeneratorSolution())
            {
                Ret = false;
                break;
            }
            TSString wordDir = GetSolutionFullPathByName(allSolutionPtr->GetSolutionName());

            bool     IsXidl   = TSIDLPluginMgr::Instance()->GetFileNameWithXidl();
            if( IsXidl )
            {
                wordDir += "_xidl";
            }
            else 
            {
                //不加
            }
			BuildJarFile(wordDir, allSolutionPtr->GetSolutionName());
        }
    }
	return Ret;
}

void TSSolutionMgr::SetSolutionPath( const TSString& solutionpathName)
{
    _savepath = solutionpathName;
    boost::algorithm::replace_all(_savepath, "\\","/");
}

TSString TSSolutionMgr::GetSolutionFullPathByName( const TSString & SolutionName )
{
    TSString SavePath = _savepath;

	if(SavePath.empty())
	{
        SavePath = Option<TSString>(SavePathKey);

        if(SavePath.empty())
        {
           SavePath = (boost::filesystem::current_path() / TS_TEXT("Projects")).string();
        }
	}

	return SavePath + FILE_SPLITE + SolutionName;
}

std::vector<TSSolutionPtr> TSSolutionMgr::GetSolutions()
{
    return _p->_Solutions;
}

struct TSProjFilePrivate
{
	TSString      _FileExt;
	TSString      _FileContent;
};

TSProjFile::TSProjFile()
	:_p(new TSProjFilePrivate)
{
    
}

TSProjFile::~TSProjFile()
{
	delete _p;
}

const TSString & TSProjFile::GetContent()
{
	return _p->_FileContent;
}

void TSProjFile::SetContent( const TSString & Content )
{
	_p->_FileContent = Content;
}
void TSSolutionMgr::CleanSolution()
{
   std::vector<TSSolutionPtr> tmp;
   _p->_Solutions.swap(tmp);
   TSString tmpsavepath;
   _savepath.swap(tmpsavepath);
}


#if defined(_MSC_VER)&&(BOOST_VERSION >= 106700)
void TSSolutionMgr::BuildJarFile(TSString javaWorkDir,TSString solutionName)
{
    TSString codeType = TSIDLPluginMgr::Instance()->GetCodeType();
    if( codeType != "java")
    {
        return;
    }

	//所有class文件存放的目录
	TSString AllClassDir = javaWorkDir + "\\java\\allClass";
    javaWorkDir += "\\java\\generic\\";

	if (!boost::filesystem::exists(AllClassDir))
	{
		boost::filesystem::create_directory(AllClassDir);
	}
	else if(boost::filesystem::is_directory(AllClassDir))
	{
		std::cout << GB18030ToTSString("allClass 目录已存在") << std::endl;
	}
    
    TSString workDir = boost::filesystem::current_path().string();

    //获取generic下面所有的文件夹名称
    boost::filesystem::path path(javaWorkDir);
    if( !boost::filesystem::exists(path))
    {
        TSString Exp = GB18030ToTSString("未找到java文件目录");
        std::cout << Exp << std::endl;
        throw(Exp);
    }

    //std::vector<TSString> allDirName;
    boost::filesystem::directory_iterator end_iter;
    for(boost::filesystem::directory_iterator iter(path); iter !=end_iter; ++iter)
    {
        if(  boost::filesystem::is_directory(iter->status()) )
        {
            _allDirName.push_back(iter->path().string());
        }
    
    }

    TSString classFileName = "";
	TSString javaFileName = "";
	TSString sourceFiles = "";
    for(int i = 0; i < _allDirName.size(); i++)
    {
		sourceFiles += _allDirName[i];;
		sourceFiles += "\\*.java ";
        classFileName += "\"" + _allDirName[i];
        classFileName += "\\*.class\" ";
    }


    TSString delClass = "DEL " + classFileName;

	for (int i = 0; i < _allDirName.size(); i++)
	{
		javaFileName = _allDirName[i];
		javaFileName += "\\* ";
		TSString classFile = "javac " + javaFileName + "-cp " + workDir + "/LinkStudio4Java.jar " + sourceFiles + " -d " + AllClassDir;
		boost::algorithm::replace_all(classFile, "\\", "/");
		bp::child geclass = start_child(classFile); //生成class文件
	}

    TSString jarFile = "jar cf " + solutionName + ".jar -C " + AllClassDir + FILE_SPLITE + " .";
    boost::algorithm::replace_all(jarFile, "\\","/");
    boost::algorithm::replace_all(delClass, "/","\\");

	bp::child geJar = start_child(jarFile);     //生成jar包
	//bp::child gedel = start_child(delClass);//删除文件夹里面的.class文件
    std::cout<< GB18030ToTSString("已经在SDK下生成")<<solutionName<< GB18030ToTSString(".jar文件")<<std::endl;
}
#else
void TSSolutionMgr::BuildJarFile(TSString javaWorkDir, TSString solutionName)
{
	TSString codeType = TSIDLPluginMgr::Instance()->GetCodeType();
	if (codeType != "java")
	{
		return;
	}
	javaWorkDir += "\\java\\generic\\";

	TSString workDir = boost::filesystem::current_path().string();

	//获取generic下面所有的文件夹名称
	boost::filesystem::path path(javaWorkDir);
	if (!boost::filesystem::exists(path))
	{
		TSString Exp = GB18030ToTSString("未找到java文件目录");
		std::cout << Exp << std::endl;
		throw(Exp);
	}

	//std::vector<TSString> allDirName;
	boost::filesystem::directory_iterator end_iter;
	for (boost::filesystem::directory_iterator iter(path); iter != end_iter; ++iter)
	{
		if (boost::filesystem::is_directory(iter->status()))
		{
			_allDirName.push_back(iter->path().string());
		}

	}

	TSString javaFileName = "";
	TSString classFileName = "";
	for (int i = 0; i < _allDirName.size(); i++)
	{
		javaFileName += _allDirName[i];
		javaFileName += "\\* ";
		classFileName += _allDirName[i];
		classFileName += "\\*.class ";
	}

	TSString delClass = "DEL " + classFileName;
	TSString classFile = "javac " + javaFileName + "-cp " + workDir + "//LinkStudio4Java.jar;";
	TSString jarFile = "jar cf " + solutionName + ".jar -C " + javaWorkDir + FILE_SPLITE + " .";
	boost::algorithm::replace_all(classFile, "\\", "/");
	boost::algorithm::replace_all(jarFile, "\\", "/");

	boost::algorithm::replace_all(delClass, "/", "\\");
	system(classFile.c_str());//生成class文件
#if defined(XSIM3_2) || defined(XSIM3_3)
	Sleep(100000);
#else
	TSOSMisc::USleep(100000);
#endif // XSIM3_2 || XSIM3_3
	system(jarFile.c_str());//生成jar 包
#if defined(XSIM3_2) || defined(XSIM3_3)
	Sleep(100000);
#else
	TSOSMisc::USleep(100000);
#endif // XSIM3_2 || XSIM3_3
	system(delClass.c_str());//删除文件夹里面的.class文件
	std::cout << GB18030ToTSString("已经在SDK下生成") << solutionName << GB18030ToTSString(".jar文件") << std::endl;
}
#endif

const TSString & TSProjFile::GetFileExt()
{
	return _p->_FileExt;
}

void TSProjFile::SetFileExt( const TSString & FileExt )
{
	_p->_FileExt = FileExt;
}

void TSProjFile::SetShortName( const TSString & FileShortName )
{
	TSProjFileBasic::SetShortName(FileShortName);
}

TSString TSProjFile::GetAbsoluteName()
{
	return TSProjFileBasic::GetAbsoluteName() + GetFileExt();
}

void TSProjFile::AppendFirst( const TSString & FirstContent )
{
	_p->_FileContent = FirstContent + _p->_FileContent;
}

void TSProjFile::AppendLast( const TSString & LastContent )
{
	_p->_FileContent = _p->_FileContent + LastContent;
}

void TSProjFile::SetFileType( const FileType & Filetype )
{
    _FileType = Filetype;
}

const FileType & TSProjFile::GetFileType()
{
    return _FileType;
}

struct TSProjectPrivate
{
    ProjFileMap_T      _Files;
};

TSProject::TSProject()
	:_p(new TSProjectPrivate)
{
    _IsNodeFilePro = false;
    _IsXsimPro = false;
}

TSProject::~TSProject()
{
	delete _p;
}

TSProjFilePtr TSProject::GetOrCreateProjFile( const TSString & FileShortName ,const TSString & Ext)
{
    TSString FileName = FileShortName + Ext;
    TSProjFilePtr result = GetOrCreateProj_T(shared_from_this(),FileName,_p->_Files);
    result->SetFileExt(Ext);

	return result;
}

const ProjFileMap_T & TSProject::GetProjFiles()
{
	return _p->_Files;
}

TSString TSProject::GetBasicDir()
{
	return TSProjFileBasic::GetBasicDir() + FILE_SPLITE + GetShortName();
}

const bool & TSProject::GetNodeFilePro()
{
    return _IsNodeFilePro;
}

void TSProject::SetNodeFilePro( const bool & IsNodeFile )
{
    _IsNodeFilePro = IsNodeFile;
}

const FileType & TSProject::GetProFileType()
{
    return _ProFileType;
}

void TSProject::SetProFileType(const FileType & ProFileType)
{
   _ProFileType = ProFileType;
}

bool TSProject::GetIsXismPro()
{
    return _IsXsimPro;
}

void TSProject::SetIsXismPro( const bool & IsXsimPro )
{
    _IsXsimPro = IsXsimPro;
}

struct TSProjFileBasicPrivate
{
	TSString            _BasicDir;
	TSString            _ShortName;
	TSProjFileBasicWPtr _SupperProj; 
};

TSProjFileBasic::TSProjFileBasic()
	:_p(new TSProjFileBasicPrivate)
{

}

TSProjFileBasic::~TSProjFileBasic()
{
	delete _p;
}

TSProjFileBasicPtr TSProjFileBasic::GetSupperProj()
{
	return _p->_SupperProj.lock();
}

void TSProjFileBasic::SetSupperProj( TSProjFileBasicPtr BasicProjDesc )
{
	_p->_SupperProj = BasicProjDesc;
}

TSString  TSProjFileBasic::GetShortName()
{
	return _p->_ShortName;
}

void TSProjFileBasic::SetShortName( const TSString & FileShortName )
{
	_p->_ShortName = FileShortName;
}

void TSProjFileBasic::SetBasicDir( const TSString & BasicDir )
{
	_p->_BasicDir = BasicDir;
}

TSString  TSProjFileBasic::GetBasicDir()
{
	return _p->_BasicDir;
}

TSString TSProjFileBasic::GetAbsoluteName()
{
	return _p->_BasicDir + FILE_SPLITE + _p->_ShortName;
}

struct TSSolutionPrivate
{
	TSString _SlnFullPath;
    ProjMap_T _Projs;
};

TSSolutionMgr * SolutionMgr()
{
	return TSSolutionMgr::Instance();
}

void TSSolution::SetFullPath( const TSString & SlnFullPath )
{
	_p->_SlnFullPath = SlnFullPath;

	SetBasicDir(GetAbsoluteDir(SlnFullPath));
	SetShortName(GetBaseName(SlnFullPath));
}

const TSString & TSSolution::GetFullPath()
{
	return _p->_SlnFullPath;
}

TSProjectPtr TSSolution::GetOrCreateProj( const TSString & ProjName)
{

    return GetOrCreateProj_T(shared_from_this(),ProjName,_p->_Projs);
}

const ProjMap_T & TSSolution::GetProjects()
{
    return _p->_Projs;
}

TSSolution::TSSolution()
	:_p(new TSSolutionPrivate)
{

}

TSSolution::~TSSolution()
{
	delete _p;
}

void TSSolution::SetSolutionName( const TSString & SolutionName )
{
    _SolutionName = SolutionName;
}

const TSString & TSSolution::GetSolutionName()
{
    return _SolutionName;
}
