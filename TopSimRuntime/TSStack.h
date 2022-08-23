#ifndef __TSRDGSERIALIZER__H__
#define __TSRDGSERIALIZER__H__

#include "TSTopicTypes.h"
#include "TSISerializer.h"

struct TSSerializerPrivate;
class TOPSIMDATAINTERFACE_DECL TSSerializer : public TSISerializer
{
public:
	enum Alignment
	{
		ALIGN_NONE,
		ALIGN_INITIALIZE,
		ALIGN_CDR
	};

	enum SerializeConfig
	{
		kCheckInit,
		kNoCheckInit
	};

public:
	TSSerializer(TSISerializerContext * Context = 0,SerializeConfig Cfg = kCheckInit);
	TSSerializer(TSByteBufferPtr Buffer,TSISerializerContext * Context = 0,SerializeConfig Cfg = kCheckInit);
	TSSerializer(TSDataContextPtr DataCtx,TSISerializerContext * Context = 0,SerializeConfig Cfg = kCheckInit);
	virtual ~TSSerializer(void);

private:
	TSSerializer(const TSSerializer &other);
	TSSerializer& operator=(const TSSerializer &other);

public:

	void SetSwap(bool swap);
	bool Swap() const;

	Alignment Align() const;
	void SetAlign(Alignment align);

	/// Align for reading: moves ReadPtr() past the alignment padding.
	/// Alignments of 2, 4, or 8 are supported by CDR and this implementation.
	int AlignRead(size_t alignment);
	int AlignWrite(size_t alignment);

	bool GoodBit() const;

	void ReadString(wchar_t *&dest);

	bool Skip(UINT16 n,int size = 1);
    virtual bool WriteBytes(const char* src, size_t size);
    virtual bool ReadBytes(char *dest,size_t size);

	TSByteBufferPtr GetBuffer() const;
	TSDataContextPtr GetDataContext() const;

