#include "stdafx.h"

#include "TSISerializer.h"
#include "TSTypeSupport.h"

#include <boost/date_time/gregorian/gregorian.hpp>

extern "C"
{
	bool USE_STD_DDS_PROPTOCOL = getenv("USE_STD_DDS_PROPTOCOL");
};


TSISerializer::TSISerializer(TSISerializerContext * Context)
	:Context(Context)
{
	
}

TSISerializer::~TSISerializer()
{
	
}

void TSISerializer::PushComplexType( const TSString& TypeName ,const void * Data,MarshalType MType)
{
	_ComplexTypes.push_back(TypeName);
}

void TSISerializer::PopComplexType()
{
	_ComplexTypes.pop_back();
}

const std::vector<TSString>& TSISerializer::GetComplexTypes()
{
	return _ComplexTypes;
}

void TSISerializer::BeginArray( UINT32& Size,MarshalType MType )
{
	if(MType == TSISerializer::Marshal)
	{
		this->operator <<(Size);
	}
	else
	{
		this->operator >>(Size);
	}
}

void TSISerializer::BeginMap( UINT32& Size,MarshalType MType )
{
	if(MType == TSISerializer::Marshal)
	{
		this->operator <<(Size);
	}
	else
	{
		this->operator >>(Size);
	}
}

void TSISerializer::EndArray(UINT8 AlginSize,MarshalType MType)
{
	return;
}

void TSISerializer::EndMap(UINT8 AlginSize, MarshalType MType)
{
	return;
}

bool TSISerializer::PerpareForSerialize( const UINT32 & VersionCode,const TSTime & Time )
{
	return true;
}

bool TSISerializer::PerpareForDeserialize(const TSChar* TypeName,const UINT32 & VersionCode, TSTime & Time)
{
	return true;
}
