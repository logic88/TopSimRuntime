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
	/// <summary>	获取输出的文件基本名称(不含扩展名) </summary>
	///
	/// <returns>	文件名称 </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const TSString& GetBaseOutFileName();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	打开记录文件. </summary>
	///
	/// <remarks>	TSMF Team, 2014/12/8. </remarks>
	///
	/// <param name="FileExt"> 	文件扩展名. </param>
	/// <param name="FileSign">	文件签名. </param>
	/// <param name="FileVer"> 	文件版本. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void OpenRecordFile(const TSString & FileExt,const TSString & FileSign,UINT32 FileVer);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	写入数据. </summary>
	///
	/// <remarks>	TSMF Team, 2014/12/8. </remarks>
	///
	/// <param name="data">	等写入数据. </param>
	/// <param name="len"> 	数据长度. </param>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void WriteData(boost::shared_ptr<char> data,std::size_t len);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	关闭记录文件. </summary>
	///
	/// <remarks>	TSMF Team, 2014/12/8. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void CloseRecordFile();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	判断数据记录服务助手是否打开. </summary>
	///
	/// <remarks>	TSMF Team, 2014/12/8. </remarks>
	///
	/// <returns>	是否打开. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool IsOpen() const;
private:
	void WriteFileImmediately(boost::shared_ptr<char> data,std::size_t len);
	void CloseFileImmediately();
};

CLASS_PTR_DECLARE(TSDataRecorderHelper);

#endif //#ifndef  _WRS_KERNEL
#endif // __TSDATARECORDHELPER__H__

