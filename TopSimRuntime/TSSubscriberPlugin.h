#ifndef __TSIDLSUBSCRIBERPLUGIN_H__
#define __TSIDLSUBSCRIBERPLUGIN_H__ 

#include "TopSimIDL.h"
#include "TSBuiltinPlugin.h"
#include "TSIDLUtils.h"
#include "TSSolutionMgr.h"

struct TOPSIMIDL_DECL TSDataMember
{
    TSString Name;
    TSString Value;
};
STRUCT_PTR_DECLARE(TSDataMember);

struct TOPSIMIDL_DECL TSDataType
{
    TSString Times;
    std::vector<TSDataMemberPtr> Members;
};

STRUCT_PTR_DECLARE(TSDataType);

struct TOPSIMIDL_DECL TSNodePubSubData
{
    std::vector<TSString> PubFullName;
    std::vector<TSString> PubType;
    std::vector<TSString> PubFileName;
    std::vector<TSString> SubFullName;
    std::vector<TSString> SubType;
    std::vector<TSString> SubFileName;
    TSConUnorderedMap<TSString,TSDataTypePtr> DataMap;
};
STRUCT_PTR_DECLARE(TSNodePubSubData);

struct TOPSIMIDL_DECL TSNodeFile
{
    TSString SourcePath;
    TSString LorisDefaultDomain;
    std::vector<TSString> Nodes;
    std::vector<TSString> ApplicationName;
    std::vector<TSNodePubSubDataPtr> NodeData;
   
};
STRUCT_PTR_DECLARE(TSNodeFile);

struct TSSubscriberPluginPrivate;
class TOPSIMIDL_DECL TSSubscriberPlugin : public TSIIDLPlugin
{
    TS_MetaType(TSSubscriberPlugin,TSIIDLPlugin);
public:
    TSSubscriberPlugin(void);
    ~TSSubscriberPlugin(void);

public:
    virtual TSString         GetName();
    virtual void             OnPluginLoaded();
    virtual bool             DoParse();
    virtual bool             DoBuild();
    virtual void             Clean();
   
private:
    void BuildExport(TSString FileName);
    
    bool LoadNodeFile(pugi::xml_document& doc, TSString NodeFile);
    TSString ParserNodeFile(pugi::xml_document& doc, std::vector<TSNodeFilePtr>& NodeFilePtr);
    TSString ParserPubSub(pugi::xml_object_range<pugi::xml_node_iterator> nodes, TSNodeFilePtr& NodeFilePtr);
    TSString ParserNode(pugi::xml_node& interfaceObjectModel, TSNodePubSubDataPtr& PubSubData,TSString Str);
    void GenerateCpp(std::vector<TSNodeFilePtr> nodeFilePtr);
    void GenerateCsharp(std::vector<TSNodeFilePtr> nodeFilePtr);
    void GenerateJava(std::vector<TSNodeFilePtr> nodeFilePtr);
    void GeneratePubSubFile(const TSString & FileName,const TSString & Coder);
private:
    TSSubscriberPluginPrivate * _p;
    std::vector<TSNodeFilePtr> _nodeFilePtr;
};


#endif