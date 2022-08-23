#ifndef __TSTOPICTYPES_H__
#define __TSTOPICTYPES_H__

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimUtil/TSOSMisc.h>
#include <TopSimTypes/TSOrderTypeMgr.h>
#include <TopSimTypes/TSRWSpinLock.h>
#else
#include <TopSimRuntime/TSOSMisc.h>
#include <TopSimRuntime/TSOrderTypeMgr.h>
#include <TopSimRuntime/TSRWSpinLock.h>
#include <TopSimRuntime/TSAsyncEventDispatcher.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TopSimDataInterface.h"

#define TOPIC_FILE_SIGN		"xtp1"
#define TOPIC_FILE_VERSION	0x00000001
#define TOPIC_FILE_EXT		TS_TEXT(".xtp1")
#define PUBLISH_FILE_EXT    TS_TEXT(".xps1")

#define SESSION_APP_NAME "TSSession"

template<typename T>
inline size_t GenerateHash(const T &value)
{
	return 0;
}

struct TSTopicHandleValue
{
	UINT32 _value;
};

struct TOPSIMDATAINTERFACE_DECL TSInterObject
{
	virtual ~TSInterObject(){}
};

STRUCT_PTR_DECLARE(TSInterObject);
TS_DECLARE_METATYPE(TSInterObject);
/*
  主题句柄
*/
struct TOPSIMDATAINTERFACE_DECL TSTOPICHANDLE 
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    TS_MetaType(TSTOPICHANDLE);
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	UINT32 _value;	//句柄值

	FORCEINLINE TSTOPICHANDLE(UINT32 value = 0)
		: _value(value)
	{
	}

	FORCEINLINE bool operator < (const TSTOPICHANDLE &other) const
	{
		return _value < other._value;
	}

	FORCEINLINE TSTOPICHANDLE(const TSTOPICHANDLE &r)
	{
		_value = r._value;
	}

	FORCEINLINE bool operator!() const
	{
		return _value == 0 || _value==TS_INVALID;
	}

	FORCEINLINE operator UINT32() const
	{
		return _value;
	}

	FORCEINLINE TSTOPICHANDLE & operator=(TSTOPICHANDLE r)
	{
		_value = r._value;
		return *this;
	}

	FORCEINLINE TSTOPICHANDLE & operator=(UINT32 r)
	{
		_value = r;
		return *this;
	}
};

template<>
inline size_t GenerateHash(const TSTOPICHANDLE &value)
{
	return std::hash<UINT32>()(value._value);
}

struct TOPSIMDATAINTERFACE_DECL TSFrontAppUniquelyId
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    TS_MetaType(TSFrontAppUniquelyId);
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	UINT32 _value;

	FORCEINLINE TSFrontAppUniquelyId(UINT32 value = 0)
		: _value(value)
	{

	}

	FORCEINLINE bool operator < (const TSFrontAppUniquelyId &other) const
	{
		return _value < other._value;
	}

	FORCEINLINE TSFrontAppUniquelyId(const TSFrontAppUniquelyId &r)
	{
		_value = r._value;
	}

	FORCEINLINE bool operator!() const
	{
		return _value == 0 || _value==TS_INVALID;
	}

	FORCEINLINE operator UINT32() const
	{
		return _value;
	}

	FORCEINLINE TSFrontAppUniquelyId & operator=(const TSFrontAppUniquelyId& r)
	{
		_value = r._value;

		return *this;
	}
};

template<>
inline size_t GenerateHash(const TSFrontAppUniquelyId &value)
{
	return std::hash<UINT32>()(value._value);
}

