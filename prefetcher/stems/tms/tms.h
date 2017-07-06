/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_TMS_TMS_H
#define PREFETCHER_STEMS_TMS_TMS_H

#include "tms_types.h"

namespace stems {
namespace tms {

/**
 * The temporal, TMS-like, part of STeMS.
 * Embedded inside the STeMS prefetcher because it deviates from the original TMS algorithm and it cannot (correctly) function as a prefetcher on its own.
 */
class tms {
    index_table m_index_table;

    access_count m_last_count;

    rmob m_rmob;

    rmob::index_type m_current_rmob_index;

public:
    tms(rmob::size_type rmob_size);

    const class rmob& m_const_rmob;

    rmob::index_type get_rmob_index(address address) const;

    void inform_miss(pc pc, address address, access_count access_count);
};

}
}

#endif
