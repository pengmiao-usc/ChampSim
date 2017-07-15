/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_TMS_SEQUENCE_ELEMENT_H
#define PREFETCHER_STEMS_TMS_SEQUENCE_ELEMENT_H

#include "../stems_types.h"

namespace stems {
namespace tms {

struct sequence_element {
    pc m_pc;

    address m_address;

    /**
     * Delta for STeMS reconstruction.
     */
    reconstruction_delta m_delta;

    /**
     * This will indicate whether or not this sequence element is actually just empty space in the RMOB, or actually useful data.
     */
    bool m_valid;
};

}
}

#endif
