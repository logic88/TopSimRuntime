#ifndef __TSBINARYSOURCE__H__
#define __TSBINARYSOURCE__H__

#ifndef  _WRS_KERNEL

#include "TopSimDataInterface.h"
#include "TSTopicTypes.h"

struct TOPSIMDATAINTERFACE_DECL TSCollectionData
{
    boost::shared_ptr<TSTopicRecordStruct> Data;

    const TSTOPICHANDLE GetTopicHandle();
    const UINT64 GetTime();
    const void * GetData();
    const UINT32 GetLength();
	 
	TSTOPICHANDLE _TopicId;
};

typedef boost::shared_ptr<TSCollectionData> TSCollectionDataPtr;

struct TSBinarySourcePrivate;
class TOPSIMDATAINTERFACE_DECL TSBinarySource
{
    TS_MetaType(TSBinarySource);
    TS_DECLARE_PRIVATE(TSBinarySource);
public:
    TSBinarySource(void);
    ~TSBinarySource(void);
protected:
    PrivateType * _Private_Ptr;
    TSBinarySource(PrivateType * p);

public:
    virtual bool LoadTopicFile(const TSString& FilePath,
        const std::vector<TSString>& FileName,
        const std::vector<TSTOPICHANDLE>& ExcludeTopics = std::vector<TSTOPICHANDLE>());

	virtual void JumpTo(TSTime & ReachTime,std::list<TSCollectionDataPtr> & DataList);
	virtual const TSTime & GetNextValidDataTime(); 
    virtual TSCollectionDataPtr ReadNext(TSTime& ReachTime);

    const std::vector<TSTOPICHANDLE>& GetIncludeTopics();
    
    void Close();
};

typedef boost::shared_ptr<TSBinarySource> TSBinarySourcePtr;

#endif

#endif//_WRS_KERNEL
