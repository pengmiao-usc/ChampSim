/*
 * Implementation author: Arka Majumdar
 */

#include "svb.h"

namespace stems {

svb::svb(size_type size, stats& stats) :
		super(size), m_stats(stats), m_current_lookahead(), m_const_current_lookahead(
				m_current_lookahead) {
}

template<typename SVBEntry>
void svb::push_front(SVBEntry&& value) {
	m_current_lookahead[value.m_origin]++;
	super::push_front(std::forward<SVBEntry>(value));
}

void svb::pre_eviction(const svb_entry& value) {
	m_current_lookahead[value.m_origin]--;
	m_stats["svb-evictions"]++;
}

}
