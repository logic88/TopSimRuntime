#ifndef TSDataCollection_h__
#define TSDataCollection_h__

#ifndef  _WRS_KERNEL

#include "TopSimDataInterface.h"
#include "TSDataInteractManager.h"
#include "TSDataRecorderHelper.h"

struct TSDataCollectionPrivate;
class TOPSIMDATAINTERFACE_DECL TSDataCollection
{
    TS_MetaType(TSDataCollection);
    TS_DECLARE_PRIVATE(TSDataCollection);
public:
    TSDataCollection();
    ~TSDataCollection();

	virtual void Init();
	virtual void SetTopicName(const TSString & TopicName);
	virtual void SetOutputPath(const TSString & Path);
    virtual void Write(const TSTopicContextPtr Ctx);
    virtual void Write(TSTOPICHANDLE TopicId,const TSInterObject * Object,const TSTime & Timestamp );
    virtual void Close();
private:
    PrivateType * _Private_Ptr;
};

#endif // _WRS_KERNEL
#endif // TSDataCollection_h__
