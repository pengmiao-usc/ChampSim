/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_SMS_SMS_H
#define PREFETCHER_STEMS_SMS_SMS_H

#include "../lru_map.h"

#include "sms_types.h"

#include "agt/agt.h"
#include "pst/pst.h"

namespace stems {
namespace sms {

/**
 * The spatial SMS-like part of STeMS.
 * Embedded inside the STeMS prefetcher because it deviates from the original SMS algorithm and it cannot (correctly) function as a prefetcher on its own.
 */
class sms {
    pst m_pst;

    agt m_agt;

    void commit_to_pst(spatial_region_tag tag);

    friend class agt;

    /**
     * The mask to get only the spatial region offset.
     */
    const address m_spatial_region_mask;

    /**
     * How many bits wide the spatial region offset is.
     */
    const unsigned int m_spatial_region_shift;

    /**
     * Get the low order bits of the address.
     */
    inline spatial_region_offset get_spatial_region_offset(
            address address) const;

    /**
     * Get the high-order bits of the address.
     */
    inline spatial_region_tag get_spatial_region_tag(address address) const;

    /**
     * Shift the PC to the high order bits of the address and use the low order bits to store the spatial region offset.
     */
    inline pc_offset construct_pc_offset_tag(pc pc,
            spatial_region_offset offset) const;

    static inline void check_pot(address spatial_region_size);

    static inline unsigned int get_spatial_region_shift(
            address spatial_region_size);

    stats& m_stats;

public:
    sms(address sptial_region_size, pst::size_type pst_size,
            agt::size_type agt_size, stats& stats);

    sequence get_sequence(pc pc, address address) const;

    void inform_eviction(address address);

    void inform_access(pc pc, address address, access_count access_count);

    bool is_trigger_access(address address);
};

}
}

#endif
