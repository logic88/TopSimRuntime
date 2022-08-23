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
    /// <summary>   �����ļ�,�첽����. </summary>
    ///
    /// <remarks>   ������, 2019/1/10. </remarks>
    ///
    /// <param name="FilePath">  �ļ�·��. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void RecevieFile(const TSString & FilePath);

private:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   Ԥ���ӷ���������ص�. </summary>
    ///
    /// <remarks>   ������, 2019/1/10. </remarks>
    ///
    /// <param name="Ctx">  ��������. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void Topic_SenderOnData(TSTopicContextPtr Ctx);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   ��ȡ�ļ�����ص�,�ûص������յ����ļ�����д��ָ���ļ���. </summary>
    ///
    /// <remarks>   ������, 2019/1/10. </remarks>
    ///
    /// <param name="Ctx">  �ļ�����. </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void Topic_FileOnData(TSTopicContextPtr Ctx);
};

CLASS_PTR_DECLARE(TopSimFileWriter);

#endif //#ifndef  _WRS_KERNEL
#endif
