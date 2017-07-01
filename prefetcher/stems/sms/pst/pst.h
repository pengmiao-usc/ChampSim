/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_SMS_PST
#define PREFETCHER_STEMS_SMS_PST

#include "../../lru_map.h"

#include "../sms_types.h"
#include "../sequence.h"

namespace stems {
namespace sms {

/**
 * The Pattern Sequence Table.
 */
class pst: public lru_map<pc_offset, sequence> {
	using super = lru_map<pc_offset, sequence>;

	stats& m_stats;

	void pre_map_eviction(const value_type& elem) override;

public:
	pst(size_type max_size, stats& stats);
};

}
}

#endif
