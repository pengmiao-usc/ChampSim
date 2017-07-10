/*
 * Author: Arka Majumdar
 */

#ifndef PREFETCHER_BB_SPATIAL_BITVECTOR_H
#define PREFETCHER_BB_SPATIAL_BITVECTOR_H

#include <array>
#include <stdexcept>
#include <cassert>
#include <vector>
#include <cstdint>

namespace bb {

template<uint16_t NumBefore, uint16_t NumAfter>
class spatial_bitvector {
    std::array<bool, NumBefore> m_before;

    std::array<bool, NumAfter> m_after;

public:
    using reference = bool&;

    using const_reference = const bool&;

    using index_type = int32_t;

    spatial_bitvector() {
        m_before.fill(false);
        m_after.fill(false);
    }

    bool in_range(index_type pos) const {
        return pos >= -((index_type) NumBefore)
                && pos <= ((index_type) NumAfter) && pos != 0;
    }

    const_reference operator[](index_type pos) const {
        if (!in_range(pos)) {
            throw std::out_of_range("\"pos\" is out of range.");
        }
        if (pos < 0) {
            return m_before[-pos - 1];
        } else if (pos > 0) {
            return m_after[pos - 1];
        } else {
            throw std::out_of_range("\"pos\" cannot be 0.");
        }
    }

    reference operator[](index_type pos) {
        return const_cast<reference>(static_cast<const spatial_bitvector&>(*this).operator[](
                pos));
    }

    std::vector<index_type> deltas() const {
        std::vector<index_type> return_value;
        for (index_type i = -((index_type) NumBefore);
                i <= ((index_type) NumAfter); i++) {
            if (i == 0) {
                continue;
            }
            if (operator[](i)) {
                return_value.push_back(i);
            }
        }
        return return_value;
    }
};

}

#endif
