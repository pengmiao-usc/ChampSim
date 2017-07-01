/*
 * Implementation author: Arka Majumdar
 */

#include "lru_queue.h"

namespace stems {

template<typename T>
void lru_queue<T>::use(
		typename lru_queue<T>::iterator element) {
	if (element != lru_queue<T>::begin()) {
		lru_queue<T>::splice(lru_queue<T>::begin(), *this, element);
	}
}

template<typename T>
void lru_queue<T>::pre_eviction(
		const typename lru_queue<T>::value_type& value) {
}

template<typename T>
template<typename _T>
void lru_queue<T>::push_front(_T&& value) {
	if (m_max_size == 0) { // never do anything
		return;
	}
	while (lru_queue<T>::size() >= m_max_size) {
		this->pre_eviction(lru_queue<T>::back()); // call to specialized virtual method
		lru_queue<T>::pop_back();
	}
	super::push_front(std::forward<_T>(value));
}

template<typename T>
typename lru_queue<T>::size_type lru_queue<T>::max_size() const {
	return m_max_size;
}

template<typename T>
lru_queue<T>::lru_queue(
		typename lru_queue<T>::size_type max_size) :
		m_max_size(max_size) {
}

}
