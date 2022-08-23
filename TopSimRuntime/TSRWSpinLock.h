#ifndef __TSITRANSFERMIDDLEWARE_H__
#define __TSITRANSFERMIDDLEWARE_H__

#include "TopSimDataInterface.h"
#include "TSTopicTypes.h"
#include "TSWaitSet.h"
#include "TSQosStruct.h"

#define CONFIG_PRIX "Config/Middleware"
#define TRANSPORT_RFM "RFM"
#define TRANSPORT_TCP "TCP"
#define TRANSPORT_TCP_USE_UDP_DISC "TCP_USE_UDP_DISC"
#define TRANSPORT_UDP "UDP"

typedef boost::function<void(const TSDomainId& DomainId,TSTOPICHANDLE THandle,TSDataContextPtr Ctx)> DataAvaliableCallback;
typedef boost::function<void(const TSDomainId& DomainId, TSTOPICHANDLE THandle, TSDataContextPtr Ctx)> DataRemoveCallback;
typedef boost::function<void(const TSDomainId& DomainId, TSTOPICHANDLE THandle, UINT32 AppId, bool IsPub)> DataTopicMatchedCallback;
typedef boost::function<bool(const TSDomainId & DomainId, TSTOPICHANDLE TopicId, TSDataContextPtr Ctx, UINT32 AppId)> DataContentFilterCallback;


struct TSITransferMiddlewarePrivate;
class TOPSIMDATAINTERFACE_DECL TSITransferMiddleware
{
	TS_MetaType(TSITransferMiddleware);
public:
	~TSITransferMiddleware(void);

protected:
	typedef TSITransferMiddlewarePrivate PrivateType;
	PrivateType * _Private_Ptr;
	TSITransferMiddleware(PrivateType * p);

public:
	virtual bool Initialize(int argc,char ** argv) = 0;
	virtual void Clean() = 0;

public:
	virtual void MakeAlive(const TSFrontAppUniquelyId& FrontAppId) = 0;
	virtual void MakeDead(const TSFrontAppUniquelyId& FrontAppId) = 0;
	virtual bool Subscribe(const TSDomainId& DomainId,TSTOPICHANDLE THandle,bool LocalOnly = false) = 0;
    virtual bool Publish(const TSDomainId& DomainId,TSTOPICHANDLE THandle) = 0;
    virtual void Unsubscribe(const TSDomainId& DomainId,TSTOPICHANDLE THandle) = 0;
    virtual void Unpublish(const TSDomainId& DomainId,TSTOPICHANDLE THandle) = 0;

	virtual bool UpdateTopic(const TSDomainId& DomainId,
		TSTOPICHANDLE THandle,
		TSDataContextPtr Ctx) = 0;
    virtual bool SendData(const TSDomainId& DomainId,
        TSTOPICHANDLE THandle,
        TSDataContextPtr Ctx,const TSFrontAppUniquelyId& Receiver) = 0;
	virtual bool DeleteTopic(const TSDomainId & DomainId,TSTOPICHANDLE THandle,TSDataContextPtr Ctx,bool IsNotice){return false;}
    virtual bool CreateDomain( const TSDomainId& DomainId ,const TSFrontAppUniquelyId& FrontAppUniquelyId) = 0;
	virtual void DestroyDomain(const TSDomainId& DomainId) = 0;
	virtual bool WaitForDiscoveryComplete(const TSDomainId & DomainId, const TSTopicQuerySet & QuerySet,const TSFrontAppUniquelyId & AppId,UINT32 Timeout,TSWaitSet::WaitMode Mode) = 0;
	virtual void OutputLogger(UINT32 Catalog,const TSString & LoggerMsg) = 0;

	/* 根据用户Qos参数创建域 */
	virtual bool CreateDomainWithQos(const TSDomainId & domainId, const ParticipantQosPolicy & participantQosPolicy, const TSFrontAppUniquelyId& FrontAppUniquelyId) = 0;

	/* 获取参与者Qos */
	virtual bool GetParticipantQosPolicy(const TSDomainId & domainId, ParticipantQosPolicy & participantQosPolicy) = 0;
	/* 设置参与者Qos */
	virtual bool SetParticipantQosPolicy(const TSDomainId & domainId, const ParticipantQosPolicy & participantQosPolicy) = 0;

	/* 获取数据读者Qos */
	virtual bool GetDataReaderQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy) = 0;
	/* 设置数据读者Qos */
	virtual bool SetDataReaderQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy) = 0;

	/* 获取数据写者Qos */
	virtual bool GetDataWriterQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy) = 0;
	/* 设置数据写者Qos */
	virtual bool SetDataWriterQos(const TSDomainId & domainId, TSTOPICHANDLE tHandle, const DataWriterQosPolicy & dataWriterQosPolicy) = 0;

public:
	DataAvaliableCallback       TopicCallback;
	DataRemoveCallback          RemoveCallback;
	DataTopicMatchedCallback    TopicMatchedCallback;
	DataTopicMatchedCallback    LeaveDomainCallback;
	DataContentFilterCallback   ContentFilterCallback;
};


CLASS_PTR_DECLARE(TSITransferMiddleware); 

#endif



