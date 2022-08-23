#include "stdafx.h"

#include <boost/filesystem.hpp>
#include <TopSimIDL/TSIDLXmlCoder.h>
#include "temp/temp.h"

using namespace TSIDLUtils;

TSIDLXmlCoder::TSIDLXmlCoder(void)
{
    _Plugin = GetPluginT<TSIBuiltinPlugin>();
}

TSIDLXmlCoder::~TSIDLXmlCoder(void)
{

}

void FindPackageNode(pugi::xml_node& package, pugi::xml_node& root, TSString TrueOrFalse, 
    TSString idlName, std::vector<TSString> packages,TSString fullName,TSString Name)
{
    pugi::xpath_node_set packageNodes = root.select_nodes("Package");
    bool isFind = false, isFindOnce = false;
    size_t packagesIndex = 0;

    pugi::xpath_node  packageNodeIndex;

    for (size_t i = 0 ; i < packages.size(); i++)
    {
        pugi::xpath_node_set::const_iterator it = packageNodes.begin();
        for (; it != packageNodes.end() ; ++it)
        {
            TSString fileName = it->node().first_attribute().next_attribute().next_attribute().value();
            TSString names = it->node().first_attribute().value();
            if (names == packages[i] && (fileName == idlName || i > 0))
            {
                bool isfinds = false;
                pugi::xml_node parentNode = it->node().parent();
                if (i > 0)
                {
                    for (int j = i - 1; j >= 0; j--,parentNode = parentNode.parent())
                    {
                        if(parentNode.first_attribute().value() != packages[j])
                        {
                            isfinds = true;
                            break;
                        }
                        
                    }
                }

                if (strcmp(parentNode.value(),"Package") == 0)
                {
                    isfinds = true;
                }

                if (!isfinds)
                {
                    packageNodeIndex = it->node();
                    isFindOnce = true;
                    packageNodes = it->node().select_nodes("Package");
                    break;
                }
            }                           
        }

        if (isFindOnce)
        {           
            packagesIndex = i;
            isFind = true;
            isFindOnce = false;
        }
        else
        {
            break;
        }
    }

    if (isFind)
    {
        package = packageNodeIndex.node();
        TSString names = package.first_attribute().value();
        for (size_t i = packagesIndex + 1; i <packages.size(); i++)
        {
            package = package.append_child(TS_TEXT("Package"));
            package.append_attribute(TS_TEXT("Name")).set_value(TS_TEXT(packages[i].c_str()));
        }
    }
    else
    {
        if (packages.empty())
        {               
            if(packageNodes.size() > 0 )
            {
                bool isTrue = false;
                pugi::xpath_node_set::const_iterator it = packageNodes.begin();
                for (; it != packageNodes.end() ; it++)
                {
                    TSString str = it->node().first_attribute().value();
                    TSString fileName = it->node().attribute("FileName").value();
                    if (str.empty() && fileName == idlName)
                    {
                        isTrue = true;
                        break;
                    }
                }

                if (isTrue)
                {
                    package = it->node();
                }
                else
                {
                    package = root.append_child("Package");
                    package.append_attribute("Name").set_value("");
                    package.append_attribute("FullName").set_value("");
                    package.append_attribute("FileName").set_value(idlName.c_str());
                    package.append_attribute(TS_TEXT("IsImprot")).set_value(TrueOrFalse.c_str());
                }
            }
            else
            {
                package = root.append_child("Package");
                package.append_attribute("Name").set_value("");
                package.append_attribute("FullName").set_value("");
                package.append_attribute("FileName").set_value(idlName.c_str());
                package.append_attribute(TS_TEXT("IsImprot")).set_value(TrueOrFalse.c_str());
            }
        }
        else
        {
            package = root.append_child("Package");
            package.append_attribute("Name").set_value(packages[0].c_str());
            boost::algorithm::replace_all(fullName , "::" + Name, "");
            package.append_attribute("FullName").set_value(fullName.c_str());
            package.append_attribute("FileName").set_value(idlName.c_str());
            package.append_attribute(TS_TEXT("IsImprot")).set_value(TrueOrFalse.c_str());
            for (size_t i = 1 ; i < packages.size() ; i++)
            {
                package = package.append_child("Package");
                package.append_attribute("Name").set_value(packages[i].c_str());
            }
        }
    }
    if (!package)
    {
        package = packageNodeIndex.node();
    }
}

