/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_LRU_QUEUE_H
#define PREFETCHER_STEMS_LRU_QUEUE_H

#include <list>

namespace stems {

/**
 * Queue that maintains a maximum size limit and evicts elements using the LRU policy.
 */
template<typename T>
class lru_queue: public std::list<T> {
	using super = typename std::list<T>;

public:
	template<typename _T>
	void push_front(_T&& value);

	typename lru_queue<T>::size_type max_size() const;

	/**
	 * Moves an element to the front of the list.
	 *
	 * Does not invalidate any iterators or references.
	 */
	void use(typename lru_queue<T>::iterator value);

	lru_queue(typename lru_queue<T>::size_type max_size);

protected:
	virtual void pre_eviction(const typename lru_queue<T>::value_type& value);

	virtual ~lru_queue();

private:
	typename lru_queue<T>::size_type m_max_size;
};

}

#endif
