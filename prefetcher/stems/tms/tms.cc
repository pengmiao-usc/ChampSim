/*
 * Implementation author: Arka Majumdar
 */

#include "tms.h"

namespace stems {
namespace tms {

tms::tms(rmob::size_type rmob_size) :
        m_index_table(), m_last_count(0), m_rmob(rmob_size), m_current_rmob_index(
                -1), m_const_rmob(m_rmob) {
}

rmob::index_type tms::get_rmob_index(address address) const {
    index_table::const_iterator it = m_index_table.find(address);
    if (it != m_index_table.end()) {
        rmob::index_type rmob_index = it->second;
        if (m_rmob[rmob_index].m_address == address) {
            return rmob_index;
        }
    }
    return -1;
}

void tms::inform_miss(pc pc, address address, access_count access_count) {
    m_rmob[++m_current_rmob_index] = {pc, address, access_count - 1 - m_last_count, true};
    m_index_table[address] = m_current_rmob_index;
    m_last_count = access_count;
}

}
}
