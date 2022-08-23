#include "stdafx.h"

#ifndef  _WRS_KERNEL

#include <boost/filesystem.hpp>
#include <TopSimDataInterface/TSSerializer.h>

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimTypes/TSByteOrder.h>
#else
#include <TopSimRuntime/TSByteOrder.h>
#include <TopSimRuntime/TSFileSystem.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include "TSTopicTypeManager.h"
#include "TSDataCollection.h"
#include "TSDataCollection_p.h"

BEGIN_METADATA(TSDataCollection)
END_METADATA()

TSDataCollection::TSDataCollection()
	:_Private_Ptr(new PrivateType)
{
	//XSim5.4版本将文件夹名称改为小写，并兼容之前版本
#ifndef __VXWORKS__
	boost::filesystem::path p = GetWorkDirPath() + TS_TEXT("/rtmdata");

	if (!boost::filesystem::exists(p))
	{
		p = GetWorkDirPath() + TS_TEXT("/RTMData");

		if (!boost::filesystem::exists(p))
		{
			p = GetWorkDirPath() + TS_TEXT("/rtmdata");

			boost::filesystem::create_directories(GetWorkDirPath() + TS_TEXT("/rtmdata"));
		}
	}

	_Private_Ptr->_CollectionPath = p.string();
#else
	TSString p = GetWorkDirPath() + TS_TEXT("/rtmdata");
	if (!TSDir::Exists(p))
	{
		p = GetWorkDirPath() + TS_TEXT("/RTMData");
		
		if (!TSDir::Exists(p))
		{
			p = GetWorkDirPath() + TS_TEXT("/rtmdata");
			TSDir::CreateDirectory(p);
		}
	}
	_Private_Ptr->_CollectionPath = p;
#endif;
}

TSDataCollection::~TSDataCollection()
{

}

void TSDataCollection::Write(const TSTopicContextPtr Ctx)
{
    T_D();

	if(d->_RecordHelper)
	{
		if(!d->_RecordHelper->IsOpen())
		{
			TSRWSpinLock::WriteLock lock(d->_RecordHelperLock);

			if(!d->_RecordHelper->IsOpen())
			{
				d->_RecordHelper->OpenRecordFile(TOPIC_FILE_EXT,TOPIC_FILE_SIGN,TOPIC_FILE_VERSION);
			}
		}
	}

	if(d->_RecordHelper 
		&& d->_RecordHelper->IsOpen())
	{
		std::size_t DataSize  = sizeof(TSTopicRecordStruct);

		TSByteBufferPtr TBuff = Ctx->GetCtx()->GetBinary();

#if defined(XSIM3_3) || defined(XSIM3_2)
		if (TBuff->GetOffsetW() > 0)
		{
			DataSize += TBuff->GetOffsetW() - 1;
		}
#else
		if (TBuff->TellW() > 0)
		{
			DataSize += TBuff->TellW() - 1;
		}
#endif // defined(XSIM3_3) || defined(XSIM3_2)

		TSTopicRecordStruct * Record = (TSTopicRecordStruct *)malloc(DataSize);
		memset(Record,0,DataSize);
		Record->SizeOfThis = (UINT32)DataSize;
		Record->KeyHash = Ctx->GetCtx()->KeyHash();
		Record->Time = TIME2MICSECS(Ctx->GetTime());

#if defined(XSIM3_3) || defined(XSIM3_2)
		memcpy(Record->Extendable, TBuff->Data(), TBuff->GetOffsetW());
#elif defined(XSIM4_3)
		memcpy(Record->Extendable, TBuff->Data(), TBuff->TellW());
#else
		memcpy(Record->Extendable, TBuff->Begin(), TBuff->TellW()); 
#endif // defined(XSIM3_3) || defined(XSIM3_2)

		SwapUInt32(Record->KeyHash);
		SwapUInt64(Record->Time);
		SwapUInt32(Record->SizeOfThis);

		boost::shared_ptr<char> data = boost::shared_ptr<char>((char*)Record);
		d->_RecordHelper->WriteData(data,DataSize);
	}
}

void TSDataCollection::Close()
{
    T_D();

	TSRWSpinLock::WriteLock lock(d->_RecordHelperLock);

    if (d->_RecordHelper)
    {
        d->_RecordHelper->CloseRecordFile();
		d->_RecordHelper.reset();
    }
}

void TSDataCollection::SetOutputPath( const TSString & Path )
{
	T_D();

	d->_CollectionPath = Path;
}

void TSDataCollection::SetTopicName( const TSString & TopicName )
{
	T_D();

	d->_TopicName = TopicName;
}

void TSDataCollection::Init()
{
	T_D();

	TSRWSpinLock::WriteLock lock(d->_RecordHelperLock);

	d->_RecordHelper = boost::make_shared<TSDataRecorderHelper>();

	TSString FileFullName = d->_CollectionPath + TS_TEXT("/") + d->_TopicName;

	d->_RecordHelper->SetBaseOutFileName(FileFullName);
}

void TSDataCollection::Write(TSTOPICHANDLE TopicId, const TSInterObject *Object, const TSTime &Timestamp)
{
    T_D();

    if(d->_RecordHelper)
    {
        if(!d->_RecordHelper->IsOpen())
        {
            TSRWSpinLock::WriteLock lock(d->_RecordHelperLock);

            if(!d->_RecordHelper->IsOpen())
            {
                d->_RecordHelper->OpenRecordFile(TOPIC_FILE_EXT,TOPIC_FILE_SIGN,TOPIC_FILE_VERSION);
            }
        }
    }

    if(d->_RecordHelper
       && d->_RecordHelper->IsOpen())
    {
        std::size_t DataSize  = sizeof(TSTopicRecordStruct);

        TSDataContextPtr DataCtx;

        if(TSTypeSupportPtr Support = HandleToSupport(TopicId))
        {
            TSSerializer Ser;
            if (Ser.PerpareForSerialize(Support->GetTypeVersionHashCode(), Timestamp))
            {
                Support->Serialize(Object, Ser);
                Ser.MarshalComplete();
                DataCtx = Ser.GetDataContext();


                TSByteBufferPtr TBuff = DataCtx->GetBinary();

#if defined(XSIM3_3) || defined(XSIM3_2)
                if (TBuff->GetOffsetW() > 0)
		{
			DataSize += TBuff->GetOffsetW() - 1;
		}
#else
                if (TBuff->TellW() > 0)
                {
                    DataSize += TBuff->TellW() - 1;
                }
#endif // defined(XSIM3_3) || defined(XSIM3_2)

                TSTopicRecordStruct * Record = (TSTopicRecordStruct *)malloc(DataSize);
                memset(Record,0,DataSize);
                Record->SizeOfThis = (UINT32)DataSize;
                Record->KeyHash = DataCtx->KeyHash();
                Record->Time = TIME2MICSECS(Timestamp);

#if defined(XSIM3_3) || defined(XSIM3_2)
                memcpy(Record->Extendable, TBuff->Data(), TBuff->GetOffsetW());
#elif defined(XSIM4_3)
                memcpy(Record->Extendable, TBuff->Data(), TBuff->TellW());
#else
                memcpy(Record->Extendable, TBuff->Begin(), TBuff->TellW());
#endif // defined(XSIM3_3) || defined(XSIM3_2)

                SwapUInt32(Record->KeyHash);
                SwapUInt64(Record->Time);
                SwapUInt32(Record->SizeOfThis);

                boost::shared_ptr<char> data = boost::shared_ptr<char>((char*)Record);
                d->_RecordHelper->WriteData(data,DataSize);
            }
        }
    }
}

#endif //_WRS_KERNEL
