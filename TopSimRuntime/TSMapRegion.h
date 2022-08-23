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
    /// <summary>   �����ļ�. </summary>
    ///
    /// <remarks>   ������, 2019/1/10. </remarks>
    ///
    /// <param name="FilePah">   �ļ�·��. </param>
    /// <param name="IsBinary">  trueΪ�������ļ�. </param>
    /// <param name="group">     trueΪȺ��,falseΪ��Ե��Ԥ�������ʹ�� </param>
    /// <param name="AsyncSend"> trueΪ�첽,falseΪͬ�� </param>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void SendFileAsync(const TSString & FilePah, bool IsBinary = false,const TSFrontAppUniquelyId & AppId = 0);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   �����ļ�. </summary>
    ///
    /// <remarks>   ������, 2019/2/27. </remarks>
    ///
    /// <param name="FilePah">  �ļ�·��. </param>
    /// <param name="IsBinary"> �Ƕ�����. </param>
    /// <param name="AppId">    Ӧ�ó���ID. </param>
    ///
    /// <returns>   true ���ͳɹ�, false ����ʧ��. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool SendFile(const TSString & FilePah, bool IsBinary = false,const TSFrontAppUniquelyId & AppId = 0);

private:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   ��ȡ�ļ���. </summary>
    ///
    /// <remarks>   ������, 2019/1/10. </remarks>
    ///
    /// <param name="FileName"> �ļ�ȫ��. </param>
    ///
    /// <returns>   �ļ��̵�����. </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    TSString GetFileShortName(const TSString & FileName);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>   Ԥ����. </summary>
    ///
    /// <remarks>   ������, 2019/1/10. </remarks>
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool SendFileInfo(const TSString & FilePah, bool IsBinary,const TSFrontAppUniquelyId & AppId);
};
CLASS_PTR_DECLARE(TopSimFileReader);

#endif //#ifndef  _WRS_KERNEL
#endif
