#include "stdafx.h"

#include "TSPSMatcher_p.h"
#include "TSPSMatcher.h"

/* 查找并增加主题匹配项计数 */
TopSimDataInterface::ReturnCode FindAndIncrease(TSTopicHandleArray_T& HandleArray,TSTOPICHANDLE THandle)
{
	//HandleArray数组扩容
    if (HandleArray.size() <= THandle)
    {
        HandleArray.grow_to_at_least(THandle + 100);
        HandleArray[THandle].Count++;
    }
    else
    {
		 HandleArray[THandle].Count++;

		 if(HandleArray[THandle].Count > 1)
		 {
			 return TopSimDataInterface::Exist;
		 }
		 else
		 {
			 return TopSimDataInterface::Ok;
		 }
    }

    return TopSimDataInterface::Ok;
}

/* 查找并减少主题匹配器项计数 */
TopSimDataInterface::ReturnCode FindAndDecrease(TSTopicHandleArray_T & HandleArray,TSTOPICHANDLE THandle)
{
	if(HandleArray.size() > THandle)
    {
        if(--HandleArray[THandle].Count <= 0)
        {
            HandleArray[THandle].Count = 0;
            return TopSimDataInterface::Ok;
        }
        else
        {
            return TopSimDataInterface::Exist;
        }
    }
    
	return TopSimDataInterface::Failed;
}

BEGIN_METADATA(TSPSMatcher)
	REG_BASE(TSIPSMatcher);
END_METADATA()

TSPSMatcher::TSPSMatcher(void)
	:_Private_Ptr(new PrivateType)
{
}

TSPSMatcher::TSPSMatcher( PrivateType * p )
	:_Private_Ptr(p)
{

}

TSPSMatcher::TSPSMatcher( const TSPSMatcher& o )
	:_Private_Ptr(new PrivateType)
{
	_Private_Ptr->_Publishers = o._Private_Ptr->_Publishers;
	_Private_Ptr->_Subscribers = o._Private_Ptr->_Subscribers;
}

TSPSMatcher::~TSPSMatcher(void)
{
	delete _Private_Ptr;
}

/* 订阅主题 */
TopSimDataInterface::ReturnCode TSPSMatcher::Subscribe( TSTOPICHANDLE THandle )
{
	TSRWSpinLock::WriteLock lock(_Private_Ptr->_Lock);
	return FindAndIncrease(_Private_Ptr->_Subscribers,THandle);
}

/* 发布主题 */
TopSimDataInterface::ReturnCode TSPSMatcher::Publish(  TSTOPICHANDLE THandle )
{
	TSRWSpinLock::WriteLock lock(_Private_Ptr->_Lock);
	return FindAndIncrease(_Private_Ptr->_Publishers,THandle);
}

/* 是否是主题的订阅者，主题是否已经在订阅数组中 */
bool TSPSMatcher::IsSubscriber( TSTOPICHANDLE THandle )
{
	TSRWSpinLock::ReadLock lock(_Private_Ptr->_Lock);
    if (_Private_Ptr->_Subscribers.size() > THandle)
    {
        if (_Private_Ptr->_Subscribers[THandle])
        {
            return true;
        }
    }

	return  false;
}

/* 是否是主题的发布者，主题是否已在发布数组中 */
bool TSPSMatcher::IsPublisher( TSTOPICHANDLE THandle )
{
	TSRWSpinLock::ReadLock lock(_Private_Ptr->_Lock);
    if (_Private_Ptr->_Publishers.size() > THandle)
    {
        if (_Private_Ptr->_Publishers[THandle])
        {
            return true;
        }
    }

	return false; 
}

/* 取消主题的订阅 */
TopSimDataInterface::ReturnCode TSPSMatcher::Unsubscribe( TSTOPICHANDLE THandle )
{
	TSRWSpinLock::WriteLock lock(_Private_Ptr->_Lock);

	return FindAndDecrease(_Private_Ptr->_Subscribers,THandle);
}

/* 取消主题发布 */
TopSimDataInterface::ReturnCode TSPSMatcher::Unpublish(TSTOPICHANDLE THandle)
{
	TSRWSpinLock::WriteLock lock(_Private_Ptr->_Lock);

	return FindAndDecrease(_Private_Ptr->_Publishers,THandle);
}

/* 获取发布的数组 */
TSTopicHandleArray_T & TSPSMatcher::GetPublish()
{
    TSRWSpinLock::ReadLock lock(_Private_Ptr->_Lock);

    return  _Private_Ptr->_Publishers;
}

/* 获取订阅的数组 */
TSTopicHandleArray_T & TSPSMatcher::GetSubscriber()
{
    TSRWSpinLock::ReadLock lock(_Private_Ptr->_Lock);

    return  _Private_Ptr->_Subscribers;
}

