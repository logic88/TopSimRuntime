#include "stdafx.h"

#ifndef  _WRS_KERNEL

#include "TSXMLSerializer.h"

#define APPEND_VAL(TYPE,a)\
	ASSERT(_p->_ComplexChildren.size());\
	pugi::xml_node Child = _p->_ComplexChildren.back().Append();\
	Child.append_attribute("Type").set_value(#TYPE);\
	Child.append_child(pugi::node_pcdata).set_value(TSValue_Cast<TSString>(a).c_str());\

#define APPEND_ARRAY(TYPE,Vec)\
	ASSERT(_p->_ComplexChildren.size());\
	pugi::xml_node Child = _p->_ComplexChildren.back().Append();\
	Child.append_attribute("Type").set_value("Array");\
	Child.append_attribute("Count").set_value(TSStringUtil::FromNumber(Vec.size()).c_str());\
	for(size_t i = 0; i < Vec.size(); ++i)\
	{\
		pugi::xml_node Item = Child.append_child((TSString("Item_") + TSStringUtil::FromNumber(i)).c_str());\
		Item.append_attribute("Type").set_value(#TYPE);\
		Item.append_child(pugi::node_pcdata).set_value(TSValue_Cast<TSString>(Vec[i]).c_str());\
	}\

#define APPEND_ARRAY_COUNT(Size)\
    ASSERT(_p->_ComplexChildren.size());\
    TSXMLNodeItem Item;\
    Item.Root = _p->_ComplexChildren.back().Append();\
    Item.Root.append_attribute("Type").set_value("Array");\
    Item.Root.append_attribute("Count").set_value(TSStringUtil::FromNumber(Size).c_str());\
    _p->_ComplexChildren.push_back(Item);\

#define LOAD_VAL(TYPE,a)\
	ASSERT(_p->_ComplexChildren.size());\
	pugi::xml_node Child = _p->_ComplexChildren.back().Next();\
	ASSERT(strcmp(Child.attribute("Type").as_string(),#TYPE) == 0);\
	a = TSValue_Cast<TYPE>(TSString(Child.child_value()));\

#define LOAD_ARRAY(TYPE,Vec)\
	ASSERT(_p->_ComplexChildren.size());\
	pugi::xml_node Child = _p->_ComplexChildren.back().Next();\
	size_t Count = Child.attribute("Count").as_int();\
	Vec.resize(Count);\
	for(size_t i = 0; i < Count; ++i)\
	{\
	pugi::xml_node Item = Child.child((TSString("Item_") + TSStringUtil::FromNumber(i)).c_str());\
	ASSERT(strcmp(Child.attribute("Type").as_string(),#TYPE) == 0);\
	Vec[i] = TSValue_Cast<TYPE>(TSString(Item.child_value()));\
	}\

#define LOAD_ARRAY_COUNT(Size)\
    ASSERT(_p->_ComplexChildren.size());\
    TSXMLNodeItem Item;\
    Item.Root = _p->_ComplexChildren.back().Next();\
    Size = Item.Root.attribute("Count").as_int();\
    _p->_ComplexChildren.push_back(Item);\

struct TSXMLNodeItem
{
	TSXMLNodeItem()
		:Index(0)
	{}

	pugi::xml_node Root;
	size_t         Index;

	pugi::xml_node Append()
	{
		return Root.append_child((TSString("Item_") + TSStringUtil::FromNumber(Index++)).c_str());
	}

	pugi::xml_node Next()
	{
		return Root.child((TSString("Item_") + TSStringUtil::FromNumber(Index++)).c_str());
	}
};

struct TSXMLSerializerPrivate
{
	TSXMLSerializerPrivate(pugi::xml_node& node)
		:_Root(node)
	{
		_RootItem.Root = _Root;
	}

	pugi::xml_node& _Root;
	TSXMLNodeItem   _RootItem;
	std::vector<TSXMLNodeItem>  _ComplexChildren;
	bool _HeaderMarshaled;
};

UINT64 TSXMLSerializer::TellW()
{
	return 0;
}

UINT64 TSXMLSerializer::TellR()
{
	return 0;
}

bool TSXMLSerializer::MarshalKey( const TSDataKeyCompare& Key )
{
	return true;
}

bool TSXMLSerializer::MarshalComplete()
{
	return true;
}

TSXMLSerializer::TSXMLSerializer( pugi::xml_node& Root,TSISerializerContext * Context )
	:TSISerializer(Context),_p(new TSXMLSerializerPrivate(Root))
{
	
}


TSXMLSerializer::~TSXMLSerializer( void )
{
	delete _p;
}

void TSXMLSerializer::PushComplexType( const TSString& TypeName ,const void * Data,TSISerializer::MarshalType MType)
{
	TSISerializer::PushComplexType(TypeName,Data,MType);

	if(MType == TSISerializer::Marshal)
	{
		TSXMLNodeItem Item;
		if(!_p->_ComplexChildren.size())
		{
			Item.Root = _p->_RootItem.Append();
		}
		else
		{
			Item.Root = _p->_ComplexChildren.back().Append();
		}

		Item.Root.append_attribute("Type").set_value(TypeName.c_str());
		_p->_ComplexChildren.push_back(Item);
	}
	else
	{
		TSXMLNodeItem Item;
		if(!_p->_ComplexChildren.size())
		{
			Item.Root = _p->_RootItem.Next();
			
		}
		else
		{
			Item.Root = _p->_ComplexChildren.back().Next();
		}

		ASSERT(TypeName == Item.Root.attribute("Type").as_string());
		_p->_ComplexChildren.push_back(Item);
	}
}

void TSXMLSerializer::PopComplexType()
{
	_p->_ComplexChildren.pop_back();

	TSISerializer::PopComplexType();
}

bool TSXMLSerializer::ReadBytes( char * val,size_t len)
{
	return true;
}

bool TSXMLSerializer::WriteBytes( const char * val,size_t len)
{
    return true;
}

void TSXMLSerializer::BeginArray( UINT32& Size,MarshalType MType )
{
    if(MType == TSISerializer::Marshal)
    {
        APPEND_ARRAY_COUNT(Size);
    }
    else
    {
        LOAD_ARRAY_COUNT(Size);
    }
}

void TSXMLSerializer::EndArray()
{
    _p->_ComplexChildren.pop_back();
}

void TSXMLSerializer::BeginMap( UINT32& Size,MarshalType MType )
{
	if(MType == TSISerializer::Marshal)
	{
		APPEND_ARRAY_COUNT(Size);
	}
	else
	{
		LOAD_ARRAY_COUNT(Size);
	}
}

void TSXMLSerializer::EndMap()
{
	 _p->_ComplexChildren.pop_back();
}

#define IMPL1(TYPE)\
	TSISerializer& TSXMLSerializer::operator<<( const TYPE &from )\
{\
	APPEND_VAL(TYPE,from);\
	return *this;\
}\
	TSISerializer& TSXMLSerializer::operator>>( TYPE &dest )\
{\
	LOAD_VAL(TYPE,dest);\
	return *this;\
}\

#define IMPL(TYPE)\
	IMPL1(TYPE)\
TSISerializer& TSXMLSerializer::operator<<( const std::vector<TYPE> &from )\
{\
	APPEND_ARRAY(TYPE,from);\
	return *this;\
}\
TSISerializer& TSXMLSerializer::operator>>( std::vector<TYPE> &dest )\
{\
	LOAD_ARRAY(TYPE,dest);\
	return *this;\
}\

#define NO_IMPL1(TYPE)\
	TSISerializer& TSXMLSerializer::operator<<( const TYPE &from )\
{\
	return *this;\
}\
	TSISerializer& TSXMLSerializer::operator>>( TYPE &dest )\
{\
	return *this;\
}\

#define NO_IMPL_NO_REF(TYPE)\
	TSISerializer& TSXMLSerializer::operator<<( const TYPE from )\
{\
	return *this;\
}\
	TSISerializer& TSXMLSerializer::operator>>( TYPE dest )\
{\
	return *this;\
}\

#define NO_IMPL(TYPE)\
	NO_IMPL1(TYPE)\
	TSISerializer& TSXMLSerializer::operator<<( const std::vector<TYPE> &from )\
{\
	return *this;\
}\
	TSISerializer& TSXMLSerializer::operator>>( std::vector<TYPE> &dest )\
{\
	return *this;\
}\

TS_BEGIN_RT_NAMESPACE
template<>
FORCEINLINE TSString TSValue_Cast<TSString,char>(const char & arg)
{
	return TSString().append(0,arg);
}

template<>
FORCEINLINE char TSValue_Cast<char,TSString>(const TSString & arg)
{
	return arg[0];
}
TS_END_RT_NAMESPACE

#define HANDLE_CAST_IMPL(TYPE)\
TS_BEGIN_RT_NAMESPACE \
template<>\
FORCEINLINE TSString TSValue_Cast<TSString,TYPE>(const TYPE & arg)\
{\
	return TSStringUtil::FromNumber(arg._value);\
}\
template<>\
FORCEINLINE TYPE TSValue_Cast<TYPE,TSString>(const TSString & arg)\
{\
	return boost::lexical_cast<UINT32>(arg);\
}\
TS_END_RT_NAMESPACE \

#define TSVECTOR_CAST_IMPL(TYPE)\
TS_BEGIN_RT_NAMESPACE \
template<>\
FORCEINLINE TSString TSValue_Cast<TSString,TSVector2<TYPE> >(const TSVector2<TYPE> & arg)\
{\
	return TSString().append(TSValue_Cast<TSString>(arg.x))\
		.append(",")\
		.append(TSValue_Cast<TSString>(arg.y));\
}\
template<>\
FORCEINLINE TSVector2<TYPE> TSValue_Cast<TSVector2<TYPE>,TSString>(const TSString & arg)\
{\
	std::vector<TSString> SplitVec = TSMetaType4Link::Split(arg,",");\
	if(SplitVec.size() >= 2)\
	{\
		TSVector2<TYPE> Vec;\
		Vec.x = TSValue_Cast<TYPE>(SplitVec[0]);\
		Vec.y = TSValue_Cast<TYPE>(SplitVec[1]);\
		return Vec;\
	}\
	return TSVector2<TYPE>();\
}\
template<>\
FORCEINLINE TSString TSValue_Cast<TSString,TSVector3<TYPE> >(const TSVector3<TYPE> & arg)\
{\
	return TSString().append(TSValue_Cast<TSString>(arg.x))\
		.append(",")\
		.append(TSValue_Cast<TSString>(arg.y))\
		.append(",")\
		.append(TSValue_Cast<TSString>(arg.z));\
}\
template<>\
FORCEINLINE TSVector3<TYPE> TSValue_Cast<TSVector3<TYPE>,TSString>(const TSString & arg)\
{\
	std::vector<TSString> SplitVec = TSMetaType4Link::Split(arg,",");\
	if(SplitVec.size() >= 2)\
	{\
		TSVector3<TYPE> Vec;\
		Vec.x = TSValue_Cast<TYPE>(SplitVec[0]);\
		Vec.y = TSValue_Cast<TYPE>(SplitVec[1]);\
		Vec.z = TSValue_Cast<TYPE>(SplitVec[2]);\
		return Vec;\
	}\
	return TSVector3<TYPE>();\
}\
TS_END_RT_NAMESPACE \

#ifndef  _TSRUNTIME_NO_WCHAR_T
TS_BEGIN_RT_NAMESPACE
template<>
FORCEINLINE TSString TSValue_Cast<TSString,TSWideString>(const TSWideString & arg)
{
	return WideString2TSString(arg);
}
template<>
FORCEINLINE TSWideString TSValue_Cast<TSWideString,TSString>(const TSString & arg)
{
	return TSString2WideString(arg);
}
TS_END_RT_NAMESPACE

#endif //#ifndef  _TSRUNTIME_NO_WCHAR_T
HANDLE_CAST_IMPL(TSHANDLE)
HANDLE_CAST_IMPL(TSTemplateId)
HANDLE_CAST_IMPL(TSModelDataId)
HANDLE_CAST_IMPL(TSAssetTypeId)
HANDLE_CAST_IMPL(TSModelTmplOrClassId)
HANDLE_CAST_IMPL(TSTOPICHANDLE)
HANDLE_CAST_IMPL(TSFrontAppUniquelyId)

TSVECTOR_CAST_IMPL(DOUBLE)
TSVECTOR_CAST_IMPL(FLOAT)
TSVECTOR_CAST_IMPL(INT32)

using namespace std;

IMPL(bool)
IMPL(char)
IMPL(INT8)
IMPL(UINT8)
IMPL(INT16)
IMPL(UINT16)
IMPL(INT32)
IMPL(UINT32)
IMPL(INT64)
IMPL(UINT64)
IMPL(FLOAT)
IMPL(DOUBLE)
NO_IMPL_NO_REF(char*)
NO_IMPL_NO_REF(wchar_t*)
NO_IMPL(TSByteBuffer)
IMPL(string)
#ifndef  _TSRUNTIME_NO_WCHAR_T
IMPL(wstring)
#endif //#ifndef  _TSRUNTIME_NO_WCHAR_T
IMPL(TSTime)
IMPL(TSTimeDuration)
IMPL(TSHANDLE)
IMPL(TSTemplateId)
IMPL(TSModelDataId)
IMPL(TSAssetTypeId)
IMPL(TSModelTmplOrClassId)
IMPL(TSVector2i)
IMPL(TSVector3i)
IMPL(TSVector2f)
IMPL(TSVector3f)
IMPL(TSVector2d)
IMPL(TSVector3d)
IMPL1(TSTOPICHANDLE)
IMPL1(TSFrontAppUniquelyId)
NO_IMPL1(TSInstanceHandle)
NO_IMPL1(TSObjectHandle)
NO_IMPL1(TSObjectIdentity)

#endif //#ifndef  _WRS_KERNEL
