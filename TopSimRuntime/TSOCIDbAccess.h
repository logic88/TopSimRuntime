#ifndef __TSDATAINTERACTMANAGER_H__
#define __TSDATAINTERACTMANAGER_H__

#include "TopSimDataInterface.h"
#include "TSDomain.h"
#include "TSTopicTypes.h"
#include "TSIMonitor.h"
#include "TSPSMatcher.h"
#include "TSIDataDelegate.h"

#define LOGM_GENERATE_MESSAGE(Catalog,Msg) TSDataInteractManager::Instance()->OutputLogger(Catalog,Msg);

#define LOGM_ERROR(Msg)                       LOGM_GENERATE_MESSAGE(1,Msg)
#define LOGM_WARNNING(Msg)                    LOGM_GENERATE_MESSAGE(2,Msg)
#define LOGM_TRACE(Msg)                       LOGM_GENERATE_MESSAGE(3,Msg)
#define LOGM_KERNEL(Msg)                      LOGM_GENERATE_MESSAGE(4,Msg)
#define LOGM_RPC_SEND_REQUEST(Msg)            LOGM_GENERATE_MESSAGE(6,Msg)
#define LOGM_RPC_SEND_RESPONSE(Msg)           LOGM_GENERATE_MESSAGE(7,Msg)
#define LOGM_RPC_RECV_REQUEST(Msg)            LOGM_GENERATE_MESSAGE(8,Msg)
#define LOGM_RPC_RECV_RESPONSE(Msg)           LOGM_GENERATE_MESSAGE(9,Msg)
#define LOGM_RPC_SERVER_INIT(Msg)             LOGM_GENERATE_MESSAGE(10,Msg)
#define LOGM_RPC_SERVER_CLEAN(Msg)            LOGM_GENERATE_MESSAGE(11,Msg)
#define LOGM_RPC_SERVER_INACTIVATION(Msg)     LOGM_GENERATE_MESSAGE(12,Msg)
#define LOGM_RPC_RECV_PROXY_CONNECTION(Msg)   LOGM_GENERATE_MESSAGE(13,Msg)
#define LOGM_RPC_PROXY_INIT(Msg)              LOGM_GENERATE_MESSAGE(14,Msg)
#define LOGM_RPC_PROXY_CLEANUP(Msg)           LOGM_GENERATE_MESSAGE(15,Msg)
#define LOGM_RPC_PROXY_CONNECTING(Msg)        LOGM_GENERATE_MESSAGE(16,Msg)
#define LOGM_RPC_PROXY_CONNECTED(Msg)         LOGM_GENERATE_MESSAGE(17,Msg)
#define LOGM_RPC_PROXY_INACTIVATION(Msg)      LOGM_GENERATE_MESSAGE(18,Msg)

TOPSIMDATAINTERFACE_DECL TSFrontAppUniquelyId BuildFromString(const TSString& AppId);

struct TSDataInteractManagerPrivate;
struct ParticipantQosPolicy;
struct DataReaderQosPolicy;
struct DataWriterQosPolicy;

/*
  ���ݽӿڹ�����(����)
*/
class TOPSIMDATAINTERFACE_DECL TSDataInteractManager : public TSSingleton<TSDataInteractManager>
{
	friend class TSDomain;

