/*
 * Author: Arka Majumdar
 */

#ifndef PREFETCHER_BB_GHB_H
#define PREFETCHER_BB_GHB_H

#include <vector>
#include <unordered_map>

#include "bb_types.h"
#include "ghb_entry.h"

namespace bb {

class ghb: public std::vector<ghb_entry> {
    using super = std::vector<ghb_entry>;

public:
    ghb(size_type n);

    reference operator[](size_type pos);

    const_reference operator[](size_type pos) const;

    template<typename GHBEntry>
    size_type append(GHBEntry&& entry);

private:
    size_type m_counter;
};

}

#endif
