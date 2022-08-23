#include "stdafx.h"

#include "TSITransferMiddleware_p.h"
#include "TSITransferMiddleware.h"

BEGIN_METADATA(TSITransferMiddleware)
	END_METADATA()

TSITransferMiddleware::TSITransferMiddleware( PrivateType * p )
	:_Private_Ptr(p)
{

}

TSITransferMiddleware::~TSITransferMiddleware( void )
{
	delete _Private_Ptr;
}

