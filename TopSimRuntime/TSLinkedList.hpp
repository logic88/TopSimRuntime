#ifndef __TSLINKEDLIST_BF50F96E_77E8_4B75_95FC_EF916ABDAC6C_H__
#define __TSLINKEDLIST_BF50F96E_77E8_4B75_95FC_EF916ABDAC6C_H__

TS_BEGIN_RT_NAMESPACE

template <typename T>
class TSDoublyLinkedListNode 
{
public:
	TSDoublyLinkedListNode();

	void SetPrev(T*);
	void SetNext(T*);

	T* Prev() const;
	T* Next() const;
protected:
	T * _Prev;
	T * _Next;
};

template <typename T>
FORCEINLINE TSDoublyLinkedListNode<T>::TSDoublyLinkedListNode() 
{
	SetPrev(NULL);
	SetNext(NULL);
}

template <typename T>
FORCEINLINE void TSDoublyLinkedListNode<T>::SetPrev(T* prev) 
{
	static_cast<T*>(this)->_Prev = prev;
}

template <typename T>
FORCEINLINE void TSDoublyLinkedListNode<T>::SetNext(T* next) 
{
	static_cast<T*>(this)->_Next = next;
}

template <typename T>
FORCEINLINE T* TSDoublyLinkedListNode<T>::Prev() const 
{
	return static_cast<const T*>(this)->_Prev;
}

template <typename T>
FORCEINLINE T* TSDoublyLinkedListNode<T>::Next() const 
{
	return static_cast<const T*>(this)->_Next;
}

template <typename T, typename PointerType = T*>
class TSDoublyLinkedList 
{
public:
	TSDoublyLinkedList();

	bool IsEmpty() const;
	std::size_t Size() const; 
	void Clear();

	T* Head() const;
	T* RemoveHead();

	T* Tail() const;

	void Push(T*);
	void Append(T*);
	void Remove(T*);

protected:
	PointerType _Head;
	PointerType _Tail;
	std::size_t _Size;
};

template <typename T, typename PointerType>
FORCEINLINE TSDoublyLinkedList<T, PointerType>::TSDoublyLinkedList()
	: _Head(NULL), _Tail(NULL),_Size(0)
{
}

template <typename T, typename PointerType>
FORCEINLINE bool TSDoublyLinkedList<T, PointerType>::IsEmpty() const 
{
	return !_Head;
}

template <typename T, typename PointerType>
FORCEINLINE std::size_t TSDoublyLinkedList<T, PointerType>::Size() const 
{
	return _Size;
}

template <typename T, typename PointerType>
FORCEINLINE void TSDoublyLinkedList<T, PointerType>::Clear() 
{
	_Head = NULL;
	_Tail = NULL;
	_Size = 0;
}

template <typename T, typename PointerType>
FORCEINLINE T* TSDoublyLinkedList<T, PointerType>::Head() const 
{
	return _Head;
}

template <typename T, typename PointerType>
FORCEINLINE T* TSDoublyLinkedList<T, PointerType>::Tail() const 
{
	return _Tail;
}

template <typename T, typename PointerType>
FORCEINLINE void TSDoublyLinkedList<T, PointerType>::Push(T* node) 
{
	++_Size;

	if (!_Head) 
	{
		_Head = node;
		_Tail = node;
		node->SetPrev(NULL);
		node->SetNext(NULL);
		return;
	}
	_Head->SetPrev(node);
	node->SetNext(_Head);
	node->SetPrev(NULL);
	_Head = node;
}

template <typename T, typename PointerType>
FORCEINLINE void TSDoublyLinkedList<T, PointerType>::Append(T* node) 
{
	++_Size;

	if (!_Tail) 
	{
		_Head = node;
		_Tail = node;
		node->SetPrev(NULL);
		node->SetNext(NULL);
		return;
	}
	_Tail->SetNext(node);
	node->SetPrev(_Tail);
	node->SetNext(NULL);
	_Tail = node;
}

template <typename T, typename PointerType>
FORCEINLINE void TSDoublyLinkedList<T, PointerType>::Remove(T* node) 
{
	if (node->Prev()) 
	{
		node->Prev()->SetNext(node->Next());
	} 
	else 
	{
		_Head = node->Next();
	}

	if (node->Next()) 
	{
		node->Next()->SetPrev(node->Prev());

		node->SetNext(NULL);
	} 
	else 
	{
		_Tail = node->Prev();
	}

	node->SetPrev(NULL);

	--_Size;
}

template <typename T, typename PointerType>
FORCEINLINE T* TSDoublyLinkedList<T, PointerType>::RemoveHead() 
{
	T* node = Head();
	if (node)
	{
		Remove(node);
	}

	return node;
}

TS_END_RT_NAMESPACE

#endif // __TSLINKEDLIST_BF50F96E_77E8_4B75_95FC_EF916ABDAC6C_H__

