#include "stdafx.h"
#include "TopSimDataInterface/TSFileSender.h"
#include "TopSimDataInterface/TSDataInteractManager.h"
#include "boost/filesystem.hpp"

TSFileSender::TSFileSender(TSDomainPtr domainPtr) :
	_fileTransfersDomain(domainPtr),
	_blockSize(1024 * 60),
	_senderAppId(0),
	_countSyncIosRun(5),
	_indexCurIosRun(0),
	_asyncIosRun(boost::make_shared<TSIOSStruct>())
{
	if (!_fileTransfersDomain)
	{
		return;
	}
	_senderAppId = DataManager->GetFrontAppUniquelyId();

	_fileTransfersDomain->PublishTopic(Huaru_FileTransfers_SenderFileInfo);
	_fileTransfersDomain->PublishTopic(Huaru_FileTransfers_SenderFileTransfersInfo);
	_fileTransfersDomain->PublishTopic(Huaru_FileTransfers_TransfersFileErr);

	_fileTransfersDomain->RegisterTopicCallback(Huaru_FileTransfers_ReceiverFileInfoRet, boost::bind(&TSFileSender::_CallBackReceiverFileInfoRet, this, _1));
	_fileTransfersDomain->SubscribeTopic(Huaru_FileTransfers_ReceiverFileInfoRet);
	_fileTransfersDomain->RegisterTopicCallback(Huaru_FileTransfers_ReceiverFileTransfersInfoRet, boost::bind(&TSFileSender::_CallBackReceiverFileTransfersInfoRet, this, _1));
	_fileTransfersDomain->SubscribeTopic(Huaru_FileTransfers_ReceiverFileTransfersInfoRet);
	_fileTransfersDomain->RegisterTopicCallback(Huaru_FileTransfers_DownloadFileRequest, boost::bind(&TSFileSender::_CallBackReceiverDownloadFileRequest, this, _1));
	_fileTransfersDomain->SubscribeTopic(Huaru_FileTransfers_DownloadFileRequest);
	_fileTransfersDomain->RegisterTopicCallback(Huaru_FileTransfers_TransfersFileErr, boost::bind(&TSFileSender::_CallBackTransfersFileErr, this, _1));
	_fileTransfersDomain->SubscribeTopic(Huaru_FileTransfers_TransfersFileErr);

	_asyncIosRun->RunAlways();
	for (int i = 0; i < _countSyncIosRun; ++i)
	{
#ifndef __VXWORKS__
		_vetSyncIosRun.push_back(std::move(boost::make_shared<TSIOSStruct>()));
#else
		_vetSyncIosRun.push_back(boost::make_shared<TSIOSStruct>());
#endif

		_vetSyncIosRun.back()->RunAlways();
	}
}

TSFileSender::~TSFileSender()
{
	TSRWSpinLock::WriteLock l(_lockFileMarkId2SenderFileInfo);
#ifndef __VXWORKS__
	for (auto it = _mapFileMarkId2SenderFileInfo.end(); it != _mapFileMarkId2SenderFileInfo.end(); )
#else
	for (std::map<UINT32, SenderFileInfo*>::iterator it = _mapFileMarkId2SenderFileInfo.end(); it != _mapFileMarkId2SenderFileInfo.end(); )
#endif

	{
		if (it->second)
		{
			SenderFileInfo * t_transfersFileInfoPrt = it->second;
			it = _mapFileMarkId2SenderFileInfo.erase(it);

			if (t_transfersFileInfoPrt)
			{
				if (t_transfersFileInfoPrt->_ifStream.is_open())
				{
					t_transfersFileInfoPrt->_ifStream.close();
				}
				if (t_transfersFileInfoPrt->_buff)
				{
					delete(t_transfersFileInfoPrt->_buff);
				}
				t_transfersFileInfoPrt->_conditionVariable.notify_one();
				delete(t_transfersFileInfoPrt);
			}
		}
		else
		{
			++it;
		}
	}
}

/* ?????????????????????????? */
void TSFileSender::RegRelaticePath2RealPathCallBack(std::function<TSString(const TSString &)> & realativePath2RealPathFun)
{
	_relativePath2RealPathFun = realativePath2RealPathFun;
}

/* ???????????????????????????? */
void TSFileSender::RegLogSenderFileInfoFunCallBack(std::function<void(const LogSenderFileInfo &)> & logSenderFileInfoFun)
{
	_logSenderFileInfoFun = logSenderFileInfoFun;
}

/* ?????????????????????????????????? */
void TSFileSender::RegLogSenderFileInfoAckFunCallBack(std::function<void(const LogSenderFileInfoAck &)> & logSenderFileInfoAckFun)
{
	_logSenderFileInfoAckFun = logSenderFileInfoAckFun;
}

