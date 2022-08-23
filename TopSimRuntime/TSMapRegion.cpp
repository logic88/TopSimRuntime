#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#ifndef  _WRS_KERNEL

#include <boost/filesystem.hpp>
#include <TransfersInfo/Topic.h>
#include <TransfersInfo/Defined.h>
#include <TopSimDataInterface/TSDomain.h>
#include <TopSimDataInterface/TSDataInteractManager.h>
#include "TopSimFileReader_p.h"
#include "TopSimFileReader.h"

TopSimFileReader::TopSimFileReader() : TopSimFileTransfers(new PrivateType)
{

}

TopSimFileReader::~TopSimFileReader() 
{

}

void TopSimFileReader::SendFileAsync( const TSString & FilePath, bool IsBinary,const TSFrontAppUniquelyId & AppId)
{
#if defined(XSIM3_3) || defined(XSIM3_2)
	GetIOSService()->post(boost::bind(&TopSimFileReader::SendFileInfo,this,FilePath,IsBinary,AppId));
#else
	GetIOSService()->Post(boost::bind(&TopSimFileReader::SendFileInfo,this,FilePath,IsBinary,AppId));
#endif // defined(XSIM3_3) || defined(XSIM3_2)
    
}

bool TopSimFileReader::SendFileInfo( const TSString & FilePath, bool IsBinary,const TSFrontAppUniquelyId & AppId )
{
    T_D();
    d->_domainptr->PublishTopic(Topic_File);
    TSTransfersInfoPtr TransferInfo = BeginBuild(AppId);
    

    d->_celldata = new char[d->_piecesize];
    memset(d->_celldata,0,d->_piecesize);
    if (IsBinary)
    {
        d->_ifStream.open(FilePath.c_str(),std::ios_base::in | std::ios_base::binary);
    }
    else
    {
        d->_ifStream.open(FilePath.c_str(),std::ios_base::in);
    }

    if(d->_ifStream.good())
    {
		DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TOPSIMFILEREADER_CPP_45) << FilePath;
    }
    else
    {
		DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TOPSIMFILEREADER_CPP_49) << FilePath;
        return false;
    }

    FileDentity::DataType FiledataInfo;
    FiledataInfo.Binaryflag = IsBinary;
    FiledataInfo.FileName = GetFileShortName(FilePath);
    d->_fileShortName = FiledataInfo.FileName;
#ifndef  _WRS_KERNEL
	//计算文件的大小 返回是unsigned long long，数据类型需要改一下
    FiledataInfo.FileSize = boost::filesystem::file_size(FilePath);
    if (!FiledataInfo.FileSize)
    {
		DEF_LOG_KERNEL(TRS_TOPSIMDATAINTERFACE_TOPSIMFILEREADER_CPP_60) << FilePath;
        return false;
    }
#endif //#ifndef  _WRS_KERNEL
    FiledataInfo.revcevie.streamID = TransferInfo->StreamId;
    FiledataInfo.revcevie.RecevierAppId = TransferInfo->AppId;

    if(FiledataInfo.FileSize % d->_piecesize)
    {
        d->_peachDataNum = FiledataInfo.FileSize / d->_piecesize + 1;

    }
    else
    {
        d->_peachDataNum = FiledataInfo.FileSize / d->_piecesize;

    }

	//实际读取的文件总字节数
    size_t SumData = 0;
	//实际读取的文件分段编号
    size_t peachNum = 1;
	//实际读取到的字节数
    int realCount = 0;
    do
    {
        d->_ifStream.read(d->_celldata,d->_piecesize);
        realCount = d->_ifStream.gcount();
        if (!realCount || d->_StopSend)
        {
            break;
        }

		//从新分配vector的capacity,此处使用resize()是否会减少系统开销
        FiledataInfo.FileCell.reserve(realCount*sizeof(char));
		//把数据复制到vector中
        FiledataInfo.FileCell.assign(&d->_celldata[0],&d->_celldata[realCount]);
        if (peachNum == d->_peachDataNum)
        {
            FiledataInfo.Enflag = true;
        }
        else
        {
            FiledataInfo.Enflag = false;         
        }

        if (AppId._value)
        {
            if(!d->_domainptr->SendTopicDirect(Topic_File,&FiledataInfo,d->_recevierAppID))
            {
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TOPSIMFILEREADER_CPP_104) << d->_streamID << peachNum;
                return false;
            }
        }
        else
        {
            if(!d->_domainptr->UpdateTopic(Topic_File,&FiledataInfo))
            {
                DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TOPSIMFILEREADER_CPP_112) << d->_streamID << peachNum;
                return false;
            }
        }
        memset(d->_celldata,0,d->_piecesize);
        FiledataInfo.FileCell.clear();
        peachNum++;
        SumData += realCount; 
        d->_FileInfo->Schedule = SumData * 1.0 / FiledataInfo.FileSize; 
        d->_FileInfo->FileName = FilePath;
        d->_FileInfo->StreamInfo->AppId = AppId;
        d->_FileInfo->StreamInfo->StreamId = TransferInfo->StreamId;

		printf("TopSimFileReader()->File[%s]:FileSize[%u], SendBlockNum[%d], StreamId[%u], DataSize[%d]\n",
			d->_FileInfo->FileName.c_str(), FiledataInfo.FileSize, peachNum-1, TransferInfo->StreamId, realCount);
    }while(realCount > 0);

    delete d->_celldata;
    d->_ifStream.close();
    EndBuild(TransferInfo);
    return true;
}

void TopSimFileReader::StopSend()
{
    T_D();
    d->_StopSend = true;
}

TSString TopSimFileReader::GetFileShortName( const TSString & FileName )
{
    T_D();
    std::string::size_type pos1 = FileName.find_last_of("/");
    std::string::size_type pos2 = FileName.find_last_of("\\");
    std::string::size_type posN = 0;
    if (pos1 != TSString::npos)
    {
        posN = pos1;
    }
    else if (pos2 != TSString::npos)
    {
        posN = pos2;
    }
     
    return FileName.substr(posN, FileName.length() - posN);
}

bool TopSimFileReader::SendFile( const TSString & FilePath, bool IsBinary, const TSFrontAppUniquelyId & AppId)
{
    if(!SendFileInfo(FilePath,IsBinary,AppId))
    {
        return false;
    }

    return true;
}
#endif //#ifndef  _WRS_KERNEL
