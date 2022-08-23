#ifndef __TSLRUCACHE__H__
#define __TSLRUCACHE__H__
#define TBB_PREVIEW_CONCURRENT_LRU_CACHE 1
#include <tbb/concurrent_lru_cache.h>

TS_BEGIN_RT_NAMESPACE

//TSLRUCache was insteaded by concurrent_lru_cache.
template <typename key_type, typename value_type, typename value_functor_T = value_type (*)(key_type) >
class TSLRUCache : public tbb::concurrent_lru_cache<key_type,value_type,value_functor_T>
{
public:
	TSLRUCache(value_functor_T f, std::size_t number_of_lru_history_items)
		: tbb::concurrent_lru_cache<key_type,value_type,value_functor_T>(f,number_of_lru_history_items)
	{
	
	}
};
TS_END_RT_NAMESPACE


#endif // __TSLRUCACHE__H__

