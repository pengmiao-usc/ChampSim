/*
 * Author: Arka Majumdar
 */

#ifndef PREFETCHER_BB_GHB_ENTRY_H
#define PREFETCHER_BB_GHB_ENTRY_H

#include "bb_types.h"
#include "bb_spatial_bitvector.h"

namespace bb {

struct ghb_entry {
    ghb_entry();

    ghb_entry(block trigger);

    block m_trigger;

    bb_spatial_bitvector m_spatial;

    bool m_valid;
};

}

#endif
