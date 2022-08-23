#ifndef __TSIMONITOR_H__
#define __TSIMONITOR_H__

#include "TopSimDataInterface.h"
#include "TSTopicTypes.h"



struct TSIMonitorPrivate;
class TOPSIMDATAINTERFACE_DECL TSIMonitor
{
	TS_MetaType(TSIMonitor);
	TS_DECLARE_PRIVATE(TSIMonitor);

public:
	/* �����������ö�� */
	enum OBJTYPE
	{
		DEFAULT = 0,
		PUBLISH = 1,
		SUBSCRIBE = 2,
	};

public:
	TSIMonitor(void);
	virtual ~TSIMonitor(void);

protected:
	PrivateType * _Private_Ptr;
	TSIMonitor(PrivateType * p);

public:
	virtual void Initialize() {}
	virtual void Clean() {}
	virtual void OnCreateDomain(TSDomainPtr Domain) = 0;
	virtual void OnDestroyDomain(TSDomainPtr Domain) = 0;
	virtual void OnPublishTopic(const TSDomainId& DomainId, TSTOPICHANDLE THandle) = 0;
	virtual void OnSubscribeTopic(const TSDomainId& DomainId, TSTOPICHANDLE THandle) = 0;
	virtual void OnUnsubscribeTopic(const TSDomainId& DomainId, TSTOPICHANDLE THandle) = 0;
	virtual void OnUnpublishTopic(const TSDomainId& DomainId, TSTOPICHANDLE THandle) = 0;
	virtual void OnUpdateTopic(const TSDomainId& DomainId, TSITopicContextPtr Ctx) = 0;
	virtual void OnReceivedTopic(const TSDomainId& DomainId, TSITopicContextPtr Ctx) = 0;

	/* �жϷ�������ǰ�Ƿ������ѭ������ */
	virtual bool IsUpdateTopicCycle(TSTOPICHANDLE THandle) { return false; };

	/* ��ȡ�ö����ǽ��ն˶����Ƿ��Ͷ˶��� */
	virtual UINT32 GetObjType() { return DEFAULT; };

	/* ����������Ӧ����Ϣ */
	virtual void UpdateHostAppInfo() { return; };

	/* �ж��Ƿ����ü�ع��� */
	virtual bool IsEnable() { return true; }
};

CLASS_PTR_DECLARE(TSIMonitor);

#endif