struct TOPSIMDATAINTERFACE_DECL TSInstanceHandle
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    TS_MetaType(TSInstanceHandle);
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	TSTOPICHANDLE _TopicHandle;
	TSTOPICHANDLE _DataTopicHandle;
	UINT32        _InstanceIndex;
    bool          _IsFromNetWork;

	FORCEINLINE TSInstanceHandle()
		: _TopicHandle(0)
		,_DataTopicHandle(0)
		,_InstanceIndex(TS_INVALID)
        ,_IsFromNetWork(true)
	{

	}

	FORCEINLINE TSInstanceHandle(TSTOPICHANDLE TopicHandle)
		:_TopicHandle(TopicHandle)
		,_DataTopicHandle(TopicHandle)
		,_InstanceIndex(TS_INVALID)
        ,_IsFromNetWork(true)
	{
		
	}

	FORCEINLINE TSInstanceHandle(TSTOPICHANDLE TopicHandle,TSTOPICHANDLE DataTopicHandle)
		:_TopicHandle(TopicHandle)
		,_DataTopicHandle(DataTopicHandle)
		,_InstanceIndex(TS_INVALID)
        ,_IsFromNetWork(true)
	{
		
	}

	FORCEINLINE TSInstanceHandle(TSTOPICHANDLE TopicHandle,TSTOPICHANDLE DataTopicHandle,UINT32 InstanceIndex,bool IsFromNetWork = true)
		: _TopicHandle(TopicHandle)
		,_DataTopicHandle(DataTopicHandle)
		,_InstanceIndex(InstanceIndex)
        ,_IsFromNetWork(IsFromNetWork)
	{
		
	}

	FORCEINLINE TSInstanceHandle(const TSInstanceHandle &r)
	{
		_TopicHandle = r._TopicHandle;
		_DataTopicHandle = r._DataTopicHandle;
		_InstanceIndex = r._InstanceIndex;
        _IsFromNetWork = r._IsFromNetWork;
	}

	FORCEINLINE bool operator<(const TSInstanceHandle &other) const
	{
		if(this->_InstanceIndex < other._InstanceIndex) return true;
		if(other._InstanceIndex < this->_InstanceIndex) return false;
		if(this->_DataTopicHandle < other._DataTopicHandle) return true;
		if(other._DataTopicHandle < this->_DataTopicHandle) return false;
		if(this->_TopicHandle < other._TopicHandle) return true;
		if(other._TopicHandle < this->_TopicHandle) return false;
		return false;
	}

	FORCEINLINE bool operator!() const
	{
		return !_TopicHandle
			|| !_DataTopicHandle 
            || _InstanceIndex == TS_INVALID;
	}

	FORCEINLINE TSInstanceHandle & operator=(const TSInstanceHandle& r)
	{
		_TopicHandle = r._TopicHandle;
		_DataTopicHandle = r._DataTopicHandle;
		_InstanceIndex = r._InstanceIndex;
        _IsFromNetWork = r._IsFromNetWork;

		return *this;
	}

	FORCEINLINE bool operator==(const TSInstanceHandle& r) const 
	{
		return _TopicHandle == r._TopicHandle
			&& _DataTopicHandle == r._DataTopicHandle
			&& _InstanceIndex == r._InstanceIndex;
	}

	FORCEINLINE bool operator!=(const TSInstanceHandle& r) const 
	{
		return _TopicHandle != r._TopicHandle
			|| _DataTopicHandle != r._DataTopicHandle
			|| _InstanceIndex != r._InstanceIndex;
	}

	bool IsValid();

    void SetIsFromNetwork(bool IsFromNetWork);
	void SetTopicHandle(TSTOPICHANDLE THandle);
	void SetDataTopicHandle(TSTOPICHANDLE THandle);
	void SetInstanceIndex(UINT32 Index);

	TSTOPICHANDLE& GetTopicHandle();
	TSTOPICHANDLE& GetDataTopicHandle();
	UINT32&        GetInstanceIndex();
    bool           GetIsFromNetwork();

	const TSTOPICHANDLE& GetTopicHandle()const ;
	const TSTOPICHANDLE& GetDataTopicHandle()const ;
	const UINT32&        GetInstanceIndex()const ;
    const bool&          GetIsFromNetwork()const ;
};

#define IS_VALID_INSTANCE_HANVLE(a) ((a).GetInstanceIndex() != TS_INVALID)

template<>
inline size_t GenerateHash(const TSInstanceHandle &value)
{
	return GenerateHash(value._TopicHandle)
		^ GenerateHash(value._DataTopicHandle)
		^ std::hash<UINT32>()(value._InstanceIndex);
}