    SINGLETON_DECLARE(TSDataInteractManager);
    TS_DECLARE_PRIVATE(TSDataInteractManager);
private:
    TSDataInteractManager();
    PrivateType * _Private_Ptr;

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	ʹ�������ļ���ʼ��DataManager. </summary>
	///
	/// <remarks>	������, 2017/11/9. </remarks>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool InitializeWithConfigure(int argc,char ** argv,const TSString& ConfigureFileName = "",const TSFrontAppUniquelyId& FrontAppId = TSFrontAppUniquelyId());
	bool InitializeWithConfigure(const TSString& ConfigureFileName = "", const TSFrontAppUniquelyId& FrontAppId = TSFrontAppUniquelyId());

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�Ƿ��Ѿ�����ʼ��. </summary>
	///
	/// <remarks>	������, 2017/11/9. </remarks>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool WasInitialize();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	Initializes this object. </summary>
    ///
    /// <remarks>	������, 2017/6/27. </remarks>
    ///
    /// <param name="argc">		 	The argc. </param>
    /// <param name="argv">		 	[in,out] If non-null, the argv. </param>
    /// <param name="FrontAppId">	Identifier for the front application. </param>
    ///
    /// <returns>	true if it succeeds, false if it fails. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Initialize(int argc,char ** argv,const TSFrontAppUniquelyId& FrontAppId = TSFrontAppUniquelyId());
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Initializes this object. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <param name="FrontAppId">	Identifier for the front application. </param>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Initialize(const TSFrontAppUniquelyId& FrontAppId = TSFrontAppUniquelyId());

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Creates a domain. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <param name="Id">	The identifier. </param>
	///
	/// <returns>	. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	TSDomainPtr CreateDomain(const TSDomainIdOrPartName& Id);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Destory domain. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <param name="Domain">	The domain. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void DestroyDomain(TSDomainPtr Domain);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets a domain. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <param name="Id">	The identifier. </param>
	///
	/// <returns>	The domain. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	TSDomainPtr GetDomain(const TSDomainId& Id);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Cleanups this object. </summary>
	///
	/// <param name="Id">	��Id��Ϊ�գ�����Id�������.��Ϊ�գ������������ݣ��������봫���� </param>
	/// 
	/// <remarks>	������, 2017/6/27. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void Cleanup(const TSDomainId& Id = "");

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets the front application uniquely identifier. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <returns>	The front application uniquely identifier. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	TSFrontAppUniquelyId GetFrontAppUniquelyId();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Sets an application name. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <param name="AppName">	Name of the application. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetAppName(const TSString & AppName);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets the application name. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <returns>	The application name. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	const TSString& GetAppName();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Sets an application author. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <param name="AppAuthor">	The application author. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetAppAuthor(const TSString & AppAuthor);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets the application author. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <returns>	The application author. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	const TSString& GetAppAuthor();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Sets an application version. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <param name="AppVersion">	The application version. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetAppVersion(const TSString & AppVersion);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets the application version. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <returns>	The application version. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	const TSString& GetAppVersion();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Sets an application description. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <param name="Description">	The description. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetAppDescription(const TSString & Description);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets the application description. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <returns>	The application description. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	const TSString& GetAppDescription();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Sets a user data. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <param name="UserData">	[in,out] Information describing the user. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetUserData(TSVariant & UserData);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets the user data. </summary>
	///
	/// <remarks>	������, 2017/6/27. </remarks>
	///
	/// <returns>	The user data. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	const TSVariant & GetUserData();

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
	/// <returns>	The  flag. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	UINT64 GetUserFlag();

public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	�������С� </summary>
    /// 
    /// ���ô˺�����ʹ��������������ģʽ���У�ֱ��Stop����������.
    ///	
    /// <remarks>	������, 2016/12/29. </remarks>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void EvokeCallbacksAlways();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	���������С� </summary>
    /// 
    /// ���ô˺�����ʹ���������Է�����ģʽ���У�ֱ��Stop����������.
    ///	
    /// <remarks>	������, 2016/12/29. </remarks>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void EvokeCallbacksAlwaysNoBlock();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	�������У�ֱ����ǰû�лص�ִ�У�Ȼ�󷵻ء�</summary>
    /// 
    ///	
    /// <remarks>	������, 2016/12/29. </remarks>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void EvokeCallbacks();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	�������У����ִ��һ���ص��������ǰû�лص���ص�ִ����ϣ��򷵻�</summary>
    /// 
    ///	
    /// <remarks>	������, 2016/12/29. </remarks>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void EvokeCallback();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>	ֹͣ����. </summary>
    ///
    /// <remarks>	������, 2016/12/29. </remarks>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void Stop();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	����������. </summary>
	///
	/// <remarks>	������, 2016/12/29. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void CleanDomainData(const TSDomainId & DomainId);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Log���. </summary>
	///
	/// <remarks>	������, 2016/12/29. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void OutputLogger(UINT32 Catalog,const TSString & LoggerMsg);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�����û�Qos���������� </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="Id">	(����)��ID. </param>
	/// <param name="participantQosPolicy">	struct ParticipantQosPolicy. </param>
	///
	/// <returns>	. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSDomainPtr CreateDomainWithQos(const TSDomainIdOrPartName& Id, const ParticipantQosPolicy & participantQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ������Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="domainId">	��ID </param>
	/// <param name="participantQosPolicy">	struct ParticipantQosPolicy. </param>
	///
	/// <returns>	. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetParticipantQosPolicy(const TSDomainId& domainId, ParticipantQosPolicy & participantQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	���ò�����Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="domainId">	��ID </param>
	/// <param name="participantQosPolicy">	struct ParticipantQosPolicy. </param>
	///
	/// <returns>	. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool SetParticipantQosPolicy(const TSDomainId& domainId, const ParticipantQosPolicy & participantQosPolicy);
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ���ݶ���Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="domainId">	��ID </param>
	/// <param name="tHandle">	������ </param>
	/// <param name="dataReaderQosPolicy">	struct dataReaderQosPolicy. </param>
	///
	/// <returns>	. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetDataReaderQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, DataReaderQosPolicy & dataReaderQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�������ݶ���Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="domainId">	��ID </param>
	/// <param name="tHandle">	������ </param>
	/// <param name="dataReaderQosPolicy">	struct dataReaderQosPolicy. </param>
	///
	/// <returns>	. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool SetDataReaderQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, const DataReaderQosPolicy & dataReaderQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ����д��Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="domainId">	��ID </param>
	/// <param name="tHandle">	������ </param>
	/// <param name="dataWriterQosPolicy">	struct dataWriterQosPolicy. </param>
	///
	/// <returns>	. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetDataWriterQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, DataWriterQosPolicy & dataWriterQosPolicy);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��������д��Qos </summary>
	///
	/// <remarks>	��ϲ��, 2021/11/3. </remarks>
	///
	/// <param name="domainId">	��ID </param>
	/// <param name="tHandle">	������ </param>
	/// <param name="dataWriterQosPolicy">	struct dataWriterQosPolicy. </param>
	///
	/// <returns>	. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool SetDataWriterQos(const TSDomainId& domainId, TSTOPICHANDLE tHandle, const DataWriterQosPolicy & dataWriterQosPolicy);
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ��������б� </summary>
	///
	/// <remarks>	wyg. </remarks>

