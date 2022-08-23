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
	/* �������� */
	TopSimDataInterface::ReturnCode Subscribe(TSTOPICHANDLE Handle);
	/* �������� */
	TopSimDataInterface::ReturnCode Publish(TSTOPICHANDLE Handle);
	/* ȡ������Ķ��� */
	TopSimDataInterface::ReturnCode Unsubscribe(TSTOPICHANDLE Handle);
	/* ȡ�����ⷢ�� */
	TopSimDataInterface::ReturnCode Unpublish(TSTOPICHANDLE Handle);

public:
	/* �����Ƿ��Ѿ��ڶ��������� */
	bool IsSubscriber(TSTOPICHANDLE Handle);
	/* �����Ƿ����ڷ��������� */
	bool IsPublisher(TSTOPICHANDLE Handle);

	/* ��ȡ���������� */
    TSTopicHandleArray_T &GetPublish();
	/* ��ȡ���ĵ����� */
    TSTopicHandleArray_T &GetSubscriber();
};

//TSPSMatcher����ָ�붨��
CLASS_PTR_DECLARE(TSPSMatcher);

#endif



