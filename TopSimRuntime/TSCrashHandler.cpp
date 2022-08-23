#include "stdafx.h"
#include "HRStructDecl.h"

BEGIN_METADATA(HRTopicDataConstructInfo)
	REG_PROP_FIELD(Handle);
	REG_PROP_FIELD(Data);
END_METADATA()

HRTopicDataConstructInfo::HRTopicDataConstructInfo()
:Handle(TS_INVALID)
{

}

HRTopicDataConstructInfo::HRTopicDataConstructInfo( TSTOPICHANDLE Handle, VoidPtr Data )
	:Handle(Handle), Data(Data)
{

}

BEGIN_METADATA(HRTopicDataConstructInfos)
	REG_PROP_FIELD(Data);
END_METADATA()
