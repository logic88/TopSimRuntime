#include "stdafx.h"

#include "TSIMonitor.h"
#include "TSIMonitor_p.h"

BEGIN_METADATA(TSIMonitor)
	END_METADATA()

TSIMonitor::TSIMonitor( void )
	:_Private_Ptr(new PrivateType)
{

}

TSIMonitor::TSIMonitor( PrivateType * p )
	:_Private_Ptr(p)
{

}

TSIMonitor::~TSIMonitor( void )
{
	delete _Private_Ptr;
}

