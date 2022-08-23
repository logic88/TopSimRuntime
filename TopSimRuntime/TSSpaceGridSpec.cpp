#include "stdafx.h"

#include "TSPSMatcher_p.h"
#include "TSPSMatcher.h"

/* ���Ҳ���������ƥ������� */
TopSimDataInterface::ReturnCode FindAndIncrease(TSTopicHandleArray_T& HandleArray,TSTOPICHANDLE THandle)
{
	//HandleArray��������
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

/* ���Ҳ���������ƥ��������� */
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

/* �������� */
TopSimDataInterface::ReturnCode TSPSMatcher::Subscribe( TSTOPICHANDLE THandle )
{
	TSRWSpinLock::WriteLock lock(_Private_Ptr->_Lock);
	return FindAndIncrease(_Private_Ptr->_Subscribers,THandle);
}

/* �������� */
TopSimDataInterface::ReturnCode TSPSMatcher::Publish(  TSTOPICHANDLE THandle )
{
	TSRWSpinLock::WriteLock lock(_Private_Ptr->_Lock);
	return FindAndIncrease(_Private_Ptr->_Publishers,THandle);
}

/* �Ƿ�������Ķ����ߣ������Ƿ��Ѿ��ڶ��������� */
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

/* �Ƿ�������ķ����ߣ������Ƿ����ڷ��������� */
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

/* ȡ������Ķ��� */
TopSimDataInterface::ReturnCode TSPSMatcher::Unsubscribe( TSTOPICHANDLE THandle )
{
	TSRWSpinLock::WriteLock lock(_Private_Ptr->_Lock);

	return FindAndDecrease(_Private_Ptr->_Subscribers,THandle);
}

/* ȡ�����ⷢ�� */
TopSimDataInterface::ReturnCode TSPSMatcher::Unpublish(TSTOPICHANDLE THandle)
{
	TSRWSpinLock::WriteLock lock(_Private_Ptr->_Lock);

	return FindAndDecrease(_Private_Ptr->_Publishers,THandle);
}

/* ��ȡ���������� */
TSTopicHandleArray_T & TSPSMatcher::GetPublish()
{
    TSRWSpinLock::ReadLock lock(_Private_Ptr->_Lock);

    return  _Private_Ptr->_Publishers;
}

/* ��ȡ���ĵ����� */
TSTopicHandleArray_T & TSPSMatcher::GetSubscriber()
{
    TSRWSpinLock::ReadLock lock(_Private_Ptr->_Lock);

    return  _Private_Ptr->_Subscribers;
}

