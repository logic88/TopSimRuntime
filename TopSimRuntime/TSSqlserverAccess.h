#ifndef __TSSegment_H__
#define __TSSegment_H__

#ifndef  _WRS_KERNEL

#include "TSTopicTypes.h"

#include <boost/interprocess/managed_shared_memory.hpp>

class TOPSIMDATAINTERFACE_DECL TSSegment : public TSSingleton<TSSegment>
{
	SINGLETON_DECLARE(TSSegment);
public:
	enum CreateTag
	{
		CreateOnly,
		OpenOnly,
		OpenReadOnly,
		OpenOrCreate,
		OpenCopyOnWrite
	};

	void Init(CreateTag Tag = CreateOnly,size_t MaxSize = 1024*1024*1024,const TSString& MemoryName = TS_TEXT("TSSegment"));
	void Cleanup();
	boost::interprocess::managed_shared_memory * GetSegmentMemory();

private:
	TSSegment();
	boost::interprocess::managed_shared_memory*            _Segment;
	TSString                                               _MemoryName;
	bool                                                   _IsNewCreate;
};

#define GetSegment() TSSegment::Instance()->GetSegmentMemory()

#endif //#ifndef  _WRS_KERNEL
#endif



