#ifndef __TSTRANSFERPROXY_H__
#define __TSTRANSFERPROXY_H__

#include <TopSimDataInterface/TSITransferLinker.h>

#include "TopSimDataInterface.h"

struct TSTransferProxyPrivate;
class TSTransferProxy : public TSITransferLinker
{
	TS_MetaType(TSTransferProxy,TSITransferLinker);
	TS_DECLARE_PRIVATE(TSTransferProxy);
public:
	TSTransferProxy(void);
	~TSTransferProxy(void);

protected:
	TSTransferProxy(PrivateType * p);

public:
	virtual bool Initialize(int argc,char ** argv,TSIDataDelegate * DataDelegate);

	virtual void Clean();

public:
	virtual void MakeAlive(const TSFrontAppUniquelyId& FrontAppId);
	virtual void MakeDead(const TSFrontAppUniquelyId& FrontAppId); 
	virtual bool Subscribe(const TSDomainId& DomainId,TSTOPICHANDLE THandle,bool LocalOnly = false);
	virtual bool Publish(const TSDomainId& DomainId,TSTOPICHANDLE THandle);
	virtual void Unsubscribe(const TSDomainId& DomainId,TSTOPICHANDLE THandle);
	virtual void Unpublish(const TSDomainId& DomainId,TSTOPICHANDLE THandle);

	virtual TSTopicContextPtr UpdateTopic(const TSDomainId& DomainId,
		TSTOPICHANDLE Handle,
		const void * Data,const TSTime & Time, bool IsNeedSerializer) ;

	virtual TSTopicContextPtr SendData(const TSDomainId& DomainId,
        TSTOPICHANDLE THandle,
        const void * Data,const TSTime & Time,const TSFrontAppUniquelyId& Receiver);

    virtual TSTopicContextPtr SendBinary(const TSDomainId& DomainId,
        TSTOPICHANDLE THandle,
        TSDataContextPtr DataCtx,const TSTime & Time,const TSFrontAppUniquelyId& Receiver);

	virtual bool DeleteTopic(const TSDomainId & DomainId,TSTopicContextPtr Context,bool IsNotice);
	virtual bool DeleteTopic(const TSDomainId & DomainId,TSTOPICHANDLE TopicHandle,void * KeyData,bool IsNotice);

	virtual TSDomainPtr CreateDomain( const TSDomainId& DomainId  ,const TSFrontAppUniquelyId& FrontAppUniquelyId);
	virtual void DestroyDomain(const TSDomainId& DomainId);
    virtual void CleanDomainData(const TSDomainId& Domain);
	virtual TSDomainPtr GetDomain(const TSDomainId& DomainId);

	TSTopicFindSetType CreateTopicFindSet(TSHANDLE Handle,const TSTopicQuerySet & Query,const TSDomainId & DomainId);
	TSTopicFindSetType CreateTopicFindSet(const TSTopicQuerySet & Query,const TSDomainId & DomainId);
	TSTopicFindSetType CreateTopicFindSet(DOUBLE CenterLon,DOUBLE CenterLat,FLOAT SpacialRange,const TSTopicQuerySet & Query,const TSDomainId & DomainId);

	virtual TSTopicContextPtr GetFirstTopicByHandle(const TSDomainId & DomainId,TSTOPICHANDLE THandle);
	virtual TSTopicContextPtr GetFirstTopicByHandle(const TSDomainId & DomainId,TSHANDLE Handle,TSTOPICHANDLE THandle);
	TSTopicContextPtr GetFirstObject(TSTopicFindSetType FindSet);
	TSTopicContextPtr GetNextObject(TSTopicFindSetType FindSet);
	bool WaitForDiscoveryComplete(const TSDomainId & DomainId, const TSTopicQuerySet & QuerySet,const TSFrontAppUniquelyId & AppId,UINT32 Timeout,TSWaitSet::WaitMode Mode);
	virtual void OutputLogger(UINT32 Catalog,const TSString & LoggerMsg);

	/* �����û�Qos���������� */
	virtual TSDomainPtr CreateDomainWithQos(const TSDomainId& domainId, const ParticipantQosPolicy & participantQosPolicy, const TSFrontAppUniquelyId& FrontAppUniquelyId);
	/* ��ȡ������Qos */
	virtual bool GetParticipantQosPolicy(const TSDomainId& domainId, ParticipantQosPolicy & participantQosPolicy);
	/* ���ò�����Qos */
	virtual bool SetParticipantQosPolicy(const TSDomainId& domainId, const ParticipantQosPolicy & participantQosPolicy);
	/* ��ȡ���ݶ���Qos */
	virtual bool GetDataReaderQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy);
	/* �������ݶ���Qos */
	virtual bool SetDataReaderQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy);
	/* ��ȡ����д��Qos */
	virtual bool GetDataWriterQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy);
	/* ��������д��Qos */
	virtual bool SetDataWriterQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, const DataWriterQosPolicy & dataWriterQosPolicy);
	/*��ȡ�����������*/
	virtual UINT8 GetJoinedDomains(TSStringArray& domainIDVec);

private:
	bool DeleteTopic_p(const TSDomainId & DomainId,TSTOPICHANDLE THandle,TSDataContextPtr Ctx,bool IsNotice);
	TSTopicContextPtr UpdateLocal_p(const TSDomainId& DomainId,
		TSTOPICHANDLE Handle,
		const void * Data,const TSTime & Time,const TSFrontAppUniquelyId& Receiver,bool IsNeedSerializer);
    TSTopicContextPtr SendBinary_p(const TSDomainId& DomainId,
        TSTOPICHANDLE Handle,
        TSDataContextPtr DataContext,const TSTime & Time,
        const TSFrontAppUniquelyId& Receiver);
	TSDomainPtr CreateDomain_p(const TSDomainId& DomainId);
	bool Configure(int argc,char ** argv);
	void OnTopicAvaliable(const TSDomainId& DomainId,TSTOPICHANDLE Handle,TSDataContextPtr Ctx);
	void OnDataRemove(const TSDomainId & DomainId,TSTOPICHANDLE Handle,TSDataContextPtr Ctx);
	void OnTopicMatched(const TSDomainId& DomainId, TSTOPICHANDLE THandle, UINT32 AppId, bool IsPub);
	void OnLeaveDomain(const TSDomainId& DomainId, TSTOPICHANDLE THandle, UINT32 AppId, bool IsPub);
	bool OnContentFilterCallback(const TSDomainId & DomainId, TSTOPICHANDLE TopicId, TSDataContextPtr Ctx, UINT32 AppId);
};


#endif


