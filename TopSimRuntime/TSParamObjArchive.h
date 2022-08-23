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

//���յ�����ص�����
typedef boost::function<void(TSTopicContextPtr Ctx)> TSTopicCallbackType;
//����ƥ��ص�����
typedef boost::function<void(TSDomainId DomainId, TSTOPICHANDLE THandle, UINT32 AppId, bool IsPub)> TSTopicMatchedCallbackType;
//������˻ص�����
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

/* ��˽�г�Ա�����ṹ */
struct TSDomainPrivate;
/*
  ��
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
	/// <summary>	�ж�������Ƿ񱻳�ʼ��. </summary>
	///
	/// <remarks>	������, 2017/8/25. </remarks>
	///
	/// <returns>	true if Initialized. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool WasInitialized();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets the domain identifier. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <returns>	The domain identifier. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const TSDomainId& GetDomainId();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���ط����࣬����ʹ��RegisterAppע����Ҫ�����ֵ�Ӧ��. </summary>
	///
	/// <remarks>	������, 2017/6/30. </remarks>
	///
	/// <returns>	The discovery. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSDiscoveryPtr GetDiscovery();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	��������. </summary>
    ///
    /// <remarks>	������, 2016/12/29. luxiang,20180428</remarks>
    ///
    /// <param name="Handle">   	������. </param>
    /// <param name="LocalOnly">   	true,ֻ���ı�������. </param>
    ///
    /// <returns>	ö�� 0-�ɹ���1-ʧ�ܣ�2-�˳� </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TopSimDataInterface::ReturnCode SubscribeTopic(TSTOPICHANDLE Handle, bool LocalOnly = false);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	��������. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="Handle">   	������. </param>
    ///
    /// <returns>	ö�� 0-�ɹ���1-ʧ�ܣ�2-�˳� </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TopSimDataInterface::ReturnCode PublishTopic(TSTOPICHANDLE Handle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	ȡ������. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="Handle">   ������. </param>
	///
	/// <returns>	ö�� 0-�ɹ���1-ʧ�ܣ�2-�˳� </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TopSimDataInterface::ReturnCode UnsubscribeTopic(TSTOPICHANDLE Handle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	ȡ������. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="Handle">   	������. </param>
	///
	/// <returns>	ö�� 0-�ɹ���1-ʧ�ܣ�2-�˳� </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TopSimDataInterface::ReturnCode UnpublishTopic(TSTOPICHANDLE Handle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	�Ƿ�������Ķ�����. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="THandle">	������. </param>
    ///
    /// <returns>	�Ƕ����߷���true�����򷵻�false. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool IsTopicSubscriber(TSTOPICHANDLE THandle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	�Ƿ�������ķ�����. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="THandle">	������. </param>
    ///
    /// <returns>	�Ƿ����߷���true�����򷵻�false. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool IsTopicPublisher(TSTOPICHANDLE THandle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	������������. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="Handle">   	������. </param>
    /// <param name="Data">			��������ָ��. </param>
    ///
    /// <returns>	��������ʵ�������ͨ���þ���ɲ��ҵ���Ӧ������ʵ��. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicContextPtr UpdateTopic(TSTOPICHANDLE Handle,const TSInterObject * Data);


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	ɾ����������. </summary>
	///
	/// <remarks>	������, 2016/12/29. </remarks>
	///
	/// <param name="Handle">  ��������������. </param>
	///
	/// <returns>	���������Ƿ�ɾ���ɹ�. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool DeleteTopic(TSTopicContextPtr Ctx,bool IsNotice);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	ɾ����������. </summary>
	///
	/// <remarks>	������, 2016/12/29. </remarks>
	///
	/// <param name="Handle">  ����ID. </param>
	/// <param name="KeyData">  ����Key����������. </param>
	/// <returns>	���������Ƿ�ɾ���ɹ�. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool DeleteTopic(TSTOPICHANDLE TopicHandle,const TSInterObject * KeyData,bool IsNotice);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�����������⡣������շ�ΪTS_INVALID����ֻ�����ݵĶ��������յ��� </summary>
	///
	/// <remarks>	������, 2016/12/29. </remarks>
	///
	/// <param name="THandle">   	������. </param>
	/// <param name="Parameter"> 	��������ָ��. </param>
	/// <param name="ReceiverId">	���շ�Ψһ��ʶ. </param>
	///
	/// <returns>	���������� </returns>
	///
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicContextPtr SendTopicDirect(TSTOPICHANDLE THandle,const TSInterObject * Data,
		const TSFrontAppUniquelyId& ReceiverId);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	���Ͷ��������ݡ� </summary>
    ///
    /// <remarks>	luxiang, 2018/02/10. </remarks>
    ///
    /// <param name="THandle">  ������. </param>
    /// <param name="Data"> 	��������ָ��. </param>
    /// <param name="Length">	���ݳ���. </param>
	///
	/// <returns>	���������� </returns>
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicContextPtr SendBinary(TSTOPICHANDLE THandle,const void * Data,
        const UINT32 Length);

	TSTopicContextPtr SendBinaryDirect(TSTOPICHANDLE THandle,const void * Data,
		const UINT32 Length,const TSFrontAppUniquelyId & Receiver);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���Ͷ��������ݡ� </summary>
	///
	/// <remarks>	luxiang, 2018/02/10. </remarks>
	///
	/// <param name="THandle">  ������. </param>
	/// <param name="DataBuffer"> 	��������ָ��. </param>
	///
	/// <returns>	���������� </returns>
	///
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicContextPtr SendBinary(TSTOPICHANDLE THandle,TSByteBufferPtr DataBuffer);
	TSTopicContextPtr SendBinaryDirect(TSTOPICHANDLE THandle,TSByteBufferPtr DataBuffer,const TSFrontAppUniquelyId & Receiver);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���Ͷ��������ݡ� </summary>
	///
	/// <remarks>	luxiang, 2018/02/10. </remarks>
	///
	/// <param name="THandle">  ������. </param>
	/// <param name="DataContext"> 	��������ָ��. </param>
	///
	/// <returns>	���������� </returns>
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
	/// <summary>	ͨ�������Query�������Ҽ�. </summary>
	///
	/// <remarks>	������, 2016/12/29. </remarks>
	///
	/// <param name="Handle">	���. </param>
	/// <param name="THandle">	��������Query. </param>
	/// <returns>	���Ҽ�. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicFindSetType CreateTopicFindSet(TSHANDLE Handle,const TSTopicQuerySet & Query);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	ͨ��������������������Ҽ�. </summary>
	///
	/// <remarks>	������, 2016/12/29. </remarks>
	///
	/// <param name="Handle">	���. </param>
	/// <param name="THandle">	��������Query. </param>
	/// <returns>	���Ҽ�. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicFindSetType CreateTopicFindSet(TSHANDLE Handle,TSTOPICHANDLE Topic);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	ͨ���������������Ҽ�. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="THandle">	������. </param>
    ///
    /// <returns>	���Ҽ�. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicFindSetType CreateTopicFindSet(TSTOPICHANDLE THandle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	��������λ�ù��˵Ĳ��Ҽ�.�ò��Ҽ�ֻ�Ծ���kSpacialRangeType���Ա�ʶ�ġ�����ΪTSVector3d����������
	///             ���Դ洢��Ϊ��γ����Ϣ�����ȣ���</summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="CenterLon">	���ĵ㾭��. </param>
    /// <param name="CenterLat">	���ĵ�γ��. </param>
    /// <param name="SpacialRange">	�뾶. </param>
    /// <returns>	���Ҽ�. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicFindSetType CreateTopicFindSet(DOUBLE CenterLon,DOUBLE CenterLat,FLOAT SpacialRange,TSTOPICHANDLE Topic);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��������λ�ù��˵Ĳ��Ҽ�.�ò��Ҽ�ֻ�Ծ���kSpacialRangeType���Ա�ʶ�ġ�����ΪTSVector3d����������
	///             ���Դ洢��Ϊ��γ����Ϣ�����ȣ���</summary>
	///
	/// <remarks>	������, 2016/12/29. </remarks>
	///
	/// <param name="CenterLon">	���ĵ㾭��. </param>
	/// <param name="CenterLat">	���ĵ�γ��. </param>
	/// <param name="SpacialRange">	�뾶. </param>
	/// <returns>	���Ҽ�. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicFindSetType CreateTopicFindSet(DOUBLE CenterLon,DOUBLE CenterLat,FLOAT SpacialRange,const TSTopicQuerySet & Query);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	��������ʵ�����Ĳ��Ҽ�. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="TSHANDLE">	ʵ����. </param>
    /// <returns>	���Ҽ�. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicFindSetType CreateTopicFindSet(TSHANDLE HKey);

	////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	��ȡ��һ��ʵ�����. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="FindSet">	���Ҽ�. </param>
    /// <param name="IHandle">	�����ʵ�����. </param>
	///
	/// <returns>	���������� </returns>
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
	/// <summary>	��ȡʱ��ί��. </summary>
	///
	/// <remarks>	������, 2017/2/17. </remarks>
	///
	/// <returns>	The time delegate. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSAbstractTimeHolderPtr  GetTimeHolder();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	����ʱ��ί��. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <param name="TimeHolder">	The time holder. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetTimeHolder( TSAbstractTimeHolderPtr TimeHolder);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�����û���ʶ. </summary>
	///
	/// <remarks>	������, 2017/7/7. </remarks>
	///
	/// <param name="Flag">	The flag. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetUserFlag(UINT64 Flag);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ�û���ʶ. </summary>
	///
	/// <remarks>	������, 2017/7/7. </remarks>
	///
	/// <returns>	The user flag. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT64 GetUserFlag();

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	ע��/ȡ��ע���������ݻص�. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ///
    /// <param name="THandle">	������. </param>
    /// <param name="cb">	  	�ص�����. </param>
    /// <param name="stub">	  	�ص����. </param>
    /// <param name="stub">	  	ָ���ص��Ƿ����첽�ص��̣߳����������ݽ����߳� </param>
    /// <returns>	�ص����.�ô������ȡ��ע�� </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    TSTopicCallbackStubType RegisterTopicCallback(TSTOPICHANDLE THandle,TSTopicCallbackType cb,bool AsyncCallbackThread = true);
    void   UnregisterTopicCallback(TSTOPICHANDLE THandle,UINT32 stub);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	ע��/ȡ��ע���������ݻص�. </summary>
	///
	/// <remarks>	������, 2016/12/29. </remarks>
	///
	/// <param name="THandle">	������. </param>
	/// <param name="cb">	  	�ص�����. </param>
	/// <param name="stub">	  	�ص����. </param>
	/// <param name="stub">	  	ָ���ص��Ƿ����첽�ص��̣߳����������ݽ����߳� </param>
	/// <returns>	�ص����.�ô������ȡ��ע�� </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool RegisterTopicListener(TSITopicListener * Listener);
	void UnregisterTopicListener(TSITopicListener * Listener);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Post. </summary>
	///
	/// <param name="stub">	  	��һ���ص����󣬸�����������ݹ������Ļص��߳��б�����. </param>
	/// <remarks>	������, 2017/8/23. </remarks>
	//////////////oo//////////////////////////////////////////////////////////////////////////////////////
	void Post(TSDomainRoutine_T Routine);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	���ö����Ʋɼ�������ļ�Ŀ¼.�ýӿڿ��Զ�ε��ã�ÿ�ε���ʱ���Ὣ��һ�εĲɼ��ļ��رգ������¿����µ����ݲɼ� </summary>
    ///
    /// <param name="Path">	  	�ɼ�·��. </param>
    /// <remarks>	luxiang, 2018/05/02. </remarks>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void SetOutputPath(const TSString & OutPath);

	////////////////////////////////////////////////////////////////////////////////////////////////////
  /// <summary>	��ȡ�����Ʋɼ�������ļ�Ŀ¼. </summary>
  ///
  /// <param name="Path">	  	�ɼ�·��. </param>
  /// <remarks>	luxiang, 2018/05/02. </remarks>
  ////////////////////////////////////////////////////////////////////////////////////////////////////
	const TSString & GetOutputPath();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	ע�� �������ѻص�. </summary>
	///
	/// <remarks>	����, 2020/5/26. </remarks>
	///
	/// <param name="cb">	  	�ص�����. </param>
	/// <param name="stub">	  	�ص����. </param>
	/// <param name="stub">	  	ָ���ص��Ƿ����첽�ص��̣߳����������ݽ����߳� </param>
	/// <returns>	�ص����.�ô������ȡ��ע�� </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicCallbackStubType RegisterTopicMatchedCallback(TSTOPICHANDLE THandle, TSTopicMatchedCallbackType cb, bool AsyncCallbackThread = true);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	ע�� �������ѻص�. </summary>
	///
	/// <remarks>	����, 2020/5/26. </remarks>
	///
	/// <param name="cb">	  	�ص�����. </param>
	/// <param name="stub">	  	�ص����. </param>
	/// <param name="stub">	  	ָ���ص��Ƿ����첽�ص��̣߳����������ݽ����߳� </param>
	/// <returns>	�ص����.�ô������ȡ��ע�� </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSTopicCallbackStubType RegisterLeaveDomainCallback(TSTOPICHANDLE THandle, TSTopicMatchedCallbackType cb, bool AsyncCallbackThread = true);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�������ݹ��˻ص�. </summary>
	///
	/// <remarks>	����, 2020/5/26. </remarks>
	///
	/// <param name="stub">	  	�ص�. </param>
	/// <returns>	��һ���ص� </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSDataContentFilterCallbackType SetContentFilterCallback(TSDataContentFilterCallbackType Callback);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�������ݹ��˻ص�. </summary>
	///
	/// <remarks>	����, 2020/5/26. </remarks>
	///
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void ResetContentFilterCallback();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	�ɼ�����. </summary>
    ///
    /// <remarks>	����, 2020/5/26. </remarks>
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void CollectTopicData(TSITopicContextPtr Ctx);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	�ɼ�����. </summary>
    ///
    /// <remarks>	����, 2020/5/26. </remarks>
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void CollectTopicData(TSTOPICHANDLE TopicId,const TSInterObject * Obj,const TSTime & Timestamp);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ��Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="participantQosPolicy">	struct ParticipantQosPolicy. </param>
	///
	/// <returns>	�ɹ�-true��ʧ��-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetDomainQosPolicy(ParticipantQosPolicy & participantQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	������Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="participantQosPolicy">	struct ParticipantQosPolicy. </param>
	///
	/// <returns>	�ɹ�-true��ʧ��-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool SetDomainQosPolicy(const ParticipantQosPolicy & participantQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ������Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="tHandle">	������ </param>
	/// <param name="dataReaderQosPolicy">	struct dataReaderQosPolicy. </param>
	///
	/// <returns>	�ɹ�-true��ʧ��-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetSubscribeQos(TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���ö�����Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="tHandle">	������ </param>
	/// <param name="dataReaderQosPolicy">	struct dataReaderQosPolicy. </param>
	///
	/// <returns>	�ɹ�-true��ʧ��-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool SetSubscribeQos(TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ������Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="tHandle">	������ </param>
	/// <param name="dataWriterQosPolicy">	struct dataWriterQosPolicy. </param>
	///
	/// <returns>	�ɹ�-true��ʧ��-false </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetPublishQos(TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���÷�����Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="tHandle">	������ </param>
	/// <param name="dataWriterQosPolicy">	struct dataWriterQosPolicy. </param>
	///
	/// <returns>	�ɹ�-true��ʧ��-false </returns>
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
	/* �ж������Ƿ�Ϊ����������� */
	bool		 IsNoCacheTopicHandle(TSTOPICHANDLE topic);  
};

#endif