namespace tbb {
	template<>
	class tbb_hash<TSInstanceHandle>
	{
	public:
		tbb_hash() {}

		size_t operator()(const TSInstanceHandle& key) const
		{
			return GenerateHash(key);
		}
	};
}

struct TOPSIMDATAINTERFACE_DECL TSObjectHandle
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    TS_MetaType(TSObjectHandle);
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	TSTOPICHANDLE        _TopicHandle;
	TSFrontAppUniquelyId _FrontAppId;
	UINT32               _Handle;
	TSEndpoint           _Endpoint;
	bool                 _IsFromNetwork;

	FORCEINLINE TSObjectHandle()
		:_TopicHandle(TS_INVALID),
        _FrontAppId(TS_INVALID),
        _Handle(TS_INVALID),
        _Endpoint(""),
        _IsFromNetwork(false)
	{

	}

	FORCEINLINE bool operator < (const TSObjectHandle &other) const
	{
		if(this->_Handle < other._Handle) return true;
		if(other._Handle < this->_Handle) return false;
		if(this->_FrontAppId < other._FrontAppId) return true;
		if(other._FrontAppId < this->_FrontAppId) return false;
		if(this->_TopicHandle < other._TopicHandle) return true;
		if(other._TopicHandle < this->_TopicHandle) return false;
		if(this->_Endpoint < other._Endpoint) return true;
		if(other._Endpoint < this->_Endpoint) return false;
		return false;
	}

	FORCEINLINE TSObjectHandle(TSTOPICHANDLE THandle,
		TSFrontAppUniquelyId AppId,
		UINT32 Handle,
		const TSEndpoint& ep = TSEndpoint())
		: _TopicHandle(THandle)
		,_FrontAppId(AppId)
		,_Handle(Handle)
		,_Endpoint(ep)
		,_IsFromNetwork(false)
	{

	}

	FORCEINLINE TSObjectHandle(const TSObjectHandle &r)
	{
		_TopicHandle = r._TopicHandle;
		_FrontAppId = r._FrontAppId;
		_Handle = r._Handle;
		_Endpoint = r._Endpoint;
		_IsFromNetwork = r._IsFromNetwork;
	}

	FORCEINLINE bool operator!() const
	{
		return !_FrontAppId
			|| !_TopicHandle
			|| _Handle == 0 || _Handle == TS_INVALID;
	}

	FORCEINLINE TSObjectHandle & operator=(const TSObjectHandle& r)
	{
		_TopicHandle = r._TopicHandle;
		_FrontAppId = r._FrontAppId;
		_Handle = r._Handle;
		_Endpoint = r._Endpoint;
		_IsFromNetwork = r._IsFromNetwork;

		return *this;
	}

	FORCEINLINE bool operator==(const TSObjectHandle& r) const 
	{
		return _TopicHandle == r._TopicHandle
			&& _FrontAppId == r._FrontAppId
			&& _Handle == r._Handle
			&& _Endpoint == r._Endpoint;
	}

	FORCEINLINE bool operator!=(const TSObjectHandle& r) const 
	{
		return _TopicHandle != r._TopicHandle
			|| _FrontAppId != r._FrontAppId
			|| _Handle != r._Handle
			|| _Endpoint != r._Endpoint;
	}

	 void  SetTopicHandle(TSTOPICHANDLE THandle);
	 void  SetHandle(UINT32 h);
	 void  SetFrontAppUniquelyId(TSFrontAppUniquelyId Id);
	 void  SetEndpoint(const TSEndpoint& Ep);

	 TSTOPICHANDLE&        GetTopicHandle();
	 UINT32&               GetHandle();
	 TSFrontAppUniquelyId& GetFrontAppUniquelyId();
	 TSEndpoint&           GetEndpoint();

	 const TSEndpoint&     GetEndpoint() const;
	 const TSFrontAppUniquelyId& GetFrontAppUniquelyId() const;
	 const UINT32&         GetHandle() const;
	 const TSTOPICHANDLE&  GetTopicHandle() const;

	 void SetIsFromNetwork(bool IsFromNetwork);
	 bool GetIsFromNetwork() const;
};

