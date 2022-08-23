#ifndef __TOPSIMFILETRANSFERS__H__
#define __TOPSIMFILETRANSFERS__H__

#ifndef  _WRS_KERNEL

#include <iostream>

#include <TopSimDataInterface/TopSimDataInterface.h>
#include <TopSimDataInterface/TopSimStreamTransfers.h>

struct TopSimFileTransfersPrivate;
class TOPSIMDATAINTERFACE_DECL TopSimFileTransfers : public TopSimStreamTransfers
{
   TS_DECLARE_PRIVATE(TopSimFileTransfers);
public:
   TopSimFileTransfers();
   ~TopSimFileTransfers();
protected:
    TopSimFileTransfers(PrivateType * p);
public:

   ////////////////////////////////////////////////////////////////////////////////////////////////////
   /// <summary>   ��ȡ���ڷ��͵��ļ���. </summary>
   ///
   /// <remarks>   ������, 2019/1/25. </remarks>
   ///
   /// <returns>   ���ڷ��͵��ļ�. </returns>
   ////////////////////////////////////////////////////////////////////////////////////////////////////

   TSString GetSendingFileName();

   virtual TSFileInfoPtr GetFileInfo();

private:
};
CLASS_PTR_DECLARE(TopSimFileTransfers);


#endif //#ifndef  _WRS_KERNEL
#endif
