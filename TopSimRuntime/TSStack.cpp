#include "stdafx.h"
#include "TSSerializer.h"
#include <boost/crc.hpp>

#include "TSTypeSupport.h"
#include "TSSerializer_p.h"

#if defined(XSIM3_2) || defined(XSIM3_3)
#else
#include "TopSimRuntime/TSLogsUtil.h"
#endif

const char  TSSerializer::ALIGN_PAD[] = {0};
static char ZeroChar = '\0';

#ifndef __VXWORKS__
	#define TSGEN_SCALAR_VEC_SERIALIZE(TYPE) \
	TSISerializer& TSSerializer::operator<<( const std::vector<TYPE> &from ) \
	{\
		UINT32 size = (UINT32)from.size(); \
		(*this) << size; \
		\
		if(size == 0)\
		{ \
			return (*this); \
		} \
		\
		Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(TYPE)); \
		WriteArray(reinterpret_cast<const char*>(from.data()),sizeof(TYPE),size); \
		return (*this);\
	}
#else
	#define TSGEN_SCALAR_VEC_SERIALIZE(TYPE) \
	TSISerializer& TSSerializer::operator<<( const std::vector<TYPE> &from ) \
	{\
		UINT32 size = (UINT32)from.size(); \
		(*this) << size; \
		\
		if(size == 0)\
		{ \
			return (*this); \
		} \
		\
		Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(TYPE)); \
		if(from.size() > 0)\
		{\
			WriteArray(reinterpret_cast<const char*>(&from.at(0)),sizeof(TYPE),size); \
		}\
		return (*this);\
	}

#endif


#ifndef __VXWORKS__
	#define TSGEN_SCALAR_VEC_DESERIALIZE(TYPE) \
	TSISerializer& TSSerializer::operator>>( std::vector<TYPE> &dest ) \
	{\
		UINT32 size;\
		(*this) >> size; \
		dest.clear(); \
		if(size == 0) \
		{ \
			return (*this); \
		} \
	\
		dest.resize(size); \
		Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(TYPE)); \
		ReadArray(reinterpret_cast<char *>(const_cast<TYPE *>(dest.data())),sizeof(TYPE),size); \
		return (*this); \
	}
#else
	#define TSGEN_SCALAR_VEC_DESERIALIZE(TYPE) \
	TSISerializer& TSSerializer::operator>>( std::vector<TYPE> &dest ) \
	{\
		UINT32 size;\
		(*this) >> size; \
		dest.clear(); \
		if(size == 0) \
		{ \
			return (*this); \
		} \
	\
		dest.resize(size); \
		Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(TYPE)); \
		if(dest.size() > 0)\
		{\
			ReadArray(reinterpret_cast<char *>(const_cast<TYPE *>(&dest.at(0))),sizeof(TYPE),size); \
		}\
		return (*this); \
	}
#endif


TSGEN_SCALAR_VEC_SERIALIZE(char);
TSGEN_SCALAR_VEC_SERIALIZE(INT8);
TSGEN_SCALAR_VEC_SERIALIZE(UINT8);
TSGEN_SCALAR_VEC_SERIALIZE(INT16);
TSGEN_SCALAR_VEC_SERIALIZE(UINT16);
TSGEN_SCALAR_VEC_SERIALIZE(INT32);
TSGEN_SCALAR_VEC_SERIALIZE(UINT32);
TSGEN_SCALAR_VEC_SERIALIZE(INT64);
TSGEN_SCALAR_VEC_SERIALIZE(UINT64);
TSGEN_SCALAR_VEC_SERIALIZE(FLOAT);
TSGEN_SCALAR_VEC_SERIALIZE(DOUBLE);

TSGEN_SCALAR_VEC_DESERIALIZE(char);
TSGEN_SCALAR_VEC_DESERIALIZE(INT8);
TSGEN_SCALAR_VEC_DESERIALIZE(UINT8);
TSGEN_SCALAR_VEC_DESERIALIZE(INT16);
TSGEN_SCALAR_VEC_DESERIALIZE(UINT16);
TSGEN_SCALAR_VEC_DESERIALIZE(INT32);
TSGEN_SCALAR_VEC_DESERIALIZE(UINT32);
TSGEN_SCALAR_VEC_DESERIALIZE(INT64);
TSGEN_SCALAR_VEC_DESERIALIZE(UINT64);
TSGEN_SCALAR_VEC_DESERIALIZE(FLOAT);
TSGEN_SCALAR_VEC_DESERIALIZE(DOUBLE);