struct TSObjectIdentity
{
#if defined(XSIM3_3) || defined(XSIM3_2)
    TS_MetaType(TSObjectIdentity);
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	TSObjectHandle _ObjectHandle;

	FORCEINLINE TSObjectIdentity()
	{

	}

	FORCEINLINE TSObjectIdentity(const TSObjectIdentity &r)
	{
		this->_ObjectHandle = r._ObjectHandle;
	}

	FORCEINLINE TSObjectIdentity(const TSObjectHandle &r)
	{
		this->_ObjectHandle = r;
	}

	FORCEINLINE bool operator < (const TSObjectIdentity &other) const
	{
		return this->_ObjectHandle < other._ObjectHandle;
	}

	FORCEINLINE bool operator!() const
	{
		return !this->_ObjectHandle;
	}

	FORCEINLINE TSObjectIdentity & operator=(const TSObjectIdentity& r)
	{
		this->_ObjectHandle = r._ObjectHandle;
		return *this;
	}

	FORCEINLINE bool operator==(const TSObjectIdentity& r) const 
	{
		return this->_ObjectHandle == r._ObjectHandle;
	}

	FORCEINLINE bool operator!=(const TSObjectIdentity& r) const 
	{
		return this->_ObjectHandle != r._ObjectHandle;
	}
};

template<>
inline size_t GenerateHash(const TSObjectHandle &value)
{
	return GenerateHash(value._FrontAppId)
		^ std::hash<UINT32>()(value._Handle);
}

namespace tbb {
	template<>
	class tbb_hash<TSObjectHandle>
	{
	public:
		tbb_hash() {}

		size_t operator()(const TSObjectHandle& key) const
		{
			return GenerateHash(key);
		}
	};
}

typedef std::vector<TSTOPICHANDLE> TopicHandleArray;
typedef std::vector<TSFrontAppUniquelyId> FrontAppUniquelyIdArray;
typedef std::vector<TSObjectHandle> ObjectHandleArray;
typedef std::vector<TSInstanceHandle> InstanceHandleArray;

TS_DECLARE_METATYPE(TSTOPICHANDLE);
TS_DECLARE_METATYPE(TSFrontAppUniquelyId);
TS_DECLARE_METATYPE(TSInstanceHandle);
TS_DECLARE_METATYPE(TSObjectHandle);
TS_DECLARE_METATYPE(TSObjectIdentity);

/* 
  IO事件队列调度框架结构
*/
struct TOPSIMDATAINTERFACE_DECL TSIOSStruct
{
	TSIOSStruct();
	~TSIOSStruct();

	/* 循环阻塞运行 */
	void RunAlwaysBlock();
	/* 非阻塞循环运行 */
	void RunAlways();
	/* 不抛弃队列中的事件，终止IO事件队列 */
	void Stop();
	/* */
	void RunInternal();
	/* 获取异步IO事件队列调度对象 */
	TSIOSericePtr GetIOS();

private:

#if defined(XSIM3_3) || defined(XSIM3_2)
	boost::shared_ptr<boost::asio::io_service> _Ios;
	boost::shared_ptr<boost::asio::io_service::work> _Work;
	boost::shared_ptr<boost::thread> _Thread;
#else
	TSIOSericePtr _Ios;		//异步IO事件队列调度对象
	boost::thread_group   _Threads;	//线程对象
#endif // defined(XSIM3_3) || defined(XSIM3_2)
};

typedef boost::shared_ptr<TSIOSStruct> TSIOSStructPtr;

typedef UINT64 TopicDataUuidType;

typedef boost::intrusive_ptr<class TSTypeSupport> TSTypeSupportPtr;

struct TOPSIMDATAINTERFACE_DECL TSTopicTypes
{
	static TSTOPICHANDLE Uuid2TopicIdType(TopicDataUuidType Uuid);
	static UINT32 Uuid2Index(TopicDataUuidType Uuid);
	static TopicDataUuidType GenerateTopicDataUuidType(TSTOPICHANDLE TopicId,UINT32 Index);
};

TOPSIMDATAINTERFACE_DECL void BufferToU8(std::vector<UINT8>& Vec,const void * Data,size_t size);
TOPSIMDATAINTERFACE_DECL void U8ToBuffer(const std::vector<UINT8>& Vec,TSByteBufferPtr Buffer);

