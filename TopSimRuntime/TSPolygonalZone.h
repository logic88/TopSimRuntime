#ifndef __TSFILERECEIVER_H__
#define __TSFILERECEIVER_H__

#include "TopSimDataInterface/TSFileTransfers/Defined.h"
#include "TopSimDataInterface/TopSimDataInterface.h"

struct TOPSIMDATAINTERFACE_DECL LogReceiverFileInfo
{
	UINT8	_ret;						//结果码
	UINT8	_transfersType;				//传输方式：0-发送端主动推送文件到接收端，1-接收端设置接收目录，2-接收端请求下载
	UINT8	_transfersMode;				//传输模式：0-异步，1-同步 
	UINT8	_transfersFormat;			//传输格式：0-二进制，1-文本
	UINT8	_verifyBlockType;			//传输块内容校验方式：0-不校验，1-MD5，2-SHA1，3-CRC32

	UINT32	_fileMarkId;				//文件传输标识
	UINT32	_senderAppId;				//发送者AppId
	UINT32	_blockSize;					//传输块大小
	UINT64	_fileSize;					//文件大小

	TSString	_receiveDir;			//接收文件存放路径
	TSString	_fileName;				//文件名称

	LogReceiverFileInfo() :
		_ret(0),
		_transfersType(0),
		_transfersMode(0),
		_transfersFormat(0),
		_verifyBlockType(0),
		_fileMarkId(0),
		_senderAppId(0),
		_blockSize(0),
		_fileSize(0)
	{

	}

	LogReceiverFileInfo(const LogReceiverFileInfo & other)
	{
		_ret = other._ret;
		_transfersType = other._transfersType;
		_transfersMode = other._transfersMode;
		_transfersFormat = other._transfersFormat;
		_verifyBlockType = other._verifyBlockType;
		_fileMarkId = other._fileMarkId;
		_senderAppId = other._senderAppId;
		_blockSize = other._blockSize;
		_fileSize = other._fileSize;
		_receiveDir = other._receiveDir;
		_fileName = other._fileName;
	}

	LogReceiverFileInfo & operator=(const LogReceiverFileInfo & other)
	{
		_ret = other._ret;
		_transfersType = other._transfersType;
		_transfersMode = other._transfersMode;
		_transfersFormat = other._transfersFormat;
		_verifyBlockType = other._verifyBlockType;
		_fileMarkId = other._fileMarkId;
		_senderAppId = other._senderAppId;
		_blockSize = other._blockSize;
		_fileSize = other._fileSize;
		_receiveDir = other._receiveDir;
		_fileName = other._fileName;

		return *this;
	}
};

struct TOPSIMDATAINTERFACE_DECL LogReceiverContentInfo
{
	UINT8	_ret;						//结果码
	UINT32	_fileMarkId;				//文件传输标识
	UINT32	_currBlockIndex;			//当前发送块索引

	LogReceiverContentInfo() :
		_ret(0),
		_fileMarkId(0),
		_currBlockIndex(0)
	{

	}

	LogReceiverContentInfo(const LogReceiverContentInfo & other)
	{
		_ret = other._ret;
		_fileMarkId = other._fileMarkId;
		_currBlockIndex = other._currBlockIndex;
	}

	LogReceiverContentInfo & operator=(const LogReceiverContentInfo & other)
	{
		_ret = other._ret;
		_fileMarkId = other._fileMarkId;
		_currBlockIndex = other._currBlockIndex;
		return *this;
	}
};

struct TOPSIMDATAINTERFACE_DECL ReceiverFileInfo
{
	UINT32	_fileMarkId;				//文件传输任务标识
	UINT32	_senderAppId;				//发送者AppId
	UINT32	_blockSize;					//文件块大小
	UINT32	_blockNum;					//文件总块数
	UINT32	_lastBlockSize;				//文件最后一块内容大小
	UINT32	_currBlockIndex;			//文件当前块的索引
	UINT8	_threadIndex;				//线程Id;
	UINT8	_transfersType;				//传输方式：0-主动发送，1-被动请求下载传输
	UINT8	_transfersMode;				//传输模式：0-异步，1-同步 
	UINT8	_transfersFormat;			//传输格式：0-二进制，1-文本
	UINT8	_verifyBlockType;			//传输块内容校验方式：0-不校验，1-MD5，2-SHA1，3-CRC32
	UINT64	_fileSize;					//文件大小
	char *	_buff;						//用于接收文块内容缓冲区
	TSString	_receiveDir;			//接收文件存放路径
	TSString	_fileName;				//文件名称
	std::vector<bool> _completeBlocks;	//文件块完成情况
	std::ofstream	_ofStream;			//文件流对象

	ReceiverFileInfo() :
		_fileMarkId(0),
		_senderAppId(0),
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

