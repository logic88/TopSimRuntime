#ifndef __TOPSIMFILEREADER_H__
#define __TOPSIMFILEREADER_H__

#ifndef  _WRS_KERNEL

#include <TopSimDataInterface/TopSimFileTransfers.h>
#include <boost/system/error_code.hpp>

struct TopSimFileReaderPrivate;
class TOPSIMDATAINTERFACE_DECL TopSimFileReader : public TopSimFileTransfers
{
    TS_DECLARE_PRIVATE(TopSimFileReader);
public:
    TopSimFileReader();
    ~TopSimFileReader();
public:


    void StopSend();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   发送文件. </summary>
    ///
    /// <remarks>   李子洋, 2019/1/10. </remarks>
    ///
    /// <param name="FilePah">   文件路径. </param>
    /// <param name="IsBinary">  true为二进制文件. </param>
    /// <param name="group">     true为群发,false为点对点和预连接配合使用 </param>
    /// <param name="AsyncSend"> true为异步,false为同步 </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void SendFileAsync(const TSString & FilePah, bool IsBinary = false,const TSFrontAppUniquelyId & AppId = 0);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   发送文件. </summary>
    ///
    /// <remarks>   李子洋, 2019/2/27. </remarks>
    ///
    /// <param name="FilePah">  文件路径. </param>
    /// <param name="IsBinary"> 是二进制. </param>
    /// <param name="AppId">    应用程序ID. </param>
    ///
    /// <returns>   true 发送成功, false 发送失败. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool SendFile(const TSString & FilePah, bool IsBinary = false,const TSFrontAppUniquelyId & AppId = 0);

private:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   获取文件名. </summary>
    ///
    /// <remarks>   李子洋, 2019/1/10. </remarks>
    ///
    /// <param name="FileName"> 文件全名. </param>
    ///
    /// <returns>   文件短的名字. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    TSString GetFileShortName(const TSString & FileName);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   预连接. </summary>
    ///
    /// <remarks>   李子洋, 2019/1/10. </remarks>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool SendFileInfo(const TSString & FilePah, bool IsBinary,const TSFrontAppUniquelyId & AppId);
};
CLASS_PTR_DECLARE(TopSimFileReader);

#endif //#ifndef  _WRS_KERNEL
#endif