#define TSGEN_INNERTYPE_VEC_SERIALIZE(TYPE) \
TSISerializer& TSSerializer::operator<<( const std::vector<TYPE> &from ) \
{\
	UINT32 size = (UINT32)from.size(); \
	(*this) << size; \
	\
	for(UINT32 i = 0; i < size ;++i) \
	{ \
		(*this) << from[i]; \
	} \
	return (*this);\
}

#define TSGEN_INNERTYPE_VEC_DESERIALIZE(TYPE) \
TSISerializer& TSSerializer::operator>>( std::vector<TYPE> &dest ) \
{\
	UINT32 size;\
	(*this) >> size; \
	dest.clear(); \
	if(size == 0) \
	{ \
		return (*this); \
	} \
	\
	dest.resize(size); \
	for(UINT32 i = 0; i < size ;++i) \
	{ \
		(*this) >> dest[i]; \
	} \
	return (*this); \
}

TSGEN_INNERTYPE_VEC_SERIALIZE(TSTime);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSTimeDuration);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSHANDLE);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSTemplateId);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSModelDataId);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSAssetTypeId);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSModelTmplOrClassId);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSByteBuffer);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSVector2i);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSVector3i);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSVector2f);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSVector3f);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSVector2d);
TSGEN_INNERTYPE_VEC_SERIALIZE(TSVector3d);

TSGEN_INNERTYPE_VEC_DESERIALIZE(TSTime);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSTimeDuration);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSHANDLE);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSTemplateId);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSModelDataId);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSAssetTypeId);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSModelTmplOrClassId);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSByteBuffer);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSVector2i);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSVector3i);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSVector2f);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSVector3f);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSVector2d);
TSGEN_INNERTYPE_VEC_DESERIALIZE(TSVector3d);

TSSerializer::TSSerializer(TSISerializerContext * Context,SerializeConfig Cfg)
	:TSISerializer(Context),
	_Private_Ptr(new PrivateType())
{
	_Private_Ptr->_Ctx = new TSDataContext();
	_Private_Ptr->_Cfg = Cfg;
	_Private_Ptr->_Align = ALIGN_CDR;

	ASSERT(_Private_Ptr->_Ctx);
	ASSERT(_Private_Ptr->_Ctx->GetBinary());

	CheckInit();
}

TSSerializer::TSSerializer( TSByteBufferPtr Buffer ,TSISerializerContext * Context,SerializeConfig Cfg)
	:TSISerializer(Context),
	_Private_Ptr(new PrivateType())
{
	_Private_Ptr->_Ctx = new TSDataContext(Buffer);
	_Private_Ptr->_Cfg = Cfg;
	_Private_Ptr->_Align = ALIGN_CDR;

	ASSERT(_Private_Ptr->_Ctx);
	ASSERT(_Private_Ptr->_Ctx->GetBinary());

	CheckInit();
}

TSSerializer::TSSerializer( TSDataContextPtr DataCtx,TSISerializerContext * Context /*= 0*/ ,SerializeConfig Cfg)
	:_Private_Ptr(new PrivateType)
{
	_Private_Ptr->_Ctx = DataCtx;
	_Private_Ptr->_Cfg = Cfg;
	_Private_Ptr->_Align = ALIGN_CDR;

	ASSERT(_Private_Ptr->_Ctx);
	ASSERT(_Private_Ptr->_Ctx->GetBinary());

	CheckInit();
}

TSSerializer::TSSerializer( PrivateType * p )
	:TSISerializer(Context),
	_Private_Ptr(p)
{
	ASSERT(_Private_Ptr->_Ctx);
	ASSERT(_Private_Ptr->_Ctx->GetBinary());
	_Private_Ptr->_Align = ALIGN_CDR;

	CheckInit();
}


void TSSerializer::CheckInit()
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    UINT64 OriOffsetR = _Private_Ptr->_Ctx->GetBinary()->GetOffsetR();
    UINT64 OriOffsetW = _Private_Ptr->_Ctx->GetBinary()->GetOffsetW();
#else
    UINT64 OriOffsetR = _Private_Ptr->_Ctx->GetBinary()->TellR();
    UINT64 OriOffsetW = _Private_Ptr->_Ctx->GetBinary()->TellW();
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	_Private_Ptr->_AlignReadShift = 
		ptrdiff_t(OriOffsetR) % TSSerializer::MAX_ALIGN;

	_Private_Ptr->_AlignWriteShift = 
		ptrdiff_t(OriOffsetW) % TSSerializer::MAX_ALIGN;

	bool Checked = false;

	//这里暂时只检测厂商版本，不检测协议版本。提高效率

	if (_Private_Ptr->_Ctx->GetRtpsVendorIdMajor() == RTPS_VENDORID_MAJOR
		&& _Private_Ptr->_Ctx->GetRtpsVendorIdMinor() == RTPS_VENDORID_MINOR)
	{
		Checked = true;
	}
	else
	{
		Checked = false;
	}

	if (Checked)
	{
		_Private_Ptr->_Ctx->CheckInit();

		//校验成功
	}

	//todo check失败，即接收到的是其它异构系统的数据，构建key
}




