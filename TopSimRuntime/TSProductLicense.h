#ifndef __TSFILESENDER_H__
#define __TSFILESENDER_H__

#include "TopSimDataInterface/TSFileTransfers/Defined.h"
#include "TopSimDataInterface/TopSimDataInterface.h"

struct TOPSIMDATAINTERFACE_DECL LogSenderFileInfo
{
	UINT8	_ret;						//结果码
	UINT8	_transfersType;				//传输方式：0-发送端主动推送文件到接收端，1-接收端设置接收目录，2-接收端请求下载
	UINT8	_transfersMode;				//传输模式：0-异步，1-同步 
	UINT8	_transfersFormat;			//传输格式：0-二进制，1-文本
	UINT8	_verifyBlockType;			//传输块内容校验方式：0-不校验，1-MD5，2-SHA1，3-CRC32

	UINT32	_fileMarkId;				//文件传输标识
	UINT32	_receiverAppId;				//接收者AppId
	UINT32	_blockSize;					//传输块大小
	UINT64	_fileSize;					//文件大小

	TSString	_fileName;				//文件名称

	LogSenderFileInfo() :
		_ret(0),
		_transfersType(0),
		_transfersMode(0),
		_transfersFormat(0),
		_verifyBlockType(0),
		_fileMarkId(0),
		_receiverAppId(0),
		_blockSize(0),
		_fileSize(0)
	{

	}

	LogSenderFileInfo(const LogSenderFileInfo & other)
	{
		_ret = other._ret;
		_transfersType = other._transfersType;
		_transfersMode = other._transfersMode;
		_transfersFormat = other._transfersFormat;
		_verifyBlockType = other._verifyBlockType;
		_fileMarkId = other._fileMarkId;
		_receiverAppId = other._receiverAppId;
		_blockSize = other._blockSize;
		_fileSize = other._fileSize;
		_fileName = other._fileName;
	}

	LogSenderFileInfo & operator=(const LogSenderFileInfo & other)
	{
		_ret = other._ret;
		_transfersType = other._transfersType;
		_transfersMode = other._transfersMode;
		_transfersFormat = other._transfersFormat;
		_verifyBlockType = other._verifyBlockType;
		_fileMarkId = other._fileMarkId;
		_receiverAppId = other._receiverAppId;
		_blockSize = other._blockSize;
		_fileSize = other._fileSize;
		_fileName = other._fileName;

		return *this;
	}
};

struct TOPSIMDATAINTERFACE_DECL LogSenderFileInfoAck
{
	UINT8	_ret;						//结果码
	UINT8	_receiverRet;				//接收端结果码
	UINT32	_fileMarkId;				//文件传输标识
	UINT32	_receiverAppId;				//接收者AppId

	LogSenderFileInfoAck() :
		_ret(0),
		_receiverRet(0),
		_fileMarkId(0),
		_receiverAppId(0)
	{

	}

	LogSenderFileInfoAck(const LogSenderFileInfoAck & other)
	{
		_ret = other._ret;
		_receiverRet = other._receiverRet;
		_fileMarkId = other._fileMarkId;
		_receiverAppId = other._receiverAppId;
	}

	LogSenderFileInfoAck & operator=(const LogSenderFileInfoAck & other)
	{
		_ret = other._ret;
		_receiverRet = other._receiverRet;
		_fileMarkId = other._fileMarkId;
		_receiverAppId = other._receiverAppId;

		return *this;
	}
};

struct TOPSIMDATAINTERFACE_DECL LogSenderContentInfo
{
	UINT8	_ret;						//结果码
	UINT32	_fileMarkId;				//文件传输标识
	UINT32	_currBlockIndex;			//当前发送块索引

	LogSenderContentInfo() :
		_ret(0),
		_fileMarkId(0),
		_currBlockIndex(0)
	{
		
	}

	LogSenderContentInfo(const LogSenderContentInfo & other)
	{
		_ret = other._ret;
		_fileMarkId = other._fileMarkId;
		_currBlockIndex = other._currBlockIndex;
	}

	LogSenderContentInfo & operator=(const LogSenderContentInfo & other)
	{
		_ret = other._ret;
		_fileMarkId = other._fileMarkId;
		_currBlockIndex = other._currBlockIndex;
		return *this;
	}
};

