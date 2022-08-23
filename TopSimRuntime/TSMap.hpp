#ifndef __TSMAP_579B44DA_95FC_433C_B845_BE067D4B30EE_H__
#define __TSMAP_579B44DA_95FC_433C_B845_BE067D4B30EE_H__
#include "TSTree.h"

TS_BEGIN_RT_NAMESPACE

template <class Type> 
class TSMapData 
{
public:
	TSMapData() 
	{

	}

	~TSMapData() 
	{

	}

	Type data;
};


template <class Key,class Type> 
class TSMap : public TSTree<Key,TSMapData<Type> > 
{
public:
	TSMap() { }
	~TSMap() { }

	typename TSMap<Key,Type>::Iterator findData(const Type &t) const 
	{
		typename TSMap<Key,Type>::Iterator end_it = TSTree<Key,TSMapData<Type> >::end();
		for(typename TSMap<Key,Type>::Iterator it = TSTree<Key,TSMapData<Type> >::begin(); it != end_it; ++it) 
		{
			if(it->data == t) return it;
		}
		return end_it;
	}

	FORCEINLINE Type &operator[](const Key &key) 
	{
		int change = 0;
		typename TSMap<Key,Type>::Node *parent = 0;
		typename TSMap<Key,Type>::Node *node = TSTree<Key,TSMapData<Type> >::append_proc(key,TSTree<Key,TSMapData<Type> >::root,parent,change);
		return node->data;
	}

	FORCEINLINE Type &get(const Key &key) 
	{
		int change = 0;
		typename TSMap<Key,Type>::Node *parent = 0;
		typename TSMap<Key,Type>::Node *node = TSTree<Key,TSMapData<Type> >::append_proc(key,TSTree<Key,TSMapData<Type> >::root,parent,change);
		return node->data;
	}

	FORCEINLINE Type &append(const Key &key) 
	{
		int change = 0;
		typename TSMap<Key,Type>::Node *parent = 0;
		typename TSMap<Key,Type>::Node *node = TSTree<Key,TSMapData<Type> >::append_proc(key,TSTree<Key,TSMapData<Type> >::root,parent,change);
		return node->data;
	}
	FORCEINLINE void append(const Key &key,const Type &t) 
	{
		int change = 0;
		typename TSMap<Key,Type>::Node *parent = 0;
		typename TSMap<Key,Type>::Node *node = TSTree<Key,TSMapData<Type> >::append_proc(key,TSTree<Key,TSMapData<Type> >::root,parent,change);
		node->data = t;
	}
};

TS_END_RT_NAMESPACE

#endif // __TSMAP_579B44DA_95FC_433C_B845_BE067D4B30EE_H__
