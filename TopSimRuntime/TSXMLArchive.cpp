#include "stdafx.h"
#include "TSWaitSet.h"

#include "TSDomain.h"

bool TSWaitSet::WaitForDiscoveryComplete( TSDomainPtr Domain,const TSTopicQuerySet & QuerySet, 
	const TSFrontAppUniquelyId & AppId /*= TSFrontAppUniquelyId()*/,UINT32 Timeout /*= 3000*/ ,WaitMode Mode)
{
	return Domain->WaitForDiscoveryComplete(QuerySet,AppId,Timeout,Mode);
}
