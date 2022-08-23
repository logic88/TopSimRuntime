#ifndef __TSSMBACKENDINTERACT_H__
#define __TSSMBACKENDINTERACT_H__

#ifndef  _WRS_KERNEL

#include "TopSimDataInterface.h"
#include <TopSimDataInterface/TSIDataRecorder.h>

typedef boost::shared_ptr<class TSFrontAppDataInteract> TSFrontAppDataInteractPtr;

struct TSSMBackendInteractPrivate;
class TOPSIMDATAINTERFACE_DECL TSSMBackendInteract
{
    TS_DECLARE_PRIVATE(TSSMBackendInteract);
public:
	TSSMBackendInteract(void);
	~TSSMBackendInteract(void);

protected:
	PrivateType * _Private_Ptr;
	TSSMBackendInteract(PrivateType * p);

public:
	bool Initialize(int argc,char ** argv);
	void Cleanup();

	void SetTimeHolder(TSAbstractTimeHolderPtr TimeHoder);
	TSAbstractTimeHolderPtr GetTimeHolder();
	void Listen();

private:
	void Heartbeat(const boost::system::error_code & ec);
	void OnDeleteTopic(TSFrontAppDataInteractPtr Interact,TSTopicDataMqItemSharedPtr Item);
	void OnUpdateTopicDirect(TSFrontAppDataInteractPtr Interact,TSTopicDataMqItemSharedPtr Item);
	void OnUpdateTopic(TSFrontAppDataInteractPtr Interact,TSTopicDataMqItemSharedPtr Item);
	void InsertToSharedMemory(TSTopicDataMqItemSharedPtr Item);
	void EraseFromSharedMemory(TSTopicDataMqItemSharedPtr Item);
	bool Configure(int argc,char ** argv);
	void OnTopicAvaliable(const TSDomainId& DomainId,TSTOPICHANDLE THandle,TSDataContextPtr Ctx);
	void OnTopicRemoved(const TSDomainId& DomainId,TSTOPICHANDLE THandle,TSDataContextPtr Ctx);
	const TSString & GetLocalIPAddress();
	void EventDispatcherThread_();
};

#endif //#ifndef  _WRS_KERNEL
#endif

