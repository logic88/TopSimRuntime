#include "stdafx.h"

#include "TSITransferLog.h"
#include "TSITransferLog_p.h"

BEGIN_METADATA(TSITransferLog)
    END_METADATA()

TSITransferLog::TSITransferLog( PrivateType * p )
    :_Private_Ptr(p)
{

}

TSITransferLog::~TSITransferLog( void )
{
    delete _Private_Ptr;
}

