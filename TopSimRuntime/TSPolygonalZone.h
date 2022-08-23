#ifndef __TSFILERECEIVER_H__
#define __TSFILERECEIVER_H__

#include "TopSimDataInterface/TSFileTransfers/Defined.h"
#include "TopSimDataInterface/TopSimDataInterface.h"

struct TOPSIMDATAINTERFACE_DECL LogReceiverFileInfo
{
	UINT8	_ret;						//�����
	UINT8	_transfersType;				//���䷽ʽ��0-���Ͷ����������ļ������նˣ�1-���ն����ý���Ŀ¼��2-���ն���������
	UINT8	_transfersMode;				//����ģʽ��0-�첽��1-ͬ�� 
	UINT8	_transfersFormat;			//�����ʽ��0-�����ƣ�1-�ı�
	UINT8	_verifyBlockType;			//���������У�鷽ʽ��0-��У�飬1-MD5��2-SHA1��3-CRC32

	UINT32	_fileMarkId;				//�ļ������ʶ
	UINT32	_senderAppId;				//������AppId
	UINT32	_blockSize;					//������С
	UINT64	_fileSize;					//�ļ���С

	TSString	_receiveDir;			//�����ļ����·��
	TSString	_fileName;				//�ļ�����

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
	UINT8	_ret;						//�����
	UINT32	_fileMarkId;				//�ļ������ʶ
	UINT32	_currBlockIndex;			//��ǰ���Ϳ�����

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
	UINT32	_fileMarkId;				//�ļ����������ʶ
	UINT32	_senderAppId;				//������AppId
	UINT32	_blockSize;					//�ļ����С
	UINT32	_blockNum;					//�ļ��ܿ���
	UINT32	_lastBlockSize;				//�ļ����һ�����ݴ�С
	UINT32	_currBlockIndex;			//�ļ���ǰ�������
	UINT8	_threadIndex;				//�߳�Id;
	UINT8	_transfersType;				//���䷽ʽ��0-�������ͣ�1-�����������ش���
	UINT8	_transfersMode;				//����ģʽ��0-�첽��1-ͬ�� 
	UINT8	_transfersFormat;			//�����ʽ��0-�����ƣ�1-�ı�
	UINT8	_verifyBlockType;			//���������У�鷽ʽ��0-��У�飬1-MD5��2-SHA1��3-CRC32
	UINT64	_fileSize;					//�ļ���С
	char *	_buff;						//���ڽ����Ŀ����ݻ�����
	TSString	_receiveDir;			//�����ļ����·��
	TSString	_fileName;				//�ļ�����
	std::vector<bool> _completeBlocks;	//�ļ���������
	std::ofstream	_ofStream;			//�ļ�������

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
	TSDomainPtr							_fileTransfersDomain;	//�ļ�������
	UINT32								_receiverAppId;			//�ļ�������AppId
	TSString							_defaultReceiveDir;		//Ĭ�Ͻ���·��(Ĭ�ϳ�������Ŀ¼)

	TSIOSStructPtr						_asyncIosRun;			//�첽�����߳�

	//ͬ�������߳�
	UINT32								_countSyncIosRun;		//ͬ�������߳�����(Ĭ��5��)
	TSRWSpinLock						_lockSyncIosRun;		//ͬ�������߳���
	UINT32								_indexCurIosRun;		//ͬ�������̵߳�ǰ��Post�����߳�Index
	std::vector<TSIOSStructPtr>			_vetSyncIosRun;			//ͬ�������߳���

	std::function<TSString (const TSString &)>	_setSaveFilePathFun;  //���ý����ļ��ı���λ��
	std::function<void (const LogReceiverFileInfo &)> _logReceiverFileInfoFun;	//�ļ���Ϣ������־�ص�
	std::function<void (const LogReceiverContentInfo &)> _logReceiverContentInfoFun;	//�ļ����ݽ�����־�ص�

	//�����ļ������б�
	TSRWSpinLock						_lockFileMarkId2ReceiverFileInfo;	
	std::map<UINT32, ReceiverFileInfo*>	_mapFileMarkId2ReceiverFileInfo;

public:
	TSFileReceiver(TSDomainPtr domainPtr);
	~TSFileReceiver();

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ע�������ļ����մ洢��·���Ļص��������� </summary>
	///
	/// <remarks>   ��ϲ��, 2021/7/29. </remarks>
	///
	/// <param name="saveFilePath">  �û��ص�����. </param>
	///
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void RegSetSaveFilePathCallBack(const std::function<TSString (const TSString &)> & saveFilePathFun);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ע���ļ�������Ϣ��־�ص� </summary>
	///
	/// <remarks>   ��ϲ��, 2021/7/29. </remarks>
	///
	/// <param name="logReceiverFileInfo">  �û��ص�����. </param>
	///
	/// <returns> void	</returns>
	void RegLogReceiverFileInfoCallBack(const std::function<void (const LogReceiverFileInfo &)> & logReceiverFileInfo);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ע���ļ����ݽ�����־�ص�</summary>
	///
	/// <remarks>   ��ϲ��, 2021/7/29. </remarks>
	///
	/// <param name="logReceiverContentInfo">  �û��ص�����. </param>
	///
	/// <returns> void	</returns>
	void RegLogReceiverContentInfoCallBack(const std::function<void (const LogReceiverContentInfo &)> & logReceiverContentInfo);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ���������ļ� </summary>
	///
	/// <remarks>   ��ϲ��, 2021/8/4. </remarks>
	///
	/// <param name="senderAppId">  �ṩ�ļ����ط���˵�AppID </param>
	/// <param name="transfersFormat">  �����ļ��Ĵ��䷽ʽ 0-�첽 1-ͬ�� </param>
	/// <param name="verifyBlockType">  ������У�鷽ʽ���첽��У�飩��0-��У�� 1-MD5 2-SHA1 3-CRC32 </param>
	/// <param name="_filePath">  ���������ļ�����·�������ṩ���ض˽�����·��ת��Ϊ����ʵ��·�����䣩 </param>
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void RequestDownloadFile(UINT32 senderAppId, UINT8 transfersFormat, UINT8 transfersMode, UINT8 verifyBlockType, TSString _filePath);

private:
	/* �ļ����ն˽��յ��ļ�������������� */
	void _CallBackSenderFileInfo(TSTopicContextPtr Ctx);
	void _CallBackSenderFileTransfersInfo(TSTopicContextPtr Ctx);
	void _CallBackTransfersFileErr(TSTopicContextPtr Ctx);

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>   ���ַ����� </summary>
	///
	/// <remarks>   ��ϲ��, 2020/7/10. </remarks>
	///
	/// <param name="senderAppId">   ������AppId. </param>
	///
	/// <returns> void	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool _DiscoverySender(UINT32 senderAppId);

	/* ͬ�������ļ����ݿ� */
	void _ReceiveFileContent(Huaru::FileTransfers::SenderFileTransfersInfo::DataTypePtr dtPtr);
	/* �첽�����ļ����ݿ� */
	void _ReceiveFileContentAsync(Huaru::FileTransfers::SenderFileTransfersInfo::DataTypePtr dtPtr);

private:
	ReceiverFileInfo * _SelectReceiverFileInfoByFileMarkId(UINT32 fileMarkId);
	bool _InsertReceiverFileInfo(const ReceiverFileInfo & receiverFileInfo);
	bool _UpdateReceiverFileInfo(ReceiverFileInfo & receiverFileInfo);
	bool _DeleteReveiverFileInfoByFileMarkId(UINT32 fileMarkId);
	
};

#endif // !__TSFILERECEIVER_H__
