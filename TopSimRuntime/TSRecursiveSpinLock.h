#ifndef __TSITRANSFERLINKER_H__
#define __TSITRANSFERLINKER_H__

#include "TSIDataDelegate.h"
#include "TSIPSMatcher.h"
#include "TSDomain.h"

struct TSITransferLinkerPrivate;
struct ParticipantQosPolicy;
struct DataReaderQosPolicy;
struct DataWriterQosPolicy;

class TOPSIMDATAINTERFACE_DECL TSITransferLinker
{
	TS_MetaType(TSITransferLinker);

public:
	virtual ~TSITransferLinker();

public:
	typedef TSITransferLinkerPrivate PrivateType;
	PrivateType * _Private_Ptr;
	TSITransferLinker(PrivateType * p);

public:
	virtual bool Initialize(int argc,char ** argv,TSIDataDelegate * DataDelegate) = 0;
	virtual void Clean() = 0;

public:
	virtual bool Subscribe(const TSDomainId& DomainId,TSTOPICHANDLE THandle,bool LocalOnly = false) = 0;
	virtual bool Publish(const TSDomainId& DomainId,TSTOPICHANDLE THandle) = 0;
	virtual void Unsubscribe(const TSDomainId& DomainId,TSTOPICHANDLE THandle) = 0;
	virtual void Unpublish(const TSDomainId& DomainId,TSTOPICHANDLE THandle) = 0;

	virtual TSTopicContextPtr UpdateTopic(const TSDomainId& DomainId,
		TSTOPICHANDLE Handle,
		const void * Data,const TSTime & Time,bool IsNeedSerializer) = 0;

	virtual TSTopicContextPtr SendData(const TSDomainId& DomainId,
        TSTOPICHANDLE THandle,
        const void * Data,const TSTime & Time,const TSFrontAppUniquelyId& Receiver) = 0;

    virtual TSTopicContextPtr SendBinary(const TSDomainId& DomainId,
        TSTOPICHANDLE THandle,
        TSDataContextPtr DataCtx,const TSTime & Time,const TSFrontAppUniquelyId& Receiver) = 0;

	virtual bool DeleteTopic(const TSDomainId & DomainId,TSTopicContextPtr Context,bool IsNotice) = 0;
	virtual bool DeleteTopic(const TSDomainId & DomainId,TSTOPICHANDLE TopicHandle,void * KeyData,bool IsNotice) = 0;

    virtual TSDomainPtr CreateDomain( const TSDomainId& DomainId ,const TSFrontAppUniquelyId& FrontAppUniquelyId) = 0;
	virtual void DestroyDomain(const TSDomainId& Domain) = 0;
    virtual void CleanDomainData(const TSDomainId& Domain) = 0;
	virtual TSDomainPtr GetDomain(const TSDomainId& DomainId) = 0;

	virtual TSTopicFindSetType CreateTopicFindSet(TSHANDLE Handle,const TSTopicQuerySet & Query,const TSDomainId & DomainId) = 0;
	virtual TSTopicFindSetType CreateTopicFindSet(const TSTopicQuerySet & Query,const TSDomainId & DomainId) = 0;
	virtual TSTopicFindSetType CreateTopicFindSet(DOUBLE CenterLon,DOUBLE CenterLat,FLOAT SpacialRange,const TSTopicQuerySet & Query,const TSDomainId & DomainId) = 0;

	virtual TSTopicContextPtr GetFirstTopicByHandle(const TSDomainId & DomainId,TSTOPICHANDLE THandle) = 0;
	virtual TSTopicContextPtr GetFirstTopicByHandle(const TSDomainId & DomainId,TSHANDLE Handle,TSTOPICHANDLE THandle) = 0;
	virtual TSTopicContextPtr GetFirstObject(TSTopicFindSetType FindSet) = 0;
	virtual TSTopicContextPtr GetNextObject(TSTopicFindSetType FindSet) = 0;

	virtual bool WaitForDiscoveryComplete(const TSDomainId & DomainId, const TSTopicQuerySet & QuerySet,const TSFrontAppUniquelyId & AppId,UINT32 Timeout,TSWaitSet::WaitMode Mode) = 0;
	virtual void OutputLogger(UINT32 Catalog,const TSString & LoggerMsg) = 0;

	/* 根据用户Qos参数创建域 */
	virtual TSDomainPtr CreateDomainWithQos(const TSDomainId& domainId, const ParticipantQosPolicy & participantQosPolicy, const TSFrontAppUniquelyId& FrontAppUniquelyId) = 0;
	/* 获取参与者Qos */
	virtual bool GetParticipantQosPolicy(const TSDomainId& domainId, ParticipantQosPolicy & participantQosPolicy) = 0;
	/* 设置参与者Qos */
	virtual bool SetParticipantQosPolicy(const TSDomainId& domainId, const ParticipantQosPolicy & participantQosPolicy) = 0;
	/* 获取数据读者Qos */
	virtual bool GetDataReaderQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy) = 0;
	/* 设置数据读者Qos */
	virtual bool SetDataReaderQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy) = 0;
	/* 获取数据写者Qos */
	virtual bool GetDataWriterQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy) = 0;
	/* 设置数据写者Qos */
	virtual bool SetDataWriterQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, const DataWriterQosPolicy & dataWriterQosPolicy) = 0;

	virtual UINT8 GetJoinedDomains( TSStringArray& domainIDVec)=0;
};

CLASS_PTR_DECLARE(TSITransferLinker);

#endif