/*
	发送文件信息
*/
struct TOPSIMDATAINTERFACE_DECL SenderFileInfo
{
	UINT32	_fileMarkId;				//文件传输标识
	UINT32	_receiverAppId;				//接收者AppId
	UINT32	_blockSize;					//传输块大小
	UINT32	_blockNum;					//传输块数量
	UINT32	_lastBlockSize;				//最后一个传输块大小
	UINT32	_currBlockIndex;			//当前发送块索引
	UINT8	_threadIndex;				//线程Id;
	UINT8	_transfersType;				//传输方式：0-发送端主动推送文件到接收端，1-接收端设置接收目录，2-接收端请求下载
	UINT8	_transfersMode;				//传输模式：0-异步，1-同步 
	UINT8	_transfersFormat;			//传输格式：0-二进制，1-文本
	UINT8	_verifyBlockType;			//传输块内容校验方式：0-不校验，1-MD5，2-SHA1，3-CRC32
	UINT64	_fileSize;					//文件大小
	char *	_buff;						//发送缓冲区
	TSString	_fileName;				//文件名称
	TSString	_receiveDir;			//接收端文件存放路径
	std::vector<bool> _completeBlocks;	//每个文件块传输情况
	std::ifstream	_ifStream;			//文件读取流
	boost::mutex _mutexLock;			//文件同步传输锁
	boost::condition_variable _conditionVariable;	//文件同步传输条件变量

	SenderFileInfo() :
		_fileMarkId(0),
		_receiverAppId(0),
		_blockSize(0),
		_blockNum(0),
		_lastBlockSize(0),
		_currBlockIndex(0),
		_threadIndex(0xFF),
		_transfersType(0),
		_transfersMode(0),
		_transfersFormat(0),
		_verifyBlockType(0),
		_fileSize((UINT64)0),
		_buff(NULL)
	{
		_completeBlocks.resize(_blockNum);

		for (std::vector<bool>::iterator it = _completeBlocks.begin(); it != _completeBlocks.end(); ++it)
		{
			*it = false;
		}
	}

	SenderFileInfo(UINT32 blockNum) :
		_fileMarkId(0),
		_receiverAppId(0),
		_blockSize(0),
		_blockNum(blockNum),
		_lastBlockSize(0),
		_currBlockIndex(0),
		_threadIndex(0xFF),
		_transfersType(0),
		_transfersMode(0),
		_transfersFormat(0),
		_verifyBlockType(0),
		_fileSize((UINT64)0),
		_buff(NULL)
	{
		_completeBlocks.resize(blockNum);

		for (std::vector<bool>::iterator it = _completeBlocks.begin(); it != _completeBlocks.end(); ++it)
		{
			*it = false;
		}
	}

	SenderFileInfo(const SenderFileInfo & other)
	{
		_fileMarkId = other._fileMarkId;
		_receiverAppId = other._receiverAppId;
		_blockSize = other._blockSize;
		_blockNum = other._blockNum;
		_lastBlockSize = other._lastBlockSize;
		_currBlockIndex = other._currBlockIndex;
		_threadIndex = other._threadIndex;
		_transfersType = other._transfersType;
		_transfersMode = other._transfersMode;
		_transfersFormat = other._transfersFormat;
		_verifyBlockType = other._verifyBlockType;
		_buff = NULL;
		_fileSize = other._fileSize;
		_fileName = other._fileName;
		_receiveDir = other._receiveDir;
		_completeBlocks = other._completeBlocks;
	}

	SenderFileInfo & operator=(const SenderFileInfo & other)
	{
		_fileMarkId = other._fileMarkId;
		_receiverAppId = other._receiverAppId;
		_blockSize = other._blockSize;
		_blockNum = other._blockNum;
		_lastBlockSize = other._lastBlockSize;
		_currBlockIndex = other._currBlockIndex;
		_threadIndex = other._threadIndex;
		_transfersType = other._transfersType;
		_transfersMode = other._transfersMode;
		_transfersFormat = other._transfersFormat;
		_verifyBlockType = other._verifyBlockType;
		_buff = NULL;
		_fileSize = other._fileSize;
		_fileName = other._fileName;
		_receiveDir = other._receiveDir;
		_completeBlocks = other._completeBlocks;
		return *this;
	}
};

