#ifndef __TOPSIMSTREAMTRANSFERS_H__
#define __TOPSIMSTREAMTRANSFERS_H__

#include <TopSimDataInterface/TopSimDataInterface.h>
#include <TopSimDataInterface/TSDomain.h>
#include "TSTransfersInfo.h"

struct TopSimStreamTransfersPrivate;
class TOPSIMDATAINTERFACE_DECL TopSimStreamTransfers
{
    TS_DECLARE_PRIVATE(TopSimStreamTransfers);
public:
    TopSimStreamTransfers();
    ~TopSimStreamTransfers();
protected:
    TSIOSericePtr GetIOSService();
    PrivateType * _Private_Ptr;
    TopSimStreamTransfers(PrivateType * p);
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   ������. </summary>
    ///
    /// <remarks>    ������, 2019/1/10. </remarks>
    ///
    /// <param name="domainptr"> ��ָ��. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetDomain(TSDomainPtr domainptr);
   
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   ���÷��͵İ���С. </summary>
    ///
    /// <remarks>   ������, 2019/1/10. </remarks>
    ///
    /// <param name="pieceSize">����С. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetPieceSize(const UINT32 & pieceSize);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   ��ʼ�������Ͷ�����Ϣ. </summary>
    ///
    /// <remarks>   ������, 2019/2/26. </remarks>
    ///
    /// <returns>   . </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    TSTransfersInfoPtr BeginBuild(const TSFrontAppUniquelyId & AppId);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   ͬ������. </summary>
    ///
    /// <remarks>   ������, 2019/2/12. </remarks>
    ///
    /// <param name="StreamData">  ������ָ��. </param>
    /// <param name="StreamSize">  ���ݳ���. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool EndBuild(TSTransfersInfoPtr TsTansfers);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   �������ݰ�����ID��Ϣ. </summary>
    ///
    /// <remarks>   ������, 2019/2/26. </remarks>
    ///
    /// <param name="StreamData">   ������ָ��. </param>
    /// <param name="StreamSize">   ���ݳ���. </param>
    /// <param name="Tansfers">     ��ID��Ϣ. </param>
    ///
    /// <returns>   true if it succeeds, false if it fails. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Send(char* StreamData,const UINT32 & StreamSize,TSTransfersInfoPtr Tansfers);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   �첽����. </summary>
    ///
    /// <remarks>   ������, 2019/2/22. </remarks>
    ///
    /// <param name="StreamData">   ������ָ��. </param>
    /// <param name="StreamSize">   �����ݳ���. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void SendAsync(char* StreamData,const UINT32 & StreamSize,TSTransfersInfoPtr Tansfers);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   ���ݽ���. </summary>
    ///
    /// <remarks>   ������, 2019/2/27. </remarks>
    ///
    /// <param name="Data">             ����ָ��. </param>
    /// <param name="Data_Count">       ���ݳ���. </param>
    /// <param name="TransfersInfo">    ������Ϣ. </param>
    ///
    /// <returns>   . </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    UINT32 Recevie(char* Data,const UINT32 & Data_Count,TSRecevieInfoPtr TransfersInfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   �첽����. </summary>
    ///
    /// <remarks>   ������, 2019/2/22. </remarks>
    ///
    /// <param name="Data">         �ڴ�ָ��. </param>
    /// <param name="Data_Count">   �ڴ��С. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void RecevieAsync(char* Data,const UINT32 & Data_Count);
private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   ��ȡ�ļ�����ص�,�ûص������յ�������д��������ڴ���. </summary>
    ///
    /// <remarks>   ������, 2019/1/10. </remarks>
    ///
    /// <param name="Ctx">  �ļ�����. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void Topic_StreamToGetData(TSTopicContextPtr Ctx);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   ����������. </summary>
    ///
    /// <remarks>   ������, 2019/2/13. </remarks>
    ///
    /// <param name="StreamData">   ������ָ��. </param>
    /// <param name="StreamSize">   �������ݳ���. </param>
    ///
    /// <returns>   true if it succeeds, false if it fails. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool SendStreamDataInfo(char* StreamData,const UINT32 & StreamSize,TSTransfersInfoPtr Tansfers);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   . </summary>
    ///
    /// <remarks>   ������, 2019/2/26. </remarks>
    ///
    /// <param name="TransfersInfo">    ���������Ϣ. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void InsertIdMap(TSTransfersInfoPtr TransfersInfo);
};
CLASS_PTR_DECLARE(TopSimStreamTransfers);
#endif