#ifndef __TSLOCKFREEQUEUE_F051BBA4_26F6_436F_8EF2_07BBD90234A8_H__
#define __TSLOCKFREEQUEUE_F051BBA4_26F6_436F_8EF2_07BBD90234A8_H__
#include <boost/atomic.hpp>

TS_BEGIN_RT_NAMESPACE

struct TSLockfreeNode
{
	boost::atomic<TSLockfreeNode> * Next;
	uintptr_t Version;

	TSLockfreeNode()
		: Next(NULL)
		, Version(0)
	{
	}

	TSLockfreeNode(boost::atomic<TSLockfreeNode>* next, uintptr_t version)
		: Next(next)
		, Version(version)
	{

	}
};

template <class T>
class TSLockfreeQueue
{
public:
	typedef TSLockfreeNode Node;
	typedef boost::atomic<Node> AtomicNode;
	typedef typename AtomicNode T::* NodeMemberPointer;

	TSLockfreeQueue(NodeMemberPointer node)
		: _NodeMember(node)
	{
		_Sentinel = Node(&_Sentinel, PreviousVersion(1));
		_Head = Node(&_Sentinel, 1);
		_Tail = Node(&_Sentinel, 1);
	}

	void Push(const T& element)
	{
		AtomicNode* node = ToNode(element);

		PushNode(node, 0);
	}

	T* Pop()
	{
		AtomicNode* result = PopNode();

		return result ? ToElement(result) : NULL;
	}

private:
	void PushNode(AtomicNode* node, uintptr_t generation)
	{
		Node tail = _Tail.load(boost::memory_order_acquire);

		Node last;
		bool queued = false;

		for (;;)
		{
			if (tail.Next == &_Sentinel)
			{
				last = _Sentinel.load(boost::memory_order_acquire); 
				last.Version = PreviousVersion(tail.Version);

				Node head(&_Sentinel, last.Version);
				Node newHead(last.Next, tail.Version);

				_Head.compare_exchange_strong(head, 
					newHead, 
					boost::memory_order_acq_rel);
			}
			else
			{
				last = Node(&_Tail, tail.Version);
			}

			if (node == &_Sentinel && tail.Version != generation)
			{
				queued = true;
			}

			if (queued)
			{
				break;
			}
			if (node != &_Sentinel)
			{
				node->store(Node(&_Tail, tail.Version), boost::memory_order_relaxed);
			}

			Node newLast(node, tail.Version);

			if (tail.Next->compare_exchange_strong(last, 
				newLast, 
				boost::memory_order_acq_rel))
			{

				last = newLast;
				queued = true;
			}

			Node newTail(last.Next, (last.Next == &_Sentinel) ? 
				NextVersion(tail.Version) : 
			tail.Version);

			if (_Tail.compare_exchange_strong(tail, 
				newTail, 
				boost::memory_order_acq_rel))
			{
				tail = newTail;
			}
		}
	}

	AtomicNode* PopNode()
	{
		AtomicNode* result = 0;

		Node head = _Head.load(boost::memory_order_acquire);

		while (!result)
		{

			Node first = head.Next->load(boost::memory_order_acquire);


			if (head.Next != &_Sentinel)
			{
				Node newHead(first.Next, head.Version);

				if (_Head.compare_exchange_strong(head, 
					newHead, 
					boost::memory_order_acq_rel))
				{
					Node emptyNode(NULL, 0);
					head.Next->store(emptyNode, boost::memory_order_release);

					result = head.Next;
				}
			}
			else
			{

				if (first.Version != head.Version)
				{
					break;
				}

				PushNode(&_Sentinel, head.Version);

				head = _Head.load(boost::memory_order_acquire);
			}
		}

		return result;
	}

	static uintptr_t NextVersion(uintptr_t version)
	{
		return version != UINTPTR_MAX ? version + 1 : uintptr_t(1);
	}

	static uintptr_t PreviousVersion(uintptr_t version)
	{
		return version != 1 ? version - 1 : UINTPTR_MAX;
	}

	T* ToElement(AtomicNode* node) const
	{
		uintptr_t offset = uintptr_t(&(static_cast<T*>(0)->*_NodeMember));

		return reinterpret_cast<T*>(uintptr_t(node) - offset);
	}

	AtomicNode* ToNode(const T& element) const
	{
		return const_cast<AtomicNode*>(&(element.*_NodeMember));
	}

private:
	TS_DECL_ALIGN(64) NodeMemberPointer _NodeMember;
	TS_DECL_ALIGN(64) AtomicNode _Sentinel;
	TS_DECL_ALIGN(64) AtomicNode _Head; 
	TS_DECL_ALIGN(64) AtomicNode _Tail;
};


TS_END_RT_NAMESPACE

#endif // __TSLOCKFREEQUEUE_F051BBA4_26F6_436F_8EF2_07BBD90234A8_H__