/* ???????????????????????????? */
void TSFileSender::RegLogSenderContentInfoFunCallBack(std::function<void(const LogSenderContentInfo &)> & logSenderContentInfoFun)
{
	_logSenderContentInfoFun = logSenderContentInfoFun;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ????????????????????. </summary>
///
/// <remarks>   ??????, 2020/7/7. </remarks>
///
/// <param name="filePath">   ????????. </param>
/// <param name="receiverAppId">  ??????AppId. </param>
/// <param name="transfersFormat">??????????0-????????1-???? </param>
/// <param name="verifyBlockType"> ????????????????0-????????1-MD5??2-SHA1??3-CRC32 </param>
/// <param name="receiveDir"> ?????????????????? </param>
///
/// <returns>	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 TSFileSender::SendFileToReceiver(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat, UINT8 verifyBlockType, const TSString & receiveDir)
{
	return _CreateSendFileTask(filePath, receiverAppId, (UINT8)1, transfersFormat, verifyBlockType, receiveDir, (UINT8)0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ????????????????????. </summary>
///
/// <remarks>   ??????, 2020/7/7. </remarks>
///
/// <param name="filePath">   ????????. </param>
/// <param name="receiverAppId">  ??????AppId. </param>
/// <param name="transfersFormat">??????????0-????????1-???? </param>
/// <param name="receiveDir"> ?????????????????? </param>
///
/// <returns>	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 TSFileSender::SendFileToReceiverAsync(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat, const TSString & receiveDir)
{
	return _CreateSendFileTask(filePath, receiverAppId, (UINT8)0, transfersFormat, (UINT8)0, receiveDir, (UINT8)0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ??????????????????????????????????????????????????. </summary>
///
/// <remarks>   ??????, 2020/7/7. </remarks>
///
/// <param name="filePath">   ????????. </param>
/// <param name="receiverAppId">  ??????AppId. </param>
/// <param name="transfersFormat">??????????0-????????1-???? </param>
/// <param name="verifyBlockType"> ????????????????0-????????1-MD5??2-SHA1??3-CRC32 </param>
///
/// <returns>	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 TSFileSender::TransferFileToReceiver(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat, UINT8 verifyBlockType)
{
	return _CreateSendFileTask(filePath, receiverAppId, (UINT8)1, transfersFormat, verifyBlockType, "", (UINT8)1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ??????????????????????????????????????????????????. </summary>
///
/// <remarks>   ??????, 2020/7/7. </remarks>
///
/// <param name="filePath">   ????????. </param>
/// <param name="receiverAppId">  ??????AppId. </param>
/// <param name="transfersFormat">??????????0-????????1-???? </param>
///
/// <returns>	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 TSFileSender::TransferFileToReceiverAsync(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat)
{
	return _CreateSendFileTask(filePath, receiverAppId, (UINT8)0, transfersFormat, (UINT8)0, "", (UINT8)1);
}



/* ???????????????????????????????? */
void TSFileSender::_CallBackReceiverFileInfoRet(TSTopicContextPtr Ctx)
{
	Huaru::FileTransfers::ReceiverFileInfoRet::DataTypePtr dtPtr =
		TS_CAST(Ctx->GetTopic(), Huaru::FileTransfers::ReceiverFileInfoRet::DataTypePtr);

	if (dtPtr->_senderAppId != _senderAppId)
	{
		return;
	}

	//????????????
	Huaru::FileTransfers::TransfersFileErr::DataType dt;
	dt._fileMarkId = dtPtr->_fileMarkId;
	dt._errCode = Huaru::FileTransfers::FTS_ACK_OK;

	//????????????????????????
	LogSenderFileInfoAck t_logSenderFileInfoAck;
	t_logSenderFileInfoAck._ret = Huaru::FileTransfers::FTS_ACK_OK;
	t_logSenderFileInfoAck._receiverAppId = dtPtr->_receiverAppId;
	t_logSenderFileInfoAck._fileMarkId = dtPtr->_fileMarkId;
	t_logSenderFileInfoAck._receiverRet = dtPtr->_receiveFileReady;

	//????????????????????
	SenderFileInfo * t_senderFileInfoPtr = _SelectSenderFileInfoByFileMarkId(dtPtr->_fileMarkId);
	//????????????????????????????????????????????
	if (!t_senderFileInfoPtr)
	{
		dt._errCode = Huaru::FileTransfers::FTS_ACK_NOTASK_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_TransfersFileErr, &dt, dtPtr->_receiverAppId);
		
		if (_logSenderFileInfoAckFun)
		{
			t_logSenderFileInfoAck._ret = Huaru::FileTransfers::FTS_ACK_NOTASK_ERR;
			_logSenderFileInfoAckFun(t_logSenderFileInfoAck);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????????????????????????????!!! FileMarkId=[%1]")) << dtPtr->_fileMarkId;
		}
		return;
	}

	//??????????????????????????????????????????????????????????
	if (dtPtr->_receiveFileReady != 0)
	{
		dt._errCode = Huaru::FileTransfers::FTS_ACK_RECEIVE_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_TransfersFileErr, &dt, dtPtr->_receiverAppId);

		if (_logSenderFileInfoAckFun)
		{
			t_logSenderFileInfoAck._ret = Huaru::FileTransfers::FTS_ACK_RECEIVE_ERR;
			_logSenderFileInfoAckFun(t_logSenderFileInfoAck);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("??????????????????????????!!! FileName=[%1], ErrCode=[%2]")) << t_senderFileInfoPtr->_fileName << dtPtr->_receiveFileReady;
		}

		//??????????????????
		_DeleteSenderFileInfoByFileMarkId(dtPtr->_fileMarkId);
		return;
	}

	//????????
	if (t_senderFileInfoPtr->_transfersMode == 0)
	{
		//??????????????????????????????????????
#if defined(XSIM3_2) || defined(XSIM3_3)
		_asyncIosRun->GetIOS()->post(boost::bind(&TSFileSender::_SendFileContentAsync, this, dtPtr->_fileMarkId, dtPtr->_receiverAppId));
#else
		_asyncIosRun->GetIOS()->Post(boost::bind(&TSFileSender::_SendFileContentAsync, this, dtPtr->_fileMarkId, dtPtr->_receiverAppId));
#endif

		if (_logSenderFileInfoAckFun)
		{
			t_logSenderFileInfoAck._ret = Huaru::FileTransfers::FTS_ACK_OK;
			_logSenderFileInfoAckFun(t_logSenderFileInfoAck);
		}
		else
		{
			DEF_LOG_KERNEL(GB18030ToTSString("????????????????????????... FileName=[%1]")) << t_senderFileInfoPtr->_fileName;
		}
	}
	//????????
	else
	{
		TSRWSpinLock::WriteLock l(_lockSyncIosRun);
		t_senderFileInfoPtr->_threadIndex = _indexCurIosRun;
#if defined(XSIM3_2) || defined(XSIM3_3)
		_vetSyncIosRun[_indexCurIosRun]->GetIOS()->post(boost::bind(&TSFileSender::_SendFileContent, this, dtPtr->_fileMarkId, dtPtr->_receiverAppId));
#else
		_vetSyncIosRun[_indexCurIosRun]->GetIOS()->Post(boost::bind(&TSFileSender::_SendFileContent, this, dtPtr->_fileMarkId, dtPtr->_receiverAppId));
#endif

		if (_logSenderFileInfoAckFun)
		{
			t_logSenderFileInfoAck._ret = Huaru::FileTransfers::FTS_ACK_OK;
			_logSenderFileInfoAckFun(t_logSenderFileInfoAck);
		}
		else
		{
			DEF_LOG_KERNEL(GB18030ToTSString("????????????????????????... FileName=[%1]??ThreadIndex=[%2]")) << t_senderFileInfoPtr->_fileName << _indexCurIosRun;
		}
		_indexCurIosRun = (++_indexCurIosRun) % _countSyncIosRun;
	}
	return;
}

/* ?????????????????????????????????????? */
void TSFileSender::_CallBackReceiverFileTransfersInfoRet(TSTopicContextPtr Ctx)
{
	Huaru::FileTransfers::ReceiverFileTransfersInfoRet::DataTypePtr dtPtr =
		TS_CAST(Ctx->GetTopic(), Huaru::FileTransfers::ReceiverFileTransfersInfoRet::DataTypePtr);

	if (dtPtr->_senderAppId != _senderAppId)
	{
		return;
	}

	Huaru::FileTransfers::TransfersFileErr::DataType dtErr;
	dtErr._fileMarkId = dtPtr->_fileMarkId;
	dtErr._senderAppId = _senderAppId;
	dtErr._receiverAppId = dtPtr->_receiverAppId;
	dtErr._errCode = Huaru::FileTransfers::FTS_CTX_OK;

	LogSenderContentInfo t_logSenderContentInfo;
	t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_OK;

	SenderFileInfo * t_senderFileInfoPtr = _SelectSenderFileInfoByFileMarkId(dtPtr->_fileMarkId);
	if (!t_senderFileInfoPtr)
	{
		dtErr._errCode = Huaru::FileTransfers::FTS_CTX_FINISH_OK;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_TransfersFileErr, &dtErr, dtPtr->_receiverAppId);

		if (_logSenderContentInfoFun)
		{
			t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_NOTASK_ERR;
			_logSenderContentInfoFun(t_logSenderContentInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????????????????????????????!!! FileMarkId=[%1]")) << dtPtr->_fileMarkId;
		}
		return;
	}

	t_senderFileInfoPtr->_currBlockIndex = dtPtr->_expectBlockIndex;
	//??????????????????????????
	boost::mutex::scoped_lock l(t_senderFileInfoPtr->_mutexLock);
	t_senderFileInfoPtr->_conditionVariable.notify_one();
	return;
}

/* ?????????????????????????????????? */
void TSFileSender::_CallBackReceiverDownloadFileRequest(TSTopicContextPtr Ctx)
{
	Huaru::FileTransfers::DownloadFileRequest::DataTypePtr dtPtr =
		TS_CAST(Ctx->GetTopic(), Huaru::FileTransfers::DownloadFileRequest::DataTypePtr);

	if (dtPtr->_senderAppId != _senderAppId)
	{
		return;
	}

	TSString realPath = dtPtr->_filePath;
	if (_relativePath2RealPathFun)
	{
		realPath = _relativePath2RealPathFun(dtPtr->_filePath);
	}

	_CreateSendFileTask(realPath, dtPtr->_receiverAppId, dtPtr->_transfersMode, dtPtr->_transfersFormat, dtPtr->_verifyBlockType, "", (UINT8)2);

	return;
}

/* ???????????????????????????????????? */
void TSFileSender::_CallBackTransfersFileErr(TSTopicContextPtr Ctx)
{
	Huaru::FileTransfers::TransfersFileErr::DataTypePtr dtPtr =
		TS_CAST(Ctx->GetTopic(), Huaru::FileTransfers::TransfersFileErr::DataTypePtr);

	if (dtPtr->_senderAppId != _senderAppId)
	{
		return;
	}

	if (dtPtr->_errCode == 0)
	{
		return;
	}

	_DeleteSenderFileInfoByFileMarkId(dtPtr->_fileMarkId);
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ?????????? </summary>
///
/// <remarks>   ??????, 2020/7/10. </remarks>
///
/// <param name="receiverAppId">   ??????AppId. </param>
///
/// <returns> void	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
bool TSFileSender::_DiscoveryReceiver(UINT32 receiverAppId)
{
	TSTopicQuerySet publisherQuerySet;
	TSTopicQuerySet scriberQuerySet;

	scriberQuerySet.push_back(Huaru_FileTransfers_ReceiverFileInfoRet);
	scriberQuerySet.push_back(Huaru_FileTransfers_ReceiverFileTransfersInfoRet);
	scriberQuerySet.push_back(Huaru_FileTransfers_TransfersFileErr);

	publisherQuerySet.push_back(Huaru_FileTransfers_SenderFileInfo);
	publisherQuerySet.push_back(Huaru_FileTransfers_SenderFileTransfersInfo);
	publisherQuerySet.push_back(Huaru_FileTransfers_TransfersFileErr);

	//????????????
	if (TSWaitSet::WaitForDiscoveryComplete(_fileTransfersDomain, publisherQuerySet, receiverAppId, 5000, TSWaitSet::kPublisher) &&
		TSWaitSet::WaitForDiscoveryComplete(_fileTransfersDomain, scriberQuerySet, receiverAppId, 5000, TSWaitSet::kSubscriber))
	{
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ???????????????? </summary>
///
/// <remarks>   ??????, 2020/7/10. </remarks>
///
/// <param name="fileMarkId">   ????????????????. </param>
///
/// <returns> void	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
void TSFileSender::_SendFileContent(UINT32 fileMarkId, UINT32 receiverAppId)
{
	//????????????
	Huaru::FileTransfers::TransfersFileErr::DataType dtErr;
	dtErr._senderAppId = _senderAppId;
	dtErr._receiverAppId = receiverAppId;
	dtErr._fileMarkId = fileMarkId;
	dtErr._errCode = Huaru::FileTransfers::FTS_CTX_OK;

	//????????????????????
	LogSenderContentInfo t_logSenderContentInfo;
	t_logSenderContentInfo._ret = 0;
	t_logSenderContentInfo._fileMarkId = fileMarkId;
	t_logSenderContentInfo._currBlockIndex = 0;

	//????????????????????????????
	SenderFileInfo * t_senderFileInfoPtr = _SelectSenderFileInfoByFileMarkId(fileMarkId);
	if (!t_senderFileInfoPtr)
	{
		dtErr._errCode = Huaru::FileTransfers::FTS_CTX_NOTASK_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_TransfersFileErr, &dtErr, receiverAppId);

		if (_logSenderContentInfoFun)
		{
			t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_NOTASK_ERR;
			_logSenderContentInfoFun(t_logSenderContentInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????????????????????????????!!! FileMarkId=[%1]")) << fileMarkId;
		}
		return;
	}


	if (t_senderFileInfoPtr->_buff == NULL)
	{
		t_senderFileInfoPtr->_buff = new char[_blockSize];
	}
	
	memset(t_senderFileInfoPtr->_buff, 0, _blockSize);
	std::streamsize	t_realReadCount = 0;
	UINT32	t_blockIndex = 0;
	do
	{
		t_logSenderContentInfo._currBlockIndex = t_blockIndex;

		memset(t_senderFileInfoPtr->_buff, 0, _blockSize);
		t_realReadCount = 0;

		//??????????????????????
		t_senderFileInfoPtr->_ifStream.seekg(t_senderFileInfoPtr->_currBlockIndex * _blockSize, (std::ios_base::seekdir)0);
		t_senderFileInfoPtr->_ifStream.read(t_senderFileInfoPtr->_buff, _blockSize);
		t_realReadCount = t_senderFileInfoPtr->_ifStream.gcount();
		if (!t_realReadCount)
		{
			break;
		}
		Huaru::FileTransfers::SenderFileTransfersInfo::DataType dt;
		dt._fileMarkId = t_senderFileInfoPtr->_fileMarkId;
		dt._senderAppId = _senderAppId;
		dt._receiverAppId = receiverAppId;
		dt._blockIndex = t_senderFileInfoPtr->_currBlockIndex;
		dt._blockRealSize = t_realReadCount;
		dt._isEndBlock = (t_blockIndex == t_senderFileInfoPtr->_blockNum - 1) ? 1 : 0;
		dt._verifyBlockType = 0;
		dt._blockContent.resize(t_realReadCount);
		dt._blockContent.assign(&t_senderFileInfoPtr->_buff[0], &t_senderFileInfoPtr->_buff[t_realReadCount]);

		//??????????????????????????
		boost::mutex::scoped_lock l(t_senderFileInfoPtr->_mutexLock);

		//??????????????????
		if (_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_SenderFileTransfersInfo, &dt, t_senderFileInfoPtr->_receiverAppId))
		{
			t_senderFileInfoPtr->_completeBlocks[t_blockIndex] = true;
		}
		else
		{
			t_senderFileInfoPtr->_completeBlocks[t_blockIndex] = false;

			if (_logSenderContentInfoFun)
			{
				t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_SENDTOPIC_ERR;
				_logSenderContentInfoFun(t_logSenderContentInfo);
			}
			else
			{
				DEF_LOG_ERROR(GB18030ToTSString("??????????????????????!!! FileName=[%1], BlockIndex=[%2]"))
					<< t_senderFileInfoPtr->_fileName << t_blockIndex << t_senderFileInfoPtr->_threadIndex;
			}
			continue;
		}

		//??????????????????????
		if (!t_senderFileInfoPtr->_conditionVariable.timed_wait(l, TSMilliseconds(5000)))
		{
			dtErr._errCode = Huaru::FileTransfers::FTS_CTX_ACK_TIMEOUT_ERR;
			_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_TransfersFileErr, &dtErr, receiverAppId);

			if (_logSenderContentInfoFun)
			{
				t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_ACK_TIMEOUT_ERR;
				_logSenderContentInfoFun(t_logSenderContentInfo);
			}
			else
			{
				DEF_LOG_ERROR(GB18030ToTSString("??????????????????????????!!! FileName=[%1], BlockIndex=[%2]"))
					<< t_senderFileInfoPtr->_fileName << t_blockIndex;
			}

			break;
		}

		//????????
		if (_logSenderContentInfoFun)
		{
			t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_OK;
			_logSenderContentInfoFun(t_logSenderContentInfo);
		}
		else
		{
			DEF_LOG_KERNEL(GB18030ToTSString("??????????????...FileName=[%1], BlockIndex=[%2], ThreadIndex=[%3]"))
				<< t_senderFileInfoPtr->_fileName << t_blockIndex << t_senderFileInfoPtr->_threadIndex;
		}

		//??????????????
		++t_blockIndex;

		//????????
		if (t_blockIndex == t_senderFileInfoPtr->_blockNum)
		{
			if (_logSenderContentInfoFun)
			{
				t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_FINISH_OK;
				_logSenderContentInfoFun(t_logSenderContentInfo);
			}
			else
			{
				DEF_LOG_KERNEL(GB18030ToTSString("????????????????...FileMarkId=[%1], FileName=[%2], ThreadIndex=[%3]"))
					<< dt._fileMarkId << t_senderFileInfoPtr->_fileName << t_senderFileInfoPtr->_threadIndex;
				t_senderFileInfoPtr->_ifStream.close();
			}
		}

	} while (t_blockIndex != t_senderFileInfoPtr->_blockNum);

	//????????????????????????????
	_DeleteSenderFileInfoByFileMarkId(t_senderFileInfoPtr->_fileMarkId);
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ???????????????? </summary>
///
/// <remarks>   ??????, 2020/7/10. </remarks>
///
/// <param name="fileMarkId">   ????????????????. </param>
///
/// <returns> void	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
void TSFileSender::_SendFileContentAsync(UINT32 fileMarkId, UINT32 receiverAppId)
{
	//????????????
	Huaru::FileTransfers::TransfersFileErr::DataType dtErr;
	dtErr._senderAppId = _senderAppId;
	dtErr._receiverAppId = receiverAppId;
	dtErr._fileMarkId = fileMarkId;
	dtErr._errCode = 0;

	//????????????????????
	LogSenderContentInfo t_logSenderContentInfo;
	t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_OK;
	t_logSenderContentInfo._fileMarkId = fileMarkId;
	t_logSenderContentInfo._currBlockIndex = 0;

	//??????????????
	SenderFileInfo * t_senderFileInfoPtr = _SelectSenderFileInfoByFileMarkId(fileMarkId);
	if (!t_senderFileInfoPtr)
	{
		dtErr._errCode = Huaru::FileTransfers::FTS_ACK_NOTASK_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_TransfersFileErr, &dtErr, receiverAppId);

		if (_logSenderContentInfoFun)
		{
			t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_ACK_NOTASK_ERR;
			_logSenderContentInfoFun(t_logSenderContentInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????????????????????????????!!! FileMarkId=[%1]")) << fileMarkId;
		}
		return;
	}

	

	if (t_senderFileInfoPtr->_buff == NULL)
	{
		t_senderFileInfoPtr->_buff = new char[_blockSize];
	}
	memset(t_senderFileInfoPtr->_buff, 0, _blockSize);
	std::streamsize	t_realReadCount = 0;
	UINT32	t_blockIndex = 0;
	do
	{
		t_logSenderContentInfo._currBlockIndex = t_blockIndex;

		memset(t_senderFileInfoPtr->_buff, 0, _blockSize);
		t_realReadCount = 0;

		t_senderFileInfoPtr->_ifStream.read(t_senderFileInfoPtr->_buff, _blockSize);
		t_realReadCount = t_senderFileInfoPtr->_ifStream.gcount();
		if (!t_realReadCount)
		{
			break;
		}
		Huaru::FileTransfers::SenderFileTransfersInfo::DataType dt;
		
		dt._fileMarkId = t_senderFileInfoPtr->_fileMarkId;
		dt._senderAppId = _senderAppId;
		dt._receiverAppId = t_senderFileInfoPtr->_receiverAppId;
		dt._blockIndex = t_blockIndex;
		dt._blockRealSize = t_realReadCount;
		dt._isEndBlock = (t_blockIndex == t_senderFileInfoPtr->_blockNum - 1) ? 1 : 0;
		dt._verifyBlockType = 0;
		dt._blockContent.resize(t_realReadCount);
		dt._blockContent.assign(&t_senderFileInfoPtr->_buff[0], &t_senderFileInfoPtr->_buff[t_realReadCount]);
		
		//??????????????????
		if (_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_SenderFileTransfersInfo, &dt, t_senderFileInfoPtr->_receiverAppId))
		{
			t_senderFileInfoPtr->_completeBlocks[t_blockIndex] = true;
		}
		else
		{
			t_senderFileInfoPtr->_completeBlocks[t_blockIndex] = false;

			if (_logSenderContentInfoFun)
			{
				t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_SENDTOPIC_ERR;
				_logSenderContentInfoFun(t_logSenderContentInfo);
			}
			else
			{
				DEF_LOG_KERNEL(GB18030ToTSString("??????????????????????!!! FileName=[%1], BlockIndex=[%2]"))
					<< t_senderFileInfoPtr->_fileName << t_blockIndex;
			}

			continue;
		}

		//??????????????????????
		if (_logSenderContentInfoFun)
		{
			t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_OK;
			_logSenderContentInfoFun(t_logSenderContentInfo);
		}
		else
		{
			DEF_LOG_KERNEL(GB18030ToTSString("??????????????...FileName=[%1], BlockIndex=[%2]"))
				<< t_senderFileInfoPtr->_fileName << t_blockIndex;
		}
		
		t_senderFileInfoPtr->_currBlockIndex = ++t_blockIndex;
		
		if (t_senderFileInfoPtr->_currBlockIndex == t_senderFileInfoPtr->_blockNum)
		{
			if (_logSenderContentInfoFun)
			{
				t_logSenderContentInfo._ret = Huaru::FileTransfers::FTS_CTX_FINISH_OK;
				_logSenderContentInfoFun(t_logSenderContentInfo);
			}
			else
			{
				DEF_LOG_KERNEL(GB18030ToTSString("????????????????...FileMarkId=[%1], FileName=[%2]"))
					<< dt._fileMarkId << t_senderFileInfoPtr->_fileName;
			}
			t_senderFileInfoPtr->_ifStream.close();
		}

	} while (t_blockIndex != t_senderFileInfoPtr->_blockNum);

	//????????????????????????????
	_DeleteSenderFileInfoByFileMarkId(t_senderFileInfoPtr->_fileMarkId);
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ?????????????????????? </summary>
///
/// <remarks>   ??????, 2020/7/7. </remarks>
///
/// <param name="fileName">   ????????. </param>
/// <param name="receiverAppId">   ??????AppId. </param>
///
/// <returns> bool??true-??????????false-????????	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 TSFileSender::_GetTransfersFileMarkId(const TSString & fileName, UINT32 receiverAppId)
{
	TSString t_tmpStr = fileName + TSValue_Cast<TSString>(_senderAppId) + TSValue_Cast<TSString>(receiverAppId);
	return GetCRC32(TSString2Ascii(t_tmpStr));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ???????????????????????? </summary>
///
/// <remarks>   ??????, 2020/7/7. </remarks>
///
/// <param name="filePath">   ????????. </param>
/// <param name="fileName">   ????????. </param>
///
/// <returns> bool??true-??????????false-????????	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
bool TSFileSender::_GetFileNameByFilePath(const TSString & filePath, TSString & fileName)
{
	try
	{
		fileName = boost::filesystem::path(filePath).filename().string();
	}
	catch (boost::filesystem::filesystem_error & e)
	{
		e.path1();
		e.what();
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ???????????????????????? </summary>
///
/// <remarks>   ??????, 2020/7/7. </remarks>
///
/// <param name="filePath">   ????????. </param>
/// <param name="fileSize">   ????????. </param>
///
/// <returns> bool??true-??????????false-????????	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
bool TSFileSender::_GetFileSizeByFilePath(const TSString & filePath, UINT64 & fileSize)
{
	fileSize = 0;
	try
	{
		fileSize = boost::filesystem::file_size(filePath);
	}
	catch (boost::filesystem::filesystem_error & e)
	{
		e.path1();
		e.what();
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ????????????. </summary>
///
/// <remarks>   ??????, 2020/7/7. </remarks>
///
/// <param name="filePath">   ????????. </param>
/// <param name="receiverAppId">  ??????AppId. </param>
/// <param name="receiveDir"> ?????????????????? </param>
/// <param name="transfersMode">  ??????????0-??????1-???? </param>
/// <param name="transfersFormat"> ??????????0-????????1-???? </param>
/// <param name="verifyBlockType">  ????????????????0-????????1-MD5??2-SHA1??3-CRC32 </param>
/// <param name="transfersType">  ??????????0-????????????????????????????1-????????????????????2-??????????????. </param>
///
/// <returns> UINT32	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 TSFileSender::_CreateSendFileTask(
	const TSString & filePath,
	UINT32 receiverAppId,
	UINT8 transfersMode,
	UINT8 transfersFormat,
	UINT8 verifyBlockType,
	const TSString & receiveDir,
	UINT8 transfersType)
{
	TSString t_fileName;
	UINT32	t_fileMarkId = 0;
	UINT32	t_blockNum = 0;
	UINT64	t_fileSize = 0;

	LogSenderFileInfo t_logSenderFileInfo;
	t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_OK;
	t_logSenderFileInfo._receiverAppId = receiverAppId;
	t_logSenderFileInfo._blockSize = _blockSize;
	t_logSenderFileInfo._transfersFormat = transfersFormat;
	t_logSenderFileInfo._transfersMode = transfersMode;
	t_logSenderFileInfo._transfersType = transfersType;
	t_logSenderFileInfo._verifyBlockType = verifyBlockType;
	t_logSenderFileInfo._fileName = filePath;

	//??????????
	if (!_DiscoveryReceiver(receiverAppId))
	{
		if (_logSenderFileInfoFun)
		{
			t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_DISCOVERY_ERR;
			_logSenderFileInfoFun(t_logSenderFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("??????????????????!!! ReceiverAppId=[%1]")) << receiverAppId;
		}

		return Huaru::FileTransfers::FTS_DISCOVERY_ERR;
	}

	//????????????????????????
	if (!_GetFileNameByFilePath(filePath, t_fileName) || t_fileName.empty())
	{
		if (_logSenderFileInfoFun)
		{
			t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_FILENAME_ERR;
			_logSenderFileInfoFun(t_logSenderFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("??????????????????????????????!!! FilePath=[%1]")) << filePath;
		}
		return Huaru::FileTransfers::FTS_FILENAME_ERR;
	}

	//????????????
	if (!_GetFileSizeByFilePath(filePath, t_fileSize) || t_fileSize == 0)
	{
		if (_logSenderFileInfoFun)
		{
			t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_FILESIZE_ERR;
			_logSenderFileInfoFun(t_logSenderFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????????!!! FilePath=[%1]")) << filePath;
		}
		return Huaru::FileTransfers::FTS_FILESIZE_ERR;
	}
	t_logSenderFileInfo._fileSize = t_fileSize;

	//??????????????????????????????+??????ID+??????ID??
	t_fileMarkId = _GetTransfersFileMarkId(t_fileName, receiverAppId);
	if (t_fileMarkId == 0)
	{
		if (_logSenderFileInfoFun)
		{
			t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_FILEMARKID_ERR;
			_logSenderFileInfoFun(t_logSenderFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????????!!! FilePath=[%1], SenderAppId=[%2], ReceiverAppId=[%3]"))
				<< filePath << _senderAppId << receiverAppId;
		}
		return Huaru::FileTransfers::FTS_FILEMARKID_ERR;
	}
	t_logSenderFileInfo._fileMarkId = t_fileMarkId;

	//??????????????
	if (_SelectSenderFileInfoByFileMarkId(t_fileMarkId))
	{
		if (_logSenderFileInfoFun)
		{
			t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_TASKEXIST_ERR;
			_logSenderFileInfoFun(t_logSenderFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????????!!! FilePath=[%1], FileMarkId=[%2]")) << filePath << t_fileMarkId;
		}

		return Huaru::FileTransfers::FTS_TASKEXIST_ERR;

	}

	t_blockNum = (UINT32)(t_fileSize / _blockSize) + (t_fileSize % _blockSize ? 1 : 0);
	SenderFileInfo t_senderFileInfo(t_blockNum);
	t_senderFileInfo._fileMarkId = t_fileMarkId;
	t_senderFileInfo._receiverAppId = receiverAppId;
	t_senderFileInfo._blockSize = _blockSize;
	t_senderFileInfo._blockNum = t_blockNum;
	t_senderFileInfo._lastBlockSize = t_fileSize % _blockSize ? t_fileSize % _blockSize : _blockSize;
	t_senderFileInfo._currBlockIndex = 0;
	t_senderFileInfo._threadIndex = 0;
	t_senderFileInfo._transfersType = transfersType;
	t_senderFileInfo._transfersMode = transfersMode;
	t_senderFileInfo._transfersFormat = transfersFormat;
	t_senderFileInfo._verifyBlockType = verifyBlockType;
	t_senderFileInfo._fileSize = t_fileSize;
	t_senderFileInfo._fileName = t_fileName;
	t_senderFileInfo._receiveDir = receiveDir;

	//??????????????
	if (!_InsertSenderFileInfo(t_senderFileInfo))
	{
		if (_logSenderFileInfoFun)
		{
			t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_CREATETASK_ERR;
			_logSenderFileInfoFun(t_logSenderFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????????!!! FilePath=[%1]")) << filePath;
		}

		return Huaru::FileTransfers::FTS_CREATETASK_ERR;
	}

	SenderFileInfo * t_senderFileInfoPtr = _SelectSenderFileInfoByFileMarkId(t_fileMarkId);
	if (!t_senderFileInfoPtr)
	{
		if (_logSenderFileInfoFun)
		{
			t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_CREATETASK_ERR;
			_logSenderFileInfoFun(t_logSenderFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????????!!! FilePath=[%1]")) << filePath;
		}

		return Huaru::FileTransfers::FTS_CREATETASK_ERR;
	}

	//??????
	if (transfersFormat == 0)
	{
		t_senderFileInfoPtr->_ifStream.open(filePath.c_str(), std::ios_base::in | std::ios_base::binary);
	}
	else if (transfersFormat == 1)
	{
		t_senderFileInfoPtr->_ifStream.open(filePath.c_str(), std::ios_base::in);
	}

	//????????????
	if (!t_senderFileInfoPtr->_ifStream.good())
	{
		if (_logSenderFileInfoFun)
		{
			t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_OPENFILE_ERR;
			_logSenderFileInfoFun(t_logSenderFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????!!! FilePath=[%1]")) << filePath;	
		}

		_DeleteSenderFileInfoByFileMarkId(t_fileMarkId);

		return Huaru::FileTransfers::FTS_OPENFILE_ERR;
	}

	Huaru::FileTransfers::SenderFileInfo::DataType dt;
	dt._senderAppId = _senderAppId;
	dt._receiverAppId = receiverAppId;
	dt._blockSize = t_senderFileInfo._blockSize;
	dt._fileMarkId = t_senderFileInfo._fileMarkId;
	dt._blockNum = t_senderFileInfo._blockNum;
	dt._transfersType = t_senderFileInfo._transfersType;
	dt._transfersFormat = t_senderFileInfo._transfersFormat;
	dt._transfersMode = t_senderFileInfo._transfersMode;
	dt._verifyBlockType = t_senderFileInfo._verifyBlockType;
	dt._fileSize = t_senderFileInfo._fileSize;
	dt._fileName = t_senderFileInfo._fileName;
	dt._receiveDir = t_senderFileInfo._receiveDir;

	if (!_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_SenderFileInfo, &dt, receiverAppId))
	{
		if (_logSenderFileInfoFun)
		{
			t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_SENDTOPIC_ERR;
			_logSenderFileInfoFun(t_logSenderFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("????????????????????!!! FilePath=[%1]")) << filePath;
		}

		//??????????????????
		_DeleteSenderFileInfoByFileMarkId(t_senderFileInfo._fileMarkId);

		return Huaru::FileTransfers::FTS_SENDTOPIC_ERR;
	}

	if (_logSenderFileInfoFun)
	{
		t_logSenderFileInfo._ret = Huaru::FileTransfers::FTS_OK;
		_logSenderFileInfoFun(t_logSenderFileInfo);
	}
	else
	{
		DEF_LOG_KERNEL(GB18030ToTSString("????????????????... FileMarkId=[%1], FileName=[%2], ReceiverAppId=[%3]"))
			<< dt._fileMarkId << dt._fileName << dt._receiverAppId;
	}

	return Huaru::FileTransfers::FTS_OK;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   ???????????????????????????? </summary>
///
/// <remarks>   ??????, 2020/7/8. </remarks>
///
/// <param name="fileMarkId">   ????????. </param>
/// <param name="transfersFileInfo">	????????????????	</param>
///
/// <returns> bool??true-??????false-??????	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
SenderFileInfo * TSFileSender::_SelectSenderFileInfoByFileMarkId(UINT32 fileMarkId)
{
	TSRWSpinLock::ReadLock l(_lockFileMarkId2SenderFileInfo);

	std::map<UINT32, SenderFileInfo*>::iterator it = _mapFileMarkId2SenderFileInfo.find(fileMarkId);

	if (it == _mapFileMarkId2SenderFileInfo.end() || !(it->second))
	{
		return NULL;
	}

	return it->second;
}
bool TSFileSender::_InsertSenderFileInfo(const SenderFileInfo & senderFileInfo)
{
	TSRWSpinLock::WriteLock l(_lockFileMarkId2SenderFileInfo);

	std::map<UINT32, SenderFileInfo*>::iterator it = _mapFileMarkId2SenderFileInfo.find(senderFileInfo._fileMarkId);

	if (it != _mapFileMarkId2SenderFileInfo.end() || senderFileInfo._fileMarkId == 0)
	{
		return false;
	}

	SenderFileInfo * t_senderFileInfoPrt = new SenderFileInfo(senderFileInfo);
	if (!t_senderFileInfoPrt)
	{
		return false;
	}
	std::pair<UINT32, SenderFileInfo*> t_pair(t_senderFileInfoPrt->_fileMarkId, t_senderFileInfoPrt);
	std::pair<std::map<UINT32, SenderFileInfo*>::iterator, bool> t_itInsert = _mapFileMarkId2SenderFileInfo.insert(t_pair);
	
	return t_itInsert.second;
	
}
bool TSFileSender::_UpdateSenderFileInfo(SenderFileInfo & senderFileInfo)
{
	TSRWSpinLock::WriteLock l(_lockFileMarkId2SenderFileInfo);

	std::map<UINT32, SenderFileInfo*>::iterator it = _mapFileMarkId2SenderFileInfo.find(senderFileInfo._fileMarkId);

	if (it == _mapFileMarkId2SenderFileInfo.end() || senderFileInfo._fileMarkId == 0)
	{
		return false;
	}
	*(it->second) = senderFileInfo;
	return true;
}
bool TSFileSender::_DeleteSenderFileInfoByFileMarkId(UINT32 fileMarkId)
{
	TSRWSpinLock::WriteLock l(_lockFileMarkId2SenderFileInfo);

	std::map<UINT32, SenderFileInfo*>::iterator it = _mapFileMarkId2SenderFileInfo.find(fileMarkId);

	if (it == _mapFileMarkId2SenderFileInfo.end())
	{
		return false;
	}
	SenderFileInfo * t_transfersFileInfoPrt = it->second;
	it = _mapFileMarkId2SenderFileInfo.erase(it);

	if (t_transfersFileInfoPrt)
	{
		if (t_transfersFileInfoPrt->_ifStream.is_open())
		{
			t_transfersFileInfoPrt->_ifStream.close();
		}
		if (t_transfersFileInfoPrt->_buff)
		{
			delete(t_transfersFileInfoPrt->_buff);
		}
		t_transfersFileInfoPrt->_conditionVariable.notify_one();
		delete(t_transfersFileInfoPrt);
	}
	return true;
}
