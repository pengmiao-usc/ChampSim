/*
 * Implementation author: Arka Majumdar
 */

#include "agt.h"
#include "../sms.h"

namespace stems {
namespace sms {

agt::agt(size_type size, sms& sms, stats& stats) :
        super(size), m_sms(sms), m_stats(stats) {
}

void agt::pre_map_eviction(const value_type& elem) {
    m_stats["agt-evictions"]++;
    m_sms.commit_to_pst(elem.first);
}

}
}
