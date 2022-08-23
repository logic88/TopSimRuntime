#ifndef __TSITOPICDESCRIPTION_P_H__
#define __TSITOPICDESCRIPTION_P_H__

#include "TSTopicHelper.h"

/* 
	TopicHelper私有成员结构
*/
struct TOPSIMDATAINTERFACE_DECL TSTopicHelperPrivate
{
	//子主题数组
    std::vector<TSTOPICHANDLE>  InheritTopics;
	//父主题数组
	std::vector<TSTOPICHANDLE>  ParentTopics;
};

#endif // __TSITOPICDESCRIPTION_P_H__


