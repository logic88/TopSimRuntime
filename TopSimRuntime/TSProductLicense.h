#ifndef __TSFILESENDER_H__
#define __TSFILESENDER_H__

#include "TopSimDataInterface/TSFileTransfers/Defined.h"
#include "TopSimDataInterface/TopSimDataInterface.h"

struct TOPSIMDATAINTERFACE_DECL LogSenderFileInfo
{
	UINT8	_ret;						//�����
	UINT8	_transfersType;				//���䷽ʽ��0-���Ͷ����������ļ������նˣ�1-���ն����ý���Ŀ¼��2-���ն���������
	UINT8	_transfersMode;				//����ģʽ��0-�첽��1-ͬ�� 
	UINT8	_transfersFormat;			//�����ʽ��0-�����ƣ�1-�ı�
	UINT8	_verifyBlockType;			//���������У�鷽ʽ��0-��У�飬1-MD5��2-SHA1��3-CRC32

	UINT32	_fileMarkId;				//�ļ������ʶ
	UINT32	_receiverAppId;				//������AppId
	UINT32	_blockSize;					//������С
	UINT64	_fileSize;					//�ļ���С

	TSString	_fileName;				//�ļ�����

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
	UINT8	_ret;						//�����
	UINT8	_receiverRet;				//���ն˽����
	UINT32	_fileMarkId;				//�ļ������ʶ
	UINT32	_receiverAppId;				//������AppId

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
	UINT8	_ret;						//�����
	UINT32	_fileMarkId;				//�ļ������ʶ
	UINT32	_currBlockIndex;			//��ǰ���Ϳ�����

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
	�����ļ���Ϣ
*/
struct TOPSIMDATAINTERFACE_DECL SenderFileInfo
{
	UINT32	_fileMarkId;				//�ļ������ʶ
	UINT32	_receiverAppId;				//������AppId
	UINT32	_blockSize;					//������С
	UINT32	_blockNum;					//���������
	UINT32	_lastBlockSize;				//���һ��������С
	UINT32	_currBlockIndex;			//��ǰ���Ϳ�����
	UINT8	_threadIndex;				//�߳�Id;
	UINT8	_transfersType;				//���䷽ʽ��0-���Ͷ����������ļ������նˣ�1-���ն����ý���Ŀ¼��2-���ն���������
	UINT8	_transfersMode;				//����ģʽ��0-�첽��1-ͬ�� 
	UINT8	_transfersFormat;			//�����ʽ��0-�����ƣ�1-�ı�
	UINT8	_verifyBlockType;			//���������У�鷽ʽ��0-��У�飬1-MD5��2-SHA1��3-CRC32
	UINT64	_fileSize;					//�ļ���С
	char *	_buff;						//���ͻ�����
	TSString	_fileName;				//�ļ�����
	TSString	_receiveDir;			//���ն��ļ����·��
	std::vector<bool> _completeBlocks;	//ÿ���ļ��鴫�����
	std::ifstream	_ifStream;			//�ļ���ȡ��
	boost::mutex _mutexLock;			//�ļ�ͬ��������
	boost::condition_variable _conditionVariable;	//�ļ�ͬ��������������

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
	TSDomainPtr	_fileTransfersDomain;		//�ļ�������
	UINT32		_senderAppId;				//���Ͷ�AppId
	UINT32		_blockSize;					//�ļ�������С

	//���������б�
	TSRWSpinLock						_lockFileMarkId2SenderFileInfo;
	std::map<UINT32, SenderFileInfo*>	_mapFileMarkId2SenderFileInfo;

	TSIOSStructPtr	_asyncIosRun;			//�첽�����߳�

	//ͬ�������߳�
	UINT32								_countSyncIosRun;		//ͬ�������߳�����
	TSRWSpinLock						_lockSyncIosRun;		//ͬ�������߳���
	UINT32								_indexCurIosRun;		//ͬ�������̵߳�ǰ��Post�����߳�Index
	std::vector<TSIOSStructPtr>			_vetSyncIosRun;			//ͬ�������߳���

	std::function<TSString (const TSString &)>	_relativePath2RealPathFun;  //���·��ת����·��
	std::function<void (const LogSenderFileInfo &)>	_logSenderFileInfoFun;	//�����ļ�������־��Ϣ�ص�
	std::function<void (const LogSenderFileInfoAck &)> _logSenderFileInfoAckFun;	//�����ļ�������ն�ȷ����Ϣ�ص�
	std::function<void (const LogSenderContentInfo &)>	_logSenderContentInfoFun;	//�����ļ����ݴ�����־��Ϣ�ص�

public:
	TSFileSender(TSDomainPtr domainPtr);
	~TSFileSender();

public:

	/* ע�����·��ת����·������ */
	void RegRelaticePath2RealPathCallBack(std::function<TSString (const TSString &)> & realativePath2RealPathFun);

	/* ע������ļ�������־��Ϣ�ص� */
	void RegLogSenderFileInfoFunCallBack(std::function<void (const LogSenderFileInfo &)> & logSenderFileInfoFun);

	/* ע������ļ�������ն�ȷ����Ϣ�ص� */
	void RegLogSenderFileInfoAckFunCallBack(std::function<void (const LogSenderFileInfoAck &)> & logSenderFileInfoAckFun);

