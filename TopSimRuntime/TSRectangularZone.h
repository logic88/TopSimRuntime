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
	/* 具体对象类型枚举 */
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

	/* 判断发送主题前是否会引起循环发送 */
	virtual bool IsUpdateTopicCycle(TSTOPICHANDLE THandle) { return false; };

	/* 获取该对象是接收端对象还是发送端对象 */
	virtual UINT32 GetObjType() { return DEFAULT; };

	/* 更新主机和应用信息 */
	virtual void UpdateHostAppInfo() { return; };

	/* 判断是否启用监控功能 */
	virtual bool IsEnable() { return true; }
};

CLASS_PTR_DECLARE(TSIMonitor);

#endif



