#include "stdafx.h"
#if defined(XSIM3_2) || defined(XSIM3_3)
#else
#include "TopSimRuntime/TSLogsUtil.h"
#endif
#include "TopSimDataInterface/TSFileReceiver.h"
#include "TopSimDataInterface/TSDataInteractManager.h"
#include "boost/filesystem.hpp"

TSFileReceiver::TSFileReceiver(TSDomainPtr domainPtr) :
	_fileTransfersDomain(domainPtr),
	_receiverAppId(0),
	_countSyncIosRun(5),
	_indexCurIosRun(0),
	_asyncIosRun(boost::make_shared<TSIOSStruct>())
{
	if (!_fileTransfersDomain)
	{
		return;
	}

	_defaultReceiveDir = boost::filesystem::current_path().string();

	_receiverAppId = DataManager->GetFrontAppUniquelyId();

	_fileTransfersDomain->PublishTopic(Huaru_FileTransfers_ReceiverFileInfoRet);
	_fileTransfersDomain->PublishTopic(Huaru_FileTransfers_ReceiverFileTransfersInfoRet);
	_fileTransfersDomain->PublishTopic(Huaru_FileTransfers_TransfersFileErr);
	_fileTransfersDomain->PublishTopic(Huaru_FileTransfers_DownloadFileRequest);

	_fileTransfersDomain->RegisterTopicCallback(Huaru_FileTransfers_SenderFileInfo, boost::bind(&TSFileReceiver::_CallBackSenderFileInfo, this, _1));
	_fileTransfersDomain->SubscribeTopic(Huaru_FileTransfers_SenderFileInfo);

	_fileTransfersDomain->RegisterTopicCallback(Huaru_FileTransfers_SenderFileTransfersInfo, boost::bind(&TSFileReceiver::_CallBackSenderFileTransfersInfo, this, _1));
	_fileTransfersDomain->SubscribeTopic(Huaru_FileTransfers_SenderFileTransfersInfo);

	_fileTransfersDomain->RegisterTopicCallback(Huaru_FileTransfers_TransfersFileErr, boost::bind(&TSFileReceiver::_CallBackTransfersFileErr, this, _1));
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

TSFileReceiver::~TSFileReceiver()
{
	TSRWSpinLock::WriteLock l(_lockFileMarkId2ReceiverFileInfo);
#ifndef __VXWORKS__
	for (auto it = _mapFileMarkId2ReceiverFileInfo.begin(); it != _mapFileMarkId2ReceiverFileInfo.end(); )
#else
	for (std::map<UINT32, ReceiverFileInfo*>::iterator it = _mapFileMarkId2ReceiverFileInfo.begin(); it != _mapFileMarkId2ReceiverFileInfo.end(); )
#endif
	{
		if (it->second)
		{
			ReceiverFileInfo * t_receiverFileInfoPtr = it->second;
			it = _mapFileMarkId2ReceiverFileInfo.erase(it);

			if (t_receiverFileInfoPtr)
			{
				//释放缓冲区空间
				if (t_receiverFileInfoPtr->_buff)
				{
					delete(t_receiverFileInfoPtr->_buff);
				}
				//关闭文件流
				t_receiverFileInfoPtr->_ofStream.close();
				//释放
				delete(t_receiverFileInfoPtr);
			}
		}
		else
		{
			++it;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   注册设置文件接收存储的路径的回调函数函数 </summary>
///
/// <remarks>   任喜发, 2021/7/29. </remarks>
///
/// <param name="saveFilePath"> 用户回调函数  </param>
///
/// <returns> void	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
void TSFileReceiver::RegSetSaveFilePathCallBack(const std::function<TSString(const TSString & saveFilePath)> & saveFilePathFun)
{
	_setSaveFilePathFun = saveFilePathFun;
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   注册文件接收信息日志回调 </summary>
///
/// <remarks>   任喜发, 2021/7/29. </remarks>
///
/// <param name="logReceiverFileInfo">  用户回调函数. </param>
///
/// <returns> void	</returns>
void TSFileReceiver::RegLogReceiverFileInfoCallBack(const std::function<void(const LogReceiverFileInfo &)> & logReceiverFileInfo)
{
	_logReceiverFileInfoFun = logReceiverFileInfo;
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   注册文件内容接收日志回调</summary>
///
/// <remarks>   任喜发, 2021/7/29. </remarks>
///
/// <param name="logReceiverContentInfo">  用户回调函数. </param>
///
/// <returns> void	</returns>
void TSFileReceiver::RegLogReceiverContentInfoCallBack(const std::function<void(const LogReceiverContentInfo &)> & logReceiverContentInfo)
{
	_logReceiverContentInfoFun = logReceiverContentInfo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   请求下载文件 </summary>
///
/// <remarks>   任喜发, 2021/8/4. </remarks>
///
/// <param name="senderAppId">  提供文件下载服务端的AppID </param>
/// <param name="transfersFormat">  下载文件的传输方式 0-异步 1-同步 </param>
/// <param name="verifyBlockType">  块内容校验方式（异步不校验）：0-不校验 1-MD5 2-SHA1 3-CRC32 </param>
/// <param name="_filePath">  请求下载文件虚拟路径（由提供下载端将虚拟路径转换为本地实际路径传输） </param>
/// <returns> void	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
void TSFileReceiver::RequestDownloadFile(UINT32 senderAppId, UINT8 transfersFormat, UINT8 transfersMode, UINT8 verifyBlockType, TSString _filePath)
{
	Huaru::FileTransfers::DownloadFileRequest::DataType dt;
	dt._senderAppId = senderAppId;
	dt._receiverAppId = _receiverAppId;
	dt._transfersFormat = transfersFormat;
	dt._transfersMode = transfersMode;
	dt._verifyBlockType = verifyBlockType;
	dt._filePath = _filePath;

	if (0 == transfersFormat)
	{
		dt._verifyBlockType = 0;
	}

	_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_DownloadFileRequest, &dt, senderAppId);
}


/* 文件接收端接收到文件传输任务的主题 */
void TSFileReceiver::_CallBackSenderFileInfo(TSTopicContextPtr Ctx)
{
	Huaru::FileTransfers::SenderFileInfo::DataTypePtr dtPtr =
		TS_CAST(Ctx->GetTopic(), Huaru::FileTransfers::SenderFileInfo::DataTypePtr);

	if (dtPtr->_receiverAppId != _receiverAppId)
	{
		return;
	}

	Huaru::FileTransfers::ReceiverFileInfoRet::DataType dt;
	dt._senderAppId = dtPtr->_senderAppId;
	dt._receiverAppId = _receiverAppId;
	dt._fileMarkId = dtPtr->_fileMarkId;
	dt._receiveFileReady = Huaru::FileTransfers::FTR_OK;

	//文件接收信息日志
	LogReceiverFileInfo t_logReceiverFileInfo;
	t_logReceiverFileInfo._ret = Huaru::FileTransfers::FTR_OK;
	t_logReceiverFileInfo._transfersFormat = dtPtr->_transfersFormat;
	t_logReceiverFileInfo._transfersMode = dtPtr->_transfersMode;
	t_logReceiverFileInfo._transfersType = dtPtr->_transfersType;
	t_logReceiverFileInfo._verifyBlockType = dtPtr->_verifyBlockType;
	t_logReceiverFileInfo._blockSize = dtPtr->_blockSize;
	t_logReceiverFileInfo._fileSize = dtPtr->_fileSize;
	t_logReceiverFileInfo._fileName = dtPtr->_fileName;

	//接收文件路径
	TSString t_receiveDir = _defaultReceiveDir;

	//发送端主动推送，接收端被动接收
	if (0 == dtPtr->_transfersType)
	{
		if (!dtPtr->_receiveDir.empty())
		{
			t_receiveDir = dtPtr->_receiveDir;
		}
	}
	//发送端发送，接收端设置接收路径
	else if (1 == dtPtr->_transfersType)
	{
		//接收文件存储路径
		if (_setSaveFilePathFun)
		{
			//将设置接收的路径复制给默认接收路径，以备下次使用
			_defaultReceiveDir = _setSaveFilePathFun(_defaultReceiveDir);
			t_receiveDir = _defaultReceiveDir;
		}
	}
	//接收端请求下载
	else if (2 == dtPtr->_transfersType)
	{
		//接收文件存储路径
		if (_setSaveFilePathFun)
		{
			//将设置接收的路径复制给默认接收路径，以备下次使用
			_defaultReceiveDir = _setSaveFilePathFun(_defaultReceiveDir);
			t_receiveDir = _defaultReceiveDir;
		}
	}
	else
	{
		dt._receiveFileReady = Huaru::FileTransfers::FTR_TRANSFERTYPE_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileInfoRet, &dt, dtPtr->_senderAppId);

		if (_logReceiverFileInfoFun)
		{
			t_logReceiverFileInfo._ret = Huaru::FileTransfers::FTR_TRANSFERTYPE_ERR;
			_logReceiverFileInfoFun(t_logReceiverFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("文件传输方式参数错误!!! TransferType=[%1]")) << dtPtr->_transfersType;
		}
		return;
	}
	t_logReceiverFileInfo._receiveDir = t_receiveDir;
	
	//发现文件发送端
	if (!_DiscoverySender(dtPtr->_senderAppId))
	{
		dt._receiveFileReady = Huaru::FileTransfers::FTR_DISCOVERY_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileInfoRet, &dt, dtPtr->_senderAppId);

		if (_logReceiverFileInfoFun)
		{
			t_logReceiverFileInfo._ret = Huaru::FileTransfers::FTR_DISCOVERY_ERR;
			_logReceiverFileInfoFun(t_logReceiverFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("发现发送端失败!!! SenderAppId=[%1]")) << dtPtr->_senderAppId;
		}

		return;
	}

	//该接收任务已存在
	if (_SelectReceiverFileInfoByFileMarkId(dtPtr->_fileMarkId))
	{
		dt._receiveFileReady = (UINT8)Huaru::FileTransfers::FTR_TASKEXIST_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileInfoRet, &dt, dtPtr->_senderAppId);

		if (_logReceiverFileInfoFun)
		{
			t_logReceiverFileInfo._ret = Huaru::FileTransfers::FTR_TASKEXIST_ERR;
			_logReceiverFileInfoFun(t_logReceiverFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("该传输任务已存在!!! FileName=[%1]")) << dtPtr->_fileName;
		}
		return;
	}

	//创建接收目录失败
	if (!t_receiveDir.empty() && !boost::filesystem::exists(t_receiveDir))
	{
		boost::system::error_code err;
		boost::filesystem::create_directories(t_receiveDir, err);
		if (err)
		{
			dt._receiveFileReady = Huaru::FileTransfers::FTR_CREATEDIR_ERR;
			_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileInfoRet, &dt, dtPtr->_senderAppId);

			if (_logReceiverFileInfoFun)
			{
				t_logReceiverFileInfo._ret = Huaru::FileTransfers::FTR_CREATEDIR_ERR;
				_logReceiverFileInfoFun(t_logReceiverFileInfo);
			}
			else
			{
				DEF_LOG_ERROR(GB18030ToTSString("创建目录失失败!!! ReceiveDir=[%1]")) << t_receiveDir;
			}
			return;
		}
	}

	//文件路径名称
	TSString t_filePathName;
	size_t t_pos1 = t_receiveDir.find_last_of("/");
	size_t t_pos2 = t_receiveDir.find_last_of("\\");
	if (t_pos1 == t_receiveDir.length() - 1 || t_pos2 == t_receiveDir.length() - 1)
	{
		t_filePathName = t_receiveDir + dtPtr->_fileName;
	}
	else
	{
		t_filePathName = t_receiveDir + "\\" + dtPtr->_fileName;
	}
	
	//创建任务
	ReceiverFileInfo t_receiverFileInfo(dtPtr->_blockNum);
	t_receiverFileInfo._fileMarkId = dtPtr->_fileMarkId;
	t_receiverFileInfo._senderAppId = dtPtr->_senderAppId;
	t_receiverFileInfo._blockSize = dtPtr->_blockSize;
	t_receiverFileInfo._blockNum = (UINT32)(dtPtr->_fileSize / dtPtr->_blockSize) + (dtPtr->_fileSize % dtPtr->_blockSize ? 1 : 0);
	t_receiverFileInfo._lastBlockSize = dtPtr->_fileSize % dtPtr->_blockSize ? dtPtr->_fileSize % dtPtr->_blockSize : dtPtr->_blockSize;
	t_receiverFileInfo._currBlockIndex = 0;
	t_receiverFileInfo._threadIndex = 0xFF;
	t_receiverFileInfo._transfersType = dtPtr->_transfersType;
	t_receiverFileInfo._transfersMode = dtPtr->_transfersMode;
	t_receiverFileInfo._transfersFormat = dtPtr->_transfersFormat;
	t_receiverFileInfo._verifyBlockType = dtPtr->_verifyBlockType;
	t_receiverFileInfo._fileName = dtPtr->_fileName;
	t_receiverFileInfo._fileSize = dtPtr->_fileSize;
	t_receiverFileInfo._receiveDir = t_receiveDir;
	

	//添加到队列中
	if (!_InsertReceiverFileInfo(t_receiverFileInfo))
	{
		dt._receiveFileReady = Huaru::FileTransfers::FTR_CREATETASK_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileInfoRet, &dt, dtPtr->_senderAppId);

		if (_logReceiverFileInfoFun)
		{
			t_logReceiverFileInfo._ret = Huaru::FileTransfers::FTR_CREATETASK_ERR;
			_logReceiverFileInfoFun(t_logReceiverFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("添加到任务列表失败!!! FilePath=[%1]")) << t_filePathName;
		}
		return;
	}

	//从队列中取出任务
	ReceiverFileInfo * t_receiverFileInfoPtr = _SelectReceiverFileInfoByFileMarkId(dtPtr->_fileMarkId);
	if (!t_receiverFileInfoPtr)
	{
		dt._receiveFileReady = Huaru::FileTransfers::FTR_CREATETASK_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileInfoRet, &dt, dtPtr->_senderAppId);

		if (_logReceiverFileInfoFun)
		{
			t_logReceiverFileInfo._ret = Huaru::FileTransfers::FTR_CREATETASK_ERR;
			_logReceiverFileInfoFun(t_logReceiverFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("添加到任务列表失败!!! FilePath=[%1]")) << t_filePathName;
		}
		return;
	}

	//二进制
	if (dtPtr->_transfersFormat == 0)
	{
		t_receiverFileInfoPtr->_ofStream.open(t_filePathName.c_str(), std::ios_base::out | std::ios_base::binary);
	}
	//文本
	else
	{
		t_receiverFileInfoPtr->_ofStream.open(t_filePathName.c_str(), std::ios_base::out);
	}

	//创建接收文件失败
	if (!t_receiverFileInfoPtr->_ofStream.good())
	{
		dt._receiveFileReady = Huaru::FileTransfers::FTR_OPENFILE_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileInfoRet, &dt, dtPtr->_senderAppId);

		if (_logReceiverFileInfoFun)
		{
			t_logReceiverFileInfo._ret = Huaru::FileTransfers::FTR_OPENFILE_ERR;
			_logReceiverFileInfoFun(t_logReceiverFileInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("打开文件失败!!! FilePath=[%1]")) << t_filePathName;
		}

		_DeleteReveiverFileInfoByFileMarkId(dtPtr->_fileMarkId);
		return;
	}

	if (!(t_receiverFileInfoPtr->_buff = new char[dtPtr->_blockSize]))
	{
		dt._receiveFileReady = Huaru::FileTransfers::FTR_CREATEBUFF_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileInfoRet, &dt, dtPtr->_senderAppId);

		if (_logReceiverFileInfoFun)
		{
			t_logReceiverFileInfo._ret = Huaru::FileTransfers::FTR_CREATEBUFF_ERR;
			_logReceiverFileInfoFun(t_logReceiverFileInfo);
	}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("分配接收缓冲区失败!!! FilePath=[%1]")) << t_filePathName;
		}

		_DeleteReveiverFileInfoByFileMarkId(dtPtr->_fileMarkId);
	}

	//同步传输
	if (dtPtr->_transfersMode == 1)
	{
		TSRWSpinLock::WriteLock l(_lockSyncIosRun);
		t_receiverFileInfoPtr->_threadIndex = _indexCurIosRun;
		_indexCurIosRun = ++_indexCurIosRun % _countSyncIosRun;
	}

	_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileInfoRet, &dt, dtPtr->_senderAppId);
	return;
}

