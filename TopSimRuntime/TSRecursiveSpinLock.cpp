#include "stdafx.h"

#include "TSITransferLinker_p.h"
#include "TSITransferLinker.h"

BEGIN_METADATA(TSITransferLinker)
END_METADATA()

TSITransferLinker::TSITransferLinker( PrivateType * p )
	:_Private_Ptr(p)
{

}

TSITransferLinker::~TSITransferLinker()
{
	delete _Private_Ptr;
}

