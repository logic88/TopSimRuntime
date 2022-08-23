#ifndef __TSITOPICDESCRIPTION__H__
#define __TSITOPICDESCRIPTION__H__

#include "TSTopicTypes.h"

//TopicHelper私有成员结构前置声明
struct TSTopicHelperPrivate;

/*
	TopicHelper类，主题继承关系
*/
class TOPSIMDATAINTERFACE_DECL TSTopicHelper 
	: public boost::intrusive_ref_counter<TSTopicHelper>
{
	/* TS_DECLARE_PRIVATE(TSTopicHelper)展开
	public:
		typedef TSTopicHelper NoCopyableType;
	private:
		//构造函数
		TSTopicHelper(const TSTopicHelper&);
		//赋值函数
		const TSTopicHelper& operator=(const TSTopicHelper&)

	protected:
		friend struct TSTopicHelperPrivate;
		inline TSTopicHelperPrivate* GetPP() const { return reinterpret_cast<TSTopicHelperPrivate *>(_Private_Ptr); }
	protected:
		typedef TSTopicHelperPrivate PrivateType;
	*/

	//成为友元，TSTopicTypeManager可以随意访问该对象成员
	friend class TSTopicTypeManager;


public:
	TSTopicHelper(void);
	virtual ~TSTopicHelper(void);

public:
	virtual bool CanConvert(TSTOPICHANDLE From);
	static  bool CanConvertStatic(TSTOPICHANDLE From);
public:
	/* 获取子主题的数组 */
	FORCEINLINE std::vector<TSTOPICHANDLE> &GetTopicInherits()
	{
		return InheritTopics;
	}

	/* 获取父主题 */
	FORCEINLINE const std::vector<TSTOPICHANDLE>& GetTopicParents()
	{
		return ParentTopics;
	}

private:
	/* 添加子主题 */
    void AddChild(TSTOPICHANDLE Child);
	/* 添加父主题 */
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


