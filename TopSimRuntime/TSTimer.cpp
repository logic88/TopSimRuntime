#include "stdafx.h"
#include "TSTopicHelper.h"
#include "TSTopicHelper_p.h"

#include "TSTopicTypeManager.h"

TSTopicHelper::TSTopicHelper(void)
{
}

TSTopicHelper::~TSTopicHelper(void)
{
	
}

/* 添加子主题 */
void TSTopicHelper::AddChild( TSTOPICHANDLE Child )
{

	std::vector<TSTOPICHANDLE>::iterator iter = 
        std::find(InheritTopics.begin(),InheritTopics.end(),Child);

	if(iter == InheritTopics.end())
	{
		InheritTopics.push_back(Child);
	}
}


bool TSTopicHelper::CanConvert( TSTOPICHANDLE From)
{
	return CanConvertStatic(From);
}

/* 添加父主题 */
void TSTopicHelper::AddParent( TSTOPICHANDLE Parent )
{

	std::vector<TSTOPICHANDLE>::iterator iter = 
		std::find(ParentTopics.begin(),ParentTopics.end(),Parent);

	if(iter == ParentTopics.end())
	{
		ParentTopics.push_back(Parent);
	}
}

bool TSTopicHelper::CanConvertStatic( TSTOPICHANDLE From)
{
	return false;
}

