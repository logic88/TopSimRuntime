#ifndef HRSTRUCTDECL_H
#define HRSTRUCTDECL_H

#include "HRUICommon.h"

struct HRUICOMMON_EXPORT HRTopicDataConstructInfo
{
	TS_MetaType(HRTopicDataConstructInfo);

	HRTopicDataConstructInfo();

	HRTopicDataConstructInfo(TSTOPICHANDLE Handle, VoidPtr Data);

	TSTOPICHANDLE Handle;
	VoidPtr Data;
};

struct HRUICOMMON_EXPORT HRTopicDataConstructInfos
{
	TS_MetaType(HRTopicDataConstructInfos);

	std::vector< HRTopicDataConstructInfo > Data;
};

#endif // HRSTRUCTDECL_H