	ReceiverFileInfo(UINT32 blockNum) :
		_fileMarkId(0),
		_senderAppId(0),
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

	ReceiverFileInfo(const ReceiverFileInfo & other)
	{
		_fileMarkId = other._fileMarkId;
		_senderAppId = other._senderAppId;
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
		_completeBlocks = other._completeBlocks;
	}

	ReceiverFileInfo & operator=(const ReceiverFileInfo & other)
	{
		_fileMarkId = other._fileMarkId;
		_senderAppId = other._senderAppId;
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
		_completeBlocks = other._completeBlocks;
		return *this;
	}
};

class TOPSIMDATAINTERFACE_DECL TSFileReceiver
{
private:
	TSDomainPtr							_fileTransfersDomain;	//文件传输域
	UINT32								_receiverAppId;			//文件接收者AppId
	TSString							_defaultReceiveDir;		//默认接收路径(默认程序运行目录)

	TSIOSStructPtr						_asyncIosRun;			//异步传输线程

	//同步传输线程
	UINT32								_countSyncIosRun;		//同步传输线程数量(默认5个)
	TSRWSpinLock						_lockSyncIosRun;		//同步传输线程锁
	UINT32								_indexCurIosRun;		//同步传输线程当前待Post任务线程Index
	std::vector<TSIOSStructPtr>			_vetSyncIosRun;			//同步传输线程组

	std::function<TSString (const TSString &)>	_setSaveFilePathFun;  //设置接收文件的保存位置
	std::function<void (const LogReceiverFileInfo &)> _logReceiverFileInfoFun;	//文件信息接收日志回调
	std::function<void (const LogReceiverContentInfo &)> _logReceiverContentInfoFun;	//文件内容接收日志回调

	//接收文件任务列表
	TSRWSpinLock						_lockFileMarkId2ReceiverFileInfo;	
	std::map<UINT32, ReceiverFileInfo*>	_mapFileMarkId2ReceiverFileInfo;

public:
	TSFileReceiver(TSDomainPtr domainPtr);
	~TSFileReceiver();

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   注册设置文件接收存储的路径的回调函数函数 </summary>
	///
	/// <remarks>   任喜发, 2021/7/29. </remarks>
	///
	/// <param name="saveFilePath">  用户回调函数. </param>
	///
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void RegSetSaveFilePathCallBack(const std::function<TSString (const TSString &)> & saveFilePathFun);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   注册文件接收信息日志回调 </summary>
	///
	/// <remarks>   任喜发, 2021/7/29. </remarks>
	///
	/// <param name="logReceiverFileInfo">  用户回调函数. </param>
	///
	/// <returns> void	</returns>
	void RegLogReceiverFileInfoCallBack(const std::function<void (const LogReceiverFileInfo &)> & logReceiverFileInfo);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   注册文件内容接收日志回调</summary>
	///
	/// <remarks>   任喜发, 2021/7/29. </remarks>
	///
	/// <param name="logReceiverContentInfo">  用户回调函数. </param>
	///
	/// <returns> void	</returns>
	void RegLogReceiverContentInfoCallBack(const std::function<void (const LogReceiverContentInfo &)> & logReceiverContentInfo);

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
	void RequestDownloadFile(UINT32 senderAppId, UINT8 transfersFormat, UINT8 transfersMode, UINT8 verifyBlockType, TSString _filePath);

private:
	/* 文件接收端接收到文件传输任务的主题 */
	void _CallBackSenderFileInfo(TSTopicContextPtr Ctx);
	void _CallBackSenderFileTransfersInfo(TSTopicContextPtr Ctx);
	void _CallBackTransfersFileErr(TSTopicContextPtr Ctx);

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   发现发送者 </summary>
	///
	/// <remarks>   任喜发, 2020/7/10. </remarks>
	///
	/// <param name="senderAppId">   发送者AppId. </param>
	///
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool _DiscoverySender(UINT32 senderAppId);

	/* 同步接收文件内容块 */
	void _ReceiveFileContent(Huaru::FileTransfers::SenderFileTransfersInfo::DataTypePtr dtPtr);
	/* 异步接收文件内容块 */
	void _ReceiveFileContentAsync(Huaru::FileTransfers::SenderFileTransfersInfo::DataTypePtr dtPtr);

private:
	ReceiverFileInfo * _SelectReceiverFileInfoByFileMarkId(UINT32 fileMarkId);
	bool _InsertReceiverFileInfo(const ReceiverFileInfo & receiverFileInfo);
	bool _UpdateReceiverFileInfo(ReceiverFileInfo & receiverFileInfo);
	bool _DeleteReveiverFileInfoByFileMarkId(UINT32 fileMarkId);
	
};

#endif // !__TSFILERECEIVER_H__