	virtual UINT64 TellW();
	virtual UINT64 TellR();
	virtual bool MarshalKey(const TSDataKeyCompare& Key);
	virtual bool MarshalComplete();

public:
	virtual TSISerializer& operator<<(const bool &from);
	virtual TSISerializer& operator<<(const std::vector<bool> &from);
	virtual TSISerializer& operator<<(const char &from);
	virtual TSISerializer& operator<<(const std::vector<char> &from);
	virtual TSISerializer& operator<<(const INT8 &from);
	virtual TSISerializer& operator<<(const std::vector<INT8> &from);
	virtual TSISerializer& operator<<(const UINT8 &from);
	virtual TSISerializer& operator<<(const std::vector<UINT8> &from);
	virtual TSISerializer& operator<<(const INT16 &from);
	virtual TSISerializer& operator<<(const std::vector<INT16> &from);
	virtual TSISerializer& operator<<(const UINT16 &from);
	virtual TSISerializer& operator<<(const std::vector<UINT16> &from);
	virtual TSISerializer& operator<<(const INT32 &from);
	virtual TSISerializer& operator<<(const std::vector<INT32> &from);
	virtual TSISerializer& operator<<(const UINT32 &from);
	virtual TSISerializer& operator<<(const std::vector<UINT32> &from);
	virtual TSISerializer& operator<<(const INT64 &from);
	virtual TSISerializer& operator<<(const std::vector<INT64> &from);
	virtual TSISerializer& operator<<(const UINT64 &from);
	virtual TSISerializer& operator<<(const std::vector<UINT64> &from);
	virtual TSISerializer& operator<<(const FLOAT &from);
	virtual TSISerializer& operator<<(const std::vector<FLOAT> &from);
	virtual TSISerializer& operator<<(const DOUBLE &from);
	virtual TSISerializer& operator<<(const std::vector<DOUBLE> &from);
	virtual TSISerializer& operator<<(const std::string &from);
	virtual TSISerializer& operator<<(const std::vector<std::string> &from);
#ifndef _TSRUNTIME_NO_WCHAR_T
	virtual TSISerializer& operator<<(const wchar_t *from);
	virtual TSISerializer& operator<<(const std::wstring &from);
	virtual TSISerializer& operator<<(const std::vector<std::wstring> &from);
#endif
	virtual TSISerializer& operator<<(const TSTime &from);
	virtual TSISerializer& operator<<(const std::vector<TSTime> &from);
	virtual TSISerializer& operator<<(const TSTimeDuration &from);
	virtual TSISerializer& operator<<(const std::vector<TSTimeDuration> &from);
	virtual TSISerializer& operator<<(const TSHANDLE &from);
	virtual TSISerializer& operator<<(const std::vector<TSHANDLE> &from);
	virtual TSISerializer& operator<<(const TSTemplateId &from);
	virtual TSISerializer& operator<<(const std::vector<TSTemplateId> &from);
	virtual TSISerializer& operator<<(const TSModelDataId &from);
	virtual TSISerializer& operator<<(const std::vector<TSModelDataId> &from);
	virtual TSISerializer& operator<<(const TSAssetTypeId &from);
	virtual TSISerializer& operator<<(const std::vector<TSAssetTypeId> &from);
	virtual TSISerializer& operator<<(const TSModelTmplOrClassId &from);
	virtual TSISerializer& operator<<(const std::vector<TSModelTmplOrClassId> &from);
	virtual TSISerializer& operator<<(const TSByteBuffer &from);
	virtual TSISerializer& operator<<(const std::vector<TSByteBuffer> &from);
	virtual TSISerializer& operator<<(const TSVector2i& from);
	virtual TSISerializer& operator<<(const std::vector<TSVector2i> &from);
	virtual TSISerializer& operator<<(const TSVector3i& from);
	virtual TSISerializer& operator<<(const std::vector<TSVector3i> &from);
	virtual TSISerializer& operator<<(const TSVector2f& from);
	virtual TSISerializer& operator<<(const std::vector<TSVector2f> &from);
	virtual TSISerializer& operator<<(const TSVector3f& from);
	virtual TSISerializer& operator<<(const std::vector<TSVector3f> &from);
	virtual TSISerializer& operator<<(const TSVector2d& from);
	virtual TSISerializer& operator<<(const std::vector<TSVector2d> &from);
	virtual TSISerializer& operator<<(const TSVector3d& from);
	virtual TSISerializer& operator<<(const std::vector<TSVector3d> &from);
	virtual TSISerializer& operator<<(const TSTOPICHANDLE &from);
	virtual TSISerializer& operator<<(const TSFrontAppUniquelyId &from);
	virtual TSISerializer& operator<<(const TSInstanceHandle &from);
	virtual TSISerializer& operator<<(const TSObjectHandle &from);
	virtual TSISerializer& operator<<(const TSObjectIdentity& from);
	virtual TSISerializer& operator>>(bool &dest);
	virtual TSISerializer& operator>>(std::vector<bool> &dest);
	virtual TSISerializer& operator>>(char &dest);
	virtual TSISerializer& operator>>(std::vector<char> &dest);
	virtual TSISerializer& operator>>(INT8 &dest);
	virtual TSISerializer& operator>>(std::vector<INT8> &dest);
	virtual TSISerializer& operator>>(UINT8 &dest);
	virtual TSISerializer& operator>>(std::vector<UINT8> &dest);
	virtual TSISerializer& operator>>(INT16 &dest);
	virtual TSISerializer& operator>>(std::vector<INT16> &dest);
	virtual TSISerializer& operator>>(UINT16 &dest);
	virtual TSISerializer& operator>>(std::vector<UINT16> &dest);
	virtual TSISerializer& operator>>(INT32 &dest);
	virtual TSISerializer& operator>>(std::vector<INT32> &dest);
	virtual TSISerializer& operator>>(UINT32 &dest);
	virtual TSISerializer& operator>>(std::vector<UINT32> &dest);
	virtual TSISerializer& operator>>(INT64 &dest);
	virtual TSISerializer& operator>>(std::vector<INT64> &dest);
	virtual TSISerializer& operator>>(UINT64 &dest);
	virtual TSISerializer& operator>>(std::vector<UINT64> &dest);
	virtual TSISerializer& operator>>(FLOAT &dest);
	virtual TSISerializer& operator>>(std::vector<FLOAT> &dest);
	virtual TSISerializer& operator>>(DOUBLE &dest);
	virtual TSISerializer& operator>>(std::vector<DOUBLE> &dest);
	virtual TSISerializer& operator>>(std::string &dest);
	virtual TSISerializer& operator>>(std::vector<std::string> &dest);
#ifndef _TSRUNTIME_NO_WCHAR_T
	virtual TSISerializer& operator>>(wchar_t *dest);
	virtual TSISerializer& operator>>(std::wstring &dest);
	virtual TSISerializer& operator>>(std::vector<std::wstring> &dest);
#endif
	virtual TSISerializer& operator>>(TSTime &dest);
	virtual TSISerializer& operator>>(std::vector<TSTime> &dest);
	virtual TSISerializer& operator>>(TSTimeDuration &dest);
	virtual TSISerializer& operator>>(std::vector<TSTimeDuration> &dest);
	virtual TSISerializer& operator>>(TSHANDLE &dest);
	virtual TSISerializer& operator>>(std::vector<TSHANDLE> &dest);
	virtual TSISerializer& operator>>(TSTemplateId &dest);
	virtual TSISerializer& operator>>(std::vector<TSTemplateId> &dest);
	virtual TSISerializer& operator>>(TSModelDataId &dest);
	virtual TSISerializer& operator>>(std::vector<TSModelDataId> &dest);
	virtual TSISerializer& operator>>(TSAssetTypeId &dest);
	virtual TSISerializer& operator>>(std::vector<TSAssetTypeId> &dest);
	virtual TSISerializer& operator>>(TSModelTmplOrClassId &dest);
	virtual TSISerializer& operator>>(std::vector<TSModelTmplOrClassId> &dest);
	virtual TSISerializer& operator>>(TSByteBuffer &dest);
	virtual TSISerializer& operator>>(std::vector<TSByteBuffer> &dest);
	virtual TSISerializer& operator>>(TSVector2i& from);
	virtual TSISerializer& operator>>(std::vector<TSVector2i> &from);
	virtual TSISerializer& operator>>(TSVector3i& from);
	virtual TSISerializer& operator>>(std::vector<TSVector3i> &from);
	virtual TSISerializer& operator>>(TSVector2f& from);
	virtual TSISerializer& operator>>(std::vector<TSVector2f> &from);
	virtual TSISerializer& operator>>(TSVector3f& from);
	virtual TSISerializer& operator>>(std::vector<TSVector3f> &from);
	virtual TSISerializer& operator>>(TSVector2d& from);
	virtual TSISerializer& operator>>(std::vector<TSVector2d> &from);
	virtual TSISerializer& operator>>(TSVector3d& from);
	virtual TSISerializer& operator>>(std::vector<TSVector3d> &from);
	virtual TSISerializer& operator>>(TSTOPICHANDLE &dest);
	virtual TSISerializer& operator>>(TSFrontAppUniquelyId &dest);
	virtual TSISerializer& operator>>(TSInstanceHandle &dest);
	virtual TSISerializer& operator>>(TSObjectHandle &dest);
	virtual TSISerializer& operator>>(TSObjectIdentity& dest);

