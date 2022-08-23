#ifndef __TSITOPICDESCRIPTION__H__
#define __TSITOPICDESCRIPTION__H__

#include "TSTopicTypes.h"

//TopicHelper˽�г�Ա�ṹǰ������
struct TSTopicHelperPrivate;

/*
	TopicHelper�࣬����̳й�ϵ
*/
class TOPSIMDATAINTERFACE_DECL TSTopicHelper 
	: public boost::intrusive_ref_counter<TSTopicHelper>
{
	/* TS_DECLARE_PRIVATE(TSTopicHelper)չ��
	public:
		typedef TSTopicHelper NoCopyableType;
	private:
		//���캯��
		TSTopicHelper(const TSTopicHelper&);
		//��ֵ����
		const TSTopicHelper& operator=(const TSTopicHelper&)

	protected:
		friend struct TSTopicHelperPrivate;
		inline TSTopicHelperPrivate* GetPP() const { return reinterpret_cast<TSTopicHelperPrivate *>(_Private_Ptr); }
	protected:
		typedef TSTopicHelperPrivate PrivateType;
	*/

	//��Ϊ��Ԫ��TSTopicTypeManager����������ʸö����Ա
	friend class TSTopicTypeManager;


public:
	TSTopicHelper(void);
	virtual ~TSTopicHelper(void);

public:
	virtual bool CanConvert(TSTOPICHANDLE From);
	static  bool CanConvertStatic(TSTOPICHANDLE From);
public:
	/* ��ȡ����������� */
	FORCEINLINE std::vector<TSTOPICHANDLE> &GetTopicInherits()
	{
		return InheritTopics;
	}

	/* ��ȡ������ */
	FORCEINLINE const std::vector<TSTOPICHANDLE>& GetTopicParents()
	{
		return ParentTopics;
	}

private:
	/* ��������� */
    void AddChild(TSTOPICHANDLE Child);
	/* ��Ӹ����� */
	void AddParent(TSTOPICHANDLE Parent);

private:
	std::vector<TSTOPICHANDLE>  InheritTopics;
	std::vector<TSTOPICHANDLE>  ParentTopics;
};


typedef TSTopicHelper TOPIC_BaseTopicHelper;


template<typename HelperType>
struct TSTopicConvertor
{
    FORCEINLINE static bool CanConvertT(TSTOPICHANDLE To)
    {
        return false;
    }
};
#endif // __TSITOPICDESCRIPTION__H__