void GreateXmlEnumCode(pugi::xml_node &root, TSEnumDefPtr TypeDef)
{
    TSString idlName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    pugi::xml_node package;
    TSString TrueOrFalse;
    TSString typePacakges = TSIBuiltinPlugin::GetFullPackageNameByPackageDef(TypeDef->SupperPkg.lock());
    std::vector<TSString> packages = TSMetaType4Link::Split(typePacakges,PACKAGE_SPLIT,false);
    TSString fullName = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef,true);
    TSString Name = TypeDef->TypeName;

    FindPackageNode(package, root, TrueOrFalse, idlName, packages, fullName, Name);

    pugi::xml_node tag = package.append_child(TS_TEXT("Enum"));
	tag.append_attribute(TS_TEXT("Name")).set_value(Name.c_str());
	tag.append_attribute("FullName").set_value((fullName + "::" + Name).c_str());
    pugi::xml_node attributes = tag.append_child(TS_TEXT("Attributes"));

    for(size_t i = 0; i < TypeDef->Fileds.size(); ++i)
    {
        if(TSEnumFiledDefPtr enumVal = TypeDef->Fileds[i])
        {
            attributes.append_child(TS_TEXT("Attribute")).append_attribute(TS_TEXT("Description")).set_value("");
            pugi::xml_node attributed = attributes.last_child();
            TSString ValName = enumVal->ValName;
			std::vector<TSString> ValNames = TSMetaType4Link::Split(ValName, "=", false);
			if (ValNames.size() > 0)
			{
				ValName = ValNames.at(0);
			}
            attributed.append_attribute(TS_TEXT("Name")).set_value(ValName.c_str());
            attributed.append_attribute(TS_TEXT("Value")).set_value(enumVal->DefaultVal.c_str());
        }
    }
}

void GreateXmlStructCode(pugi::xml_node& root, TSStructDefPtr TypeDef)
{
    TSString TrueOrFalse;
    TSString idlName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    pugi::xpath_node_set packageNodes = root.select_nodes("Package");
    TSString fullName = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef);
    TSString Name = TypeDef->TypeName;
	TSString typePacakges = TSIBuiltinPlugin::GetFullPackageNameByPackageDef(TypeDef->SupperPkg.lock());
	std::vector<TSString> packages = TSMetaType4Link::Split(typePacakges, PACKAGE_SPLIT, false);
    pugi::xml_node package;     
    FindPackageNode(package, root, TrueOrFalse, idlName, packages, fullName, Name);

    TSString ParentClassName = "";

    if (TypeDef->Extend.lock())
    {
        ParentClassName = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef->Extend.lock(),true);
    }


    pugi::xml_node structNode = package.append_child("Struct");
    structNode.append_attribute("Name").set_value(TypeDef->TypeName.c_str());
	structNode.append_attribute("FullName").set_value(fullName.c_str());
    structNode.append_attribute("Description");
    structNode.append_attribute(TS_TEXT("ParentClassName")).set_value(ParentClassName.c_str());
    pugi::xml_node attributesNode = structNode.append_child("Attributes");
   
    TSString description;

    for(size_t i = 0; i < TypeDef->Fileds.size(); ++i)
    {
        if(TSFieldDefPtr FieldDef = TypeDef->Fileds[i])
        {
            attributesNode.append_child("Attribute").append_attribute("Description").set_value(description.c_str());
            description.clear();
            pugi::xml_node attributeNode = attributesNode.last_child();
            attributeNode.append_attribute("Name").set_value(FieldDef->ValName.c_str());
            attributeNode.append_attribute("Type").set_value(FieldDef->DefTypeName.c_str());
        }
    }
}