	virtual bool PerpareForSerialize(const UINT32 & VersionCode, const TSTime & Time);
	virtual bool PerpareForDeserialize(const TSChar* TypeName, const UINT32 & VersionCode, TSTime & Time);
	void CheckInit();
	virtual void EndArray(UINT8 AlginSize,MarshalType MType);
	virtual void EndMap(UINT8 AlginSize, MarshalType MType);

	virtual bool GetSerializerKeyFlag();
	void SetSerializerKeyFlag(bool SerializerKeyFlag);
private:
	TSISerializer& operator<<(const void * &from);
	TSISerializer& operator>>(void* &dest);
private:
	UINT64 GetReadLength() const;
	void ReadArray(char *dest,size_t size,UINT32 length);
	void ReadArray(char *dest,size_t size,UINT32 length,bool swap);

	void WriteArray(const char *dest,size_t size,UINT32 length);
	void WriteArray(const char *dest,size_t size,UINT32 length,bool swap);

	bool DoRead(char *dest,size_t size,bool swap);
	/// Implementation of the actual write to the chain.
	bool DoWrite(const char* src, size_t size, bool swap);

	void SwapCpy(char *to,const char *from,size_t n);
	void SmemCpy(char *to,const char *from,size_t n);

	bool ReadCharArray(char *dest,UINT32 length);
	// This method doesn't respect alignment.
	// Any of the other public methods (which know the type) are preferred.
	void BufferRead(char *dest,size_t size,bool swap);
	/// Write to the chain from a source buffer.
	void BufferWrite(const char* src, size_t size, bool swap);
	
protected:
	typedef TSSerializerPrivate PrivateType;
	TSSerializer(PrivateType * p);
	TSSerializerPrivate * _Private_Ptr;

public:
	static const size_t MAX_ALIGN = 8;
	static const size_t WCHAR_SIZE = 2;
	static const char ALIGN_PAD[MAX_ALIGN];

#if defined IS_LITTLE_ENDIAN
	static const bool SWAP_BE = true;
#else
	static const bool SWAP_BE = false;
#endif
};
CLASS_PTR_DECLARE(TSSerializer);

#endif // __TSRDGSERIALIZER__H__


