/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_SMS_GENERATION_H
#define PREFETCHER_STEMS_SMS_GENERATION_H

#include "../sms_types.h"
#include "../sequence.h"

namespace stems{
namespace sms {

struct generation {
	pc m_trigger_pc;

	spatial_region_offset m_trigger_offset;

	access_count m_last_count;

	sequence m_sequence;
};

}
}

#endif
