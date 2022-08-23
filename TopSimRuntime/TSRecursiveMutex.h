#ifndef __TSISERIALIZER_H__
#define __TSISERIALIZER_H__

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimTypes/TSByteBuffer.h>
#else
#include <TopSimRuntime/TSByteBuffer.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TSTopicTypes.h"

extern "C"
{
	TOPSIMDATAINTERFACE_DECL extern bool USE_STD_DDS_PROPTOCOL;
};

template<typename T>	struct	TSTypeCanCopyWithMemcpyTraits					  { static const bool value = false; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<bool>               { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<char>               { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<signed char>        { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<unsigned char>      { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<short>              { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<unsigned short>     { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<int>                { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<unsigned int>       { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<long>               { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<unsigned long>      { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<long long>          { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<unsigned long long> { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<wchar_t>            { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<float>              { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<double>             { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<long double>        { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<TSVector3d>         { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<TSVector2d>         { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<TSVector3f>         { static const bool value = true; };
template<>				struct	TSTypeCanCopyWithMemcpyTraits<TSVector2f>         { static const bool value = true; };
template<typename P>	struct	TSTypeCanCopyWithMemcpyTraits<P*>				  { static const bool value = true; };


struct TSISerializerContext;

CLASS_PTR_DECLARE(TSISerializer);
class TOPSIMDATAINTERFACE_DECL TSISerializer
{
public:
	TSISerializer(TSISerializerContext * Context = 0);
	virtual ~TSISerializer();

	enum MarshalType
	{
		Marshal,
		Demarshal
	};

public:
	virtual TSISerializer& operator<<(const bool &from) = 0;
	virtual TSISerializer& operator<<(const char &from) = 0;
	virtual TSISerializer& operator<<(const UINT8 &from) = 0;
	virtual TSISerializer& operator<<(const UINT16 &from) = 0;
	virtual TSISerializer& operator<<(const UINT32 &from) = 0;
	virtual TSISerializer& operator<<(const UINT64 &from) = 0;
	virtual TSISerializer& operator<<(const INT8 &from) = 0;
	virtual TSISerializer& operator<<(const INT16 &from) = 0;
	virtual TSISerializer& operator<<(const INT32 &from) = 0;
	virtual TSISerializer& operator<<(const INT64 &from) = 0;
	virtual TSISerializer& operator<<(const FLOAT &from) = 0;
	virtual TSISerializer& operator<<(const DOUBLE &from) = 0;
	virtual TSISerializer& operator<<(const std::string &from) = 0;
#ifndef  _TSRUNTIME_NO_WCHAR_T
	virtual TSISerializer& operator<<(const wchar_t *from) = 0;
	virtual TSISerializer& operator<<(const std::wstring &from) = 0;
#endif //_TSRUNTIME_NO_WCHAR_T
	virtual TSISerializer& operator<<(const TSVector2i &from) = 0;
	virtual TSISerializer& operator<<(const TSVector3i &from) = 0;
	virtual TSISerializer& operator<<(const TSVector2f &from) = 0;
	virtual TSISerializer& operator<<(const TSVector3f &from) = 0;
	virtual TSISerializer& operator<<(const TSVector2d &from) = 0;
	virtual TSISerializer& operator<<(const TSVector3d &from) = 0;
	virtual TSISerializer& operator<<(const TSTime &from) = 0;
	virtual TSISerializer& operator<<(const TSTimeDuration &from) = 0;
	virtual TSISerializer& operator<<(const TSHANDLE &from) = 0;
	virtual TSISerializer& operator<<(const TSTemplateId &from) = 0;
	virtual TSISerializer& operator<<(const TSModelDataId &from) = 0;
	virtual TSISerializer& operator<<(const TSAssetTypeId &from) = 0;
	virtual TSISerializer& operator<<(const TSByteBuffer &from) = 0;
	virtual TSISerializer& operator<<(const TSTOPICHANDLE &from) = 0;
	virtual TSISerializer& operator<<(const TSFrontAppUniquelyId &from) = 0;
	virtual TSISerializer& operator<<(const TSInstanceHandle &from) = 0;
	virtual TSISerializer& operator<<(const TSObjectHandle &from) = 0;
	virtual TSISerializer& operator<<(const TSObjectIdentity& from) = 0;

	template<typename T>
	typename boost::enable_if<boost::is_enum<T>,TSISerializer>::type& operator<<(const T &from)
	{
		return (*this) << (INT32)from;
	}

	virtual TSISerializer& operator>>(bool &dest) = 0;
	virtual TSISerializer& operator>>(char &dest) = 0;
	virtual TSISerializer& operator>>(UINT8 &dest) = 0;
	virtual TSISerializer& operator>>(UINT16 &dest) = 0;
	virtual TSISerializer& operator>>(UINT32 &dest) = 0;
	virtual TSISerializer& operator>>(UINT64 &dest) = 0;
	virtual TSISerializer& operator>>(INT8 &dest) = 0;
	virtual TSISerializer& operator>>(INT16 &dest) = 0;
	virtual TSISerializer& operator>>(INT32 &dest) = 0;
	virtual TSISerializer& operator>>(INT64 &dest) = 0;
	virtual TSISerializer& operator>>(FLOAT &dest) = 0;
	virtual TSISerializer& operator>>(DOUBLE &dest) = 0;
	virtual TSISerializer& operator>>(std::string &dest) = 0;
#ifndef  _TSRUNTIME_NO_WCHAR_T
	virtual TSISerializer& operator>>(wchar_t *dest) = 0;
	virtual TSISerializer& operator>>(std::wstring &dest) = 0;
#endif //_TSRUNTIME_NO_WCHAR_T
	virtual TSISerializer& operator>>(TSVector2i &dest) = 0;
	virtual TSISerializer& operator>>(TSVector3i &from) = 0;
	virtual TSISerializer& operator>>(TSVector2f &dest) = 0;
	virtual TSISerializer& operator>>(TSVector3f &dest) = 0;
	virtual TSISerializer& operator>>(TSVector2d &dest) = 0;
	virtual TSISerializer& operator>>(TSVector3d &dest) = 0;
	virtual TSISerializer& operator>>(TSTime &dest) = 0;
	virtual TSISerializer& operator>>(TSTimeDuration &dest) = 0;
	virtual TSISerializer& operator>>(TSHANDLE &dest) = 0;
	virtual TSISerializer& operator>>(TSTemplateId &dest) = 0;
	virtual TSISerializer& operator>>(TSModelDataId &dest) = 0;
	virtual TSISerializer& operator>>(TSAssetTypeId &dest) = 0;
	virtual TSISerializer& operator>>(TSByteBuffer &dest) = 0;
	virtual TSISerializer& operator>>(TSTOPICHANDLE &dest) = 0;
	virtual TSISerializer& operator>>(TSFrontAppUniquelyId &dest) = 0;
	virtual TSISerializer& operator>>(TSInstanceHandle &dest) = 0;
	virtual TSISerializer& operator>>(TSObjectHandle &dest) = 0;
	virtual TSISerializer& operator>>(TSObjectIdentity& dest) = 0;

	template<typename T>
	typename boost::enable_if<boost::is_enum<T>,TSISerializer>::type& operator>>(const T &from)
	{
		return (*this) >> *(INT32*)&from;
	}

public:
	virtual UINT64 TellW() = 0;
	virtual UINT64 TellR() = 0;
	virtual bool MarshalKey(const TSDataKeyCompare& Key) = 0;
	virtual bool MarshalComplete() = 0;
    virtual bool WriteBytes(const char* src, size_t size) = 0;
    virtual bool ReadBytes(char *dest,size_t size) = 0;

	virtual const std::vector<TSString>& GetComplexTypes();
	virtual void PushComplexType(const TSString& TypeName,const void * Data,MarshalType MType);
	virtual void PopComplexType();
	virtual void BeginArray(UINT32& Size,MarshalType MType);
	virtual void EndArray(UINT8 SizeOfType,MarshalType MType);
	virtual void BeginMap(UINT32& Size,MarshalType MType);
	virtual void EndMap(UINT8 SizeOfType, MarshalType MType);
	virtual bool PerpareForSerialize(const UINT32 & VersionCode,const TSTime & Time);
	virtual bool PerpareForDeserialize(const TSChar* TypeName,const UINT32 & VersionCode, TSTime & Time);
	virtual bool GetSerializerKeyFlag() = 0;

public:
	TSISerializerContext * Context;
private:
	TSISerializer(const TSISerializer&);
	TSISerializer& operator=(const TSISerializer&);
private:
	std::vector<TSString>   _ComplexTypes;
};

template<typename T>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const T &from)
{
	if (!Ser.GetSerializerKeyFlag())
	{
		Ser << from;
	}
	
	return Ser;
}

template<typename ValueType>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const std::vector<ValueType> &Vec)
{
	if (!Ser.GetSerializerKeyFlag())
	{
		UINT32 Size = (UINT32)Vec.size();

		Ser.BeginArray(Size, TSISerializer::Marshal);

		if (boost::is_enum<ValueType>::value || TSTypeCanCopyWithMemcpyTraits<ValueType>::value)
		{
			if (!Vec.empty())
			{
#ifdef __VXWORKS__
				Ser.WriteBytes((const char *)&Vec[0], Vec.size() * sizeof(ValueType));
#else
				Ser.WriteBytes((const char *)Vec.data(), Vec.size() * sizeof(ValueType));
#endif // __VXWORKS__
			}

			Ser.EndArray(sizeof(ValueType), TSISerializer::Marshal);
		}
		else
		{
			BOOST_FOREACH(const ValueType& Data, Vec)
			{
				Marshal(Ser, Data);
			}

			Ser.EndArray(0, TSISerializer::Marshal);
		}
	}

	return Ser;
}

template<typename T>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const TSVector2<T> &from,UINT32 MarshaledFlag)
{
	if (!Ser.GetSerializerKeyFlag())
	{
		Ser << from;
	}

	return Ser;
}

template<typename T>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const TSVector3<T> &from,UINT32 MarshaledFlag)
{
	UINT32 Offset = Ser.TellW();

	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag == TopSimDataInterface::kSpacialRangeType)
	{
		Ser << from;
	}
	
	if (MarshaledFlag == TopSimDataInterface::kSpacialRangeType)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}

	return Ser;
}

template<typename T>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const T &from,UINT32 MarshaledFlag)
{
	if (!Ser.GetSerializerKeyFlag())
	{
		Ser << from;
	}

	return Ser;
}

template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const UINT8 & from,UINT32 MarshaledFlag)
{
	UINT32 Offset = Ser.TellW();

	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const UINT16 & from,UINT32 MarshaledFlag)
{
	UINT32 Offset = Ser.TellW();
	
	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const UINT32 & from,UINT32 MarshaledFlag)
{
	UINT32 Offset = Ser.TellW();
	
	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const UINT64 & from,UINT32 MarshaledFlag)
{
	UINT32 Offset = Ser.TellW();
	
	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const TSHANDLE & from,UINT32 MarshaledFlag)
{
	UINT32 Offset = Ser.TellW();
	
	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const INT8 & from,UINT32 MarshaledFlag)
{
	UINT32 Offset = Ser.TellW();
	
	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const INT16 & from,UINT32 MarshaledFlag)
{
	UINT32 Offset = Ser.TellW();
	
	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const INT32 & from,UINT32 MarshaledFlag)
{
	UINT32 Offset = Ser.TellW();
	
	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const INT64 & from,UINT32 MarshaledFlag)
{
	UINT32 Offset = Ser.TellW();
	
	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}

template<typename T>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const boost::shared_ptr<T> &from)
{
	Marshal(Ser,*from);
	return Ser;
}

struct MarshalPointerWrapper
{
	template<typename TYPE>
	static FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const TYPE *from);
};

template<typename T>
FORCEINLINE TSISerializer& MarshalPointerWrapper::Marshal(TSISerializer& Ser,const T *from)
{
	//TODO::
	//BOOST_STATIC_ASSERT(false);
	ASSERT(false && TS_TEXT("Marsharl : const T *"));

	return Ser;
}

template<>
FORCEINLINE TSISerializer& MarshalPointerWrapper::Marshal(TSISerializer& Ser,const wchar_t *from)
{
	Ser << from;

	return Ser;
}

template<typename T>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,T *from)
{
	MarshalPointerWrapper::Marshal(Ser,from);
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const std::string &from,UINT32 MarshaledFlag)
{
	INT32 Offset = Ser.TellW();
	
	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}

#ifndef _TSRUNTIME_NO_WCHAR_T
template<>
FORCEINLINE TSISerializer& Marshal(TSISerializer& Ser,const std::wstring &from,UINT32 MarshaledFlag)
{
	INT32 Offset = Ser.TellW();
	
	if (!Ser.GetSerializerKeyFlag()
		|| MarshaledFlag != TopSimDataInterface::kNone)
	{
		Ser << from;
	}

	if (MarshaledFlag != TopSimDataInterface::kNone)
	{
		TSDataKeyCompare Key;
		Key.Flag = MarshaledFlag;
		Key.Offset = Offset;
		Key.Size = Ser.TellW() - Offset;
		Ser.MarshalKey(Key);
	}
	return Ser;
}
#endif //_TSRUNTIME_NO_WCHAR_T

template<typename ValueType>
FORCEINLINE TSISerializer& Marshal(TSISerializer &Ser,const std::vector<ValueType> &Vec,UINT32 MarshaledFlag)
{
	if (!Ser.GetSerializerKeyFlag())
	{
		UINT32 Size = (UINT32)Vec.size();

		Ser.BeginArray(Size, TSISerializer::Marshal);

		if (boost::is_enum<ValueType>::value || TSTypeCanCopyWithMemcpyTraits<ValueType>::value)
		{
			if (!Vec.empty())
			{
#ifdef __VXWORKS__
				Ser.WriteBytes((const char*)&Vec[0], Vec.size() * sizeof(ValueType));
#else
				Ser.WriteBytes((const char*)Vec.data(), Vec.size() * sizeof(ValueType));
#endif // __VXWORKS__
			}

			Ser.EndArray(sizeof(ValueType), TSISerializer::Marshal);
		}
		else
		{
			BOOST_FOREACH(const ValueType& Data, Vec)
			{
				Marshal(Ser, Data, MarshaledFlag);
			}

			Ser.EndArray(0, TSISerializer::Marshal);
		}
	}

	return Ser;
}

template<typename KeyType,typename ValueType>
FORCEINLINE TSISerializer& Marshal(TSISerializer &Ser,const std::map<KeyType,ValueType> &Vec,UINT32 MarshaledFlag)
{
	if (!Ser.GetSerializerKeyFlag())
	{
		UINT32 Size = (UINT32)Vec.size();

		Ser.BeginMap(Size, TSISerializer::Marshal);

		BOOST_DEDUCED_TYPENAME std::map< KeyType, ValueType >::const_iterator it =
			Vec.begin();

		while (it != Vec.end())
		{
			Marshal(Ser, it->first);
			Marshal(Ser, it->second);
			++it;
		}

		Ser.EndMap(0, TSISerializer::Marshal);
	}

	return Ser;
}

template<typename T>
FORCEINLINE TSISerializer& Demarshal(TSISerializer& Ser,TSVector2<T>& dest)
{
	Ser >> dest;
	return Ser;
}

template<typename T>
FORCEINLINE TSISerializer& Demarshal(TSISerializer& Ser,TSVector3<T>& dest)
{
	Ser >> dest;
	return Ser;
}

template<typename T>
FORCEINLINE TSISerializer& Demarshal(TSISerializer& Ser,T &dest)
{
	Ser >> dest;
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Demarshal(TSISerializer& Ser,UINT32 &dest)
{
	Ser >> dest;
	return Ser;
}

template<typename T>
FORCEINLINE TSISerializer& Demarshal(TSISerializer& Ser,boost::shared_ptr<T> &dest)
{
	ASSERT(false && TS_TEXT("Demarshal : boost::shared_ptr<T>"));
	return Ser;
}

template<typename T>
FORCEINLINE TSISerializer& Demarshal(TSISerializer& Ser,T *&dest)
{
	ASSERT(false && TS_TEXT("Marsharl : T *&"));
	return Ser;
}

#ifndef _TSRUNTIME_NO_WCHAR_T
template<>
FORCEINLINE TSISerializer& Demarshal(TSISerializer& Ser,wchar_t *&dest)
{
	Ser >> dest;
	return Ser;
}

template<>
FORCEINLINE TSISerializer& Demarshal(TSISerializer& Ser,std::wstring &dest)
{
	Ser >> dest;
	return Ser;
}

#endif //_TSRUNTIME_NO_WCHAR_T

template<>
FORCEINLINE TSISerializer& Demarshal(TSISerializer& Ser,std::string &dest)
{
	Ser >> dest;
	return Ser;
}


template<typename ValueType>
FORCEINLINE TSISerializer& Demarshal(TSISerializer &Ser,std::vector<ValueType> &Vec)
{
	UINT32 Size = 0;
	
	Ser.BeginArray(Size,TSISerializer::Demarshal);
	Vec.resize(Size);

	if (boost::is_enum<ValueType>::value || TSTypeCanCopyWithMemcpyTraits<ValueType>::value)
	{
		if (!Vec.empty())
		{
#ifdef __VXWORKS__
			Ser.ReadBytes((char*)&Vec[0],Size * sizeof(ValueType));
#else
			Ser.ReadBytes((char*)Vec.data(),Size * sizeof(ValueType));
#endif // __VXWORKS__
		}

		Ser.EndArray(sizeof(ValueType), TSISerializer::Demarshal);
	}
	else
	{
		BOOST_FOREACH(ValueType& Data,Vec)
		{
			Demarshal(Ser,Data);
		}

		Ser.EndArray(0, TSISerializer::Demarshal);
	}

	return Ser;
}

template<typename KeyType,typename ValueType>
FORCEINLINE TSISerializer& Demarshal(TSISerializer &Ser,std::map<KeyType,ValueType> &Vec)
{
	UINT32 Size;
	
	Ser.BeginMap(Size,TSISerializer::Demarshal);

	while(Size--)
	{
		KeyType key;
		Demarshal(Ser,key);
		ValueType Val;
		Demarshal(Ser,Val);
		Vec.insert(std::make_pair(key,Val));
	}

	Ser.EndMap(0,TSISerializer::Demarshal);

	return Ser;
}

#endif
