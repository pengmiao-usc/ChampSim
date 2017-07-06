/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_RECONSTRUCTION_BUFFER_H
#define PREFETCHER_STEMS_RECONSTRUCTION_BUFFER_H

#include <vector>

#include "stems_types.h"

namespace stems {

/**
 * STeMS reconstruction buffer.
 */
class reconstruction_buffer: public std::vector<address> {
    using super = std::vector<address>;

    stats& m_stats;

public:
    reconstruction_buffer(size_type size, stats& stats);

    void clear();

    bool place(size_type index, address value);
};

}

#endif
