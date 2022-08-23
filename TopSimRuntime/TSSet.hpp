#ifndef __TSSET_A4FF3AA0_C093_4443_875F_78041B18CAA5_H__
#define __TSSET_A4FF3AA0_C093_4443_875F_78041B18CAA5_H__
#include "TSTree.h"

TS_BEGIN_RT_NAMESPACE

struct TSSetData 
{
	TSSetData() { }
	~TSSetData() { }
};

template <class Key> 
class TSSet : public TSTree<Key,TSSetData> 
{
public:
	TSSet() { }
	~TSSet() { }
	
	FORCEINLINE void append(const Key &key) 
	{
		int change = 0;
		typename TSSet<Key>::Node *parent = 0;
		TSTree<Key,TSSetData>::append_proc(key,TSTree<Key,TSSetData>::root,parent,change);
	}
};

TS_END_RT_NAMESPACE

#endif // __TSSET_A4FF3AA0_C093_4443_875F_78041B18CAA5_H__

