#ifndef __TSDOMAIN_H__
#define __TSDOMAIN_H__

#include <tbb/concurrent_unordered_set.h>

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimTypes/TSGenericHandleTable.h>
#else
#include <TopSimRuntime/TSGenericHandleTable.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TopSimDataInterface.h"
#include "TSTopicTypes.h"
#include "TSWaitSet.h"
#include "TSTopicHelper.h"

struct TSDataInteractManagerPrivate;
struct ParticipantQosPolicy;
struct DataReaderQosPolicy;
struct DataWriterQosPolicy;

typedef boost::shared_ptr<class TSDataCollection> TSDataCollectionPtr;
typedef std::vector<TSDataCollectionPtr> DataCollectionArray;

extern "C" TOPSIMDATAINTERFACE_DECL TSTOPICHANDLE BuiltinTopicRemovedHandle;

typedef TSGenericHandleTable<VoidPtr> TSHandleTable;

typedef boost::function<void()> TSDomainRoutine_T;

//接收到主题回调函数
typedef boost::function<void(TSTopicContextPtr Ctx)> TSTopicCallbackType;
//主题匹配回调函数
typedef boost::function<void(TSDomainId DomainId, TSTOPICHANDLE THandle, UINT32 AppId, bool IsPub)> TSTopicMatchedCallbackType;
//主题过滤回调函数
typedef boost::function<bool(TSTopicContextPtr Ctx, UINT32 AppId)>	TSDataContentFilterCallbackType;

typedef UINT32 TSTopicCallbackStubType;

struct TOPSIMDATAINTERFACE_DECL TSAbstractTimeHolder
{
	TS_MetaType(TSAbstractTimeHolder);
public:
	virtual TSTime GetTime(const TSDomainId& DomainId,TSTOPICHANDLE THandle,const void * d) PURE_VIRTUAL(TSTime);
};

typedef boost::shared_ptr<TSAbstractTimeHolder> TSAbstractTimeHolderPtr;

struct TOPSIMDATAINTERFACE_DECL TSITopicListener
{
public:
	virtual void OnTopic(TSTopicContextPtr Ctx) = 0;
};

struct TOPSIMDATAINTERFACE_DECL TSITopicFindSetPlugin
{
	TS_MetaType(TSITopicFindSetPlugin);
public:
	virtual void Initialize(TSDomainPtr Domain) = 0;
	virtual void Clean() = 0;
	virtual const TSString &    GetFindSetName() = 0;
	virtual TSTopicFindSetType  CreateFindSet() = 0;
	virtual void                OnTopicUpdate(TSTopicContextPtr Ctx) = 0;
}; 

STRUCT_PTR_DECLARE(TSITopicFindSetPlugin);

/* 域私有成员变量结构 */
struct TSDomainPrivate;
/*
  域
*/
class TOPSIMDATAINTERFACE_DECL TSDomain : public boost::enable_shared_from_this<TSDomain>
{
	friend class  TSDataInteractManager;
	friend struct TSTopicContext;
	friend struct TSFindHelper;
	friend class  TSWaitSet;
	friend class  TSTransferProxy;

