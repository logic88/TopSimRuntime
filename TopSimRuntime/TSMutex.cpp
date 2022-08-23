#include "stdafx.h"
#ifndef  _WRS_KERNEL
#include "TopSimDataInterface_LANG.h"
#include <boost/thread/lock_factories.hpp>
#include <TransfersInfo/Defined.h>
#include "TSDataInteractManager.h"
#include "TopSimStreamTransfers.h"
#include "TopSimStreamTransfers_p.h"

TopSimStreamTransfers::TopSimStreamTransfers():_Private_Ptr(new PrivateType)
{

}

TopSimStreamTransfers::TopSimStreamTransfers( PrivateType * p ):_Private_Ptr(p)
{

}

TSIOSericePtr TopSimStreamTransfers::GetIOSService()
{
	T_D();

    TSIOSericePtr IOS;

#if defined(XSIM3_3) || defined(XSIM3_2)
    if(!d->_NotifyIOS)
    {
        d->_NotifyIOS = boost::make_shared<TSIOSStruct>();
        d->_NotifyIOS->RunAlways();
    }
    IOS = d->_NotifyIOS->GetIOS();
#else
    IOS = TSAsyncIOService::Instance()->GetIOS();
#endif

    return IOS;
}


TopSimStreamTransfers::~TopSimStreamTransfers()
{

}

void TopSimStreamTransfers::SetDomain( TSDomainPtr domainptr )
{
    T_D();
    d->_domainptr = domainptr;
}

void TopSimStreamTransfers::SetPieceSize( const UINT32 & pieceSize )
{
    T_D();
    d->_piecesize = pieceSize;
}

bool TopSimStreamTransfers::Send( char* StreamData,const UINT32 & StreamSize,TSTransfersInfoPtr Tansfers)
{
    if(!SendStreamDataInfo(StreamData,StreamSize,Tansfers))
    {
        return false; 
    }

    return true;
}


UINT32 TopSimStreamTransfers::Recevie( char* Data,const UINT32 & Data_Count,TSRecevieInfoPtr RecevieInfo )
{
    T_D();
    d->_WirteDataFormemory = Data;
    d->_DataCount = Data_Count;
    d->_AsyncRecevice = false;
    d->_RecevieInfo = RecevieInfo;
    if (d->_IsSub && !d->_AsyncRecevice)
    {
        d->_IsStart = true;
        for (;d->_IsStart == true;)
        {
            //通知回调函数
            d->_condvarNotity.notify_one();  
        }
    }

    if (!d->_IsSub)
    {
        d->_domainptr->SubscribeTopic(Topic_Stream);
        d->_domainptr->RegisterTopicCallback(Topic_Stream,boost::bind(&TopSimStreamTransfers::Topic_StreamToGetData,this,_1));
        d->_IsSub = true;
    }

    if (!d->_AsyncRecevice)
    {
        //等待回调函数执行完毕
    	boost::unique_lock<boost::mutex> lock = boost::make_unique_lock(d->_mu);
        d->_condvar.wait(lock);
        d->_IsRecevie = false;
        return d->_RealDataCount;
    }
    return Data_Count;
}

void TopSimStreamTransfers::Topic_StreamToGetData( TSTopicContextPtr Ctx )
{
    T_D();
    if (d->_IsNotity && !d->_AsyncRecevice)
    {
    	boost::unique_lock<boost::mutex> lock = boost::make_unique_lock(d->_mu);
        d->_condvarNotity.wait(lock);
        d->_IsStart = false;
        d->_IsNotity = false;
    }
    StreamDentity::DataTypePtr Data = Ctx->GetTopicT<StreamDentity::DataType>();

    ASSERT(Data->FileSize <= d->_DataCount);
    if(!Data->IsEndBuild)
    {
       memcpy(d->_WirteDataFormemory,&Data->FileCell[0],Data->FileCell.size()); 
    }  
    d->_RecevieInfo->pieceNum = Data->PieceNum;
    d->_RecevieInfo->StreamId = Data->revcevie.streamID;
    d->_RecevieInfo->EndFlag = Data->Enflag;
    d->_RecevieInfo->IsEndBuild = Data->IsEndBuild;
    UINT32 Datasum = Data->FileCell.size();
    d->_RealDataCount = Datasum;
    d->_IsNotity = true;
    d->_IsRecevie = true;
    if (!d->_AsyncRecevice)
    {
        for (;d->_IsRecevie == true;)
        {
            d->_condvar.notify_one();
        }
    }
}

