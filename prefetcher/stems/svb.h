/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_SVB_H
#define PREFETCHER_STEMS_SVB_H

#include "lru_queue.h"
#include "svb_entry.h"

namespace stems {

class svb: public lru_queue<svb_entry> {
	using super = lru_queue<svb_entry>;

	stats& m_stats;

public:
	svb(size_type size, stats& stats);

	template<typename SVBEntry>
	void push_front(SVBEntry&& value) {
		m_current_lookahead[value.m_origin]++;
		super::push_front(std::forward<SVBEntry>(value));
	}

private:
	std::map<stream_queue_id, size_type> m_current_lookahead;

	void pre_eviction(const svb_entry& value) override;

public:
	const std::map<stream_queue_id, size_type>& m_const_current_lookahead;
};

}

#endif