	/// <param name="domainIDVec">	���б�Vector. </param>
	///
	/// <returns>	. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT8 GetJoinedDomains(TSStringArray& domainIDVec);


private:
	 bool WaitForDiscoveryComplete(const TSDomainId & DomainId, const TSTopicQuerySet & QuerySet, const TSFrontAppUniquelyId & AppId, UINT32 Timeout, TSWaitSet::WaitMode Mode);
	 TSTopicContextPtr GetFirstTopicByHandle(const TSDomainId & DomainId, TSTOPICHANDLE THandle);
	 TSTopicContextPtr GetFirstTopicByHandle(const TSDomainId & DomainId, TSHANDLE Handle, TSTOPICHANDLE THandle);
	 TopSimDataInterface::ReturnCode Subscribe(const TSDomainId& DomainId, TSTOPICHANDLE THandle, bool LocalOnly = false);
	 TopSimDataInterface::ReturnCode Publish(const TSDomainId& DomainId, TSTOPICHANDLE THandle);
	 TopSimDataInterface::ReturnCode Unsubscribe(const TSDomainId& DomainId, TSTOPICHANDLE THandle);
	 TopSimDataInterface::ReturnCode Unpublish(const TSDomainId& DomainId, TSTOPICHANDLE THandle);
	 TSTopicContextPtr UpdateTopic(const TSDomainId& DomainId, TSTOPICHANDLE THandle,
		const void * Data,const TSTime & Time, bool IsNeedSerializer);
	 bool DeleteTopic(const TSDomainId & DomainId, TSTopicContextPtr Ctx, bool IsNotice);
	 bool DeleteTopic(const TSDomainId & DomainId, TSTOPICHANDLE TopicHandle, void * KeyData, bool IsNotice);
	 TSTopicContextPtr SendData(const TSDomainId& DomainId, TSTOPICHANDLE THandle,
		const void * Parameter,const TSTime & Time,const TSFrontAppUniquelyId& ReceiverId = TSFrontAppUniquelyId());
	 TSTopicContextPtr SendBinary(const TSDomainId& DomainId,TSTOPICHANDLE THandle,
        TSDataContextPtr DataCtx,const TSTime & Time,const TSFrontAppUniquelyId& Receiver);
	void Post(TSDomainRoutine_T Routine);

	 TSTopicFindSetType CreateTopicFindSet(TSHANDLE Handle, const TSTopicQuerySet & Query, const TSDomainId & DomainId);
	 TSTopicFindSetType CreateTopicFindSet(const TSTopicQuerySet & Query, const TSDomainId & DomainId);
	 TSTopicFindSetType CreateTopicFindSet(DOUBLE CenterLon, DOUBLE CenterLat, FLOAT SpacialRange, const TSTopicQuerySet & Query, const TSDomainId & DomainId);

	 TSTopicContextPtr GetFirstObject(TSTopicFindSetType FindSet);
	 TSTopicContextPtr GetNextObject(TSTopicFindSetType FindSet);

private:
	bool Configure(int argc, char ** argv);
	void OnDomainReceivedTopic_p(TSDomainPtr Domain, TSTopicContextPtr Ctx);
	void OnDomainReceivedTopicRemoved_p(TSDomainPtr Domain, TSTopicContextPtr Ctx);
	void OnDomainReceivedTopicMatched_p(TSDomainPtr Domain, const TSDomainId& DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub);
	void OnDomainReceivedLeaveDomain_p(TSDomainPtr Domain, const TSDomainId& DomainId, TSTOPICHANDLE Thandle, UINT32 AppId, bool IsPub);
	bool OnDomainContentFilter_p(TSDomainPtr Domain, TSTopicContextPtr Ctx, UINT32 AppId);
};

#define DataManager TSDataInteractManager::Instance()
#define InitializeWithConfigures(argc, argv,configureName) DataManager->InitializeWithConfigure(argc, argv,configureName)

#endif