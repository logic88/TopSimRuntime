#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#if !defined(XSIM3_3) && !defined(XSIM3_2)
#include <TopSimRuntime/TSLogsUtil.h>
#endif // !defined(XSIM3_3) && !defined(XSIM3_2)

#include "TSTopicTypes.h"
#include "TSSerializer.h"
#include "TSTopicTypeManager.h"

BEGIN_EXTERNAL_METADATA(TSInterObject)
END_EXTERNAL_METADATA()

BEGIN_EXTERNAL_METADATA(TSTOPICHANDLE)
    REG_PROP_FIELD2(_value);
END_EXTERNAL_METADATA()

BEGIN_EXTERNAL_METADATA(TSFrontAppUniquelyId)
    REG_PROP_FIELD2(_value);
END_EXTERNAL_METADATA()

BEGIN_EXTERNAL_METADATA(TSInstanceHandle)
    REG_PROP_FIELD2(_TopicHandle);
    REG_PROP_FIELD2(_DataTopicHandle);
    REG_PROP_FIELD2(_InstanceIndex);
END_EXTERNAL_METADATA()

BEGIN_EXTERNAL_METADATA(TSObjectHandle)
    REG_PROP_FIELD2(_TopicHandle);
	REG_PROP_FIELD2(_FrontAppId);
    REG_PROP_FIELD2(_Handle);
END_EXTERNAL_METADATA()

BEGIN_EXTERNAL_METADATA(TSObjectIdentity)
    REG_PROP_FIELD2(_ObjectHandle);
END_EXTERNAL_METADATA()

TSTOPICHANDLE TSTopicTypes::Uuid2TopicIdType(TopicDataUuidType Uuid)
{
    return Uuid >> 32;
}

UINT32 TSTopicTypes::Uuid2Index(TopicDataUuidType Uuid)
{
    return Uuid & ((UINT32)-1);
}

TopicDataUuidType TSTopicTypes::GenerateTopicDataUuidType(TSTOPICHANDLE TopicId,UINT32 Index)
{
    TopicDataUuidType Uuid = TopicId._value;
    return (Uuid << 32) | Index;
}

void BufferToU8( std::vector<UINT8>& Vec,const void * Data,size_t size )
{
    Vec.resize(size);
#ifndef __VXWORKS__
    memcpy(Vec.data(),Data,size);
#else
    if(Vec.size() > 0)
    {
    	memcpy(&Vec.at(0),Data,size);
    }
#endif

}

void U8ToBuffer( const std::vector<UINT8>& Vec,TSByteBufferPtr Buffer)
{
#ifndef __VXWORKS__
	Buffer->WriteBytes(Vec.data(),Vec.size());
#else
	if(Vec.size() > 0)
	{
		Buffer->WriteBytes(&Vec.at(0),Vec.size());
	}
#endif

}

