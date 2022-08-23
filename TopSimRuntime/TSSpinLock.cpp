#include "stdafx.h"
#include "TSRDGTypes.h"

#include "TSTypeSupport.h"

#ifndef  _WRS_KERNEL
void TOPSIMDATAINTERFACE_DECL MakeSharedMemoryDataContext( TSSharedMemoryDataContext & SharedMemoryCtx,
	TSDataContextPtr Ctx )
{

#if defined(XSIM3_3) || defined(XSIM3_2)
	if(Ctx->GetBinary()->GetOffsetW() > 0)
	{
		SharedMemoryCtx.Data = GetSegment()->allocate(Ctx->GetBinary()->GetOffsetW());
		memcpy(SharedMemoryCtx.Data.get(),Ctx->GetBinary()->Data(),Ctx->GetBinary()->GetOffsetW());
	}

	SharedMemoryCtx.Length = Ctx->GetBinary()->GetOffsetW();
	SharedMemoryCtx.KeyHash = Ctx->KeyHash();
#elif defined(XSIM4_3)
	if(Ctx->GetBinary()->TellW() > 0)
	{
		SharedMemoryCtx.Data = GetSegment()->allocate(Ctx->GetBinary()->TellW());
		memcpy(SharedMemoryCtx.Data.get(),Ctx->GetBinary()->Data(),Ctx->GetBinary()->TellW());
	}

	SharedMemoryCtx.Length = Ctx->GetBinary()->TellW();
	SharedMemoryCtx.KeyHash = Ctx->KeyHash();
#else
	if(Ctx->GetBinary()->TellW() > 0)
	{
		SharedMemoryCtx.Data = GetSegment()->allocate(Ctx->GetBinary()->TellW());
		memcpy(SharedMemoryCtx.Data.get(),Ctx->GetBinary()->Begin(),Ctx->GetBinary()->TellW());
	}

	SharedMemoryCtx.Length = Ctx->GetBinary()->TellW();
	SharedMemoryCtx.KeyHash = Ctx->KeyHash();
#endif
}

void TOPSIMDATAINTERFACE_DECL FreeSharedMemoryDataContext( TSSharedMemoryDataContext & SharedMemoryCtx )
{
	if(SharedMemoryCtx.Data)
	{
		GetSegment()->deallocate(SharedMemoryCtx.Data.get());
	}
}

TSDataContextPtr MakeDataContextFromSharedMemory(const TSSharedMemoryDataContext & SharedMemoryCtx)
{
	TSDataContextPtr Ctx = new TSDataContext(MakeBufferPtr(SharedMemoryCtx.Data.get(),SharedMemoryCtx.Length));
	Ctx->SetKeyHash(SharedMemoryCtx.KeyHash);
	return Ctx;
}

TSTopicDataMqItemSharedPtr TOPSIMDATAINTERFACE_DECL MakeTopicDataShared()
{
	return TSTopicDataMqItemSharedPtr(GetSegment()->construct<TSTopicDataMqItem>(boost::interprocess::anonymous_instance)(),
		boost::interprocess::allocator<void,boost::interprocess::managed_shared_memory::segment_manager>(GetSegment()->get_segment_manager()),
		boost::interprocess::deleter<TSTopicDataMqItem,boost::interprocess::managed_shared_memory::segment_manager>(GetSegment()->get_segment_manager()));
}

TSSharedMemoryDataContext::TSSharedMemoryDataContext()
{

}

TSSharedMemoryDataContext::~TSSharedMemoryDataContext()
{
	FreeSharedMemoryDataContext(*this);
}
#endif  //_WRS_KERNEL