void TSSerializer::EndArray(UINT8 AlginSize, MarshalType MType)
{
	if (MType == TSISerializer::Marshal)
	{
		Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	}
	else
	{
		Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	}
}

void TSSerializer::EndMap(UINT8 AlginSize, MarshalType MType)
{
	if (MType == TSISerializer::Marshal)
	{
		Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	}
	else
	{
		Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	}
}

bool TSSerializer::GetSerializerKeyFlag()
{
	return _Private_Ptr->_SerializerKeyFlag;
}

void TSSerializer::SetSerializerKeyFlag(bool SerializerKeyFlag)
{
	_Private_Ptr->_SerializerKeyFlag = SerializerKeyFlag;
}

TSSerializer::~TSSerializer(void)
{
	delete _Private_Ptr;
}

void TSSerializer::SetSwap( bool swap )
{
	_Private_Ptr->_Swap = swap;
}

bool TSSerializer::Swap() const
{
	return _Private_Ptr->_Swap;
}

TSSerializer::Alignment TSSerializer::Align() const
{
	return _Private_Ptr->_Align;
}

bool TSSerializer::GoodBit() const
{
	return _Private_Ptr->_GoodBit;
}

void TSSerializer::ReadString( wchar_t *&dest )
{
	if(dest != NULL)
	{
		delete []dest;
	}

	dest = 0;
	UINT32 byteCount = 0;
	BufferRead(reinterpret_cast<char*>(&byteCount),sizeof(UINT32),_Private_Ptr->_Swap);
    _Private_Ptr->_Align == ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));

	if(!_Private_Ptr->_GoodBit)
	{
		return ;
	}

	if(byteCount <= GetReadLength())
	{
		UINT32 len = byteCount / WCHAR_SIZE;
		dest = new wchar_t[len + 1];

		if(dest == NULL)
		{
			_Private_Ptr->_GoodBit = false;
			return;
		}

		//TODO::the wchar size may be not always 2.
		ReadArray(reinterpret_cast<char *>(dest),WCHAR_SIZE,len,SWAP_BE);
        _Private_Ptr->_Align == ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));

		if(_Private_Ptr->_GoodBit)
		{
			dest[len] = L'\0';
		}
		else
		{
			delete []dest;
			dest = 0;
		}
	}
	else
	{
		_Private_Ptr->_GoodBit = false;
	}

     
}

void TSSerializer::BufferRead( char *dest,size_t size,bool swap )
{
	DoRead(dest,size,swap);
}

bool TSSerializer::DoRead( char *dest,size_t size,bool swap )
{
	if(!_Private_Ptr->_Ctx->GetBinary())
	{
		_Private_Ptr->_GoodBit = false;
		return _Private_Ptr->_GoodBit;
	}

	const size_t len = GetReadLength();
	if(size > len)
	{
		_Private_Ptr->_GoodBit = false;
		return _Private_Ptr->_GoodBit;
	}

	swap
		? SwapCpy(dest,_Private_Ptr->_Ctx->GetBinary()->Data(),size)
		: SmemCpy(dest,_Private_Ptr->_Ctx->GetBinary()->Data(),size);

#if defined(XSIM3_3) || defined(XSIM3_2)
    _Private_Ptr->_Ctx->GetBinary()->SkipR(size);
#else
    _Private_Ptr->_Ctx->GetBinary()->SeekR(size,false);
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	return _Private_Ptr->_GoodBit;
}

void TSSerializer::SwapCpy( char *to,const char *from,size_t n )
{
	switch(n)					  // 2   4   8   16
	{
	case 16:
		to[15] = from[ n - 16 ];  // x    x   x   0
	case 15:
		to[14] = from[ n - 15 ];  // x    x   x   1
	case 14:
		to[13] = from[ n - 14 ];
	case 13:
		to[12] = from[ n - 13 ];
	case 12:
		to[11] = from[ n - 12 ];
	case 11:
		to[10] = from[ n - 11 ];
	case 10:
		to[ 9] = from[ n - 10 ];
	case 9:
		to[ 8] = from[ n - 9 ];
	case 8:
		to[ 7] = from[ n - 8 ];
	case 7:
		to[ 6] = from[ n - 7 ];
	case 6:
		to[ 5] = from[ n - 6 ];
	case 5:
		to[ 4] = from[ n - 5 ];
	case 4:
		to[ 3] = from[ n - 4 ];
	case 3:
		to[ 2] = from[ n - 3 ];
	case 2:
		to[ 1] = from[ n - 2 ];
	case 1:
		to[ 0] = from[ n - 1 ];
	case 0:
		return;
	default:
		_Private_Ptr->_GoodBit = false;
	}
}

