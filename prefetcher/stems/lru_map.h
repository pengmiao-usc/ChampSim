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
			override {
		m_iterator_map.erase(value.first);
		this->pre_map_eviction(value);
	}

protected:
	virtual void pre_map_eviction(
			const typename lru_map<Key, T>::value_type& value) {
	}

	virtual ~lru_map() {
	}

public:
	lru_map(typename lru_map<Key, T>::size_type max_size) :
			super(max_size), m_iterator_map() {
	}

	template<typename _Key>
	void use(_Key&& key) {
		typename lru_map<Key, T>::iterator it = lru_map<Key, T>::find(key);
		if (it != lru_map<Key, T>::end()) {
			super::use(it);
		}
	}

	typename lru_map<Key, T>::iterator erase(
			typename lru_map<Key, T>::iterator it) {
		lru_map<Key, T>::pre_eviction(*it);
		return super::erase(it);
	}

	template<typename _Key>
	T& operator[](_Key&& key) {
		typename lru_map<Key, T>::iterator it = lru_map<Key, T>::find(key);
		if (it == lru_map<Key, T>::end()) {
			lru_map<Key, T>::push_front(std::make_pair(key, T()));
			it = lru_map<Key, T>::begin();
			m_iterator_map[key] = it;
		}
		super::use(it);
		return it->second;
	}

	typename lru_map<Key, T>::iterator find(const Key& key) {
		if (m_iterator_map.find(key) == m_iterator_map.end()) {
			return lru_map<Key, T>::end();
		}
		return m_iterator_map.at(key);
	}

	typename lru_map<Key, T>::const_iterator find(const Key& key) const {
		if (m_iterator_map.find(key) == m_iterator_map.end()) {
			return lru_map<Key, T>::end();
		}
		return m_iterator_map.at(key);
	}
};

}

#endif
