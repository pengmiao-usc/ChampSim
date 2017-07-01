/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_LRU_MAP_H
#define PREFETCHER_STEMS_LRU_MAP_H

#include <unordered_map>

#include "lru_queue.h"

namespace stems {

template<typename Key, typename T>
class lru_map: public lru_queue<std::pair<Key, T>> {
	using super = lru_queue<std::pair<Key, T>>;

	std::unordered_map<Key, typename lru_map<Key, T>::iterator> m_iterator_map;

	void pre_eviction(const typename lru_map<Key, T>::value_type& value)
			override;

protected:
	virtual void pre_map_eviction(
			const typename lru_map<Key, T>::value_type& value);

	virtual ~lru_map();

public:
	lru_map(typename lru_map<Key, T>::size_type max_size);

	template<typename _Key>
	void use(_Key&& key);

	typename lru_map<Key, T>::iterator erase(
			typename lru_map<Key, T>::iterator it);

	template<typename _Key>
	T& operator[](_Key&& key);

	typename lru_map<Key, T>::iterator find(const Key& key);

	typename lru_map<Key, T>::const_iterator find(const Key& key) const;
};

}

#endif