void TSSerializer::SmemCpy( char *to,const char *from,size_t n )
{
    std::memcpy(reinterpret_cast<void *>(to),reinterpret_cast<const void *>(from),n);
}

int TSSerializer::AlignRead( size_t alignment )
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    UINT64 OriOffsetR = _Private_Ptr->_Ctx->GetBinary()->GetOffsetR();
#else
    UINT64 OriOffsetR = _Private_Ptr->_Ctx->GetBinary()->TellR();
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	const size_t len = 
		(alignment - OriOffsetR + _Private_Ptr->_AlignReadShift) % alignment;
	
	Skip(static_cast<UINT16>(len));

	return 0;
}

bool TSSerializer::Skip( UINT16 n,int size /*= 1*/ )
{
	if(size > 1 && _Private_Ptr->_Align != ALIGN_NONE)
	{
		size_t skipSize = size_t(size) > MAX_ALIGN ? MAX_ALIGN : size_t(size);
		if(skipSize > GetReadLength())
		{
			_Private_Ptr->_GoodBit = false;
			return false;
		}
		else
		{
			
#if defined(XSIM3_3) || defined(XSIM3_2)
            _Private_Ptr->_Ctx->GetBinary()->SkipR(skipSize);
#else
            _Private_Ptr->_Ctx->GetBinary()->SeekR(skipSize,false);
#endif // defined(XSIM3_3) || defined(XSIM3_2)
		}
	}

	size_t len = static_cast<size_t>(n * size);
	const size_t cur_len = GetReadLength();
	if(cur_len < len)
	{
		_Private_Ptr->_GoodBit = false;
		return false;
	}
	else
	{
#if defined(XSIM3_3) || defined(XSIM3_2)
        _Private_Ptr->_Ctx->GetBinary()->SkipR(len);
#else
        _Private_Ptr->_Ctx->GetBinary()->SeekR(len,false);
#endif // defined(XSIM3_3) || defined(XSIM3_2)
	}

	return _Private_Ptr->_GoodBit;
}

bool TSSerializer::ReadCharArray( char *dest,UINT32 length )
{
	ReadArray(dest,sizeof(char),length);

	return _Private_Ptr->_GoodBit;
}

void TSSerializer::ReadArray( char *dest,size_t size,UINT32 length )
{
	ReadArray(dest,size,length,_Private_Ptr->_Swap);
}

void TSSerializer::ReadArray( char *dest,size_t size,UINT32 length,bool swap )
{
	if(!swap || size == 1)
	{
		BufferRead(dest,size * length,false);
	}
	else
	{
		while(length-- > 0)
		{
			BufferRead(dest,size,true);
			dest += size;
		}
	}
}

void TSSerializer::BufferWrite( const char* src, size_t size, bool swap )
{
	DoWrite(src,size,swap);
}

bool TSSerializer::DoWrite( const char* src, size_t size, bool swap )
{
	if(!_Private_Ptr->_Ctx->GetBinary())
	{
		_Private_Ptr->_GoodBit = false;
		return size;
	}

	if(swap)
	{
		SwapCpy(_Private_Ptr->tempArray,src,size);
		_Private_Ptr->_Ctx->GetBinary()->WriteBytes(_Private_Ptr->tempArray,size);
	}
	else
	{
		_Private_Ptr->_Ctx->GetBinary()->WriteBytes(src,size);
	}

	return _Private_Ptr->_GoodBit;
}

int TSSerializer::AlignWrite( size_t alignment )
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    UINT64 OriOffsetW = _Private_Ptr->_Ctx->GetBinary()->GetOffsetW();
#else
    UINT64 OriOffsetW = _Private_Ptr->_Ctx->GetBinary()->TellW();
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	size_t len = 
		(alignment - OriOffsetW + _Private_Ptr->_AlignWriteShift) % alignment;

	if(len)
	{
		_Private_Ptr->_Ctx->GetBinary()->WriteBytes(ALIGN_PAD,len);
	}

	return 0;
}

void TSSerializer::WriteArray( const char *dest,size_t size,UINT32 length )
{
	WriteArray(dest,size,length,_Private_Ptr->_Swap);
}

