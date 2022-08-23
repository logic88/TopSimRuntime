#ifndef __TSRDGTYPES__H__
#define __TSRDGTYPES__H__

#ifndef _WRS_KERNEL

#include "TopSimDataInterface.h"
#include "TSTopicTypes.h"
#include "TSSegment.h"

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/smart_ptr/shared_ptr.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include "TSRDGRobustMutex.hpp"

typedef boost::interprocess::message_queue MessageQueue_T;

struct TOPSIMDATAINTERFACE_DECL TSTopicDescMqItem
{
	TSTopicDescMqItem()
	{
		memset(DomainPartNames,0,256);
		LocalOnly = false;
	}

	TSTOPICHANDLE                Topic;
	UINT32                       DomainId;
	bool                         LocalOnly;
	char                         DomainPartNames[256];
};

struct TOPSIMDATAINTERFACE_DECL TSSharedMemoryDataContext
{
	TSSharedMemoryDataContext();
	~TSSharedMemoryDataContext();
	boost::interprocess::offset_ptr<void> Data;
	UINT32                                Length;
	UINT32                                KeyHash;
};

struct TOPSIMDATAINTERFACE_DECL TSTopicDataMqItem : public TSTopicDescMqItem
{
	TSFrontAppUniquelyId         Receiver;
	bool                         IsNotice;
	TSSharedMemoryDataContext    Ctx;
};

typedef boost::interprocess::shared_ptr<TSTopicDataMqItem,
	boost::interprocess::allocator<void,boost::interprocess::managed_shared_memory::segment_manager>,
	boost::interprocess::deleter<TSTopicDataMqItem,boost::interprocess::managed_shared_memory::segment_manager> > TSTopicDataMqItemSharedPtr; 

struct TOPSIMDATAINTERFACE_DECL TSMessageQueueSendItem
{
	enum OpFlag
	{
		kInit,
		kData,
		kClean,
		kHeartbeat
	};

	enum SubOpType
	{
		kNone,
		kPublish,
		kSublish,
		kUnpublish,
		kUnsublish,
		kUpdateTopic,
		kUpdateTopicDirect,
		kDeleteTopic,
		kCreateDomain,
		kDestoryDomain
	};

	TSMessageQueueSendItem()
	{
		LocalOnly = false;
	}

	OpFlag                 Flag;
	SubOpType              SubFlag;
	TSFrontAppUniquelyId	 Source;
	UINT8                  ExchangedDataIndex;
	bool                   LocalOnly;
};

struct TSSharedMemoryExchangedItem
{
	TSRDGRobustMutex                      _Mutex;
	TSTopicDataMqItemSharedPtr            _ExchangedData1;
	TSTopicDataMqItemSharedPtr            _ExchangedData2;
	TSTopicDataMqItemSharedPtr            _ExchangedData3;
};

TSTopicDataMqItemSharedPtr TOPSIMDATAINTERFACE_DECL MakeTopicDataShared();

TSDataContextPtr TOPSIMDATAINTERFACE_DECL MakeDataContextFromSharedMemory(const TSSharedMemoryDataContext & SharedMemoryCtx);
void TOPSIMDATAINTERFACE_DECL MakeSharedMemoryDataContext(TSSharedMemoryDataContext & SharedMemoryCtx,TSDataContextPtr Ctx);
void TOPSIMDATAINTERFACE_DECL FreeSharedMemoryDataContext(TSSharedMemoryDataContext & SharedMemoryCtx);

#endif  //_WRS_KERNEL

#endif // __TSRDGTYPES__H__

