#ifndef __TSSHAREDMEMORYMIDDLEWARE_H__
#define __TSSHAREDMEMORYMIDDLEWARE_H__

#ifndef  _WRS_KERNEL

#include "TSITransferMiddleware.h"

struct TSSharedMemoryMiddlewarePrivate;
class TOPSIMDATAINTERFACE_DECL TSSharedMemoryMiddleware : public TSITransferMiddleware
{
	TS_MetaType(TSSharedMemoryMiddleware,TSITransferMiddleware);
	TS_DECLARE_PRIVATE(TSSharedMemoryMiddleware);
public:
	TSSharedMemoryMiddleware();

protected:
	TSSharedMemoryMiddleware(PrivateType * p);

public:
	virtual bool Initialize(int argc,char ** argv);
	virtual void Clean();

public:
	virtual void MakeAlive(const TSFrontAppUniquelyId& FrontAppId);
	virtual void MakeDead(const TSFrontAppUniquelyId& FrontAppId);
	virtual bool Subscribe(const TSDomainId& DomainId,TSTOPICHANDLE THandle,bool LocalOnly = false);
	virtual bool Publish(const TSDomainId& DomainId,TSTOPICHANDLE THandle);
	virtual void Unsubscribe(const TSDomainId& DomainId,TSTOPICHANDLE THandle);
	virtual void Unpublish(const TSDomainId& DomainId,TSTOPICHANDLE THandle);

	virtual bool UpdateTopic(const TSDomainId& DomainId,
		TSTOPICHANDLE THandle,
		TSDataContextPtr Ctx);
	virtual bool SendData(const TSDomainId& DomainId,
		TSTOPICHANDLE THandle,
		TSDataContextPtr Ctx,const TSFrontAppUniquelyId& Receiver);

	virtual bool DeleteTopic(const TSDomainId & DomainId,TSTOPICHANDLE THandle,TSDataContextPtr Ctx,bool IsNotice);

	virtual bool CreateDomain( const TSDomainId& DomainId ,const TSFrontAppUniquelyId& FrontAppUniquelyId);
	virtual bool WaitForDiscoveryComplete(const TSDomainId & DomainId, const TSTopicQuerySet & QuerySet,const TSFrontAppUniquelyId & AppId,UINT32 Timeout,TSWaitSet::WaitMode Mode);
	virtual void DestroyDomain(const TSDomainId& DomainId);
	virtual void OutputLogger(UINT32 Catalog,const TSString & LoggerMsg);

	/* 根据用户Qos参数创建域 */
	virtual bool CreateDomainWithQos(const TSDomainId & domainId, const ParticipantQosPolicy & participantQosPolicy, const TSFrontAppUniquelyId& FrontAppUniquelyId);

	/* 获取参与者Qos */
	virtual bool GetParticipantQosPolicy(const TSDomainId & domainId, ParticipantQosPolicy & participantQosPolicy);
	/* 设置参与者Qos */
	virtual bool SetParticipantQosPolicy(const TSDomainId & domainId, const ParticipantQosPolicy & participantQosPolicy);

	/* 获取数据读者Qos */
	virtual bool GetDataReaderQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy);
	/* 设置数据读者Qos */
	virtual bool SetDataReaderQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy);

	/* 获取数据写者Qos */
	virtual bool GetDataWriterQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy);
	/* 设置数据写者Qos */
	virtual bool SetDataWriterQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, const DataWriterQosPolicy & dataWriterQosPolicy);
private:
	void Heartbeat(const boost::system::error_code & ec);
	void Listen();
	void EventDispatcherThread_();
};

#endif

#endif // _WRS_KERNEL
