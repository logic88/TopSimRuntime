#ifndef __TSPSMATCHER_H__
#define __TSPSMATCHER_H__

#include "TopSimDataInterface.h"
#include "TSIPSMatcher.h"

#include "TSPSMatcher_p.h"

struct TSPSMatcherPrivate;
class TOPSIMDATAINTERFACE_DECL TSPSMatcher : public TSIPSMatcher
{	
	TS_MetaType(TSPSMatcher,TSIPSMatcher);
public:
	TSPSMatcher(void);
	~TSPSMatcher(void);

	TSPSMatcher(const TSPSMatcher& o);

protected:
	typedef TSPSMatcherPrivate PrivateType;
	PrivateType * _Private_Ptr;
	TSPSMatcher(PrivateType * p);

public:
	/* 订阅主题 */
	TopSimDataInterface::ReturnCode Subscribe(TSTOPICHANDLE Handle);
	/* 发布主题 */
	TopSimDataInterface::ReturnCode Publish(TSTOPICHANDLE Handle);
	/* 取消主题的订阅 */
	TopSimDataInterface::ReturnCode Unsubscribe(TSTOPICHANDLE Handle);
	/* 取消主题发布 */
	TopSimDataInterface::ReturnCode Unpublish(TSTOPICHANDLE Handle);

public:
	/* 主题是否已经在订阅数组中 */
	bool IsSubscriber(TSTOPICHANDLE Handle);
	/* 主题是否已在发布数组中 */
	bool IsPublisher(TSTOPICHANDLE Handle);

	/* 获取发布的数组 */
    TSTopicHandleArray_T &GetPublish();
	/* 获取订阅的数组 */
    TSTopicHandleArray_T &GetSubscriber();
};

//TSPSMatcher智能指针定义
CLASS_PTR_DECLARE(TSPSMatcher);

#endif



