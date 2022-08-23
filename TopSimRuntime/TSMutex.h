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
    /// <summary>   设置域. </summary>
    ///
    /// <remarks>    李子洋, 2019/1/10. </remarks>
    ///
    /// <param name="domainptr"> 域指针. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetDomain(TSDomainPtr domainptr);
   
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   设置发送的包大小. </summary>
    ///
    /// <remarks>   李子洋, 2019/1/10. </remarks>
    ///
    /// <param name="pieceSize">包大小. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetPieceSize(const UINT32 & pieceSize);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   开始建立发送对象信息. </summary>
    ///
    /// <remarks>   李子洋, 2019/2/26. </remarks>
    ///
    /// <returns>   . </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    TSTransfersInfoPtr BeginBuild(const TSFrontAppUniquelyId & AppId);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   同步发送. </summary>
    ///
    /// <remarks>   李子洋, 2019/2/12. </remarks>
    ///
    /// <param name="StreamData">  流数据指针. </param>
    /// <param name="StreamSize">  数据长度. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool EndBuild(TSTransfersInfoPtr TsTansfers);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   发送数据伴随流ID信息. </summary>
    ///
    /// <remarks>   李子洋, 2019/2/26. </remarks>
    ///
    /// <param name="StreamData">   流数据指针. </param>
    /// <param name="StreamSize">   数据长度. </param>
    /// <param name="Tansfers">     流ID信息. </param>
    ///
    /// <returns>   true if it succeeds, false if it fails. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Send(char* StreamData,const UINT32 & StreamSize,TSTransfersInfoPtr Tansfers);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   异步发送. </summary>
    ///
    /// <remarks>   李子洋, 2019/2/22. </remarks>
    ///
    /// <param name="StreamData">   流数据指针. </param>
    /// <param name="StreamSize">   流数据长度. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void SendAsync(char* StreamData,const UINT32 & StreamSize,TSTransfersInfoPtr Tansfers);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   数据接收. </summary>
    ///
    /// <remarks>   李子洋, 2019/2/27. </remarks>
    ///
    /// <param name="Data">             数据指针. </param>
    /// <param name="Data_Count">       数据长度. </param>
    /// <param name="TransfersInfo">    数据信息. </param>
    ///
    /// <returns>   . </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    UINT32 Recevie(char* Data,const UINT32 & Data_Count,TSRecevieInfoPtr TransfersInfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   异步接收. </summary>
    ///
    /// <remarks>   李子洋, 2019/2/22. </remarks>
    ///
    /// <param name="Data">         内存指针. </param>
    /// <param name="Data_Count">   内存大小. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void RecevieAsync(char* Data,const UINT32 & Data_Count);
private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   获取文件主题回调,该回调将接收到的数据写到分配的内存中. </summary>
    ///
    /// <remarks>   李子洋, 2019/1/10. </remarks>
    ///
    /// <param name="Ctx">  文件数据. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void Topic_StreamToGetData(TSTopicContextPtr Ctx);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   发送流数据. </summary>
    ///
    /// <remarks>   李子洋, 2019/2/13. </remarks>
    ///
    /// <param name="StreamData">   数据流指针. </param>
    /// <param name="StreamSize">   发送数据长度. </param>
    ///
    /// <returns>   true if it succeeds, false if it fails. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool SendStreamDataInfo(char* StreamData,const UINT32 & StreamSize,TSTransfersInfoPtr Tansfers);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   . </summary>
    ///
    /// <remarks>   李子洋, 2019/2/26. </remarks>
    ///
    /// <param name="TransfersInfo">    传输对象信息. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void InsertIdMap(TSTransfersInfoPtr TransfersInfo);
};
CLASS_PTR_DECLARE(TopSimStreamTransfers);
#endif