	TS_DECLARE_PRIVATE(TSDomain);

protected:

public:
    TSDomain(const TSDomainId& DomainId);
	~TSDomain(void);

protected:
	PrivateType * _Private_Ptr;

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	判断这个域是否被初始化. </summary>
	///
	/// <remarks>	杨永武, 2017/8/25. </remarks>
	///
	/// <returns>	true if Initialized. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool WasInitialized();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets the domain identifier. </summary>
	///
	/// <remarks>	孟龙龙, 2017/6/27. </remarks>
	///
	/// <returns>	The domain identifier. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const TSDomainId& GetDomainId();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	返回发现类，可以使用RegisterApp注册需要被发现的应用. </summary>
	///
	/// <remarks>	孟龙龙, 2017/6/30. </remarks>
	///
	/// <returns>	The discovery. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSDiscoveryPtr GetDiscovery();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	订阅主题. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. luxiang,20180428</remarks>
    ///
    /// <param name="Handle">   	主题句柄. </param>
    /// <param name="LocalOnly">   	true,只订阅本地主题. </param>
    ///
    /// <returns>	枚举 0-成功，1-失败，2-退出 </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TopSimDataInterface::ReturnCode SubscribeTopic(TSTOPICHANDLE Handle, bool LocalOnly = false);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	发布主题. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="Handle">   	主题句柄. </param>
    ///
    /// <returns>	枚举 0-成功，1-失败，2-退出 </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TopSimDataInterface::ReturnCode PublishTopic(TSTOPICHANDLE Handle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	取消订阅. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="Handle">   主题句柄. </param>
	///
	/// <returns>	枚举 0-成功，1-失败，2-退出 </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TopSimDataInterface::ReturnCode UnsubscribeTopic(TSTOPICHANDLE Handle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	取消发布. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="Handle">   	主题句柄. </param>
	///
	/// <returns>	枚举 0-成功，1-失败，2-退出 </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TopSimDataInterface::ReturnCode UnpublishTopic(TSTOPICHANDLE Handle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	是否是主题的订阅者. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="THandle">	主题句柄. </param>
    ///
    /// <returns>	是订阅者返回true，否则返回false. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool IsTopicSubscriber(TSTOPICHANDLE THandle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	是否是主题的发布者. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="THandle">	主题句柄. </param>
    ///
    /// <returns>	是发布者返回true，否则返回false. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool IsTopicPublisher(TSTOPICHANDLE THandle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	推送主题数据. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="Handle">   	主题句柄. </param>
    /// <param name="Data">			主题数据指针. </param>
    ///
    /// <returns>	返回主题实例句柄，通过该句柄可查找到对应的数据实例. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicContextPtr UpdateTopic(TSTOPICHANDLE Handle,const TSInterObject * Data);


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	删除主题数据. </summary>
	///
	/// <remarks>	孟龙龙, 2016/12/29. </remarks>
	///
	/// <param name="Handle">  主题数据上下文. </param>
	///
	/// <returns>	返回主题是否删除成功. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool DeleteTopic(TSTopicContextPtr Ctx,bool IsNotice);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	删除主题数据. </summary>
	///
	/// <remarks>	孟龙龙, 2016/12/29. </remarks>
	///
	/// <param name="Handle">  主题ID. </param>
	/// <param name="KeyData">  含有Key的主题数据. </param>
	/// <returns>	返回主题是否删除成功. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool DeleteTopic(TSTOPICHANDLE TopicHandle,const TSInterObject * KeyData,bool IsNotice);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	立即发送主题。如果接收方为TS_INVALID，则只有数据的订阅者能收到。 </summary>
	///
	/// <remarks>	孟龙龙, 2016/12/29. </remarks>
	///
	/// <param name="THandle">   	主题句柄. </param>
	/// <param name="Parameter"> 	主题数据指针. </param>
	/// <param name="ReceiverId">	接收方唯一标识. </param>
	///
	/// <returns>	主题上下文 </returns>
	///
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicContextPtr SendTopicDirect(TSTOPICHANDLE THandle,const TSInterObject * Data,
		const TSFrontAppUniquelyId& ReceiverId);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	发送二进制数据。 </summary>
    ///
    /// <remarks>	luxiang, 2018/02/10. </remarks>
    ///
    /// <param name="THandle">  主题句柄. </param>
    /// <param name="Data"> 	主题数据指针. </param>
    /// <param name="Length">	数据长度. </param>
	///
	/// <returns>	主题上下文 </returns>
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicContextPtr SendBinary(TSTOPICHANDLE THandle,const void * Data,
        const UINT32 Length);

	TSTopicContextPtr SendBinaryDirect(TSTOPICHANDLE THandle,const void * Data,
		const UINT32 Length,const TSFrontAppUniquelyId & Receiver);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	发送二进制数据。 </summary>
	///
	/// <remarks>	luxiang, 2018/02/10. </remarks>
	///
	/// <param name="THandle">  主题句柄. </param>
	/// <param name="DataBuffer"> 	主题数据指针. </param>
	///
	/// <returns>	主题上下文 </returns>
	///
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicContextPtr SendBinary(TSTOPICHANDLE THandle,TSByteBufferPtr DataBuffer);
	TSTopicContextPtr SendBinaryDirect(TSTOPICHANDLE THandle,TSByteBufferPtr DataBuffer,const TSFrontAppUniquelyId & Receiver);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	发送二进制数据。 </summary>
	///
	/// <remarks>	luxiang, 2018/02/10. </remarks>
	///
	/// <param name="THandle">  主题句柄. </param>
	/// <param name="DataContext"> 	主题数据指针. </param>
	///
	/// <returns>	主题上下文 </returns>
	///
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicContextPtr SendBinary(TSTOPICHANDLE THandle,TSDataContextPtr DataContext);
	TSTopicContextPtr SendBinaryDirect(TSTOPICHANDLE THandle,TSDataContextPtr DataContext,const TSFrontAppUniquelyId & Receiver);