void TSFileReceiver::_CallBackSenderFileTransfersInfo(TSTopicContextPtr Ctx)
{
	Huaru::FileTransfers::SenderFileTransfersInfo::DataTypePtr dtPtr =
		TS_CAST(Ctx->GetTopic(), Huaru::FileTransfers::SenderFileTransfersInfo::DataTypePtr);

	if (dtPtr->_receiverAppId != _receiverAppId)
	{
		return;
	}

	//严重错误主题
	Huaru::FileTransfers::TransfersFileErr::DataType dt;
	dt._senderAppId = dtPtr->_senderAppId;
	dt._receiverAppId = _receiverAppId;
	dt._fileMarkId = dtPtr->_fileMarkId;
	dt._errCode = Huaru::FileTransfers::FTR_CTX_OK;

	//查询该接收任务
	ReceiverFileInfo * t_receiverFileInfoPtr = _SelectReceiverFileInfoByFileMarkId(dtPtr->_fileMarkId);
	if (!t_receiverFileInfoPtr)
	{
		dt._errCode = Huaru::FileTransfers::FTR_CTX_NOTASK_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_TransfersFileErr, &dt, dtPtr->_senderAppId);

		if (_logReceiverContentInfoFun)
		{
			LogReceiverContentInfo t_logReceiverContentInfo;
			t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_NOTASK_ERR;
			t_logReceiverContentInfo._fileMarkId = dtPtr->_fileMarkId;
			
			_logReceiverContentInfoFun(t_logReceiverContentInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("接收方方无该任务，通知发送方删除该任务!!! FileMarkId=[%1]")) << dtPtr->_fileMarkId;
		}
		return;
	}

	//异步传输
	if (t_receiverFileInfoPtr->_transfersMode == 0)
	{
#if defined(XSIM3_2) || defined(XSIM3_3)
		_asyncIosRun->GetIOS()->post(boost::bind(&TSFileReceiver::_ReceiveFileContentAsync, this, dtPtr));
#else
		_asyncIosRun->GetIOS()->Post(boost::bind(&TSFileReceiver::_ReceiveFileContentAsync, this, dtPtr));
#endif
		//DEF_LOG_KERNEL(GB18030ToTSString("接收任务投递到线程，异步传输... FileName=[%1]")) << t_receiverFileInfoPtr->_fileName;
	}
	//同步传输
	else
	{
#if defined(XSIM3_2) || defined(XSIM3_3)
		_vetSyncIosRun[t_receiverFileInfoPtr->_threadIndex]->GetIOS()->post(boost::bind(&TSFileReceiver::_ReceiveFileContent, this, dtPtr));
#else
		_vetSyncIosRun[t_receiverFileInfoPtr->_threadIndex]->GetIOS()->Post(boost::bind(&TSFileReceiver::_ReceiveFileContent, this, dtPtr));
#endif
		//DEF_LOG_KERNEL(GB18030ToTSString("接收任务投递到线程，同步传输... FileName=[%1]，ThreadIndex=[%2]")) << t_receiverFileInfoPtr->_fileName << _indexCurIosRun;
	}
	
	return;
}

