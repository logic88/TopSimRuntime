#include "stdafx.h"

#ifndef  _WRS_KERNEL

#include "TopSimFileTransfers.h"
#include "TopSimFileTransfers_p.h"

TopSimFileTransfers::TopSimFileTransfers():TopSimStreamTransfers(new PrivateType)
{

}

TopSimFileTransfers::TopSimFileTransfers(PrivateType * p) : TopSimStreamTransfers(p)
{

}

TopSimFileTransfers::~TopSimFileTransfers()
{

}

TSString TopSimFileTransfers::GetSendingFileName()
{
    T_D();
    return d->_fileShortName;
}

TSFileInfoPtr TopSimFileTransfers::GetFileInfo()
{
    T_D();
    return d->_FileInfo;
}

#endif //#ifndef  _WRS_KERNEL