void GreateXmlServiceCode(pugi::xml_node& root, TSServiceDefPtr TypeDef)
{
    TSString TrueOrFalse;
    TSString idlName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    pugi::xpath_node_set packageNodes = root.select_nodes("Package");
    TSString fullName = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef);
    TSString Name = TypeDef->TypeName;
	TSString typePacakges = TSIBuiltinPlugin::GetFullPackageNameByPackageDef(TypeDef->SupperPkg.lock());
	std::vector<TSString> packages = TSMetaType4Link::Split(typePacakges, PACKAGE_SPLIT, false);
    pugi::xml_node package;     
    FindPackageNode(package, root, TrueOrFalse, idlName, packages, fullName, Name);

    pugi::xml_node exceptionNode = package.append_child("Exception");
	exceptionNode.append_attribute("Name").set_value(TypeDef->TypeName.c_str());
	exceptionNode.append_attribute("FullName").set_value(fullName.c_str());
    exceptionNode.append_attribute("Description");
    pugi::xml_node attributesNode = exceptionNode.append_child("Attributes");
    TSString description;

    for(size_t i = 0; i < TypeDef->Functions.size(); ++i)
    {
        if(TSFunctionDefPtr FieldDef = TypeDef->Functions[i])
        {
            attributesNode.append_child("Attribute").append_attribute("Description").set_value(description.c_str());
            description.clear();
            pugi::xml_node attributeNode = attributesNode.last_child();
            attributeNode.append_attribute("Name").set_value(FieldDef->ValName.c_str());
            attributeNode.append_attribute("Type").set_value(FieldDef->DefTypeName.c_str());
        }
    }
}

void GreateXmlTopicCode(pugi::xml_node& root, TSTopicTypeDefPtr TypeDef)
{
    TSString TrueOrFalse;
    TSString idlName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    pugi::xpath_node_set packageNodes = root.select_nodes("Package");
    std::vector<TSString> packages;
	TSString structName = TypeDef->DefTypeName;
	TSString parentTopicName = TypeDef->ExtendTypeName;
    TSString fullName = TSIBuiltinPlugin::GetSuitFullTypeName(TypeDef);
    TSString Name = TypeDef->TypeName;
    pugi::xml_node package; 
    FindPackageNode(package, root, TrueOrFalse, idlName, packages, fullName, Name);

	pugi::xml_node topicNode = package.append_child("Topic");
	topicNode.append_attribute("Name").set_value(Name.c_str());
	topicNode.append_attribute("FullName").set_value(fullName.c_str());
    pugi::xml_node constDefNode = package.last_child();
    constDefNode.append_attribute("StructName").set_value(structName.c_str());
    constDefNode.append_attribute("ParentTopicName").set_value(parentTopicName.c_str());
    constDefNode.append_attribute("Description").set_value(TypeDef->TypeName.c_str());
}