TSIOSStruct::TSIOSStruct()
{
#if defined(XSIM3_3) || defined(XSIM3_2)
	_Ios = boost::make_shared<boost::asio::io_service>();
	_Work.reset(new boost::asio::io_service::work(*_Ios));
#else
	_Ios = TSAsyncEventDispatcher::CreateInstance();
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

TSIOSStruct::~TSIOSStruct()
{
#if defined(XSIM3_3) || defined(XSIM3_2)

	if (_Ios)
	{
		_Ios->stop();

		if (_Thread)
		{
			_Thread->join();
			_Thread.reset();
		}

		_Ios->poll();
	}
	
#else

	if (_Ios && _Ios->Runing())
	{
		_Ios->Stop(true);
		_Threads.join_all();
	}

#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

/* 循环阻塞运行 */
void TSIOSStruct::RunAlwaysBlock()
{
#if defined(XSIM3_3) || defined(XSIM3_2)
	boost::system::error_code ec;
	_Ios->run(ec);

	if(ec)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPES_CPP_105) << ec.message();
	}
#else
	_Ios->Run();
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

/* 非阻塞循环运行 */
void TSIOSStruct::RunAlways()
{
#if defined(XSIM3_3) || defined(XSIM3_2)
	_Thread.reset(new boost::thread(boost::bind(&TSIOSStruct::RunInternal,this)));
#else
	_Threads.add_thread(new boost::thread(boost::bind(&TSAsyncEventDispatcher::Run, _Ios.get())));
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

/* 不抛弃队列中的事件，终止IO事件队列 */
void TSIOSStruct::Stop()
{
#if defined(XSIM3_3) || defined(XSIM3_2)
	_Ios->stop();

	if(_Thread)
	{
		_Thread->join();
		_Thread.reset();
	}

	_Ios->poll();
#else

	if (_Ios)
	{
		_Ios->Stop(true);
	}

	_Threads.join_all();

#endif // defined(XSIM3_3) || defined(XSIM3_2)
	
}

void TSIOSStruct::RunInternal()
{
#if defined(XSIM3_3) || defined(XSIM3_2)
	boost::system::error_code ec;
	_Ios->run(ec);

	if(ec)
	{
		DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSTOPICTYPES_CPP_75) << ec.message();
	}
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

/* 获取异步IO事件队列调度对象 */
TSIOSericePtr TSIOSStruct::GetIOS()
{
	return _Ios;
}


TSTOPICHANDLE& TSInstanceHandle::GetTopicHandle()
{
	return _TopicHandle;
}

const TSTOPICHANDLE& TSInstanceHandle::GetTopicHandle() const
{
	return _TopicHandle;
}

TSTOPICHANDLE& TSInstanceHandle::GetDataTopicHandle()
{
	return _DataTopicHandle;
}

const TSTOPICHANDLE& TSInstanceHandle::GetDataTopicHandle() const
{
	return _DataTopicHandle;
}

UINT32& TSInstanceHandle::GetInstanceIndex()
{
	return _InstanceIndex;
}

const UINT32& TSInstanceHandle::GetInstanceIndex() const
{
	return _InstanceIndex;
}

void TSInstanceHandle::SetTopicHandle( TSTOPICHANDLE THandle )
{
	_TopicHandle = THandle;
}

void TSInstanceHandle::SetDataTopicHandle( TSTOPICHANDLE THandle )
{
	_DataTopicHandle = THandle;
}

void TSInstanceHandle::SetInstanceIndex( UINT32 Index )
{
	_InstanceIndex = Index;
}

bool TSInstanceHandle::GetIsFromNetwork()
{
    return _IsFromNetWork;
}

const bool& TSInstanceHandle::GetIsFromNetwork() const
{
    return _IsFromNetWork;
}

void TSInstanceHandle::SetIsFromNetwork( bool IsFromNetWork )
{
    _IsFromNetWork = IsFromNetWork;
}

bool TSInstanceHandle::IsValid()
{
	return !(!_TopicHandle || !_DataTopicHandle || _InstanceIndex == TS_INVALID);
}

TSTOPICHANDLE& TSObjectHandle::GetTopicHandle()
{
	return _TopicHandle;
}

const TSTOPICHANDLE& TSObjectHandle::GetTopicHandle() const
{
	return _TopicHandle;
}

TSFrontAppUniquelyId& TSObjectHandle::GetFrontAppUniquelyId()
{
	return _FrontAppId;
}

const TSFrontAppUniquelyId& TSObjectHandle::GetFrontAppUniquelyId() const
{
	return _FrontAppId;
}

TSEndpoint& TSObjectHandle::GetEndpoint()
{
	return _Endpoint;
}

const TSEndpoint& TSObjectHandle::GetEndpoint() const
{
	return _Endpoint;
}

UINT32& TSObjectHandle::GetHandle()
{
	return _Handle;
}

const UINT32& TSObjectHandle::GetHandle() const
{
	return _Handle;
}

void TSObjectHandle::SetTopicHandle( TSTOPICHANDLE THandle )
{
	_TopicHandle = THandle;
}

void TSObjectHandle::SetEndpoint( const TSEndpoint& Ep)
{
	_Endpoint = Ep;
}

void TSObjectHandle::SetFrontAppUniquelyId( TSFrontAppUniquelyId Id )
{
	_FrontAppId = Id;
}

void TSObjectHandle::SetHandle( UINT32 h )
{
	_Handle = h;
}

bool TSObjectHandle::GetIsFromNetwork() const
{
	return _IsFromNetwork;
}

void TSObjectHandle::SetIsFromNetwork( bool IsFromNetwork )
{
	_IsFromNetwork = IsFromNetwork;
}

struct TSTopicContextPrivate
{
	TSTopicHelperPtr Helper;
	TSInstanceHandle Handle;
	TSInterObjectPtr Data;
	TSRWSpinLock     DataLock;
	TSDataContextPtr Ctx;
	TSTime           Time;
};

TSInterObjectPtr TSTopicContext::GetTopic()
{
#if defined(XSIM3_2) || defined(XSIM3_3)
	TSRWSpinLock::WriteLock lock(_p->DataLock);
#else
	TSRWSpinLock::ReadLock lock(_p->DataLock);
#endif
	
	if(!_p->Data)
	{
#if !defined(XSIM3_2) && !defined(XSIM3_3)
		lock.UpgradeToWriter();
#endif

		if(!_p->Data)
		{
			if(TSTypeSupportPtr Support = HandleToSupport(_p->Handle.GetDataTopicHandle()))
			{
				_p->Data = TS_STATIC_CAST(Support->ConstructSmartDataObject(),TSInterObjectPtr);

#if defined(XSIM3_3) || defined(XSIM3_2)
                UINT64 OriOffsetR = _p->Ctx->GetBinary()->GetOffsetR();
#else
                UINT64 OriOffsetR = _p->Ctx->GetBinary()->TellR();
#endif // defined(XSIM3_3) || defined(XSIM3_2)

				//序列化
				TSSerializer Ser(_p->Ctx);
				if(Ser.PerpareForDeserialize(Support->GetTypeName(),Support->GetTypeVersionHashCode(), _p->Time))
				{
					//反序列化
					Support->Deserialize(_p->Data.get(),Ser);
				}	

				_p->Ctx->GetBinary()->SeekR(OriOffsetR);
			}
		}

#if !defined(XSIM3_2) && !defined(XSIM3_3)
		lock.DowngradeToReader();
#endif
	}

	return _p->Data;
}

bool TSTopicContext::Compare( TSTopicContext& Other )
{
	return _p->Ctx->Compare(*Other.GetCtx());
}

bool TSTopicContext::CompareFindSet( TopSimDataInterface::FindSetFlag Flag,const void * CompareData )
{
	return _p->Ctx->CompareFindSet(Flag,CompareData);
}

bool TSTopicContext::GetFindSet( TopSimDataInterface::FindSetFlag Flag,void * KeyOut )
{
	return _p->Ctx->GetFindSet(Flag,KeyOut);
}

void TSTopicContext::SetInstanceHandle( const TSInstanceHandle& Handle )
{
	_p->Handle = Handle;
}

TSTime TSTopicContext::GetTime()
{
	if(_p->Time.is_not_a_date_time())
	{
		TSRWSpinLock::WriteLock lock(_p->DataLock);

		if(_p->Time.is_not_a_date_time())
		{
			if(TSTypeSupportPtr Support = HandleToSupport(_p->Handle.GetDataTopicHandle()))
			{
				_p->Time = _p->Ctx->GetTime();
			}
		}
	}

	return _p->Time;
}

void TSTopicContext::SetTime( const TSTime & aTime )
{
	_p->Time = aTime;
}

TSDataContextPtr TSTopicContext::GetCtx()
{
	return _p->Ctx;
}

void TSTopicContext::SetCtx(TSDataContextPtr Ctx)
{
	_p->Ctx = Ctx;
}

void TSTopicContext::SetTopic(TSInterObjectPtr Obj)
{
	TSRWSpinLock::WriteLock lock(_p->DataLock);

	_p->Data = Obj;
}

TSInterObjectPtr TSTopicContext::DuplicateTopic()
{
	if (TSInterObjectPtr InterObj = GetTopic())
	{
		if (TSTypeSupportPtr Support = HandleToSupport(_p->Handle.GetDataTopicHandle()))
		{
			TSSerializer Ser;
			TSTime Time = GetTime();
			if (Ser.PerpareForSerialize(Support->GetTypeVersionHashCode(), Time))
			{
				Support->Serialize(InterObj.get(), Ser);
				Ser.MarshalComplete();

				TSInterObjectPtr CopyObj = TS_STATIC_CAST(Support->ConstructSmartDataObject(),TSInterObjectPtr);
				
				TSSerializer Deser(Ser.GetDataContext());
				if (Deser.PerpareForDeserialize(Support->GetTypeName(), Support->GetTypeVersionHashCode(), Time))
				{
					Support->Deserialize(CopyObj.get(), Deser);

					return CopyObj;
				}
			}
		}
	}

	return TSInterObjectPtr();
}

const TSTOPICHANDLE& TSTopicContext::GetTopicHandle()
{
	return _p->Handle.GetTopicHandle();
}

const TSTOPICHANDLE& TSTopicContext::GetDataTopicHandle()
{
	return _p->Handle.GetDataTopicHandle();
}

bool TSTopicContext::GetIsFromNetwork()
{
	return _p->Handle.GetIsFromNetwork();
}

const TSInstanceHandle& TSTopicContext::GetInstanceHandle()
{
	return _p->Handle;
}

bool TSTopicContext::Is( TSTOPICHANDLE TopicHandle )
{
	if(!_p->Helper)
	{
		_p->Helper = TSTopicTypeManager::Instance()->GetTopicHelperByTopic(GetDataTopicHandle());
	}

	ASSERT(_p->Helper);
	return _p->Helper->CanConvert(TopicHandle);
}

TSTopicContext::TSTopicContext()
	:_p(new TSTopicContextPrivate)
{

}

TSTopicContext::~TSTopicContext()
{
	delete _p;
}