void TSSerializer::WriteArray( const char *dest,size_t size,UINT32 length,bool swap )
{
	if(!swap || size == 1)
	{
		BufferWrite(dest,size * length,false);
	}
	else
	{
		while(length-- > 0)
		{
			BufferWrite(dest,size,true);
			dest += size;
		}
	}
}

void TSSerializer::SetAlign( Alignment align )
{
	_Private_Ptr->_Align = align;
}

TSISerializer& TSSerializer::operator<<( const bool &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(bool),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<(const std::vector<bool> &from )
{
	size_t size = from.size();
	(*this) << size;
	if(size == 0)
	{
		return *this;
	}

	for(UINT32 i = 0; i < size ;++i)
	{
		bool b = from[i];
		BufferWrite(reinterpret_cast<char *>(&b),sizeof(bool),true);
	}

	return *this;
}

TSISerializer& TSSerializer::operator<<( const char &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(char),false);
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const INT8 &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(INT8),false);
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const UINT8 &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(UINT8),false);
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const INT16 &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(INT16),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const UINT16 &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(UINT16),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const INT32 &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(INT32),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const UINT32 &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(UINT32),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const INT64 &from )
{
    BufferWrite(reinterpret_cast<const char *>(&from),sizeof(INT64),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const UINT64 &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(UINT64),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const FLOAT &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(FLOAT),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const DOUBLE &from )
{
	BufferWrite(reinterpret_cast<const char *>(&from),sizeof(DOUBLE),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));
	return *this;
}

TSISerializer& TSSerializer::operator<<( const std::string &from )
{
	//dds协议里字符串序列化需要带上\0,平台序列化会转换成utf8格式

	if (_Private_Ptr->_Ctx->GetRtpsVendorIdMajor() == RTPS_VENDORID_MAJOR
		&& _Private_Ptr->_Ctx->GetRtpsVendorIdMinor() == RTPS_VENDORID_MINOR)
	{
		TSString SerializerString = TSString2Utf8(from);

		const UINT32 stringlen = (UINT32)SerializerString.length();
		(*this) << (stringlen + 1);

		BufferWrite(reinterpret_cast<const char*>(SerializerString.data()), stringlen, false);
	}
	else
	{
		const UINT32 stringlen = (UINT32)from.length();

		(*this) << (stringlen + 1);

		BufferWrite(reinterpret_cast<const char*>(from.data()), stringlen, false);
	}

	
	BufferWrite(&ZeroChar, 1, false);

	Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));

	return (*this);
}

TSISerializer& TSSerializer::operator<<( const std::vector<std::string> &from )
{
	UINT32 size = (UINT32)from.size();

	(*this) << size;

	if(size == 0)
	{
		return (*this);
	}

	for(UINT32 i = 0; i < size ;++i)
	{
		(*this) << from[i];
	}

	return (*this);
}

#ifndef _TSRUNTIME_NO_WCHAR_T
TSISerializer& TSSerializer::operator<<( const wchar_t * from )
{
	if(from != 0)
	{
		const UINT32 length = static_cast<UINT32>(std::wcslen(from));
		(*this) << UINT32(length * TSSerializer::WCHAR_SIZE);

		//TODO::wchar_t may not be size of 2.We suppose it be 2.
		WriteArray(reinterpret_cast<const char*>(from),TSSerializer::WCHAR_SIZE,length,TSSerializer::SWAP_BE);
        Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));

	}
	else
	{
		(*this) << UINT32(0);
	}
	
	return (*this);
}

TSISerializer& TSSerializer::operator<<( const std::wstring &from )
{
	const UINT32 stringlen = (UINT32)from.length();

	(*this) << stringlen;

	if(stringlen)
	{
		BufferWrite(reinterpret_cast<const char*>(from.data()),stringlen,false);
	}

	Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));

	return (*this);
}

TSISerializer& TSSerializer::operator<<( const std::vector<std::wstring> &from )
{
	UINT32 size = (UINT32)from.size();

	(*this) << size;
	
	if(size == 0)
	{
		return (*this);
	}

	for(UINT32 i = 0; i < size ;++i)
	{
		(*this) << from[i];
	}

	return (*this);
}
#endif

TSISerializer& TSSerializer::operator<<( const TSTime &from )
{
	(*this) << TIME2TD(from).total_microseconds();
	return (*this);
}
										    
TSISerializer& TSSerializer::operator<<( const TSTimeDuration &from )
{
	(*this) << from.total_microseconds();
	return (*this);
}
										    
TSISerializer& TSSerializer::operator<<( const TSHANDLE &from )
{
	(*this) << from._value;
	return (*this);
}
										    
