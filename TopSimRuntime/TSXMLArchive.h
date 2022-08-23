#ifndef __TSWAITSET_H__
#define __TSWAITSET_H__

#include "TopSimDataInterface.h"
#include "TSTopicTypes.h"

class TOPSIMDATAINTERFACE_DECL TSWaitSet
{
public:
	enum WaitMode
	{
		kComplete = 0,
		kPublisher,
		kSubscriber
	};
public:
	static bool WaitForDiscoveryComplete(TSDomainPtr Domain,const TSTopicQuerySet & QuerySet,
		const TSFrontAppUniquelyId & AppId = TSFrontAppUniquelyId(),UINT32 Timeout = 3000,WaitMode Mode = kComplete);
};

#endif