void TSFileReceiver::_CallBackTransfersFileErr(TSTopicContextPtr Ctx)
{
	Huaru::FileTransfers::TransfersFileErr::DataTypePtr dtPtr =
		TS_CAST(Ctx->GetTopic(), Huaru::FileTransfers::TransfersFileErr::DataTypePtr);

	if (dtPtr->_receiverAppId != _receiverAppId)
	{
		return;
	}

	if (dtPtr->_errCode == 0)
	{
		return;
	}
	
	_DeleteReveiverFileInfoByFileMarkId(dtPtr->_fileMarkId);
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   发现发送者 </summary>
///
/// <remarks>   任喜发, 2020/7/10. </remarks>
///
/// <param name="senderAppId">   发送者AppId. </param>
///
/// <returns> void	</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
bool TSFileReceiver::_DiscoverySender(UINT32 senderAppId)
{
	TSTopicQuerySet publisherQuerySet;
	TSTopicQuerySet scriberQuerySet;

	scriberQuerySet.push_back(Huaru_FileTransfers_SenderFileInfo);
	scriberQuerySet.push_back(Huaru_FileTransfers_SenderFileTransfersInfo);

	publisherQuerySet.push_back(Huaru_FileTransfers_ReceiverFileInfoRet);
	publisherQuerySet.push_back(Huaru_FileTransfers_ReceiverFileTransfersInfoRet);

	//发现主题完成
	if (TSWaitSet::WaitForDiscoveryComplete(_fileTransfersDomain, publisherQuerySet, senderAppId, 5000, TSWaitSet::kPublisher) &&
		TSWaitSet::WaitForDiscoveryComplete(_fileTransfersDomain, scriberQuerySet, senderAppId, 5000, TSWaitSet::kSubscriber))
	{
		return true;
	}

	return false;
}

void TSFileReceiver::_ReceiveFileContent(Huaru::FileTransfers::SenderFileTransfersInfo::DataTypePtr dtPtr)
{
	//严重错误主题
	Huaru::FileTransfers::TransfersFileErr::DataType dtErr;
	dtErr._senderAppId = dtPtr->_senderAppId;
	dtErr._receiverAppId = dtPtr->_receiverAppId;
	dtErr._fileMarkId = dtPtr->_fileMarkId;
	dtErr._errCode = Huaru::FileTransfers::FTR_CTX_OK;

	LogReceiverContentInfo t_logReceiverContentInfo;
	t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_OK;
	t_logReceiverContentInfo._fileMarkId = dtPtr->_fileMarkId;
	t_logReceiverContentInfo._currBlockIndex = dtPtr->_blockIndex;

	ReceiverFileInfo * t_receiverFileInfoPtr = _SelectReceiverFileInfoByFileMarkId(dtPtr->_fileMarkId);
	//不存在该接收任务
	if (!t_receiverFileInfoPtr)
	{
		dtErr._errCode = Huaru::FileTransfers::FTR_CTX_NOTASK_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_TransfersFileErr, &dtErr, dtPtr->_senderAppId);

		if (_logReceiverContentInfoFun)
		{
			t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_NOTASK_ERR;
			_logReceiverContentInfoFun(t_logReceiverContentInfo);
		}
		else
		{
			DEF_LOG_KERNEL(GB18030ToTSString("接收方方无该任务，通知发送方删除该任务!!! FileMarkId=[%1]")) << dtPtr->_fileMarkId;
		}
		return;
	}

	Huaru::FileTransfers::ReceiverFileTransfersInfoRet::DataType dt;
	dt._fileMarkId = dtPtr->_fileMarkId;
	dt._receiverAppId = _receiverAppId;
	dt._senderAppId = t_receiverFileInfoPtr->_senderAppId;
	dt._receiveBlockIndex = dtPtr->_blockIndex;
	dt._expectBlockIndex = dtPtr->_blockIndex;
	dt._isReceiveEnd = 0;
	dt._receiveBlockRet = 0;

	//基本校验
	if (dtPtr->_blockContent.size() != dtPtr->_blockRealSize)
	{
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileTransfersInfoRet, &dt, t_receiverFileInfoPtr->_senderAppId);
		
		if (_logReceiverContentInfoFun)
		{
			t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_BLKSIZE_ERR;
			_logReceiverContentInfoFun(t_logReceiverContentInfo);
		}
		else
		{
			DEF_LOG_ERROR(GB18030ToTSString("接收块内容大小不正确，要求从新传输该块内容!!! FileName=[%1], BlockIndex=[%2], ReceiveBlockSize=[%3], SendBlockSize=[%4]"))
				<< t_receiverFileInfoPtr->_fileName << dtPtr->_blockIndex << dtPtr->_blockContent.size() << dtPtr->_blockRealSize;
		}
		return;
	}

	//需要校验，但校验码空，从新请求该块内容
	if (dtPtr->_verifyBlockType != 0 && dtPtr->_verifyBlockCode.empty())
	{
		
	}
	//MD5校验
	else if (dtPtr->_verifyBlockType == 1 && !dtPtr->_verifyBlockCode.empty())
	{
		
	}
	//SHA1校验
	else if (dtPtr->_verifyBlockType == 2 && !dtPtr->_verifyBlockCode.empty())
	{

	}
	//CRC32校验
	else if (dtPtr->_verifyBlockType == 3 && !dtPtr->_verifyBlockCode.empty())
	{

	}
	
	memcpy(t_receiverFileInfoPtr->_buff, &(dtPtr->_blockContent[0]), dtPtr->_blockRealSize);

	t_receiverFileInfoPtr->_ofStream.seekp(dtPtr->_blockIndex * t_receiverFileInfoPtr->_blockSize, (std::ios_base::seekdir)0);
	t_receiverFileInfoPtr->_ofStream.write(t_receiverFileInfoPtr->_buff, dtPtr->_blockRealSize);
	if (!t_receiverFileInfoPtr->_ofStream.good())
	{
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileTransfersInfoRet, &dt, t_receiverFileInfoPtr->_senderAppId);

		DEF_LOG_ERROR(GB18030ToTSString("同步写入文件块失败,要求重新传输该块内容!!! FileName=[%1], BlockIndex=[%2], ReceiveBlockSize=[%3]"))
			<< t_receiverFileInfoPtr->_fileName << dtPtr->_blockIndex << dtPtr->_blockContent.size();
		return;
	}

	t_receiverFileInfoPtr->_currBlockIndex = dtPtr->_blockIndex;
	t_receiverFileInfoPtr->_completeBlocks[dtPtr->_blockIndex] = true;
	
	if (_logReceiverContentInfoFun)
	{
		t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_OK;
		_logReceiverContentInfoFun(t_logReceiverContentInfo);
	}
	else
	{
		DEF_LOG_KERNEL(GB18030ToTSString("同步写入文件块成功... FileName=[%1], FileMarkId=[%2], BlockIndex=[%3], ThreadIndex=[%4]"))
			<< t_receiverFileInfoPtr->_fileName << dtPtr->_fileMarkId << dtPtr->_blockIndex << t_receiverFileInfoPtr->_threadIndex;
	}

	if (dtPtr->_isEndBlock == (UINT8)1 || dtPtr->_blockIndex == t_receiverFileInfoPtr->_blockNum - 1)
	{
		dt._isReceiveEnd = (UINT8)1;

		if (_logReceiverContentInfoFun)
		{
			t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_FINSH_OK;
			_logReceiverContentInfoFun(t_logReceiverContentInfo);
		}
		else
		{
			DEF_LOG_KERNEL(GB18030ToTSString("同步接收文件完毕... FileName=[%1], FileMarkId=[%2], ThreadIndex=[%3]"))
				<< t_receiverFileInfoPtr->_fileName << dtPtr->_fileMarkId << t_receiverFileInfoPtr->_threadIndex;
		}

		_DeleteReveiverFileInfoByFileMarkId(dtPtr->_fileMarkId);
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileTransfersInfoRet, &dt, t_receiverFileInfoPtr->_senderAppId);
		return;
	}
	else
	{
		dt._expectBlockIndex = ++dtPtr->_blockIndex;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_ReceiverFileTransfersInfoRet, &dt, t_receiverFileInfoPtr->_senderAppId);
	}

	return;
}