TSISerializer& TSSerializer::operator<<( const TSTemplateId &from )
{
	(*this) << from._value;
	return (*this);
}
										    
TSISerializer& TSSerializer::operator<<( const TSModelDataId &from )
{
	(*this) << from._value;
	return (*this);
}
										    
TSISerializer& TSSerializer::operator<<( const TSAssetTypeId &from )
{
	(*this) << from._value;
	return (*this);
}
										    
TSISerializer& TSSerializer::operator<<( const TSModelTmplOrClassId &from )
{
	(*this) << from._value;
	return (*this);
}
										    
TSISerializer& TSSerializer::operator<<( const TSByteBuffer &from )
{
    size_t length = from.Length();
	(*this) << (UINT32)length;

	BufferWrite(from.Data(),length,false);
	Align() == TSSerializer::ALIGN_NONE ? 0 : AlignWrite(sizeof(UINT32));

	return (*this);
}

TSISerializer& TSSerializer::operator<<( const TSTOPICHANDLE &from )
{
	(*this) << from._value;
	return (*this);
}

TSISerializer& TSSerializer::operator<<( const TSFrontAppUniquelyId &from )
{
	(*this) << from._value;
	return (*this);
}

TSISerializer& TSSerializer::operator<<( const TSInstanceHandle &from )
{
	(*this) << from._TopicHandle;
	(*this) << from._DataTopicHandle;
	(*this) << from._InstanceIndex;
	return (*this);
}

TSISerializer& TSSerializer::operator<<( const TSObjectHandle &from )
{
	(*this) << from._TopicHandle;
	(*this) << from._FrontAppId;
	(*this) << from._Handle;
	(*this) << from._Endpoint;
	return (*this);
}


TSISerializer& TSSerializer::operator>>( bool &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(bool),false);
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( std::vector<bool> &dest )
{
	UINT32 size;
	(*this) >> size;
	
	dest.clear();
	if(size == 0)
	{
		return (*this);
	}

	dest.resize(size);
	bool *b = new bool[size];
	ReadArray(reinterpret_cast<char *>(b),sizeof(bool),size);
	for(UINT32 i = 0; i < size ;++i)
	{
		dest[i] = b[i];
	}
	delete []b;

	return (*this);
}

TSISerializer& TSSerializer::operator>>( char &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(char),false);
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( INT8 &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(INT8),false);
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( UINT8 &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(UINT8),false);
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( INT16 &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(INT16),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( UINT16 &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(UINT16),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( INT32 &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(INT32),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( UINT32 &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(UINT32),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( INT64 &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(INT64),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( UINT64 &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(UINT64),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( FLOAT &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(FLOAT),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( DOUBLE &dest )
{
	BufferRead(reinterpret_cast<char*>(&dest),sizeof(DOUBLE),Swap());
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));
	return (*this);
}

TSISerializer& TSSerializer::operator>>( std::string &dest )
{
	UINT32 length = 0;
	
	(*this) >> length;

	if(length)
	{
		//dds协议里字符串序列化需要带上/0,平台序列化会转换成utf8格式

		dest.resize(length - 1);

		BufferRead(const_cast<char*>(dest.data()), length - 1, Swap());
		char temp;
		BufferRead(const_cast<char*>(&temp), 1, Swap());

		if (_Private_Ptr->_Ctx->GetRtpsVendorIdMajor() == RTPS_VENDORID_MAJOR
			&& _Private_Ptr->_Ctx->GetRtpsVendorIdMinor() == RTPS_VENDORID_MINOR)
		{
			dest = Utf8ToTSString(dest);
		}
	}

    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));

	return (*this);
}

TSISerializer& TSSerializer::operator>>( std::vector<std::string> &dest )
{
	UINT32 size;
	(*this) >> size;
	
	dest.clear();
	if(size == 0)
	{
		return (*this);
	}
	dest.resize(size);
	for(UINT32 i = 0; i < size ;++i)
	{
		(*this) >> dest[i];
	}

	return (*this);
}
#ifndef _TSRUNTIME_NO_WCHAR_T
TSISerializer& TSSerializer::operator>>( wchar_t* dest )
{
	ReadString(dest);
	return (*this);
}

TSISerializer& TSSerializer::operator>>( std::wstring &dest )
{
	UINT32 byteCount = 0;
	BufferRead(reinterpret_cast<char*>(&byteCount),sizeof(UINT32),_Private_Ptr->_Swap);
    Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));

	if(!_Private_Ptr->_GoodBit)
	{
		return (*this);
	}

	if(byteCount <= GetReadLength())
	{
		UINT32 len = byteCount / TSSerializer::WCHAR_SIZE;
		dest.resize(len);

		//TODO::the wchar size may be not always 2.
		ReadArray(reinterpret_cast<char *>((const_cast<wchar_t *>(dest.data()))),TSSerializer::WCHAR_SIZE,len,SWAP_BE);
        Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));

		if(_Private_Ptr->_GoodBit)
		{
			return (*this);
		}
	}
	
	_Private_Ptr->_GoodBit = false;

	return (*this);
}