class TOPSIMDATAINTERFACE_DECL TSFileSender
{
private:
	TSDomainPtr	_fileTransfersDomain;		//文件传输域
	UINT32		_senderAppId;				//发送端AppId
	UINT32		_blockSize;					//文件传输块大小

	//传输任务列表
	TSRWSpinLock						_lockFileMarkId2SenderFileInfo;
	std::map<UINT32, SenderFileInfo*>	_mapFileMarkId2SenderFileInfo;

	TSIOSStructPtr	_asyncIosRun;			//异步传输线程

	//同步传输线程
	UINT32								_countSyncIosRun;		//同步传输线程数量
	TSRWSpinLock						_lockSyncIosRun;		//同步传输线程锁
	UINT32								_indexCurIosRun;		//同步传输线程当前待Post任务线程Index
	std::vector<TSIOSStructPtr>			_vetSyncIosRun;			//同步传输线程组

	std::function<TSString (const TSString &)>	_relativePath2RealPathFun;  //相对路径转绝对路径
	std::function<void (const LogSenderFileInfo &)>	_logSenderFileInfoFun;	//对外文件传输日志信息回调
	std::function<void (const LogSenderFileInfoAck &)> _logSenderFileInfoAckFun;	//对外文件传输接收端确认信息回调
	std::function<void (const LogSenderContentInfo &)>	_logSenderContentInfoFun;	//对外文件内容传输日志信息回调

public:
	TSFileSender(TSDomainPtr domainPtr);
	~TSFileSender();

public:

	/* 注册相对路径转绝对路径函数 */
	void RegRelaticePath2RealPathCallBack(std::function<TSString (const TSString &)> & realativePath2RealPathFun);

	/* 注册对外文件传输日志信息回调 */
	void RegLogSenderFileInfoFunCallBack(std::function<void (const LogSenderFileInfo &)> & logSenderFileInfoFun);

	/* 注册对外文件传输接收端确认信息回调 */
	void RegLogSenderFileInfoAckFunCallBack(std::function<void (const LogSenderFileInfoAck &)> & logSenderFileInfoAckFun);

	/* 对外文件内容传输日志信息回调 */
	void RegLogSenderContentInfoFunCallBack(std::function<void (const LogSenderContentInfo &)> & logSenderContentInfoFun);


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   发送端主动推送文件给接收者（同步）. </summary>
	///
	/// <remarks>   任喜发, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   文件路径. </param>
	/// <param name="receiverAppId">  接收者AppId. </param>
	/// <param name="transfersFormat">传输格式：0-二进制，1-文本 </param>
	/// <param name="verifyBlockType"> 块内容校验方式：0-不校验，1-MD5，2-SHA1，3-CRC32 </param>
	/// <param name="receiveDir"> 接收端文件存放位置 </param>
	///
	/// <returns>	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 SendFileToReceiver(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat, UINT8 verifyBlockType, const TSString & receiveDir = "");

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   发送端主动推送文件给接收者（异步）. </summary>
	///
	/// <remarks>   任喜发, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   文件路径. </param>
	/// <param name="receiverAppId">  接收者AppId. </param>
	/// <param name="transfersFormat">传输格式：0-二进制，1-文本 </param>
	/// <param name="receiveDir"> 接收端文件存放位置 </param>
	///
	/// <returns>	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 SendFileToReceiverAsync(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat, const TSString & receiveDir = "");
	

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   发送端发送文件给接收者，接收者设置存储路径（同步）. </summary>
	///
	/// <remarks>   任喜发, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   文件路径. </param>
	/// <param name="receiverAppId">  接收者AppId. </param>
	/// <param name="transfersFormat">传输格式：0-二进制，1-文本 </param>
	/// <param name="verifyBlockType"> 块内容校验方式：0-不校验，1-MD5，2-SHA1，3-CRC32 </param>
	///
	/// <returns>	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 TransferFileToReceiver(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat, UINT8 verifyBlockType);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   发送端发送文件给接收者，接收者设置存储路径（异步）. </summary>
	///
	/// <remarks>   任喜发, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   文件路径. </param>
	/// <param name="receiverAppId">  接收者AppId. </param>
	/// <param name="transfersFormat">传输格式：0-二进制，1-文本 </param>
	///
	/// <returns>	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 TransferFileToReceiverAsync(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat);


private:
	/* 回调：收到接收者接收文件结果信息 */
	void _CallBackReceiverFileInfoRet(TSTopicContextPtr Ctx);

