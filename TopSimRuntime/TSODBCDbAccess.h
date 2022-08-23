#ifndef __TSDATARECORDHELPER__H__
#define __TSDATARECORDHELPER__H__

#ifndef  _WRS_KERNEL

#include "TopSimDataInterface.h"

class TOPSIMDATAINTERFACE_DECL TSDataRecorderHelper
{
	TS_MetaType(TSDataRecorderHelper);
	TS_NOCOPYABLE(TSDataRecorderHelper);
public:
	TSDataRecorderHelper();
	~TSDataRecorderHelper();
protected:
	typedef struct TSDataRecorderHelperPrivate PrivateType;
	PrivateType * _Private_Ptr;
	TSDataRecorderHelper(PrivateType * p);

public:
	void SetIOS(TSIOSericePtr ios);
public:
	void SetBaseOutFileName(const TSString& baseOutFileName);
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	��ȡ������ļ���������(������չ��) </summary>
	///
	/// <returns>	�ļ����� </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const TSString& GetBaseOutFileName();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�򿪼�¼�ļ�. </summary>
	///
	/// <remarks>	TSMF Team, 2014/12/8. </remarks>
	///
	/// <param name="FileExt"> 	�ļ���չ��. </param>
	/// <param name="FileSign">	�ļ�ǩ��. </param>
	/// <param name="FileVer"> 	�ļ��汾. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void OpenRecordFile(const TSString & FileExt,const TSString & FileSign,UINT32 FileVer);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	д������. </summary>
	///
	/// <remarks>	TSMF Team, 2014/12/8. </remarks>
	///
	/// <param name="data">	��д������. </param>
	/// <param name="len"> 	���ݳ���. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void WriteData(boost::shared_ptr<char> data,std::size_t len);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�رռ�¼�ļ�. </summary>
	///
	/// <remarks>	TSMF Team, 2014/12/8. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void CloseRecordFile();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	�ж����ݼ�¼���������Ƿ��. </summary>
	///
	/// <remarks>	TSMF Team, 2014/12/8. </remarks>
	///
	/// <returns>	�Ƿ��. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool IsOpen() const;
private:
	void WriteFileImmediately(boost::shared_ptr<char> data,std::size_t len);
	void CloseFileImmediately();
};

CLASS_PTR_DECLARE(TSDataRecorderHelper);

#endif //#ifndef  _WRS_KERNEL
#endif // __TSDATARECORDHELPER__H__