void TSIDLXmlCoder::CreateXml()
{

    TSString idlName = TSIDLPluginMgr::Instance()->GetCurParseFileFullPath();
    TSString base = GetBaseName(idlName);

    TSString SavePath = Option<TSString>("SavePath");
    TSString absPath(SavePath);
    if(SavePath.empty())
    {
        absPath = boost::filesystem::current_path().string();
    }
    else
    {
        boost::filesystem::path p(SavePath);
        if(!p.is_absolute())
        {
            p = boost::filesystem::current_path()/SavePath;
            if(!p.is_absolute())
            {
                throw new TSException(GB18030ToTSString("保存路径错误"));
            }
            absPath = p.string();
        }
    }
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("root");
    TSString fileabsPath = idlName;
    root.append_attribute("FileName").set_value(fileabsPath.c_str());

    std::vector<TSEnumDefPtr> _EnumDefs = _Plugin->GetAllEnumDefs();
    for (size_t i = 0; i < _EnumDefs.size(); ++i)
    {
        if(TSEnumDefPtr enumDef = _EnumDefs[i])
        {
            if (!HasOption(GListEntityKey))
            {
                GreateXmlEnumCode(root, enumDef);           
            }                    
        }
    }

    std::vector<TSServiceDefPtr> _ServiceDefs = _Plugin->GetAllServiceDefs();
    for (size_t i = 0; i < _ServiceDefs.size(); ++i)
    {
        if(TSServiceDefPtr _ServiceDef = _ServiceDefs[i])
        {                   
               GreateXmlServiceCode(root, _ServiceDef);
        }
    }

    std::vector<TSStructDefPtr> _StructDefs = _Plugin->GetAllStructDefs();
    for (size_t i = 0; i < _StructDefs.size(); ++i)
    {
        if(TSStructDefPtr _StructDef = _StructDefs[i])
        {                   
            GreateXmlStructCode(root, _StructDef);
        }
    }

    std::vector<TSTopicTypeDefPtr> _TopicDefs = _Plugin->GetAllTopicTypeDefs();
    for (size_t i = 0; i < _TopicDefs.size(); ++i)
    {
        if(TSTopicTypeDefPtr _TopicDef = _TopicDefs[i])
        {                   
            GreateXmlTopicCode(root, _TopicDef);
        }
    }

    TSString FileNameWithPath = absPath + "/" + base + ".xml";
    boost::filesystem::path p(absPath);
    if (!boost::filesystem::exists(p))
    {
        boost::system::error_code err;
        boost::filesystem::create_directories(p,err);
    }

    doc.save_file(FileNameWithPath.c_str());

    std::ifstream MyFileR(FileNameWithPath.c_str() , std::ofstream::out);
    std::stringstream StrAll;
    StrAll << MyFileR.rdbuf();
    TSString buf = StrAll.str();
    boost::algorithm::replace_all(buf, "std::string", "string");
    boost::algorithm::replace_all(buf, "std::vector", "vector");
    boost::algorithm::replace_all(buf, "std::map", "map");
    boost::algorithm::replace_all(buf, "::InterfacePtr", "");
    MyFileR.close();

    std::ofstream MyFileW(FileNameWithPath.c_str() , std::ofstream::out|std::ios::trunc);
    MyFileW << buf;
    MyFileW.close();

}

void TSIDLXmlCoder::ReadXml(const TSString &File)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(File.c_str());

    if (pugi::status_ok == result.status)
    {
        pugi::xpath_node_set packageNodes = doc.select_nodes(SELECT_NODE_PACKAG_NAME);
        for ( size_t i = 0; i < packageNodes.size(); ++i )
        {
            pugi::xml_node nodePakage = packageNodes[i].node();
            ParserPackage(nodePakage);
        }
    }
}

void TSIDLXmlCoder::ParserPackage( pugi::xml_node &node)
{
    TSString childname = node.attribute(ATTR_NAME_NAME).as_string();
    TSString packageFullName = node.attribute(ATTR_FULLNAME_NAME).as_string();

    //包解析
    pugi::xml_node nodePkg = node.child(NODE_PACKAGE_NAME);
    while (nodePkg)
    {
        ParserType(nodePkg);
        nodePkg = nodePkg.next_sibling(NODE_PACKAGE_NAME);
    }

    //解析枚举
    pugi::xml_node nodeEnum = node.child(NODE_ENUM_NAME);
    while(nodeEnum)
    {
        ParserType(node);
        nodeEnum = nodeEnum.next_sibling(NODE_ENUM_NAME);
    }

    //解析常量
    pugi::xml_node nodeConst = node.child(NODE_CONST_NAME);
    while(nodeConst)
    {
        ParserType(node);
        nodeConst = nodeConst.next_sibling(NODE_CONST_NAME);
    }

    //解析typedef定义
    pugi::xml_node nodeTypeDefine = node.child(NODE_TYPEDEFINE_NAME);
    while(nodeTypeDefine)
    {
        ParserType(node);
        nodeTypeDefine = nodeTypeDefine.next_sibling(NODE_TYPEDEFINE_NAME);
    }



}

void TSIDLXmlCoder::ParserType( pugi::xml_node &node )
{
    TSString Name = node.attribute("Name").as_string();
    TSTypeDefPtr def = TSIBuiltinPlugin::CreateTypeDefByTypeInfoName(Name.c_str());
    _TypeDefs.push_back(def);
}
