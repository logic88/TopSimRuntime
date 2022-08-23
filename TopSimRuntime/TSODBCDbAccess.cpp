#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#ifndef  _WRS_KERNEL

#include <boost/asio/io_service.hpp>

#if !defined(XSIM3_3) && !defined(XSIM3_2)
#include <TopSimRuntime/TSLogsUtil.h>
#endif // !defined(XSIM3_3) && !defined(XSIM3_2)

#include "TSTopicTypes.h"
#include "TSTopicTypeManager.h"
#include "TSDataRecorderHelper.h"
#include "TSDataRecorderHelper_p.h"

BEGIN_METADATA(TSDataRecorderHelper)
    END_METADATA()

TSDataRecorderHelperPrivate::TSDataRecorderHelperPrivate()
	:_IsOpen(false)
	, _FlushImmediately(false)
{
#if defined(XSIM3_3) || defined(XSIM3_2)
	_NotifyIOS = boost::make_shared<TSIOSStruct>();
	_NotifyIOS->RunAlways();
	_IOS = _NotifyIOS->GetIOS();
#else
	_IOS = TSAsyncIOService::Instance()->GetIOS(false);
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	TSString Conf = TSTopicTypeManager::Instance()->GetExtendCfg("FlushImmediately");

	if (!Conf.empty())
	{
		_FlushImmediately = TSValue_Cast<bool>(Conf);
	}
}

TSDataRecorderHelper::TSDataRecorderHelper()
	:_Private_Ptr(new PrivateType())
{
	
}

TSDataRecorderHelper::TSDataRecorderHelper( PrivateType * p )
	:_Private_Ptr(p)
{
	
}

TSDataRecorderHelperPrivate::~TSDataRecorderHelperPrivate()
{
#if defined(XSIM3_3) || defined(XSIM3_2)
	_NotifyIOS->Stop();
#endif // defined(XSIM3_3) || defined(XSIM3_2)
}

void TSDataRecorderHelper::OpenRecordFile( const TSString & FileExt,const TSString & FileSign,UINT32 FileVer )
{
	try
	{
		TSString OutFileFullName = _Private_Ptr->_BaseOutFileName;
		OutFileFullName += FileExt;

		_Private_Ptr->_Stream.open(OutFileFullName.c_str(),std::ios_base::out | std::ios_base::binary);

		if (!FileSign.empty() && FileVer)
		{
			TSByteBuffer Buffer;
			Buffer.WriteBytes(FileSign.c_str(),FileSign.length());
			Buffer.WriteUInt32(FileVer);

#if defined(XSIM3_3) || defined(XSIM3_2)
            _Private_Ptr->_Stream.write( Buffer.Data(),Buffer.GetOffsetW());
#elif defined(XSIM4_3)
			_Private_Ptr->_Stream.write( Buffer.Data(),Buffer.TellW());
#else
            _Private_Ptr->_Stream.write( Buffer.Begin(),Buffer.TellW());
#endif // defined(XSIM3_3) || defined(XSIM3_2)
			
			_Private_Ptr->_ValidateOffset = _Private_Ptr->_Stream.tellp();
			UINT8 Validate = 0;
			_Private_Ptr->_Stream.write((const char*)&Validate,sizeof(UINT8));
		}

		_Private_Ptr->_IsOpen = true;
	}
	catch (std::exception & e)
	{
        DEF_LOG_WARNING(TRS_TOPSIMDATAINTERFACE_TSDATARECORDERHELPER_CPP_81) <<e.what();
	}
}

void TSDataRecorderHelper::WriteData( boost::shared_ptr<char> data,std::size_t len)
{
	if (IsOpen())
	{
		if (TSIOSericePtr IOS = _Private_Ptr->_IOS)
		{
#if defined(XSIM3_3) || defined(XSIM3_2)
			IOS->post(boost::bind(&TSDataRecorderHelper::WriteFileImmediately,this,data,len));
#else
			IOS->Post(boost::bind(&TSDataRecorderHelper::WriteFileImmediately,this,data,len));
#endif
			
		}
		else
		{
			WriteFileImmediately(data,len);
		}
	}

}

void TSDataRecorderHelper::WriteFileImmediately( boost::shared_ptr<char> data,std::size_t len)
{
	_Private_Ptr->_Stream.write( data.get(),len);

	if (_Private_Ptr->_FlushImmediately)
	{
		_Private_Ptr->_Stream.flush();
	}
}

void TSDataRecorderHelper::CloseRecordFile()
{
	if (IsOpen())
	{
		if (TSIOSericePtr IOS = _Private_Ptr->_IOS)
		{
			_Private_Ptr->_WaitClose.store(true);

#if defined(XSIM3_3) || defined(XSIM3_2)
			IOS->post(boost::bind(&TSDataRecorderHelper::CloseFileImmediately,this));
#else
			IOS->Post(boost::bind(&TSDataRecorderHelper::CloseFileImmediately,this));
#endif
			size_t count = 0;

			while (_Private_Ptr->_WaitClose.load())
			{
				TSSchedYield(count++);
			}
		}
		else
		{
			CloseFileImmediately();
		}

		_Private_Ptr->_IsOpen = false;
	}
}

void TSDataRecorderHelper::CloseFileImmediately()
{
	TSByteBuffer Buffer;
	Buffer.WriteBytes(FILE_END_SIGN,strlen(FILE_END_SIGN));

#if defined(XSIM3_3) || defined(XSIM3_2)
	_Private_Ptr->_Stream.write( Buffer.Data(),Buffer.GetOffsetW() );
#elif defined(XSIM4_3)
	 _Private_Ptr->_Stream.write( Buffer.Data(),Buffer.TellW() );
#else
	_Private_Ptr->_Stream.write( Buffer.Begin(),Buffer.TellW() );
#endif // defined(XSIM3_3) || defined(XSIM3_2)

	UINT64 CurOffset = _Private_Ptr->_Stream.tellp();
	_Private_Ptr->_Stream.seekp(_Private_Ptr->_ValidateOffset);
	UINT8 Validate = true;
	_Private_Ptr->_Stream.write((const char*)&Validate,sizeof(UINT8));
	_Private_Ptr->_Stream.seekp(CurOffset);

	_Private_Ptr->_Stream.flush();
	_Private_Ptr->_Stream.close();

	_Private_Ptr->_WaitClose.store(false);
}

bool TSDataRecorderHelper::IsOpen() const
{
	return _Private_Ptr->_IsOpen;
}

void TSDataRecorderHelper::SetBaseOutFileName( const TSString& baseOutFileName )
{
	if(!IsOpen())
	{
		_Private_Ptr->_BaseOutFileName = baseOutFileName;
	}
}

const TSString& TSDataRecorderHelper::GetBaseOutFileName()
{
	return _Private_Ptr->_BaseOutFileName;
}

TSDataRecorderHelper::~TSDataRecorderHelper()
{
	delete _Private_Ptr;
}

void TSDataRecorderHelper::SetIOS( TSIOSericePtr ios )
{
	_Private_Ptr->_IOS = ios;
}
#endif //#ifndef  _WRS_KERNEL
