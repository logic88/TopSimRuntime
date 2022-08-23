#include "stdafx.h"

#ifndef  _WRS_KERNEL

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>


#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimUtil/TSFileSystem.h>
#include <TopSimUtil/TSDynamicLib.h>
#else
#include <TopSimRuntime/TSFileSystem.h>
#include <TopSimRuntime/TSDynamicLib.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TSSegment.h"

SINGLETON_IMPLEMENT(TSSegment);

using namespace boost::interprocess;

void TSSegment::Init(CreateTag Tag,size_t MaxSize,const TSString& MemoryName)
{
	_MemoryName = MemoryName;
	switch(Tag)
	{
	case CreateOnly:
	case OpenOrCreate:
		{
			shared_memory_object::remove(MemoryName.c_str());
			_Segment = new managed_shared_memory(create_only,MemoryName.c_str(),MaxSize);
			_IsNewCreate = true; 
		}
		break;
	case OpenOnly:
		{
			_Segment = new managed_shared_memory(open_only,MemoryName.c_str());
		}
		break;
	case OpenReadOnly:
		{
			_Segment = new managed_shared_memory(open_read_only,MemoryName.c_str());
		}
		break;
	case OpenCopyOnWrite:
		{
			_Segment = new managed_shared_memory(open_copy_on_write,MemoryName.c_str());
		}
		break;
	default:
		{
			throw TSException("not declare tag!");
		}
	}
}

TSSegment::TSSegment()
	:_Segment(NULL)
	,_IsNewCreate(false)
{
	
}

managed_shared_memory * TSSegment::GetSegmentMemory()
{
	ASSERT(_Segment);
	return _Segment;
}

void TSSegment::Cleanup()
{
	if(_Segment)
	{
		delete _Segment;
	}

	if(_IsNewCreate)
	{
		shared_memory_object::remove(_MemoryName.c_str());
	}

	_Segment = NULL;
}

#endif //#ifndef  _WRS_KERNEL