	/* 回调：收到接收者接收文件传输块结果信息 */
	void _CallBackReceiverFileTransfersInfoRet(TSTopicContextPtr Ctx);

	/* 回调：接收到接收端下载文件请求信息 */
	void _CallBackReceiverDownloadFileRequest(TSTopicContextPtr Ctx);

	/* 回调：收到文件传输严重错误，取消任务 */
	void _CallBackTransfersFileErr(TSTopicContextPtr Ctx);

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   发现接收者 </summary>
	///
	/// <remarks>   任喜发, 2020/7/10. </remarks>
	///
	/// <param name="receiverAppId">   接收者AppId. </param>
	///
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool _DiscoveryReceiver(UINT32 receiverAppId);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   同步发送文件内容 </summary>
	///
	/// <remarks>   任喜发, 2020/7/10. </remarks>
	///
	/// <param name="fileMarkId">   件传输的唯一标识. </param>
	///
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void _SendFileContent(UINT32 fileMarkId, UINT32 receiverAppId);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   异步发送文件内容 </summary>
	///
	/// <remarks>   任喜发, 2020/7/10. </remarks>
	///
	/// <param name="fileMarkId">   件传输的唯一标识. </param>
	///
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void _SendFileContentAsync(UINT32 fileMarkId, UINT32 receiverAppId);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   获取文件传输的唯一标识 </summary>
	///
	/// <remarks>   任喜发, 2020/7/7. </remarks>
	///
	/// <param name="fileName">   文件名称. </param>
	/// <param name="receiverAppId">   接收者AppId. </param>
	///
	/// <returns> bool：true-提取成功，false-提取失败	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 _GetTransfersFileMarkId(const TSString & fileName, UINT32 receiverAppId);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   从文件路径中获取文件名称 </summary>
	///
	/// <remarks>   任喜发, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   文件路径. </param>
	/// <param name="fileName">   文件名称. </param>
	///
	/// <returns> bool：true-提取成功，false-提取失败	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool _GetFileNameByFilePath(const TSString & filePath, TSString & fileName);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   根据文件路径获取文件大小 </summary>
	///
	/// <remarks>   任喜发, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   文件路径. </param>
	/// <param name="fileSize">   文件大小. </param>
	///
	/// <returns> bool：true-获取成功，false-获取失败	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool _GetFileSizeByFilePath(const TSString & fileNPath, UINT64 & fileSize);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   创建发送任务. </summary>
	///
	/// <remarks>   任喜发, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   文件路径. </param>
	/// <param name="receiverAppId">  接收者AppId. </param>
	/// <param name="transfersMode">  传输模式：0-异步，1-同步 </param>
	/// <param name="transfersFormat"> 传输格式：0-二进制，1-文本 </param>
	/// <param name="verifyBlockType">  块内容校验方式：0-不校验，1-MD5，2-SHA1，3-CRC32 </param>
	/// <param name="transfersType">  传输方式：0-主动发送，1-被动请求下载. </param>
	/// <param name="receiveDir"> 接收端文件存放路径 </param>
	///
	/// <returns> UINT32	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 _CreateSendFileTask(
		const TSString & filePath,
		UINT32 receiverAppId,
		UINT8 transfersMode,
		UINT8 transfersFormat,
		UINT8 verifyBlockType,
		const TSString & receiveDir,
		UINT8 transfersType);



	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   文件传输列表相关操作 </summary>
	///
	/// <remarks>   任喜发, 2020/7/8. </remarks>
	///
	/// <param name="fileMarkId">   文件路径. </param>
	/// <param name="transfersFileInfo">	文件传输信息	</param>
	///
	/// <returns> 	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	SenderFileInfo* _SelectSenderFileInfoByFileMarkId(UINT32 fileMarkId);
	bool _InsertSenderFileInfo(const SenderFileInfo & transfersFileInfo);
	bool _UpdateSenderFileInfo(SenderFileInfo & transfersFileInfo);
	bool _DeleteSenderFileInfoByFileMarkId(UINT32 fileMarkId);
};

#endif // !__TSFILESENDER_H__

