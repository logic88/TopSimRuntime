#ifndef TSBASICTYPESUPPORT_H
#define TSBASICTYPESUPPORT_H

#ifdef XSIM3_3
#include "TopSimDataInterface4XSim.h"
#elif XSIM3_2
#include "TopSimDataInterface4XSim3.2.h"
#endif // XSIM3_3

#include "TSTopicTypeManager.h"
#include "TSTypeSupport.h"
#include "TSISerializer.h"
#include "TopSimDataInterface.h"



#define SIMPLE_MARSHAL_DECLARE(TYPE)\
template<>\
FORCEINLINE TSISerializer & Marshal(TSISerializer &Ser,const TYPE &Data,UINT32 MarshaledFlag)\
{\
	Ser << Data;\
	return Ser;\
}\
template<>\
FORCEINLINE TSISerializer & Demarshal(TSISerializer &Ser,TYPE &Data)\
{\
	Ser >> Data;\
	return Ser;\
}

//SIMPLE_MARSHAL_DECLARE(TSHANDLE);
SIMPLE_MARSHAL_DECLARE(TSTOPICHANDLE);
SIMPLE_MARSHAL_DECLARE(TSFrontAppUniquelyId);
SIMPLE_MARSHAL_DECLARE(TSObjectHandle);
SIMPLE_MARSHAL_DECLARE(TSObjectIdentity);

template<>
FORCEINLINE TSISerializer & Marshal(TSISerializer &Ser,const TSInstanceHandle &Data,UINT32 MarshaledFlag)
{
	Ser << Data._TopicHandle;
	Ser << Data._DataTopicHandle;
	Ser << Data._InstanceIndex;
	return Ser;
}
template<>
FORCEINLINE TSISerializer & Demarshal(TSISerializer &Ser,TSInstanceHandle &Data)
{
	Ser >> Data._TopicHandle;
	Ser >> Data._DataTopicHandle;
	Ser >> Data._InstanceIndex;
	return Ser;
}

namespace TSBasicSdo
{
	struct TOPSIMDATAINTERFACE_DECL DataType : public TSInterObject
	{
    #ifdef XSIM3_3
        TS_MetaType(DataType);
    #endif // XSIM3_3
		DataType(){}

        virtual ~DataType()
        {}
		DataType &operator=(const DataType &other)
		{
			Handle = other.Handle;
			return *this;
		}
		bool operator<(const DataType &other) const
		{		
			return Handle < other.Handle;
		}
		bool operator==(const DataType &other) const
		{
			return Handle == other.Handle;
		}

		TSObjectHandle Handle;
	};

	typedef boost::shared_ptr<DataType> DataTypePtr;
}


TS_DECLARE_METATYPE(TSBasicSdo::DataType);

//TSBasicSdo

#define SUPNAME         TOPIC_Base
#define NAME            TOPIC_Basic_Sdo
#define DESC            TS_TEXT("SDO")

#define SUP_NAMESPACE   
#define NAMESPACE       TSBasicSdo
#define DATA_TYPE       NAMESPACE::DataType
#define SUPPORT_TYPE    TypeSupport 
#define EXPORT_MACRO    TOPSIMDATAINTERFACE_DECL
#define BEGIN_NAMESPACE namespace TSBasicSdo{
#define END_NAMESPACE   }

#define HASHTYPEVERSIONMACRO 1452873358
#define SELFVERSONMACRO     HASHTYPEVERSIONMACRO

TYPESUPPORT_DECLARE;
TOPICHELPER_DECLARE;

BEGIN_MARSHAL;
Marshal_Filed(Handle,TopSimDataInterface::kKey);
END_MARSHAL;

BEGIN_DEMARSHAL;
Demarshal_Filed(Handle,TopSimDataInterface::kKey);
END_DEMARSHAL;

#undef SUPNAME 
#undef NAME
#undef DESC
#undef SUP_NAMESPACE
#undef NAMESPACE
#undef DATA_TYPE
#undef SUPPORT_TYPE
#undef EXPORT_MACRO   
#undef BEGIN_NAMESPACE 
#undef END_NAMESPACE 
#undef SELFVERSONMACRO
#undef HASHTYPEVERSIONMACRO

#endif //TSBASICTYPESUPPORT_H
