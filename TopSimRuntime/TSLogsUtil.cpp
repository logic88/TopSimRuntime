#include "stdafx.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include "TSIDLUtils.h"
#include "TSLinuxSolution.h"
#include "temp/temp.h"
#include "temp/cmake.h"
#include "temp/templeate.h"

#define  LINKSDKDEF "LINKSDK"
#define  LORISSDK   "LorisSDK"
#define  XSIMSDK    "XSIMSDK"

BEGIN_METADATA(TSLinuxSolution)
	REG_BASE(TSSolution);
END_METADATA()

struct TSLinuxSolutionPrivate
{	
	TSString _OutPutDir;
	TSString _ThirdPartDir;
};

TSLinuxSolution::TSLinuxSolution(void)
	:_p(new TSLinuxSolutionPrivate)
{
	InitSolutionConfig();
}


TSLinuxSolution::~TSLinuxSolution(void)
{
	delete _p;
}

bool TSLinuxSolution::GeneratorSolution()
{
	TSString curPath = boost::filesystem::current_path().string();
	boost::algorithm::replace_all(curPath, "\\", "/");

	TSString thirdPart;
	//��ȡThirdpart��������
	char* Part = getenv("Thirdpart");
	if (Part != NULL)
	{
		thirdPart = Part;
		//�� \ �滻Ϊ /
		boost::algorithm::replace_all(thirdPart, "\\", "/");
	}

	//EG:AIModels_Inter
	TSString projectName = GetSolutionName();
	TSString savePath;
	bool isDefaultPath = false;

	//�ж��Ƿ�Ҫ������������ɵ�ָ��Ŀ¼
	if (HasOption(SavePathKey))
	{
		savePath = Option<TSString>(SavePathKey);

		size_t pos1 = savePath.find("/", 0);
		size_t pos2 = savePath.find("\\", 0);

		if ((pos1 == TSString::npos) && (pos2 == TSString::npos))
		{
			savePath = (boost::filesystem::current_path() / Option<TSString>(SavePathKey)).string();
			boost::algorithm::replace_all(savePath, "\\", "/");
		}
	}
	else
	{
		isDefaultPath = true;
		savePath = (boost::filesystem::current_path() / TS_TEXT("Projects")).string();
		boost::algorithm::replace_all(savePath, "\\", "/");
	}

	TSString codeType = TSIDLPluginMgr::Instance()->GetCodeType();

	TSString cppPathStr;
	
	bool isP = false;
	
	if (codeType == "c++")
	{
		// ����-p������else���
		if (TSIDLPluginMgr::Instance()->GetFileNameWithXidl())
		{
			cppPathStr = savePath + "/" + projectName + "_xidl/cpp";
		}
		else
		{
			isP = true;
			cppPathStr = savePath + "/" + projectName + "/cpp";
		}
	}
	else
	{
		cppPathStr = savePath + "/" + projectName + "_xidl/cpp";
	}
	
	TSString csharpPathStr = savePath + "/" + projectName + "_xidl/csharp";
	
	TSString tempStr = "ADD_SUBDIRECTORY(" + csharpPathStr + ")\n";

	TSString genericPathStr = cppPathStr + "/" + "generic";

	TSString implPathStr = cppPathStr + "/" + "impl";

	TSString genericList;

	TSString implPath = TSIDLPluginMgr::Instance()->GetImplFullPath();

	TSString genericPath = TSIDLPluginMgr::Instance()->GetGenericFullPath();

	//E.G: E:\\LinkSDK\\Projects/xxxxxx/cpp
	TSString SolutionDir = GetBasicDir();

	//��ȡ��Ŀ���Ƽ���Ŀ¼�е��ļ� Key: ��Ŀ����Val: �ļ���ָ��
	const ProjMap_T &Projs = GetProjects();

	std::vector<TSString> Files;

	TSString subDir;

	//cpp��CMakeListsģ��
	TSString ProCMakeLists = PROCMAKELISTS;

	//*_xidl Ŀ¼��CMakeList
	TSString CMakeLists = CMAKELISTS;
	int nCount = 1;
	
	TSString CSharpMakeLists = CSHARPCMAKELISTS;
	TSString CSSubMake = CSHARPSUBCMAKELISTS;

	//ѭ��������ĿĿ¼
	for (ProjMap_T::const_iterator iter = Projs.begin(); iter != Projs.end(); iter++)
	{
		//E.G: ��Ŀ����HuaruDataInteract_Inter��
		TSString projName = iter->first;

		//����Ŀ·��
		TSString subProjPahtStr;

		if (projectName == projName)
		{
			subProjPahtStr = cppPathStr;
		}
		else
		{
			//���񹤳�·��
			subProjPahtStr = implPathStr + "/" + projName;
		}

		//��ȡ��ǰ��ĿĿ¼�µ������ļ� Key����Ŀ����Val���ļ���ָ��
		const ProjFileMap_T files = iter->second->GetProjFiles();

		//ѭ������Ŀ¼�µ��ļ�
		for (ProjFileMap_T::const_iterator iterfile = files.begin(); iterfile != files.end(); iterfile++)
		{
			//E.G: ��Ŀ����HuaruDataInteract_Inter��
			TSString shortName = iter->second->GetShortName();

			TSString FileName = iterfile->first;
			
			//E.G: E:\\LinkSDK\\Projects/xxxxxx/cpp
			TSString SolutionPath = SolutionDir;

			if (codeType == "c#")
			{
				//�滻·���е�c#������cmake�ᱨ��
				SolutionPath.replace(SolutionPath.find("c#"), 2, "csharp");
			}

			if (iterfile->second->GetFileType() == None)
			{
				TSIDLUtils::WriteFile(FileName, iterfile->second->GetContent(), SolutionPath);
			}
			else if (iterfile->second->GetFileType() == Generic)
			{
				TSIDLUtils::WriteFile(FileName, iterfile->second->GetContent(), SolutionPath + "/generic/");
			}
			else if (iterfile->second->GetFileType() == Impl)
			{
				TSIDLUtils::WriteFile(FileName, iterfile->second->GetContent(), SolutionPath + "/impl/" + shortName);
			}
		}

		if (codeType == "c#")
		{
			TSString FilePath = SolutionDir;

			boost::algorithm::replace_all(FilePath, "\\", "/");

			if (boost::algorithm::starts_with(FilePath, savePath))
			{
				boost::algorithm::erase_first(FilePath, savePath + "/");
			}

			FilePath.replace(FilePath.find("c#"), 2, "csharp");

			ProjFileMap_T::const_iterator itr = files.begin();
			for (; itr != files.end(); ++itr)
			{
				TSString File;

				if (itr->second->GetFileType() == None)
				{
					//����� csharp·��������cmake����
					File = itr->first;
				}
				else if (itr->second->GetFileType() == Generic)
				{
					File = FilePath + "/generic/" + itr->first;
				}
				/*else if (itr->second->GetFileType() == Impl)
				{
					File = FilePath + "/impl/" + iter->second->GetShortName() + "/" + itr->first;
				}*/

				Files.push_back(File);
			}
		}

		if (codeType == "c++" || codeType == "python")
		{
			TSString currentPath = projName;
			boost::algorithm::replace_all(currentPath, ".", "_");

			//�滻*_xidl Ŀ¼��CMakeList
			boost::algorithm::replace_all(CMakeLists, "{{Project_Name}}", projectName);
			boost::algorithm::replace_all(CMakeLists, "{{cmakeFile}}", curPath);
			boost::algorithm::replace_all(CMakeLists, "{{CMakeListPath}}", cppPathStr);

			//��� implĿ¼����
			if (!implPath.empty())
			{
				//��ӱ������Ŀ¼
				subDir += "ADD_SUBDIRECTORY(subProPath)\n";

				//�滻��Ŀ¼·��
				boost::algorithm::replace_all(subDir, "subProPath", subProjPahtStr);

				/*TSString subPro = "ADD_SUBDIRECTORY(subProPath)";
				boost::algorithm::replace_all(subPro, "subProPath", implPath);
				boost::algorithm::replace_all(CMakeLists, "{{SUNPRO}}", subPro);*/
			}

			/*���沿�ִ�����������ProjectCMakeLists�ļ����������������̵Ķ�̬��*/
		
			TSString BaseName = GetSolutionName();
			TSString FileFullPath = savePath + "/" + BaseName + ".xidl";

			//���ļ�������ʽ��ȡ�ļ�
			std::ifstream ifs(FileFullPath.c_str(), std::ifstream::in | std::ifstream::binary);

			std::ostringstream oss;
			//���ļ�����������������������
			oss << ifs.rdbuf();

			//xidl�ļ�����
			TSString xidlcontent = Utf8ToAscii(oss.str()).c_str();
			//xidl�ļ��� import�ؼ��ֿ�ʼ��λ�ã�posendimport�Ľ���λ�ü�"\n"λ��
			TSString::size_type posstart, posend;
			int pos = 0;
			//�洢��ǰ��������������xidl
			std::vector<TSString> vecxidl;

			//�Ե�ǰ��������������xidl������ȡ
			while ((posstart = xidlcontent.find("import", pos)) != TSString::npos)
			{
				posend = xidlcontent.find("\n", pos + 1);
				TSString::size_type arrowstart, pospoint;
				arrowstart = xidlcontent.find("<", pos);
				pospoint = xidlcontent.find(".", pos);
				TSString strxidl = xidlcontent.substr(arrowstart + 1, pospoint - arrowstart - 1);
				vecxidl.push_back(strxidl);
				pos = posend;
			}

			//�����������xidl����ƴ��
			TSString res;
			for (auto it = vecxidl.begin(); it != vecxidl.end(); ++it)
			{
				TSString temp = *it;
				res += temp + " ";
			}

			if (projectName != projName)
			{
				genericList += projName + " ";
			}

			
			//�滻cpp��CMakeListsģ��
			boost::algorithm::replace_all(ProCMakeLists, "_TOPICXIDL_", res);
			boost::algorithm::replace_all(ProCMakeLists, "{{cmakeFile}}", curPath);
			boost::algorithm::replace_all(ProCMakeLists, "{{Projects_Path}}", savePath);

			//���XIDLѡ����--Python ��������Ӧ��CMakeLists����
			if (HasOption("Python"))
			{
				TSString pythonStr = "ADD_DEFINITIONS(-D_PYTHON)";
				TSString pydWarp = "-lTopSimDataInterfacePydWarp";

				boost::algorithm::replace_all(ProCMakeLists, "#{{PYTHON}}", pythonStr);
				boost::algorithm::replace_all(ProCMakeLists, "#{{PydWarp}}", pydWarp);
			}

			//�������impl�ļ������ڸ��ļ����´���һ��CmakeLists�ļ�
			if (!implPath.empty())
			{
				//ProjectCMakeLists
				TSString SubProCMakeLists = SUBPROCMAKELISTS;

				//�滻����Ŀ¼�ײ�CMakeLists
				boost::algorithm::replace_all(SubProCMakeLists, "_TOPICXIDL_", res);
				boost::algorithm::replace_all(SubProCMakeLists, "{{cmakeFile}}", curPath);
				boost::algorithm::replace_all(SubProCMakeLists, "{{Projects_Path}}", savePath);
				boost::algorithm::replace_all(SubProCMakeLists, "{{CPP_PATH}}", cppPathStr);
				boost::algorithm::replace_all(SubProCMakeLists, "{{CMAKEFUNCTION}}", "GENERATE_LINK_SERVICE_INTERFACE_FRAMEWORK_PROJECT");
				boost::algorithm::replace_all(SubProCMakeLists, "{{GENERICPATH}}", subProjPahtStr);
				boost::algorithm::replace_all(SubProCMakeLists, "{CURRENT_FOLDER}", currentPath);

				//���XIDLѡ����--Python ��������Ӧ��CMakeLists����
				if (HasOption("Python"))
				{
					TSString pythonStr = "ADD_DEFINITIONS(-D_PYTHON)";
					TSString pydWarp = "-lTopSimDataInterfacePydWarp";

					boost::algorithm::replace_all(SubProCMakeLists, "#{{PYTHON}}", pythonStr);
					boost::algorithm::replace_all(SubProCMakeLists, "#{{PydWarp}}", pydWarp);
				}

				if (projectName != projName)
				{
					TSIDLUtils::WriteFile("CMakeLists.txt", SubProCMakeLists, subProjPahtStr);
				}
			}
		}
		else if (codeType == "c#")
		{
			TSString files;
			
			//��� implĿ¼����
			if (!implPath.empty())
			{
				if (1 != nCount)
				{
					tempStr += "ADD_SUBDIRECTORY(" + csharpPathStr + '/' + "impl" + '/' + projName + ")\n";
				}
			}

			//ƴ�����е� .cs�ļ�
			for (int i = 0; i < Files.size(); ++i)
			{
				files += Files[i] + " ";
			}

			//�滻CSharp��CMakeLists
			boost::algorithm::replace_all(CSharpMakeLists, "{{Files}}", files);
			
			//��� implĿ¼����
			if (!implPath.empty())
			{
				if (1 != nCount)
				{
					boost::algorithm::replace_all(CSSubMake, "_SERVER_", projName);
					TSString servicePath = csharpPathStr + '/' + "impl/" + projName;
					TSIDLUtils::WriteFile("CMakeLists.txt", CSSubMake, servicePath);
					boost::algorithm::replace_all(CSSubMake, projName, "_SERVER_");
				}
			}

			nCount++;
		}
		else if (codeType == "java")
		{
			return true;
		}
	}

	std::vector<TSString> Projects;

	const TSIDLPluginMgr::ParseMap_T & parsemap = PluginMgr()->GetParserUtils();
	TSIDLPluginMgr::ParseMap_T::const_iterator iterU = parsemap.begin();
	for (; iterU != parsemap.end(); iterU++)
	{
		if (TSIDLPluginMgr::Instance()->IsImportParse(iterU->second->Parse))
		{
			TSString TempFileName;
			TSString FileName = iterU->second->Parse->GetFullFileName();
			boost::algorithm::replace_all(FileName, "\\", "/");
			TSString::size_type position = FileName.find("/");

			if (position != TSString::npos)
			{
				std::vector<TSString> FilePaths;
				TempFileName = FileName;
				boost::split(FilePaths, TempFileName, boost::is_any_of("/"));
				std::vector<TSString>::reverse_iterator it = FilePaths.rbegin();
				TempFileName = *it;
				position = TempFileName.find(".");
				if (position != TSString::npos)
				{
					std::vector<TSString> FileNameVct;
					boost::split(FileNameVct, TempFileName, boost::is_any_of("."));
					std::vector<TSString>::iterator itr = FileNameVct.begin();
					TempFileName = *itr;
				}
			}

			if (TSIDLPluginMgr::Instance()->IsDefualtXidlFile(TempFileName))
			{
				continue;
			}

			std::vector<TSString>::iterator it = std::find(Projects.begin(), Projects.end(), TempFileName);
			if (it == Projects.end())
			{
				Projects.push_back(TempFileName);

				//��ӱ������Ŀ¼
				subDir += "ADD_SUBDIRECTORY(subProPath)\n";

				// ����-p������else���
				if (TSIDLPluginMgr::Instance()->GetFileNameWithXidl())
				{
					//�滻��Ŀ¼·��
					boost::algorithm::replace_all(subDir, "subProPath", savePath + "/" + TempFileName + "_xidl" + "/cpp");
				}
				else
				{
					//�滻��Ŀ¼·��
					boost::algorithm::replace_all(subDir, "subProPath", savePath + "/" + TempFileName + "/cpp");
				}
			}
		}
	}


	if (codeType == "c++" || codeType == "python")
	{
		//��� implĿ¼����
		if (!implPath.empty())
		{
			//�滻��CMakeLists��
			boost::algorithm::replace_all(CMakeLists, "{{SUNPRO}}", subDir);
		}
		else
		{
			TSString tempStr = "ADD_SUBDIRECTORY(" + cppPathStr + ")\n";
			//�滻��CMakeLists��
			boost::algorithm::replace_all(CMakeLists, "{{SUNPRO}}", tempStr + subDir);
		}

		if (!genericPath.empty())
		{
			boost::algorithm::replace_all(ProCMakeLists, "{{CMAKEFUNCTION}}", "GENERATE_LINK_SERVICE_FRAMEWORK_PROJECT");
			//boost::algorithm::replace_all(ProCMakeLists, "{{GENERICPATH}}", genericPath);
			boost::algorithm::replace_all(ProCMakeLists, "{{GENERIC_LIST}}", genericList);
		}
		else
		{
			boost::algorithm::replace_all(ProCMakeLists, "{{CMAKEFUNCTION}}", "GENERATE_LINK_FRAMEWORK_PROJECT");
			boost::algorithm::replace_all(ProCMakeLists, "{{GENERICPATH}}", "");
			boost::algorithm::replace_all(ProCMakeLists, "${PROJECT_NAME}", projectName);
			boost::algorithm::replace_all(ProCMakeLists, "${GENERIC_PROJECT_LIST}", "");

		}

		//��CMakeд��cppĿ¼��
		TSIDLUtils::WriteFile("CMakeLists.txt", ProCMakeLists, SolutionDir);
	}
	else if (codeType == "c#")
	{
		boost::algorithm::replace_all(CMakeLists, "{{SUNPRO}}", tempStr);
		//�滻*_xidl Ŀ¼��CMakeList
		boost::algorithm::replace_all(CMakeLists, "{{Project_Name}}", projectName);
		boost::algorithm::replace_all(CMakeLists, "{{cmakeFile}}", curPath);

		boost::algorithm::replace_all(CSharpMakeLists, "{{Project_Name}}", projectName);
		TSString curPath = boost::filesystem::current_path().string();
		//�� \ �滻Ϊ /
		boost::algorithm::replace_all(curPath, "\\", "/");
		boost::algorithm::replace_all(CSharpMakeLists, "_LINK_", curPath + "/");

		TSIDLUtils::WriteFile("CMakeLists.txt", CSharpMakeLists, csharpPathStr);
	}

	//д��*_xidl Ŀ¼��CMakeList
	if (codeType == "c++")
	{
		TSIDLUtils::WriteFile("CMakeLists.txt", CMakeLists, savePath);
	}
	else
	{
		TSIDLUtils::WriteFile("CMakeLists.txt", CMakeLists, savePath + "/" + projectName + "_xidl");
	}

	//build.sh
	TSString CMakeBuild = BUILDSH;

#ifdef __x86_64__
	boost::algorithm::replace_all(CMakeBuild, "_X86PLATFORM_", "export CXXFLAGS=\"-m64\"\nexport CFLAGS=\"-m64\"");
#else
	boost::algorithm::replace_all(CMakeBuild, "_X86PLATFORM_", "");
#endif

	//�滻build.sh�ű�����
	if (!_p->_OutPutDir.empty())
	{
		boost::algorithm::replace_all(CMakeBuild, "_USERDEFOUTPUT_", "export USERDEFOUTPUT=" + _p->_OutPutDir);
	}
	else
	{
		boost::algorithm::replace_all(CMakeBuild, "_USERDEFOUTPUT_", "");
	}

	if (!_p->_ThirdPartDir.empty())
	{
		boost::algorithm::replace_all(CMakeBuild, "_THIRDPARTDIR_", "export ThirdPart=" + _p->_ThirdPartDir);
	}
	else
	{
		boost::algorithm::replace_all(CMakeBuild, "_THIRDPARTDIR_", "");
	}

	TSString srcPath;
	//�ж��Ƿ��в���-p
	if (isP)
	{
		srcPath = savePath + '/' + projectName;
	}
	else
	{
		srcPath = savePath + '/' + projectName + "_xidl";
	}

	boost::algorithm::replace_all(CMakeBuild, "{{Project_Path}}", srcPath);

	//boost::algorithm::replace_all(CMakeBuild, "{{Project_Path}}", savePath);



	boost::algorithm::replace_all(CMakeBuild, "{{SDK_PATH}}", curPath);

#ifdef __aarch64__
	boost::algorithm::replace_all(CMakeBuild, "_CMAKEPATH_", "");
#else
	boost::algorithm::replace_all(CMakeBuild, "_CMAKEPATH_", curPath + "/cmake/bin/");
#endif

	//д����Ӧ�ļ�
	TSString chmod;
	TSIDLUtils::WriteFile("build.sh", CMakeBuild, savePath);
	chmod = "chmod 777 " + savePath + "/build.sh";
	if (system(chmod.c_str()))
	{
		std::cout << GB18030ToTSString("chmodִ��ʧ�ܣ�") << std::endl;
	}
	else
	{
		std::cout << GB18030ToTSString("chmodִ����ϣ�") << std::endl;
	}

	//ִ��build.sh�ű�����CMake
	if (system((savePath + "/build.sh").c_str()))
	{
		std::cout << GB18030ToTSString("cmakeִ��ʧ�ܣ�") << std::endl;
	}
	else
	{
		std::cout << GB18030ToTSString("cmakeִ����ϣ�") << std::endl;
	}


/*------------������premake�ϴ���-------------------*/

//TSString Lua;
//	TSString PrivateFilePath = TSIDLPluginMgr::Instance()->GetPravatFilePath();
//	TSString SDKPath = TSIDLPluginMgr::Instance()->GetSDKPath();
//    TSString BaseName = GetSolutionName();
//    TSString SolutionDir = GetBasicDir();
//    TSString codeType = TSIDLPluginMgr::Instance()->GetCodeType();
//    const ProjMap_T &Projs = GetProjects();
//    for (ProjMap_T::const_iterator iter = Projs.begin(); iter != Projs.end(); iter++)
//    {
//        TSString ProjName = iter->first;
//        TSString baseDir = iter->second->GetBasicDir();
//        TSString shortName = iter->second->GetShortName();
//        bool IsNodeFilePro = iter->second->GetNodeFilePro();
//        TSString ProDir,cppFiles,csfile,javafile;
//        if (iter->second->GetProFileType() == None || iter->second->GetProFileType() == Generic)
//        {
//            if (codeType == "c++" || codeType == "python")
//            {
//                ProDir = "../cpp";
//                cppFiles = "\"generic/**.cpp\",\"generic/**.h\",\"generic/**.xidl\",\"./*cpp\",\"./*.h\",\"./*.xidl\""; 
//            }
//            else if (codeType == "c#")
//            {
//                ProDir = "../c#";
//                csfile = "\"generic/**.cs\",\"generic/**.xidl\",\"./*cs\",\"./*.xidl\""; 
//            } 
//            else if (codeType == "java")
//            {
//                ProDir = "../java";
//                javafile = "\"generic/**.java\",\"generic/**.xidl\",\"./*java\",\"./*.xidl\"";
//            }
//        }
//        else if (iter->second->GetProFileType() == Impl)
//        {
//            if (codeType == "c++" || codeType == "python")
//            {
//                ProDir = "impl/" + shortName;
//                cppFiles = "\"" + ProDir + "/**.cpp\"," + "\"" + ProDir + "/**.h\"," + "\"" + ProDir + "/**.xidl\"";
//            }
//            else if (codeType == "c#")
//            {
//                ProDir = "impl/" + shortName;
//                csfile = "\"" + ProDir + "/**.cs\","  + "\"" + ProDir + "/**.xidl\"";
//            }
//            else if (codeType == "java")
//            {
//                ProDir = "impl/" + shortName;
//                javafile = "\"" + ProDir + "/**.java\","  + "\"" + ProDir + "/**.xidl\"";
//            }   
//        }
//        TSString luaPro= ReadLuaConfig();
//
//		boost::algorithm::replace_all(luaPro, "_NAME_", ProjName);
//        boost::algorithm::replace_all(luaPro, "_NAMES_", ProDir );
//        boost::algorithm::replace_all(luaPro, "_FILES_", cppFiles);
//        boost::algorithm::replace_all(luaPro, "_FILE_", csfile);
//        boost::algorithm::replace_all(luaPro, "_FILE_", javafile);
//        boost::algorithm::replace_all(luaPro, "_SOLUTION_PRONAME_",BaseName);
//
//		if (!TSIDLPluginMgr::Instance()->GetSDKPath().empty())
//		{
//			boost::algorithm::replace_all(luaPro, "_SDK_DIR_", TSIDLPluginMgr::Instance()->GetSDKPath());
//		}
//		else
//		{
//			if (HasOption(SetSDKKey))
//			{
//				TSString sdkName = Option<TSString>(SetSDKKey);
//				if (!sdkName.empty())
//				{
//					boost::algorithm::replace_all(luaPro, "_SDK_DIR_", sdkName);
//				}
//				else
//				{
//					TSString expThro = TSString("SDKδָ��");
//					throw TSException(expThro);
//				}
//			}
//			else if (HasOption(LorisKey) ||  TSIDLPluginMgr::Instance()->GetIsPlugin())
//			{
//				boost::algorithm::replace_all(luaPro, "_LORIS_LDIR_","-lLorisInterface");
//				boost::algorithm::replace_all(luaPro, "_SDK_DIR_", LORISSDK);
//			}
//			else if (TSIDLPluginMgr::Instance()->GetHaveXimPlugin())
//			{
//				boost::algorithm::replace_all(luaPro, "_SDK_DIR_", XSIMSDK);
//				boost::algorithm::replace_all(luaPro, "_LORIS_LDIR_","");
//			}
//			else
//			{
//				boost::algorithm::replace_all(luaPro, "_SDK_DIR_", LINKSDKDEF);
//				boost::algorithm::replace_all(luaPro, "_LORIS_LDIR_","");
//			}
//		}
//
//		if (SDKPath == XSIMSDK)
//		{
//			TSString Links = "-lTopSimModels -lTopSimMMF -lTopSimBrain ";
//
//			if (IsLinkTopicProject(PrivateFilePath, ProjName))
//			{
//				Links += "-l" + ProjName + "_Topic";
//			}
//
//			boost::algorithm::replace_all(luaPro, "_XSIMLINKS_", Links);
//		}
//		else
//		{
//			boost::algorithm::replace_all(luaPro, "_XSIMLINKS_", "");
//		}
//
//        TSString VerSion;
//        if (HasOption(VerSionKey))
//        {
//            TSString verkey;
//            verkey = Option<TSString>(VerSionKey);
//            VerSion += ",\"" + verkey + "\",\"EXTERNAL_METATYPE_USED\"";
//        }
//        else
//        {
//            VerSion = ",\"XSIM3_2\",\"EXTERNAL_METATYPE_USED\"";
//        }
//        boost::algorithm::replace_all(luaPro, "_LINKER_VERSION_", VerSion);
//        TSString IncludeLibPath = "";
//        TSString SavePath = "";
//        PluginMap_T plugins =  TSIDLPluginMgr::Instance()->GetPlugins();
//        TSString CSdll = ",\"LinkStudioCS\""; 
//        if (plugins.find(TSMetaType::GetType("TSLorisPlugin")) != plugins.end() ||
//            plugins.find(TSMetaType::GetType("TSLorisPlugin4CSharp")) != plugins.end())
//        {
//            if (HasOption("Loris"))
//            {
//                CSdll += ",\"LorisInterface4CS\""; 
//            }   
//        }
//        
//        if (shortName != GetSolutionName())
//        {
//            CSdll += ",\"" + GetSolutionName() + "\"";
//        }
//        if (HasOption(SavePathKey))
//        {
//           SavePath = Option<TSString>(SavePathKey);
//           size_t pos1 = SavePath.find("/",0);
//           size_t pos2 = SavePath.find("\\",0);
//
//           if ((pos1 == TSString::npos)&&(pos2 == TSString::npos))
//           {
//               SavePath = (boost::filesystem::current_path() / Option<TSString>(SavePathKey)).string(); 
//               boost::algorithm::replace_all(SavePath, "\\","/");
//           }
//        }
//        else
//        {
//            SavePath = (boost::filesystem::current_path() / TS_TEXT("Projects")).string();
//            boost::algorithm::replace_all(SavePath, "\\","/");
//        }
//        const TSIDLPluginMgr::ParseMap_T & parsemap = PluginMgr()->GetParserUtils();
//        TSIDLPluginMgr::ParseMap_T::const_iterator iterU = parsemap.begin();
//        IncludeLibPath += ",\"" + SolutionDir + "/bin\"";
//        for (; iterU != parsemap.end(); iterU++)
//        {
//            if (TSIDLPluginMgr::Instance()->IsImportParse(iterU->second->Parse))
//            {
//                if (codeType == "c++")
//                {
//					TSString TempFileName;
//					bool IsXidl = TSIDLPluginMgr::Instance()->GetFileNameWithXidl();
//					TSString FileName = iterU->second->Parse->GetFullFileName();
//					boost::algorithm::replace_all(FileName, "\\", "/");
//					TSString::size_type position = FileName.find("/");
//					if (position != TSString::npos)
//					{
//						std::vector<TSString> FilePaths;
//						TempFileName = FileName;
//						boost::split(FilePaths, TempFileName, boost::is_any_of("/"));
//						std::vector<TSString>::reverse_iterator it = FilePaths.rbegin();
//						TempFileName = *it;
//						position = TempFileName.find(".");
//						if (position != TSString::npos)
//						{
//							std::vector<TSString> FileNameVct;
//							boost::split(FileNameVct, TempFileName, boost::is_any_of("."));
//							std::vector<TSString>::iterator itr = FileNameVct.begin();
//							TempFileName = *itr;
//						}						
//					}		
//
//					if (TSIDLPluginMgr::Instance()->IsDefualtXidlFile(TempFileName))
//					{
//						continue;
//					}
//
//                    if(IsXidl)//��true��Ӻ�׺_xidl
//                    {
//                         IncludeLibPath += ",\"" + SavePath + "/" + GetBaseName(FileName) + "_xidl/cpp/bin\"";
//                    }
//                    else
//                    {
//                        IncludeLibPath += ",\"" + SavePath + "/" + GetBaseName(FileName) + "/cpp/bin\"";
//                    }                   
//                }
//                else if (codeType == "c#")
//                {
//                    IncludeLibPath += ",\"" + SavePath + "/" + GetBaseName(iterU->second->Parse->GetFullFileName()) + "_xidl/cpp/bin\"";
//                }
//                else if (codeType == "java")
//                {
//                    IncludeLibPath += ",\"" + SavePath + "/" + GetBaseName(iterU->second->Parse->GetFullFileName()) + "_xidl/cpp/bin\"";
//                }  
//            }
//        }
//        std::vector<TSString> libPaths = TSIDLPluginMgr::Instance()->GetLibraryPaths();
//        BOOST_FOREACH(TSString libpath,libPaths)
//        {
//            IncludeLibPath += ",\"" + libpath + "\"";
//        }
//
//        boost::algorithm::replace_all(IncludeLibPath, "\\","/");
//        TSString IncludePath = "\"" + SavePath + "\"";
//        std::vector<TSString> inPaths = TSIDLPluginMgr::Instance()->GetIncludePaths();
//        BOOST_FOREACH(TSString Ipath,inPaths)
//        {
//            IncludePath += ",\"" + Ipath + "\""; 
//        }
//
//        TSString LinuxMakePath = ",\"" + SavePath + "/" + BaseName + "_xidl/cpp\"";
//        boost::algorithm::replace_all(LinuxMakePath, "\\","/");
//        boost::algorithm::replace_all(luaPro, "_MAKE_FILEPATH_",LinuxMakePath);
//
//        boost::algorithm::replace_all(IncludePath, "\\","/");
//        
//        boost::algorithm::replace_all(luaPro, "_INCLUDE_PATH_", IncludePath); 
//        boost::algorithm::replace_all(luaPro, "_INCLUDE_LIBPATH_", IncludeLibPath);
//        boost::algorithm::replace_all(luaPro, "_DLL_",CSdll);
//        TSString upperName = ProjName;
//        std::transform(upperName.begin(),upperName.end(),upperName.begin(),::toupper);
//
//        boost::algorithm::replace_all(upperName, ".", "_");
//        boost::algorithm::replace_all(upperName, "-", "_");
//        boost::algorithm::replace_all(luaPro, "_NAMEPLUSE_", upperName);
//        TSString comontSo;
//        if(IsNodeFilePro)
//        {
//             boost::algorithm::replace_all(luaPro, "SharedLib", "ConsoleApp");
//             comontSo = " -l" + BaseName;
//             boost::algorithm::replace_all(luaPro, "_COMONTSO_", comontSo);
//        }
//        else
//        {
//            comontSo = "";
//            boost::algorithm::replace_all(luaPro, "_COMONTSO_", comontSo);
//        }
//        const ProjFileMap_T files = iter->second->GetProjFiles();
//        Lua += luaPro;
//        for (ProjFileMap_T::const_iterator iterfile = files.begin(); iterfile != files.end(); iterfile++)
//        {
//            TSString FileName = iterfile->first;
//            if (iterfile->second->GetFileType() == None)
//            {
//                TSIDLUtils::WriteFile(FileName, iterfile->second->GetContent(),SolutionDir);
//            }
//            else if(iterfile->second->GetFileType() == Generic)
//            {
//                TSIDLUtils::WriteFile(FileName, iterfile->second->GetContent(),SolutionDir + "/generic/");
//            }
//            else if (iterfile->second->GetFileType() == Impl)
//            {
//                TSIDLUtils::WriteFile(FileName, iterfile->second->GetContent(),SolutionDir + "/impl/" + shortName);
//            }
//            
//        }
//    }
//
//	//********************************************************
//	//���沿�ִ�����������lua�ļ����������������̵Ķ�̬��
//	//********************************************************
//	TSString FileFullPath = SolutionDir + "/" + BaseName + ".xidl";
//	//���ļ�������ʽ��ȡ�ļ�
//	std::ifstream ifs(FileFullPath.c_str(), std::ifstream::in | std::ifstream::binary);
//	//���ļ�����������������������
//	std::ostringstream oss;
//	oss << ifs.rdbuf();
//
//	//xidl�ļ�����
//	TSString xidlcontent = Utf8ToAscii(oss.str()).c_str();
//	//xidl�ļ��� import�ؼ��ֿ�ʼ��λ�ã�posendimport�Ľ���λ�ü�"\n"λ��
//	TSString::size_type posstart, posend;
//	int pos = 0;
//	//xidl�ļ����������������ʹ��vector�洢
//	std::vector<TSString> vecxidl;
//
//	//��xidl�ļ�������������xidl������ȡ
//	while ((posstart = xidlcontent.find("import", pos)) != TSString::npos)
//	{
//		posend = xidlcontent.find("\n", pos + 1);
//		TSString::size_type arrowstart, pospoint;
//		arrowstart = xidlcontent.find("<", pos);
//		pospoint = xidlcontent.find(".", pos);
//		TSString strxidl = xidlcontent.substr(arrowstart + 1, pospoint - arrowstart - 1);
//		strxidl = "-l" + strxidl;
//		vecxidl.push_back(strxidl);
//		pos = posend;
//	}
//
//	//�����������xidl����ƴ��
//	TSString res;
//	for (auto it = vecxidl.begin(); it != vecxidl.end(); ++it)
//	{
//		TSString temp = *it;
//		res += temp + " ";
//	}
//
//	boost::algorithm::replace_all(Lua, "_TOPICXIDL_", res);
//	//д��lua�ļ�
//	TSIDLUtils::WriteFile(BaseName + ".lua", Lua, SolutionDir);
//    TSIDLUtils::Writexidl(BaseName,SolutionDir);
//
//    if (HasOption("GFileOnly"))
//    {
//        return true;
//    }
//
//    TSString SolutionPath = SolutionDir;
//    boost::filesystem::path p(SolutionPath);
//    if (!p.is_absolute())
//    {
//        SolutionPath = boost::filesystem::current_path().string() + FILE_SPLITE + SolutionDir;
//    }
//    TSString command;
//    TSString commandsln,commandslnR;
//    TSString preFileName =  SolutionPath + FILE_SPLITE + BaseName;
//
//    if (!preFileName.empty())
//    {
//        TSString linkPath = boost::filesystem::current_path().string();
//#ifdef _MSC_VER
//        std::cout<< "premake file "<<preFileName << GB18030ToTSString("����") << std::endl;
//        if (!HasOption(LinuxKey))
//        {
//            command = linkPath + "/premake5 --file=" + preFileName + ".lua ";
//        }
//        else
//        {
//            command = linkPath + "/premake5 --file=" + preFileName + ".lua --cc=gcc --os=linux gmake"; 
//        }
//#else
//        command = linkPath + "/premake5 --file=" + preFileName + ".lua --cc=gcc --os=linux gmake";
//#endif      
//    }
//    else
//    {
//        std::cout<< "premake file "<<preFileName << GB18030ToTSString("������.") << std::endl;
//    }
//
//     int IsSucc;
//    if(codeType != "java")
//    {
//       IsSucc =  system(command.c_str());
//
//        if (IsSucc)
//        {
//            std::cout << GB18030ToTSString("premake ���ڣ�") << std::endl;
//        }
//        else
//        {
//            std::cout << GB18030ToTSString("����ִ�����.") << std::endl;
//        }
//    }
//    
//    if (HasOption(CompileKey))
//    {
//        TSString comStr = Option<TSString>(CompileKey);
//        boost::replace_all(commandsln,"_PLATFORM_",comStr);
//        boost::replace_all(commandslnR,"_PLATFORM_",comStr);
//        system(("\"" + commandsln + "\"").c_str());
//        system(("\"" + commandslnR + "\"").c_str());
//    }
	return true;
}