bool TopSimStreamTransfers::SendStreamDataInfo( char* StreamData,const UINT32 & StreamSize,TSTransfersInfoPtr Tansfers)
{
    T_D();
    d->_domainptr->PublishTopic(Topic_Stream);
    StreamDentity::DataType StreamdataInfo;
    if(StreamSize % d->_piecesize)
    {
        d->_peachDataNum = StreamSize / d->_piecesize + 1;
    }
    else
    {
        d->_peachDataNum = StreamSize / d->_piecesize;
    }

    StreamdataInfo.revcevie.streamID = Tansfers->StreamId;
    StreamdataInfo.revcevie.RecevierAppId = Tansfers->AppId;
    size_t SumData = 0;
    size_t peachNum = 1;
    int realCount = 0;
    d->_celldata = StreamData;

    do
    {
        UINT32 NoSendCount = StreamSize - (peachNum - 1)*d->_piecesize;        
        realCount = NoSendCount > d->_piecesize ? d->_piecesize  : NoSendCount;
        if (!realCount)
        {
            break;
        }

        StreamdataInfo.FileCell.reserve(realCount*sizeof(char));
        StreamdataInfo.FileCell.assign(&d->_celldata[0],&d->_celldata[realCount]);
        if (peachNum == d->_peachDataNum)
        {
            StreamdataInfo.Enflag = true;
        }
        else
        {
            StreamdataInfo.Enflag = false;         
        }

        StreamdataInfo.PieceNum = peachNum;
        StreamdataInfo.FileSize = realCount;
        if (Tansfers->AppId._value)
        {
            if (!d->_domainptr->SendTopicDirect(Topic_Stream,&StreamdataInfo,Tansfers->AppId))
            {
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TOPSIMSTREAMTRANSFERS_CPP_189) << d->_streamID << peachNum ;
                return false;
            }
        }
        else
        {
            if(!d->_domainptr->UpdateTopic(Topic_Stream,&StreamdataInfo))
            {
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TOPSIMSTREAMTRANSFERS_CPP_197) << d->_streamID << peachNum ;
                return false;
            } 
        }
        
        StreamdataInfo.FileCell.clear();
        peachNum++;
        SumData += realCount; 
        d->_celldata = d->_celldata + realCount;
    }while(peachNum <= d->_peachDataNum);

    return true;
}

void TopSimStreamTransfers::SendAsync( char* StreamData,const UINT32 & StreamSize,TSTransfersInfoPtr Tansfers)
{
#if defined(XSIM3_3) || defined(XSIM3_2)
	GetIOSService()->post(boost::bind(&TopSimStreamTransfers::SendStreamDataInfo,this,StreamData,StreamSize,Tansfers));
#else
	GetIOSService()->Post(boost::bind(&TopSimStreamTransfers::SendStreamDataInfo,this,StreamData,StreamSize,Tansfers));
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

void TopSimStreamTransfers::RecevieAsync( char* Data,const UINT32 & Data_Count )
{
    T_D();
    d->_WirteDataFormemory = Data;
    d->_DataCount = Data_Count;
    d->_AsyncRecevice = true;
    if (!d->_IsSub)
    {
        d->_domainptr->SubscribeTopic(Topic_Stream);
        d->_domainptr->RegisterTopicCallback(Topic_Stream,boost::bind(&TopSimStreamTransfers::Topic_StreamToGetData,this,_1));
        d->_IsSub = true;
    }
}

TSTransfersInfoPtr TopSimStreamTransfers::BeginBuild(const TSFrontAppUniquelyId & AppId)
{
    T_D();
    TSTransfersInfoPtr tsInfo = boost::make_shared<TSTransfersInfo>();
    tsInfo->AppId = AppId;
    tsInfo->StreamId = GetCRC32(GenerateUUID());
    return tsInfo;
}

void TopSimStreamTransfers::InsertIdMap( TSTransfersInfoPtr TransfersInfo )
{
    T_D();
    IdInfoMap::iterator Iter = d->_InfoMap.find(TransfersInfo->StreamId);
    if (Iter == d->_InfoMap.end())
    {
        d->_InfoMap.insert(std::make_pair(TransfersInfo->StreamId,TransfersInfo->AppId));
    }
}

bool TopSimStreamTransfers::EndBuild( TSTransfersInfoPtr TsTansfers )
{
    T_D();
    StreamDentity::DataType StreamdataInfo;
    StreamdataInfo.IsEndBuild = true;
    StreamdataInfo.FileSize = 0;
    StreamdataInfo.Enflag = true;
    StreamdataInfo.revcevie.streamID = TsTansfers->StreamId;
    if (TsTansfers->AppId._value)
    {
        if (!d->_domainptr->SendTopicDirect(Topic_Stream,&StreamdataInfo,TsTansfers->AppId))
        {
            DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TOPSIMSTREAMTRANSFERS_CPP_261) << d->_streamID;
            return false;
        }
    }
    else
    {
        if(!d->_domainptr->UpdateTopic(Topic_Stream,&StreamdataInfo))
        {
            DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TOPSIMSTREAMTRANSFERS_CPP_269) << d->_streamID;
            return false;
        } 
    }
    IdInfoMap::iterator Iter = d->_InfoMap.find(TsTansfers->StreamId);
    if (Iter != d->_InfoMap.end())
    {
        d->_InfoMap.unsafe_erase(Iter);
    }
    return true;
}

#endif //#ifndef  _WRS_KERNEL