TSISerializer& TSSerializer::operator>>( std::vector<std::wstring> &dest )
{
	UINT32 size;
	(*this) >> size;
	
	dest.clear();
	if(size == 0)
	{
		return (*this);
	}
	dest.resize(size);
	for(UINT32 i = 0; i < size ;++i)
	{
		(*this) >> dest[i];
	}

	return (*this);
}
#endif
TSISerializer& TSSerializer::operator>>( TSTime &dest )
{
	UINT64 mic;
	(*this) >> mic;
	dest = TD2TIME(TSMicroseconds(mic));

	return (*this);
}
										    
TSISerializer& TSSerializer::operator>>( TSTimeDuration &dest )
{
	UINT64 mic;
	(*this) >> mic;
	dest = TSMicroseconds(mic);

	return (*this);
}
										    
TSISerializer& TSSerializer::operator>>( TSHANDLE &dest )
{
	(*this) >> dest._value;
	return *this;
}
										    
TSISerializer& TSSerializer::operator>>( TSTemplateId &dest )
{
	(*this) >> dest._value;
	return *this;
}
										    
TSISerializer& TSSerializer::operator>>( TSModelDataId &dest )
{
	(*this) >> dest._value;
	return *this;
}
										    
TSISerializer& TSSerializer::operator>>( TSAssetTypeId &dest )
{
	(*this) >> dest._value;
	return *this;
}
										    
TSISerializer& TSSerializer::operator>>( TSModelTmplOrClassId &dest )
{
	(*this) >> dest._value;
	return *this;
}
										    
TSISerializer& TSSerializer::operator>>( TSByteBuffer &dest )
{
    //size_t is different size in win32 and x64.
	UINT32 len = 0;
	(*this) >> len;

	char * bytes = (char *)malloc(len);


	BufferRead(bytes,len,false);
    
	dest.WriteBytes(bytes,len);

	free(bytes);

	Align() == TSSerializer::ALIGN_NONE ? 0 : AlignRead(sizeof(UINT32));

	return *this;
}

bool TSSerializer::PerpareForSerialize(const UINT32 & VersionCode, const TSTime & Time)
{
	GetDataContext()->SetTime(Time);
	GetDataContext()->SetDataTypeVersionHashCode(VersionCode);

	return true;
}

bool TSSerializer::PerpareForDeserialize(const TSChar* TypeName, const UINT32 & VersionCode, TSTime & Time)
{
	if (GetDataContext()->GetDataTypeVersionHashCode()
		&& GetDataContext()->GetDataTypeVersionHashCode() != VersionCode)
	{
		ThrowHashVersionTypeExceptionPSNoMatch(TypeName, VersionCode, GetDataContext()->GetDataTypeVersionHashCode());
		return false;
	}
	else
	{
		Time = GetDataContext()->GetTime();
		return true;
	}
}

TSISerializer& TSSerializer::operator<<( const TSVector2i& from )
{
	(*this) << from.x;
	(*this) << from.y;
	return *this;
}

TSISerializer& TSSerializer::operator<<( const TSVector3i& from )
{
	(*this) << from.x;
	(*this) << from.y;
	(*this) << from.z;
	return *this;
}

TSISerializer& TSSerializer::operator<<( const TSVector2f& from )
{
	(*this) << from.x;
	(*this) << from.y;
	return *this;
}

TSISerializer& TSSerializer::operator<<( const TSVector3f& from )
{
	(*this) << from.x;
	(*this) << from.y;
	(*this) << from.z;
	return *this;
}

TSISerializer& TSSerializer::operator<<( const TSVector2d& from )
{
	(*this) << from.x;
	(*this) << from.y;
	return *this;
}

TSISerializer& TSSerializer::operator<<( const TSVector3d& from )
{
	(*this) << from.x;
	(*this) << from.y;
	(*this) << from.z;
	return *this;
}

TSISerializer& TSSerializer::operator>>( TSVector2i& from )
{
	(*this) >> from.x;
	(*this) >> from.y;

	return *this;								
}										
										
TSISerializer& TSSerializer::operator>>( TSVector3i& from )
{		
	(*this) >> from.x;
	(*this) >> from.y;
	(*this) >> from.z;
	return *this;								
}										
										
