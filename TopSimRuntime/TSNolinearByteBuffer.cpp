#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#ifndef  _WRS_KERNEL

#if defined(XSIM3_3) || defined(XSIM3_2)
#include <TopSimUtil/TSFileSystem.h>
#else
#include <TopSimRuntime/TSFileSystem.h>
#include <TopSimRuntime/TSLogsUtil.h>
#endif // defined(XSIM3_3) || defined(XSIM3_2)

#include <TopSimDataInterface/TSTopicTypeManager.h>
#include <TopSimDataInterface/TSDataInteractManager.h>
#include <TopSimDataInterface/TSSerializer.h>

#include "TSBinarySource.h"
#include "TSBinarySource_p.h"

BEGIN_METADATA(TSBinarySource)
END_METADATA()

TSRecordFileIOStruct::TSRecordFileIOStruct()
: _ContentOffsetInFile(0),_CurrentOffsetInRegion(0),_Hdr(0),_RegionOffsetInFile(0)
{
	_CurrentDataTime = TSMinTimeValue;
	_NextDataTime = TSMinTimeValue;
}

bool TSRecordFileIOStruct::ReadBytes(void* val, size_t len)
{
	UINT64 ReadFileOffset = _RegionOffsetInFile + _CurrentOffsetInRegion;

	if (_FileSize - ReadFileOffset > len)
	{
		if (_Region->get_size()  - _CurrentOffsetInRegion < len)
		{
			try
			{
				_Region.reset(
					new boost::interprocess::mapped_region(
					*_MappingFile,
					boost::interprocess::read_only,
					ReadFileOffset,
					MIN(Region_Size,_FileSize - ReadFileOffset))
					);

				_RegionOffsetInFile = ReadFileOffset;
				_CurrentOffsetInRegion = 0;
			}
			catch (boost::interprocess::interprocess_exception & e)
			{
				std::cout << e.what();

				return false;
			}
		}

		char * DataPtr = (char *)_Region->get_address() + _CurrentOffsetInRegion;

		memcpy(val,DataPtr,len);

		_CurrentOffsetInRegion += len;

		return true;
	}

	return false;
}

UINT64 TSRecordFileIOStruct::TellR() const
{
	return _RegionOffsetInFile + _CurrentOffsetInRegion;
}

