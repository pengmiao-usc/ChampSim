/*
 * Implementation author: Arka Majumdar
 */

#include "lru_map.h"

namespace stems {

template<typename Key, typename T>
lru_map<Key, T>::lru_map(typename lru_map<Key, T>::size_type max_size) :
		super(max_size), m_iterator_map() {
}

template<typename Key, typename T>
template<typename _Key>
void lru_map<Key, T>::use(_Key&& key) {
	typename lru_map<Key, T>::iterator it = lru_map<Key, T>::find(key);
	if (it != lru_map<Key, T>::end()) {
		super::use(it);
	}
}

template<typename Key, typename T>
template<typename _Key>
T& lru_map<Key, T>::lru_map::operator[](_Key&& key) {
	typename lru_map<Key, T>::iterator it = lru_map<Key, T>::find(key);
	if (it == lru_map<Key, T>::end()) {
		lru_map<Key, T>::push_front(std::make_pair(key, T()));
		it = lru_map<Key, T>::begin();
		m_iterator_map[key] = it;
	}
	super::use(it);
	return it->second;
}

template<typename Key, typename T>
typename lru_map<Key, T>::iterator lru_map<Key, T>::find(const Key& key) {
	if (m_iterator_map.find(key) == m_iterator_map.end()) {
		return lru_map<Key, T>::end();
	}
	return m_iterator_map.at(key);
}

template<typename Key, typename T>
typename lru_map<Key, T>::const_iterator lru_map<Key, T>::find(
		const Key& key) const {
	if (m_iterator_map.find(key) == m_iterator_map.end()) {
		return lru_map<Key, T>::end();
	}
	return m_iterator_map.at(key);
}

template<typename Key, typename T>
typename lru_map<Key, T>::iterator lru_map<Key, T>::erase(
		typename lru_map<Key, T>::iterator it) {
	lru_map<Key, T>::pre_eviction(*it);
	return super::erase(it);
}

template<typename Key, typename T>
void lru_map<Key, T>::pre_map_eviction(
		const typename lru_map<Key, T>::value_type& value) {
}

template<typename Key, typename T>
void lru_map<Key, T>::pre_eviction(
		const typename lru_map<Key, T>::value_type& value) {
	m_iterator_map.erase(value.first);
	this->pre_map_eviction(value);
}

}