TSISerializer& TSSerializer::operator>>( TSVector2f& from )
{	
	(*this) >> from.x;
	(*this) >> from.y;
	return *this;								
}										
										
TSISerializer& TSSerializer::operator>>( TSVector3f& from )
{		
	(*this) >> from.x;
	(*this) >> from.y;
	(*this) >> from.z;
	return *this;								
}										
										
TSISerializer& TSSerializer::operator>>( TSVector2d& from )
{	
	(*this) >> from.x;
	(*this) >> from.y;
	return *this;								
}										
										
TSISerializer& TSSerializer::operator>>( TSVector3d& from )
{
	(*this) >> from.x;
	(*this) >> from.y;
	(*this) >> from.z;
	return *this;
}

TSISerializer& TSSerializer::operator>>( TSTOPICHANDLE &dest )
{
	(*this) >> dest._value;
	return (*this);
}

TSISerializer& TSSerializer::operator>>( TSFrontAppUniquelyId &dest )
{
	(*this) >> dest._value;
	return (*this);
}

TSISerializer& TSSerializer::operator>>( TSInstanceHandle &dest )
{
	(*this) >> dest._TopicHandle;
	(*this) >> dest._DataTopicHandle;
	(*this) >> dest._InstanceIndex;
	return (*this);
}

TSISerializer& TSSerializer::operator>>( TSObjectHandle &dest )
{
	(*this) >> dest._TopicHandle;
	(*this) >> dest._FrontAppId;
	(*this) >> dest._Handle;
	(*this) >> dest._Endpoint;
	return (*this);
}

TSISerializer& TSSerializer::operator>>( TSObjectIdentity& dest )
{
	(*this) >> dest._ObjectHandle;
	return (*this);
}


bool TSSerializer::ReadBytes( char *dest,size_t size )
{
    if(!_Private_Ptr->_Ctx->GetBinary())
    {
        _Private_Ptr->_GoodBit = false;
    }
    else
    {
        const size_t len = GetReadLength();

        if(size > len)
        {
            _Private_Ptr->_GoodBit = false;
        }
        SmemCpy(dest,_Private_Ptr->_Ctx->GetBinary()->Data(),size);

#if defined(XSIM3_3) || defined(XSIM3_2)
        _Private_Ptr->_Ctx->GetBinary()->SkipR(size);
#else
        _Private_Ptr->_Ctx->GetBinary()->SeekR(size,false);
#endif // defined(XSIM3_3) || defined(XSIM3_2)
        
    }


    return _Private_Ptr->_GoodBit;
}


bool TSSerializer::WriteBytes( const char* src, size_t size )
{
    if(!_Private_Ptr->_Ctx->GetBinary())
    {
        _Private_Ptr->_GoodBit = false;
    }
    else
    {
        _Private_Ptr->_Ctx->GetBinary()->WriteBytes(src,size);
    }

    return _Private_Ptr->_GoodBit;
}

UINT64 TSSerializer::GetReadLength() const
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    return _Private_Ptr->_Ctx->GetBinary()->GetOffsetW() - _Private_Ptr->_Ctx->GetBinary()->GetOffsetR();
#else
    return _Private_Ptr->_Ctx->GetBinary()->TellW() - _Private_Ptr->_Ctx->GetBinary()->TellR();
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

TSByteBufferPtr TSSerializer::GetBuffer() const
{
	return _Private_Ptr->_Ctx->GetBinary();
}

TSISerializer& TSSerializer::operator<<( const TSObjectIdentity& from )
{
	(*this) << from._ObjectHandle;
	return (*this);
}

TSDataContextPtr TSSerializer::GetDataContext() const
{
	return _Private_Ptr->_Ctx;
}

bool TSSerializer::MarshalKey( const TSDataKeyCompare& Key )
{
	_Private_Ptr->_Ctx->AddKey(Key);
	return true;
}

bool TSSerializer::MarshalComplete()
{
	if (!GetSerializerKeyFlag())
	{
		if (_Private_Ptr->_Cfg != TSSerializer::kCheckInit)
		{
			return true;
		}

		_Private_Ptr->_Ctx->MarshalComplete();
	}

	return true;
}

UINT64 TSSerializer::TellW()
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    return _Private_Ptr->_Ctx->GetBinary()->GetOffsetW();
#else
    return _Private_Ptr->_Ctx->GetBinary()->TellW();
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

UINT64 TSSerializer::TellR()
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    return _Private_Ptr->_Ctx->GetBinary()->GetOffsetR();
#else
    return _Private_Ptr->_Ctx->GetBinary()->TellR();
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}


