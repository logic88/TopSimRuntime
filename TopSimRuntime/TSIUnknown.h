#ifndef __TSIDLXMLCODER_H__
#define __TSIDLXMLCODER_H__

#include <iostream>

#include <TopSimIDL/TSIdlGram.h>
#include <TopSimIDL/TSBuiltinPlugin.h>
#include <TopSimIDL/TSIDLUtils.h>

#include "TSBuiltinPlugin.h"

using namespace TSIDLGram;
    
#define ATTR_FILE_NAME						"FileName"
#define SELECT_NODE_PACKAG_NAME				"root/Package"
#define NODE_PACKAGE_NAME					"Package"
#define NODE_STRUCT_NAME				    "Struct"
#define NODE_TOPIC_NAME                     "Topic"
#define NODE_ENUM_NAME						"Enum"
#define NODE_CONST_NAME                     "Const"
#define NODE_TYPEDEFINE_NAME                "Type"
#define NODE_EXCEPTION_NAME                 "Exception"
#define NODE_ATTRIBUTES_NAME				"Attributes"
#define NODE_ATTRIBUTE_NAME					"Attribute"
#define ATTR_NAME_NAME						"Name"
#define ATTR_PARENTCLASSNAME_NAME			"ParentClassName"
#define ATTR_TYPE_NAME						"Type"
#define ATTR_VALUE_NAME						"Value"
#define ATTR_DESCRIPTION_NAME				"Description"
#define ATTR_FULLNAME_NAME					"FullName"

class TOPSIMIDL_DECL TSIDLXmlCoder
{
public:
	TSIDLXmlCoder(void);
	~TSIDLXmlCoder(void);
	void CreateXml();
    void ReadXml(const TSString &File);

private:
    void ParserPackage( pugi::xml_node &node);
    void ParserType(pugi::xml_node &node);

private:
    TSIBuiltinPluginPtr _Plugin;
    std::vector<TSTypeDefPtr>  _TypeDefs;
};

#endif ///__TSIDLXMLCODER_H__