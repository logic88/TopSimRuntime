#ifndef __TSXMLSERIALIZER_H__
#define __TSXMLSERIALIZER_H__	

#ifndef  _WRS_KERNEL

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimUtil/pugixml.hpp>
#else
#include <TopSimRuntime/pugixml.hpp>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TSISerializer.h"
#include "TopSimDataInterface.h"

struct TSXMLSerializerPrivate;
class TOPSIMDATAINTERFACE_DECL TSXMLSerializer : public TSISerializer
{
public:
	TSXMLSerializer(pugi::xml_node& Root,TSISerializerContext * Context = 0);
	virtual ~TSXMLSerializer(void);

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
	virtual TSISerializer& operator<<(const char * from);
	virtual TSISerializer& operator<<(const std::string &from);
	virtual TSISerializer& operator<<(const std::vector<std::string> &from);
	virtual TSISerializer& operator<<(const wchar_t * from);
	virtual TSISerializer& operator<<(const std::wstring &from);
	virtual TSISerializer& operator<<(const std::vector<std::wstring> &from);
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
	virtual TSISerializer& operator>>(char *dest);
	virtual TSISerializer& operator>>(std::string &dest);
	virtual TSISerializer& operator>>(std::vector<std::string> &dest);
	virtual TSISerializer& operator>>(wchar_t *dest);
	virtual TSISerializer& operator>>(std::wstring &dest);
	virtual TSISerializer& operator>>(std::vector<std::wstring> &dest);
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

public:
    virtual bool WriteBytes(const char* src, size_t size);
    virtual bool ReadBytes(char *dest,size_t size);
	virtual void PushComplexType(const TSString& TypeName,const void * Data,TSISerializer::MarshalType MType);
	virtual void PopComplexType();
	virtual UINT64 TellW();
	virtual UINT64 TellR();
	virtual bool MarshalKey(const TSDataKeyCompare& Key);
	virtual bool MarshalComplete();
	virtual void BeginArray(UINT32& Size,MarshalType MType);
	virtual void EndArray();
	virtual void BeginMap(UINT32& Size,MarshalType MType);
	virtual void EndMap();

private:
	TSXMLSerializerPrivate * _p;
};

#endif //#ifndef  _WRS_KERNEL
#endif