#ifdef _cplusplus
extern "C" {
#endif

	//VXWorks编译选项
#if defined(__VXWORKS__)

	//主题数据记录
	typedef struct _tagTSTopicRecordStruct
	{
		/// <summary> 记录的大小 </summary>
		UINT32 SizeOfThis;

		/// <summary> 作战时间(相对时间),单位微妙 </summary>
		UINT64 Time;

		/// <summary> Key</summary>
		UINT32 KeyHash;

		/// <summary> 扩展数据(主题内容) </summary>
		char Extendable[1];
	}TS_PACKED_ALIGN_STRUCT_1 TSTopicRecordStruct, *pTSTopicRecordStruct;

#else
	//非VXWorks编译

#ifndef _WRS_KERNE
#pragma pack(push)
#pragma pack(1)
#endif //_WRS_KERNE

	//主题数据记录
	typedef struct _tagTSTopicRecordStruct
	{
		/// <summary> 记录的大小 </summary>
		UINT32 SizeOfThis;

		/// <summary> 作战时间(相对时间),单位微妙 </summary>
		UINT64 Time;

		/// <summary> Key</summary>
		UINT32 KeyHash;

		/// <summary> 扩展数据(主题内容) </summary>
		char Extendable[1];
	} TSTopicRecordStruct, *pTSTopicRecordStruct;

#ifndef _WRS_KERNE
#pragma pack(pop)
#endif //_WRS_KERNE

#endif // __VXWORKS__


#ifdef _cplusplus
}
#endif

/* 回调函数包装结构 */
template<typename Func>
struct TSCallbackWapper
{
	TSCallbackWapper(Func f)
		:Removed(false)
		,Functor(f)
		,AsyncCallbackThread(true)
	{

	}

	boost::atomic_bool Removed;
	bool               AsyncCallbackThread;
	Func               Functor;

private:
	TSCallbackWapper(const TSCallbackWapper& other)
	{
	}
};

template<typename T>
class TSCallbackArray : public TSConArray<T>
{
public:
	TSCallbackArray()
	{
		_Valid = false;
	}

	void Update()
	{
		_Valid = false;
		for(size_t i = 0; i < this->size(); ++i)
		{
			if(!(*this)[i]->Removed)
			{
				_Valid = true;
				break;
			}
		}
	}

	bool IsValid() const
	{
		return _Valid;
	}

private:
	bool _Valid;
};

struct TSTopicContextPrivate;
struct TOPSIMDATAINTERFACE_DECL TSTopicContext : public boost::intrusive_ref_counter<TSTopicContext, boost::thread_safe_counter>
{

public:
	TSTopicContext();
	~TSTopicContext();
	const TSTOPICHANDLE&          GetTopicHandle();
	const TSTOPICHANDLE&          GetDataTopicHandle();
	bool                          GetIsFromNetwork();
	void                          SetInstanceHandle(const TSInstanceHandle& Handle);
	const TSInstanceHandle&       GetInstanceHandle();
	TSInterObjectPtr              GetTopic();
	bool                          Is(TSTOPICHANDLE TopicHandle);
	bool                          Compare(TSTopicContext& Other);
	bool                          CompareFindSet(TopSimDataInterface::FindSetFlag Flag,const void * CompareData);
	bool                          GetFindSet(TopSimDataInterface::FindSetFlag Flag,void * KeyOut);
	TSTime                        GetTime();
	void                          SetTime(const TSTime & Time);
	TSDataContextPtr              GetCtx();
	void                          SetCtx(TSDataContextPtr Ctx);
	void                          SetTopic(TSInterObjectPtr Obj);
	TSInterObjectPtr              DuplicateTopic();

	template<typename T>
	FORCEINLINE boost::shared_ptr<T> GetTopicT()
	{
		return TS_CAST(GetTopic(), boost::shared_ptr<T>);
	}
	
private:
	TSTopicContextPrivate * _p;
};

typedef std::vector<TSTOPICHANDLE> TSTopicQuerySet;

#endif
