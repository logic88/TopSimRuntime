#ifndef __TSLOCALDATAMANAGER_H__
#define __TSLOCALDATAMANAGER_H__

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimTypes/TSRWSpinLock.h>
#else
#include <TopSimRuntime/TSRWSpinLock.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimTypes/TSKeyPair.h>
#else
#include <TopSimRuntime/TSKeyPair.h>
#endif


#include "TopSimDataInterface.h"
#include "TSTopicTypes.h"

struct TSLocalDataManagerPrivate;

class TOPSIMDATAINTERFACE_DECL TSLocalDataManager
{
    TS_DECLARE_PRIVATE(TSLocalDataManager);
public:
	TSLocalDataManager();
	~TSLocalDataManager(void);

protected:
	PrivateType * _Private_Ptr;
	TSLocalDataManager(PrivateType * p);

public:
	bool Init();
	void Clean();
    void CleanDomainData(const TSDomainId& Domianid );
public:
	TSTopicContextPtr UpdateTopic(const TSDomainId& DomainId,
		TSTOPICHANDLE THandle,
		const void * Data,const TSTime & Time,bool IsFromNetwork, bool IsNeedSerializer);
	TSTopicContextPtr UpdateTopicBinary(const TSDomainId& DomainId,
		TSTOPICHANDLE THandle,
		TSDataContextPtr Ctx,const TSTime & Time,bool IsFromNetwork,TSInterObjectPtr CopyData = TSInterObjectPtr());

	TSDataContextPtr MakeDataContext(TSTOPICHANDLE Handle,const void * Data,const TSTime & Time, bool IsNeedSerializer);
	TSTopicContextPtr FindContextByHash(const TSDomainId & DomainId,TSTOPICHANDLE Handle,TSDataContextPtr Ctx );
	void DeleteTopic(const TSDomainId & DomainId,TSTOPICHANDLE Handle,TSDataContextPtr Ctx );

	TSTopicFindSetType CreateTopicFindSet(TSHANDLE Handle, const TSTopicQuerySet & Query, const TSDomainId & DomainId);
	TSTopicFindSetType CreateTopicFindSet(const TSTopicQuerySet & Query, const TSDomainId & DomainId);
	TSTopicFindSetType CreateTopicFindSet(DOUBLE CenterLon, DOUBLE CenterLat, FLOAT SpacialRange, const TSTopicQuerySet & Query, const TSDomainId & DomainId);

	TSTopicContextPtr GetFirstTopicByHandle(const TSDomainId & DomainId, TSTOPICHANDLE THandle);
	TSTopicContextPtr GetFirstTopicByHandle(const TSDomainId & DomainId, TSHANDLE Handle, TSTOPICHANDLE THandle);
	TSTopicContextPtr GetFirstObject(TSTopicFindSetType FindSet);
	TSTopicContextPtr GetNextObject(TSTopicFindSetType FindSet);

private:
	bool UpdateTopicContext(const TSDomainId& DomainId,
		TSTOPICHANDLE THandle,
		TSTopicContextPtr Ctx,bool IsFromNetwork);
};

CLASS_PTR_DECLARE(TSLocalDataManager);

#endif



