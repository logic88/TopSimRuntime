#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#ifndef  _WRS_KERNEL
#include <boost/filesystem.hpp>
#include <TransfersInfo/Topic.h>
#include <TransfersInfo/Defined.h>
#include <TopSimDataInterface/TSDomain.h>
#include "TSDataInteractManager.h"
#include "TopSimFileWriter.h"
#include "TopSimFileWriter_p.h"

TopSimFileWriter::~TopSimFileWriter()
{

}

TopSimFileWriter::TopSimFileWriter() : TopSimFileTransfers(new PrivateType)
{

}

void TopSimFileWriter::RecevieFile( const TSString & FilePath)
{
    T_D();
    d->_FilePath = FilePath;
#ifndef  _WRS_KERNEL
    if(!boost::filesystem::exists(d->_FilePath))
    {
        boost::system::error_code err;
        boost::filesystem::create_directories(d->_FilePath,err);
        if(err)
        {
			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TOPSIMFILEWRITER_CPP_30) << FilePath ;
            return;
        }
    }
#endif //#ifndef  _WRS_KERNEL
    d->_domainptr->SubscribeTopic(Topic_File);
    d->_domainptr->RegisterTopicCallback(Topic_File,boost::bind(&TopSimFileWriter::Topic_FileOnData,this,_1));
    
}

void TopSimFileWriter::Topic_SenderOnData( TSTopicContextPtr Ctx )
{
    T_D();
    if(Ctx->Is(Topic_Sender))
    {
        Sender::DataTypePtr Data = 
            Ctx->GetTopicT<Sender::DataType>();

        if(Data->streamID == d->_streamID)
        {
            Recevier::DataType dataType;
            dataType.streamID = d->_streamID;
            dataType.RecevierAppId = d->_recevierAppID;
            d->_domainptr->SendTopicDirect(Topic_Recevier,&dataType, Data->SenderAppId);
            //DEF_LOG_TRACE("连接接收者,准备发送数据");
        }
    }
}

void TopSimFileWriter::Topic_FileOnData( TSTopicContextPtr Ctx )
{
    T_D();
    FileDentity::DataTypePtr Data = Ctx->GetTopicT<FileDentity::DataType>();
   // std::ofstream ofStream;
    TSString FilePath = d->_FilePath + "/" + Data->FileName;
    //TSString SaveFilePath = FilePath + "_" + boost::lexical_cast<TSString,UINT32>(Data->revcevie.streamID);
    if (!d->_IsOpen)
    {
        if (Data->Binaryflag)
        {
            d->_Stream.open(FilePath.c_str(),std::ios_base::out | std::ios_base::binary);

        }
        else
        {
            d->_Stream.open(FilePath.c_str(),std::ios_base::out);
        }

        if (d->_Stream.good())
        {
			DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TOPSIMFILEWRITER_CPP_79) << d->_FilePath << Data->FileName;
        }
        d->_IsOpen = true;
    }

    char* memrySize = new char[Data->FileCell.size()*sizeof(char)];
    memcpy(memrySize,&Data->FileCell[0],Data->FileCell.size());
    d->_Stream.write(memrySize,Data->FileCell.size());
    delete[] memrySize;
    d->_CellDataNum++;
    d->_SumData += Data->FileCell.size();
    d->_FileInfo->Schedule = d->_SumData * 1.0 / Data->FileSize;
    d->_FileInfo->StreamInfo->AppId = Data->revcevie.RecevierAppId;
    d->_FileInfo->StreamInfo->StreamId = Data->revcevie.streamID;
    d->_FileInfo->FileName = FilePath;
    if (d->_SumData == Data->FileSize || Data->Enflag == true)
    {
        d->_CellDataNum = 0;
        d->_SumData = 0;
        d->_Stream.close();
        d->_IsOpen = false;
    }

	printf("TopSimFileWriter()->File[%s]:FileSize[%u], ReceiveBlockNum[%d], StreamId[%u], DataSize[%d]\n",
		d->_FileInfo->FileName.c_str(), Data->FileSize, d->_CellDataNum, Data->revcevie.streamID, Data->FileCell.size());
}


#endif //#ifndef  _WRS_KERNEL
