#ifndef __TSDISCOVERY_H__
#define __TSDISCOVERY_H__

#include "TopSimDataInterface.h"
#include "TSTopicTypes.h"

#include "TSAppStatus/Defined.h"

struct TSDiscoveryPrivate;
class TOPSIMDATAINTERFACE_DECL TSDiscovery : public boost::enable_shared_from_this<TSDiscovery>
{
public:
	enum Status
	{
		AppOnline,
		AppOffline,
		AppUpdateStatus,
	};

	typedef boost::function<void(Status Sts,const TSAppStatus::DataType& Status)> TSDiscoveryCallback_T;

public:
	TSDiscovery(TSDomainPtr Domain);
	~TSDiscovery(void);

	void Stop();
	bool RegisterApp(const TSString& AppName,TSDiscoveryCallback_T Callback = TSDiscoveryCallback_T(),TSByteBufferPtr AppBuffer = TSByteBufferPtr());
	bool UpdateApp(const TSString& AppName,TSByteBufferPtr AppBuffer);
	void UnregisterApp(const TSString& AppName);

private:
	void StatusOnlineOp(const TSFrontAppUniquelyId& Id);
	void StatusOfflineOp(const TSFrontAppUniquelyId& Id);
	void StatusPushOp(const TSAppStatus::DataType& Status);
	void ProcessOnLineTimer(const boost::system::error_code& ec);
	void OnAppStatusUpdate(TSTopicContextPtr Ctx);
	void ProcessAppStatus_p(const TSAppStatus::DataType& Status);
	void EventDispatcherThread_();

	TSDiscoveryPrivate * _p;
};

#endif



