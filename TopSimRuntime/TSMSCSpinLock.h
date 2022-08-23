#ifndef __TOPSIMFILEWRITER_H__
#define __TOPSIMFILEWRITER_H__

#ifndef  _WRS_KERNEL

#include <TopSimDataInterface/TopSimFileTransfers.h>

struct TopSimFileWriterPrivate;
class TOPSIMDATAINTERFACE_DECL TopSimFileWriter : public TopSimFileTransfers
{

    TS_DECLARE_PRIVATE(TopSimFileWriter);
public:
    TopSimFileWriter();
    ~TopSimFileWriter();
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   接收文件,异步接收. </summary>
    ///
    /// <remarks>   李子洋, 2019/1/10. </remarks>
    ///
    /// <param name="FilePath">  文件路径. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void RecevieFile(const TSString & FilePath);

private:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   预连接发送者主题回调. </summary>
    ///
    /// <remarks>   李子洋, 2019/1/10. </remarks>
    ///
    /// <param name="Ctx">  主题数据. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void Topic_SenderOnData(TSTopicContextPtr Ctx);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   获取文件主题回调,该回调将接收到的文件数据写到指定文件中. </summary>
    ///
    /// <remarks>   李子洋, 2019/1/10. </remarks>
    ///
    /// <param name="Ctx">  文件数据. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void Topic_FileOnData(TSTopicContextPtr Ctx);
};

CLASS_PTR_DECLARE(TopSimFileWriter);

#endif //#ifndef  _WRS_KERNEL
#endif