	/* �����ļ����ݴ�����־��Ϣ�ص� */
	void RegLogSenderContentInfoFunCallBack(std::function<void (const LogSenderContentInfo &)> & logSenderContentInfoFun);


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ���Ͷ����������ļ��������ߣ�ͬ����. </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   �ļ�·��. </param>
	/// <param name="receiverAppId">  ������AppId. </param>
	/// <param name="transfersFormat">�����ʽ��0-�����ƣ�1-�ı� </param>
	/// <param name="verifyBlockType"> ������У�鷽ʽ��0-��У�飬1-MD5��2-SHA1��3-CRC32 </param>
	/// <param name="receiveDir"> ���ն��ļ����λ�� </param>
	///
	/// <returns>	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 SendFileToReceiver(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat, UINT8 verifyBlockType, const TSString & receiveDir = "");

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ���Ͷ����������ļ��������ߣ��첽��. </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   �ļ�·��. </param>
	/// <param name="receiverAppId">  ������AppId. </param>
	/// <param name="transfersFormat">�����ʽ��0-�����ƣ�1-�ı� </param>
	/// <param name="receiveDir"> ���ն��ļ����λ�� </param>
	///
	/// <returns>	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 SendFileToReceiverAsync(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat, const TSString & receiveDir = "");
	

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ���Ͷ˷����ļ��������ߣ����������ô洢·����ͬ����. </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   �ļ�·��. </param>
	/// <param name="receiverAppId">  ������AppId. </param>
	/// <param name="transfersFormat">�����ʽ��0-�����ƣ�1-�ı� </param>
	/// <param name="verifyBlockType"> ������У�鷽ʽ��0-��У�飬1-MD5��2-SHA1��3-CRC32 </param>
	///
	/// <returns>	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 TransferFileToReceiver(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat, UINT8 verifyBlockType);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ���Ͷ˷����ļ��������ߣ����������ô洢·�����첽��. </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   �ļ�·��. </param>
	/// <param name="receiverAppId">  ������AppId. </param>
	/// <param name="transfersFormat">�����ʽ��0-�����ƣ�1-�ı� </param>
	///
	/// <returns>	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 TransferFileToReceiverAsync(const TSString & filePath, UINT32 receiverAppId, UINT8 transfersFormat);


private:
	/* �ص����յ������߽����ļ������Ϣ */
	void _CallBackReceiverFileInfoRet(TSTopicContextPtr Ctx);

	/* �ص����յ������߽����ļ����������Ϣ */
	void _CallBackReceiverFileTransfersInfoRet(TSTopicContextPtr Ctx);

	/* �ص������յ����ն������ļ�������Ϣ */
	void _CallBackReceiverDownloadFileRequest(TSTopicContextPtr Ctx);

	/* �ص����յ��ļ��������ش���ȡ������ */
	void _CallBackTransfersFileErr(TSTopicContextPtr Ctx);

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ���ֽ����� </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/10. </remarks>
	///
	/// <param name="receiverAppId">   ������AppId. </param>
	///
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool _DiscoveryReceiver(UINT32 receiverAppId);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ͬ�������ļ����� </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/10. </remarks>
	///
	/// <param name="fileMarkId">   �������Ψһ��ʶ. </param>
	///
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void _SendFileContent(UINT32 fileMarkId, UINT32 receiverAppId);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   �첽�����ļ����� </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/10. </remarks>
	///
	/// <param name="fileMarkId">   �������Ψһ��ʶ. </param>
	///
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void _SendFileContentAsync(UINT32 fileMarkId, UINT32 receiverAppId);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ��ȡ�ļ������Ψһ��ʶ </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/7. </remarks>
	///
	/// <param name="fileName">   �ļ�����. </param>
	/// <param name="receiverAppId">   ������AppId. </param>
	///
	/// <returns> bool��true-��ȡ�ɹ���false-��ȡʧ��	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 _GetTransfersFileMarkId(const TSString & fileName, UINT32 receiverAppId);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ���ļ�·���л�ȡ�ļ����� </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   �ļ�·��. </param>
	/// <param name="fileName">   �ļ�����. </param>
	///
	/// <returns> bool��true-��ȡ�ɹ���false-��ȡʧ��	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool _GetFileNameByFilePath(const TSString & filePath, TSString & fileName);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   �����ļ�·����ȡ�ļ���С </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   �ļ�·��. </param>
	/// <param name="fileSize">   �ļ���С. </param>
	///
	/// <returns> bool��true-��ȡ�ɹ���false-��ȡʧ��	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool _GetFileSizeByFilePath(const TSString & fileNPath, UINT64 & fileSize);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ������������. </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/7. </remarks>
	///
	/// <param name="filePath">   �ļ�·��. </param>
	/// <param name="receiverAppId">  ������AppId. </param>
	/// <param name="transfersMode">  ����ģʽ��0-�첽��1-ͬ�� </param>
	/// <param name="transfersFormat"> �����ʽ��0-�����ƣ�1-�ı� </param>
	/// <param name="verifyBlockType">  ������У�鷽ʽ��0-��У�飬1-MD5��2-SHA1��3-CRC32 </param>
	/// <param name="transfersType">  ���䷽ʽ��0-�������ͣ�1-������������. </param>
	/// <param name="receiveDir"> ���ն��ļ����·�� </param>
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
	/// <summary>   �ļ������б���ز��� </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/8. </remarks>
	///
	/// <param name="fileMarkId">   �ļ�·��. </param>
	/// <param name="transfersFileInfo">	�ļ�������Ϣ	</param>
	///
	/// <returns> 	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	SenderFileInfo* _SelectSenderFileInfoByFileMarkId(UINT32 fileMarkId);
	bool _InsertSenderFileInfo(const SenderFileInfo & transfersFileInfo);
	bool _UpdateSenderFileInfo(SenderFileInfo & transfersFileInfo);
	bool _DeleteSenderFileInfoByFileMarkId(UINT32 fileMarkId);
};

#endif // !__TSFILESENDER_H__

