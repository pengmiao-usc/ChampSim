/*
 * Implementation author: Arka Majumdar
 */

#include "pst.h"

namespace stems {
namespace sms {

pst::pst(size_type max_size, stats& stats) :
        super(max_size), m_stats(stats) {
}

void pst::pre_map_eviction(const value_type& elem) {
    m_stats["pst-evictions"]++;
}

}
}
