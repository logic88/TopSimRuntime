#ifndef __TSITOPICDESCRIPTION_P_H__
#define __TSITOPICDESCRIPTION_P_H__

#include "TSTopicHelper.h"

/* 
	TopicHelper˽�г�Ա�ṹ
*/
struct TOPSIMDATAINTERFACE_DECL TSTopicHelperPrivate
{
	//����������
    std::vector<TSTOPICHANDLE>  InheritTopics;
	//����������
	std::vector<TSTOPICHANDLE>  ParentTopics;
};

#endif // __TSITOPICDESCRIPTION_P_H__


