/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_SMS_AGT_H
#define PREFETCHER_STEMS_SMS_AGT_H

#include "../../lru_map.h"

#include "../sms_types.h"

#include "generation.h"

namespace stems {
namespace sms {

class sms;

/**
 * The Active Generation Table.
 */
class agt: public lru_map<spatial_region_tag, generation> {
    using super = lru_map<spatial_region_tag, generation>;

    sms& m_sms;

    void pre_map_eviction(const value_type& elem) override;

    std::map<std::string, stat>& m_stats;

public:
    agt(size_type max_size, sms& sms, std::map<std::string, stat>& stats);

    void inform_eviction(spatial_region_tag tag);
};

}
}

#endif