TSString TSLinuxSolution::ReadLuaConfig()
{
    TSString IdlName = GetSolutionName();

    TSString Lua = "solution \"" + IdlName + "\"\n";
    //TSString FileName = boost::filesystem::current_path().string();
	
    TSString Cp = TSIDLPluginMgr::Instance()->GetCodeType();
    if(Cp == "c++"
        || Cp == "cplusplus"
        || Cp == "python")
    {
        Lua += TEMPLEATESLN;
	}
	else if(Cp == "c#"
			|| Cp == "csharp")
	{
        Lua += TEMPLEATE4CSSLN;
	}

    if (Lua.empty())
    {
        std::cerr << IdlName + ".lua "<< GB18030ToTSString("Ϊ��") << std::endl;
    }

    return Lua;
}

bool TSLinuxSolution::IsLinkTopicProject(const TSString & PrivateFilePath, const TSString & ProjName)
{
	TSString::size_type separatorPos = PrivateFilePath.find_last_of("/");
	TSString Path = PrivateFilePath.substr(0, separatorPos);
	TSString File = PrivateFilePath.substr(separatorPos + 1);

	separatorPos = File.find_first_of(".");
	TSString FileName = File.substr(0, separatorPos);

	if (FileName == ProjName)
	{
		const TSIDLPluginMgr::ParseMap_T & parsemap = PluginMgr()->GetParserUtils();
		TSIDLPluginMgr::ParseMap_T::const_iterator iterU = parsemap.begin();
		for (; iterU != parsemap.end(); iterU++)
		{
			if (TSIDLPluginMgr::Instance()->IsImportParse(iterU->second->Parse))
			{
				TSString FullName = iterU->second->Parse->GetFullFileName();

				if (GetBaseName(FullName) == FileName + "_Topic")
				{
					return true;
				}
			}
		}
	}

	return false;
}

void TSLinuxSolution::InitSolutionConfig()
{
	boost::filesystem::path p = boost::filesystem::current_path() / "config/ModelDesigner.config";
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(p.string().c_str(), pugi::parse_full, pugi::encoding_auto);
	if (result.status == pugi::status_ok)
	{
		pugi::xml_node node = doc.select_single_node(GB18030ToTSString("ȫ������/���ɴ��빤������").c_str()).node();

		pugi::xpath_node_set Nodes = node.select_nodes(GB18030ToTSString("������").c_str());

		for (pugi::xpath_node_set::const_iterator it = Nodes.begin(); it != Nodes.end(); ++it)
		{
			_p->_OutPutDir = it->node().attribute(GB18030ToTSString("��̬������·��").c_str()).value();

			_p->_ThirdPartDir = it->node().attribute(GB18030ToTSString("������·��").c_str()).value();
		}
	}
}