UINT64 TSRecordFileIOStruct::SeekR(UINT64 offset,bool beginning/* = false*/)
{
	UINT64 ori_offset = TellR();

	if (beginning)
	{
		if (offset <= _FileSize)
		{
			try
			{
				_Region.reset(
					new boost::interprocess::mapped_region(
					*_MappingFile,
					boost::interprocess::read_only,
					offset,
					MIN(Region_Size,_FileSize - offset))
					);

				_CurrentOffsetInRegion = 0;
				_RegionOffsetInFile = offset;
			}
			catch (boost::interprocess::interprocess_exception & e)
			{
				std::cout << e.what();

				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		UINT64 ReadFileOffset = _RegionOffsetInFile + _CurrentOffsetInRegion;

		if (_FileSize - ReadFileOffset > offset)
		{
			if (_Region->get_size()  - _CurrentOffsetInRegion < offset)
			{
				try
				{
					_Region.reset(
						new boost::interprocess::mapped_region(
						*_MappingFile,
						boost::interprocess::read_only,
						ReadFileOffset,
						MIN(Region_Size,_FileSize - ReadFileOffset))
						);

					_CurrentOffsetInRegion = 0;
					_RegionOffsetInFile = ReadFileOffset;
				}
				catch (boost::interprocess::interprocess_exception & e)
				{
					std::cout << e.what();

					return -1;
				}
			}

			_CurrentOffsetInRegion += offset;
		}
		else
		{
			return -1;
		}
	}

	return ori_offset;
}

TSSpinLock& TSRecordFileIOStruct::Lock()
{
	return _Lock;
}


#if defined(XSIM3_3) || defined(XSIM3_2)

std::streamoff TSRecordFileIOStruct::GetOffsetR() const
{
	return TellR();
}

bool TSRecordFileIOStruct::SkipR( std::streamoff size )
{
	// no need implement.
	return false; 
}

std::streamoff TSRecordFileIOStruct::SeekR( std::streamoff offset )
{
	return SeekR(offset);
}

#endif

TSCollectionDataPtr TSBinarySourcePrivate::ReadSample( TSRecordFileIOStructPtr IOStruct )
{
    if(TSCollectionDataPtr RecordData = ReadPBData<TSCollectionData,TSTopicRecordStruct>(IOStruct))
    {
        return RecordData;   
    }

    return TSCollectionDataPtr();
}

TSBinarySource::TSBinarySource(void)
    :_Private_Ptr(new PrivateType)
{
}


TSBinarySource::~TSBinarySource(void)
{
    delete _Private_Ptr;
}

bool TSBinarySource::LoadTopicFile(const TSString& FilePath,
    const std::vector<TSString>& FileNames,
    const std::vector<TSTOPICHANDLE>& ExcludeTopics)
{
    T_D();

    d->_FullPath = FilePath;
	d->_NextValidTime = boost::posix_time::not_a_date_time;

    std::vector<TSString> ExcludeTopicFileNames;
    for(size_t i = 0; i < ExcludeTopics.size(); ++i)
    {
        const TSString& FileName = HandleToName(ExcludeTopics[i]);
        if(FileName.empty())
        {
            ExcludeTopicFileNames.push_back(FileName);
        }
    }

    for(size_t i = 0; i < FileNames.size(); ++i)
    {
        TSTOPICHANDLE THandle = TSTopicTypeManager::Instance()->GetTopicByTopicName(boost::filesystem::basename(FileNames[i]));

		if(!THandle)
		{
			DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSBINARYSOURCE_CPP_218) << FileNames[i];
		}
		else
		{
			TSRecordFileIOStructPtr IOStruct = boost::make_shared<TSRecordFileIOStruct>();
			IOStruct->_Handle = THandle;

			d->_Includes.push_back(THandle);

			if(InitPBFile(FileNames[i],IOStruct,CN_TOPSIMDATAINTERFACE_TSBINARYSOURCE_CPP_227,TOPIC_FILE_SIGN,TOPIC_FILE_VERSION))
			{
				d->_IOStructs.push(IOStruct);
			}
			else
			{
				DEF_LOG_ERROR(TRS_TOPSIMDATAINTERFACE_TSBINARYSOURCE_CPP_233) << FileNames[i];
			}
		}
    }

	TSRecordFileIOStructPtr IOStruct;

	while(d->_IOStructs.try_pop(IOStruct))
	{
		if(IOStruct)
		{
			if(!IOStruct->_NextDataTime.is_not_a_date_time())
			{
				d->_IOStructs.push(IOStruct);
				d->_NextValidTime = IOStruct->_NextDataTime;
				break;
			}
		}
	}

    return true;
}

TSCollectionDataPtr TSBinarySource::ReadNext(TSTime & ReachTime )
{
    T_D();

    TSRecordFileIOStructPtr ValidIOStruct;
	TSRecordFileIOStructPtr IOStruct;

    TSCollectionDataPtr CollectionData;
   
    while(d->_IOStructs.try_pop(ValidIOStruct))
    {
		IOStruct = ValidIOStruct;
		ValidIOStruct.reset();

		if(IOStruct->_NextDataTime <= ReachTime)
		{
			if(CollectionData = d->ReadSample(IOStruct))
			{
				d->_IOStructs.push(IOStruct);

				CollectionData->_TopicId = IOStruct->_Handle;

				if(d->_IOStructs.try_pop(IOStruct))
				{
					if (!IOStruct->_NextDataTime.is_not_a_date_time())
					{
						if (d->_NextValidTime < IOStruct->_NextDataTime)
						{
							d->_NextValidTime = IOStruct->_NextDataTime;
						}

						d->_IOStructs.push(IOStruct);
					}
					else
					{
						continue;
					}
				}

				return CollectionData;
			}
			else
			{
				continue;
			}
		}
		else if(!IOStruct->_NextDataTime.is_not_a_date_time())
		{
			d->_NextValidTime = IOStruct->_NextDataTime;

            ValidIOStruct = IOStruct;

			d->_IOStructs.push(IOStruct);


			break;
		}
    }

	if(!ValidIOStruct)
	{
		d->_NextValidTime = boost::posix_time::not_a_date_time;
	}

    return CollectionData;

}

const std::vector<TSTOPICHANDLE>& TSBinarySource::GetIncludeTopics()
{
    T_D();

    return d->_Includes;
}

void TSBinarySource::Close()
{

}

void TSBinarySource::JumpTo( TSTime & ReachTime ,std::list<TSCollectionDataPtr> & DataList)
{
	T_D();

	std::vector<TSRecordFileIOStructPtr> IOStructs;

	if(ReachTime < d->_NextValidTime)
	{
		TSRecordFileIOStructPtr IOStruct;
		while(d->_IOStructs.try_pop(IOStruct))
		{
			JumpToForward<TSCollectionData,TSTopicRecordStruct>(IOStruct,ReachTime);

			GetCacheDataList(DataList,IOStruct);

			IOStructs.push_back(IOStruct);
		}
	}
	else if(ReachTime > d->_NextValidTime)
	{
		TSRecordFileIOStructPtr IOStruct;
		while(d->_IOStructs.try_pop(IOStruct))
		{
			JumpToBackward<TSCollectionData,TSTopicRecordStruct>(IOStruct,ReachTime);

			GetCacheDataList(DataList,IOStruct);

			IOStructs.push_back(IOStruct);
		}
	}
	else
	{
		return ;
	}

	for(size_t i = 0; i < IOStructs.size(); ++i)
	{
		d->_IOStructs.push(IOStructs[i]);
	}

	d->_NextValidTime = ReachTime;

	if(!d->_IOStructs.size())
	{
		d->_NextValidTime = boost::posix_time::not_a_date_time;
	}
}

const TSTime & TSBinarySource::GetNextValidDataTime()
{
	T_D();

	return d->_NextValidTime;
}

const void * TSCollectionData::GetData()
{

    return this->Data->Extendable;
}

const UINT32 TSCollectionData::GetLength()
{
    UINT32 size = Data->SizeOfThis - (sizeof(TSTopicRecordStruct) - sizeof(Data->Extendable));

    return size;
}

const TSTOPICHANDLE TSCollectionData::GetTopicHandle()
{
    return _TopicId;
}

const UINT64 TSCollectionData::GetTime()
{
    return Data->Time;
}

#endif //_WRS_KERNEL