	TSTopicContextPtr GetFirstTopicByHandle(TSTOPICHANDLE THandle);
	TSTopicContextPtr GetFirstTopicByHandle(TSHANDLE Handle,TSTOPICHANDLE THandle);

	template<typename T>
	boost::shared_ptr<T> GetFirstTopicByHandleT(TSTOPICHANDLE THandle)
	{
		if (TSTopicContextPtr Ctx = GetFirstTopicByHandle(THandle))
		{
			return Ctx->GetTopicT<T>();
		}

		return boost::shared_ptr<T>();
	}

	template<typename T>
	boost::shared_ptr<T> GetFirstTopicByHandleT(TSHANDLE Handle, TSTOPICHANDLE THandle)
	{
		if (TSTopicContextPtr Ctx = GetFirstTopicByHandle(Handle, THandle))
		{
			return Ctx->GetTopicT<T>();
		}

		return boost::shared_ptr<T>();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	通过句柄和Query创建查找集. </summary>
	///
	/// <remarks>	孟龙龙, 2016/12/29. </remarks>
	///
	/// <param name="Handle">	句柄. </param>
	/// <param name="THandle">	主题句柄的Query. </param>
	/// <returns>	查找集. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicFindSetType CreateTopicFindSet(TSHANDLE Handle,const TSTopicQuerySet & Query);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	通过句柄和主题句柄创建查找集. </summary>
	///
	/// <remarks>	孟龙龙, 2016/12/29. </remarks>
	///
	/// <param name="Handle">	句柄. </param>
	/// <param name="THandle">	主题句柄的Query. </param>
	/// <returns>	查找集. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicFindSetType CreateTopicFindSet(TSHANDLE Handle,TSTOPICHANDLE Topic);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	通过主题句柄创建查找集. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="THandle">	主题句柄. </param>
    ///
    /// <returns>	查找集. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicFindSetType CreateTopicFindSet(TSTOPICHANDLE THandle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	创建基于位置过滤的查找集.该查找集只对具有kSpacialRangeType属性标识的、类型为TSVector3d的主题适用
	///             属性存储的为经纬度信息（弧度）。</summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="CenterLon">	中心点经度. </param>
    /// <param name="CenterLat">	中心点纬度. </param>
    /// <param name="SpacialRange">	半径. </param>
    /// <returns>	查找集. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicFindSetType CreateTopicFindSet(DOUBLE CenterLon,DOUBLE CenterLat,FLOAT SpacialRange,TSTOPICHANDLE Topic);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	创建基于位置过滤的查找集.该查找集只对具有kSpacialRangeType属性标识的、类型为TSVector3d的主题适用
	///             属性存储的为经纬度信息（弧度）。</summary>
	///
	/// <remarks>	孟龙龙, 2016/12/29. </remarks>
	///
	/// <param name="CenterLon">	中心点经度. </param>
	/// <param name="CenterLat">	中心点纬度. </param>
	/// <param name="SpacialRange">	半径. </param>
	/// <returns>	查找集. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicFindSetType CreateTopicFindSet(DOUBLE CenterLon,DOUBLE CenterLat,FLOAT SpacialRange,const TSTopicQuerySet & Query);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	创建基于实体句柄的查找集. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="TSHANDLE">	实体句柄. </param>
    /// <returns>	查找集. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicFindSetType CreateTopicFindSet(TSHANDLE HKey);

	////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	获取第一个实例句柄. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="FindSet">	查找集. </param>
    /// <param name="IHandle">	输出的实例句柄. </param>
	///
	/// <returns>	主题上下文 </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    TSTopicContextPtr GetFirstTopic(TSTopicFindSetType FindSet);

	template<typename T>
	boost::shared_ptr<T> GetFirstTopicT(TSTopicFindSetType FindSet)
	{
		if (TSTopicContextPtr Ctx = GetFirstTopic(FindSet))
		{
			return Ctx->GetTopicT<T>();
		}

		return boost::shared_ptr<T>();
	}

    TSTopicContextPtr GetNextTopic(TSTopicFindSetType FindSet);

	template<typename T>
	boost::shared_ptr<T> GetNextTopicT(TSTopicFindSetType FindSet)
	{
		if (TSTopicContextPtr Ctx = GetNextTopic(FindSet))
		{
			return Ctx->GetTopicT<T>();
		}

		return boost::shared_ptr<T>();
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取时间委托. </summary>
	///
	/// <remarks>	孟龙龙, 2017/2/17. </remarks>
	///
	/// <returns>	The time delegate. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSAbstractTimeHolderPtr  GetTimeHolder();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置时间委托. </summary>
	///
	/// <remarks>	孟龙龙, 2017/6/27. </remarks>
	///
	/// <param name="TimeHolder">	The time holder. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetTimeHolder( TSAbstractTimeHolderPtr TimeHolder);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置用户标识. </summary>
	///
	/// <remarks>	孟龙龙, 2017/7/7. </remarks>
	///
	/// <param name="Flag">	The flag. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetUserFlag(UINT64 Flag);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取用户标识. </summary>
	///
	/// <remarks>	孟龙龙, 2017/7/7. </remarks>
	///
	/// <returns>	The user flag. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT64 GetUserFlag();

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	注册/取消注册主题数据回调. </summary>
    ///
    /// <remarks>	孟龙龙, 2016/12/29. </remarks>
    ///
    /// <param name="THandle">	主题句柄. </param>
    /// <param name="cb">	  	回调函数. </param>
    /// <param name="stub">	  	回调存根. </param>
    /// <param name="stub">	  	指定回调是否是异步回调线程，否则是数据接收线程 </param>
    /// <returns>	回调存根.该存根用于取消注册 </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicCallbackStubType RegisterTopicCallback(TSTOPICHANDLE THandle,TSTopicCallbackType cb,bool AsyncCallbackThread = true);
    void   UnregisterTopicCallback(TSTOPICHANDLE THandle,UINT32 stub);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	注册/取消注册主题数据回调. </summary>
	///
	/// <remarks>	孟龙龙, 2016/12/29. </remarks>
	///
	/// <param name="THandle">	主题句柄. </param>
	/// <param name="cb">	  	回调函数. </param>
	/// <param name="stub">	  	回调存根. </param>
	/// <param name="stub">	  	指定回调是否是异步回调线程，否则是数据接收线程 </param>
	/// <returns>	回调存根.该存根用于取消注册 </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool RegisterTopicListener(TSITopicListener * Listener);
	void UnregisterTopicListener(TSITopicListener * Listener);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Post. </summary>
	///
	/// <param name="stub">	  	抛一个回调请求，该请求会在数据管理器的回调线程中被调用. </param>
	/// <remarks>	孟龙龙, 2017/8/23. </remarks>
	//////////////oo//////////////////////////////////////////////////////////////////////////////////////
	void Post(TSDomainRoutine_T Routine);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	设置二进制采集保存的文件目录.该接口可以多次调用，每次调用时，会将上一次的采集文件关闭，并重新开启新的数据采集 </summary>
    ///
    /// <param name="Path">	  	采集路径. </param>
    /// <remarks>	luxiang, 2018/05/02. </remarks>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void SetOutputPath(const TSString & OutPath);

	////////////////////////////////////////////////////////////////////////////////////////////////////
  /// <summary>	获取二进制采集保存的文件目录. </summary>
  ///
  /// <param name="Path">	  	采集路径. </param>
  /// <remarks>	luxiang, 2018/05/02. </remarks>
  ////////////////////////////////////////////////////////////////////////////////////////////////////
	const TSString & GetOutputPath();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	注册 上线提醒回调. </summary>
	///
	/// <remarks>	杨阳, 2020/5/26. </remarks>
	///
	/// <param name="cb">	  	回调函数. </param>
	/// <param name="stub">	  	回调存根. </param>
	/// <param name="stub">	  	指定回调是否是异步回调线程，否则是数据接收线程 </param>
	/// <returns>	回调存根.该存根用于取消注册 </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicCallbackStubType RegisterTopicMatchedCallback(TSTOPICHANDLE THandle, TSTopicMatchedCallbackType cb, bool AsyncCallbackThread = true);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	注册 离线提醒回调. </summary>
	///
	/// <remarks>	杨阳, 2020/5/26. </remarks>
	///
	/// <param name="cb">	  	回调函数. </param>
	/// <param name="stub">	  	回调存根. </param>
	/// <param name="stub">	  	指定回调是否是异步回调线程，否则是数据接收线程 </param>
	/// <returns>	回调存根.该存根用于取消注册 </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicCallbackStubType RegisterLeaveDomainCallback(TSTOPICHANDLE THandle, TSTopicMatchedCallbackType cb, bool AsyncCallbackThread = true);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置内容过滤回调. </summary>
	///
	/// <remarks>	杨阳, 2020/5/26. </remarks>
	///
	/// <param name="stub">	  	回调. </param>
	/// <returns>	上一个回调 </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSDataContentFilterCallbackType SetContentFilterCallback(TSDataContentFilterCallbackType Callback);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	重置内容过滤回调. </summary>
	///
	/// <remarks>	杨阳, 2020/5/26. </remarks>
	///
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void ResetContentFilterCallback();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	采集数据. </summary>
    ///
    /// <remarks>	杨阳, 2020/5/26. </remarks>
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void CollectTopicData(TSITopicContextPtr Ctx);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	采集数据. </summary>
    ///
    /// <remarks>	杨阳, 2020/5/26. </remarks>
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void CollectTopicData(TSTOPICHANDLE TopicId,const TSInterObject * Obj,const TSTime & Timestamp);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取域Qos </summary>
	///
	/// <remarks>	任喜发, 2021/11/3. </remarks>
	///
	/// <param name="participantQosPolicy">	struct ParticipantQosPolicy. </param>
	///
	/// <returns>	成功-true，失败-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetDomainQosPolicy(ParticipantQosPolicy & participantQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置域Qos </summary>
	///
	/// <remarks>	任喜发, 2021/11/3. </remarks>
	///
	/// <param name="participantQosPolicy">	struct ParticipantQosPolicy. </param>
	///
	/// <returns>	成功-true，失败-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool SetDomainQosPolicy(const ParticipantQosPolicy & participantQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取订阅者Qos </summary>
	///
	/// <remarks>	任喜发, 2021/11/3. </remarks>
	///
	/// <param name="tHandle">	主题句柄 </param>
	/// <param name="dataReaderQosPolicy">	struct dataReaderQosPolicy. </param>
	///
	/// <returns>	成功-true，失败-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetSubscribeQos(TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置订阅者Qos </summary>
	///
	/// <remarks>	任喜发, 2021/11/3. </remarks>
	///
	/// <param name="tHandle">	主题句柄 </param>
	/// <param name="dataReaderQosPolicy">	struct dataReaderQosPolicy. </param>
	///
	/// <returns>	成功-true，失败-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool SetSubscribeQos(TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取发布者Qos </summary>
	///
	/// <remarks>	任喜发, 2021/11/3. </remarks>
	///
	/// <param name="tHandle">	主题句柄 </param>
	/// <param name="dataWriterQosPolicy">	struct dataWriterQosPolicy. </param>
	///
	/// <returns>	成功-true，失败-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetPublishQos(TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	设置发布者Qos </summary>
	///
	/// <remarks>	任喜发, 2021/11/3. </remarks>
	///
	/// <param name="tHandle">	主题句柄 </param>
	/// <param name="dataWriterQosPolicy">	struct dataWriterQosPolicy. </param>
	///
	/// <returns>	成功-true，失败-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool SetPublishQos(TSTOPICHANDLE tHandle, const DataWriterQosPolicy & dataWriterQosPolicy);

private:
	bool         WaitForDiscoveryComplete(const TSTopicQuerySet & QuerySet,const TSFrontAppUniquelyId & AppId,UINT32 Timeout,TSWaitSet::WaitMode Mode);
	void         Cleanup();
	void         ClearDomainData();
	bool         Initialize();
	void         OnReceiveTopic(TSTopicContextPtr Ctx);
	void         OnTopicRemoved(TSTopicContextPtr Ctx);
	void         OnRecieveTopicCallback(TSTOPICHANDLE TopicHandle,TSTopicContextPtr Ctx);
	void		 OnReceiveTopicMatched(TSDomainPtr Domain, const TSDomainId& DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub);
	void		 OnReceiveLeaveDomain(TSDomainPtr Domain, const TSDomainId& DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub);
    void         CollectData(TSITopicContextPtr Ctx);
	bool         OnContentFilterCallback(TSDomainPtr Domain, TSTopicContextPtr Ctx, UINT32 AppId);
	/* 判断主题是否为不缓存的主题 */
	bool		 IsNoCacheTopicHandle(TSTOPICHANDLE topic);  
};

#endif