void TSFileReceiver::_ReceiveFileContentAsync(Huaru::FileTransfers::SenderFileTransfersInfo::DataTypePtr dtPtr)
{
	//严重错误主题
	Huaru::FileTransfers::TransfersFileErr::DataType dtErr;
	dtErr._senderAppId = dtPtr->_senderAppId;
	dtErr._receiverAppId = _receiverAppId;
	dtErr._fileMarkId = dtPtr->_fileMarkId;
	dtErr._errCode = Huaru::FileTransfers::FTR_CTX_OK;

	LogReceiverContentInfo t_logReceiverContentInfo;
	t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_OK;
	t_logReceiverContentInfo._fileMarkId = dtPtr->_fileMarkId;
	t_logReceiverContentInfo._currBlockIndex = dtPtr->_blockIndex;

	ReceiverFileInfo * t_receiverFileInfoPtr = _SelectReceiverFileInfoByFileMarkId(dtPtr->_fileMarkId);

	//不存在该接收任务
	if (!t_receiverFileInfoPtr)
	{
		dtErr._errCode = Huaru::FileTransfers::FTR_CTX_NOTASK_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_TransfersFileErr, &dtErr, dtPtr->_senderAppId);

		if (_logReceiverContentInfoFun)
		{
			t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_NOTASK_ERR;
			_logReceiverContentInfoFun(t_logReceiverContentInfo);
		}
		else
		{
			DEF_LOG_KERNEL(GB18030ToTSString("接收方方无该任务，通知发送方删除该任务!!! FileMarkId=[%1]")) << dtPtr->_fileMarkId;
		}
		return;
	}

	//基本校验
	if (dtPtr->_blockContent.size() != dtPtr->_blockRealSize)
	{
		dtErr._errCode = Huaru::FileTransfers::FTR_CTX_BLKSIZE_ERR;
		_fileTransfersDomain->SendTopicDirect(Huaru_FileTransfers_TransfersFileErr, &dtErr, dtPtr->_senderAppId);

		if (_logReceiverContentInfoFun)
		{
			t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_BLKSIZE_ERR;
			_logReceiverContentInfoFun(t_logReceiverContentInfo);
		}
		else
		{
			DEF_LOG_KERNEL(GB18030ToTSString("异步接收块内容大小不正确，要求从新传输该块内容!!! FileName=[%1], BlockIndex=[%2], ReceiveBlockSize=[%3], SendBlockSize=[%4]"))
				<< t_receiverFileInfoPtr->_fileName << dtPtr->_blockIndex << dtPtr->_blockContent.size() << dtPtr->_blockRealSize;
		}
		return;
	}

	memcpy(t_receiverFileInfoPtr->_buff, &(dtPtr->_blockContent[0]), dtPtr->_blockRealSize);

	t_receiverFileInfoPtr->_ofStream.write(t_receiverFileInfoPtr->_buff, dtPtr->_blockRealSize);

	t_receiverFileInfoPtr->_currBlockIndex = dtPtr->_blockIndex;
	t_receiverFileInfoPtr->_completeBlocks[dtPtr->_blockIndex] = true;

	if (_logReceiverContentInfoFun)
	{
		t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_OK;
		_logReceiverContentInfoFun(t_logReceiverContentInfo);
	}
	else
	{
		DEF_LOG_KERNEL(GB18030ToTSString("异步接收块...FileName=[%1], BlockNo=[%2]")) << dtPtr->_fileMarkId << dtPtr->_blockIndex;
	}

	//文件接收结束
	if (dtPtr->_isEndBlock == (UINT8)1 || dtPtr->_blockIndex == t_receiverFileInfoPtr->_blockNum - 1)
	{
		if (_logReceiverContentInfoFun)
		{
			t_logReceiverContentInfo._ret = Huaru::FileTransfers::FTR_CTX_FINSH_OK;
			_logReceiverContentInfoFun(t_logReceiverContentInfo);
		}
		else
		{
			DEF_LOG_KERNEL(GB18030ToTSString("异步接收文件完毕... FileName=[%1], FileMarkId=[%2]"))
				<< t_receiverFileInfoPtr->_fileName << dtPtr->_fileMarkId;
		}

		_DeleteReveiverFileInfoByFileMarkId(dtPtr->_fileMarkId);
	}

	return;
}


ReceiverFileInfo * TSFileReceiver::_SelectReceiverFileInfoByFileMarkId(UINT32 fileMarkId)
{
	TSRWSpinLock::ReadLock l(_lockFileMarkId2ReceiverFileInfo);
#ifndef __VXWORKS__
	auto it = _mapFileMarkId2ReceiverFileInfo.find(fileMarkId);
#else
	std::map<UINT32, ReceiverFileInfo*>::iterator it = _mapFileMarkId2ReceiverFileInfo.find(fileMarkId);
#endif

	if (it == _mapFileMarkId2ReceiverFileInfo.end())
	{
		return NULL;
	}

	return it->second;
}
bool TSFileReceiver::_InsertReceiverFileInfo(const ReceiverFileInfo & receiverFileInfo)
{
	TSRWSpinLock::WriteLock l(_lockFileMarkId2ReceiverFileInfo);

	std::map<UINT32, ReceiverFileInfo*>::iterator it = _mapFileMarkId2ReceiverFileInfo.find(receiverFileInfo._fileMarkId);

	if (it != _mapFileMarkId2ReceiverFileInfo.end())
	{
		return false;
	}
	ReceiverFileInfo * t_receiverFileInfoPtr = new ReceiverFileInfo(receiverFileInfo);
	if (!t_receiverFileInfoPtr)
	{
		return false;
	}

	std::pair<UINT32, ReceiverFileInfo*> t_pair(t_receiverFileInfoPtr->_fileMarkId, t_receiverFileInfoPtr);
	std::pair<std::map<UINT32, ReceiverFileInfo*>::iterator, bool> itInsert = _mapFileMarkId2ReceiverFileInfo.insert(t_pair);

	return itInsert.second;
}
bool TSFileReceiver::_UpdateReceiverFileInfo(ReceiverFileInfo & receiverFileInfo)
{
	TSRWSpinLock::WriteLock l(_lockFileMarkId2ReceiverFileInfo);
#ifndef __VXWORKS__
	auto it = _mapFileMarkId2ReceiverFileInfo.find(receiverFileInfo._fileMarkId);
#else
	std::map<UINT32, ReceiverFileInfo*>::iterator it = _mapFileMarkId2ReceiverFileInfo.find(receiverFileInfo._fileMarkId);
#endif


	if (it == _mapFileMarkId2ReceiverFileInfo.end())
	{
		return false;
	}

	*(it->second) = receiverFileInfo;
	return true;
}
bool TSFileReceiver::_DeleteReveiverFileInfoByFileMarkId(UINT32 fileMarkId)
{
	TSRWSpinLock::WriteLock l(_lockFileMarkId2ReceiverFileInfo);
#ifndef __VXWORKS__
	auto it = _mapFileMarkId2ReceiverFileInfo.find(fileMarkId);
#else
	std::map<UINT32, ReceiverFileInfo*>::iterator it = _mapFileMarkId2ReceiverFileInfo.find(fileMarkId);
#endif

	if (it == _mapFileMarkId2ReceiverFileInfo.end())
	{
		return false;
	}

	ReceiverFileInfo * t_receiverFileInfoPtr = it->second;
	it = _mapFileMarkId2ReceiverFileInfo.erase(it);

	if (t_receiverFileInfoPtr)
	{
		//释放缓冲区空间
		if (t_receiverFileInfoPtr->_buff)
		{
			delete(t_receiverFileInfoPtr->_buff);
		}
		//关闭文件流
		t_receiverFileInfoPtr->_ofStream.close();
		//释放
		delete(t_receiverFileInfoPtr);
	}
	return true;
}
