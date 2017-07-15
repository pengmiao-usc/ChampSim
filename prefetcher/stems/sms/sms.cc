/*
 * Implementation author: Arka Majumdar
 */

#include <cassert>

#include "sms.h"

namespace stems {
namespace sms {

sms::sms(address spatial_region_size, pst::size_type pst_size,
        agt::size_type agt_size, std::map<std::string, stat>& stats) :
        m_pst(pst_size, stats), m_agt(agt_size, *this, stats), m_spatial_region_mask(
                spatial_region_size - 1), m_spatial_region_shift(
                get_spatial_region_shift(spatial_region_size)), m_stats(stats) {
}

void sms::commit_to_pst(spatial_region_tag tag) {
    m_stats["pst-commit-attempts"]++;
    agt::iterator it = m_agt.find(tag); // find the address in the AGT
    if (it != m_agt.end()) {
        generation& entry = it->second;
        // the spatial region exists in the AGT
        if (!entry.m_sequence.empty()) { // make sure it's not just the trigger sequence
            m_stats["pst-commits"]++;
            m_pst[construct_pc_offset_tag(entry.m_trigger_pc,
                    entry.m_trigger_offset)] = std::move(entry.m_sequence); // move the sequence to the PST
        }
        m_agt.erase(it); // remove the entry from the AGT
    }
}

void sms::check_pot(address spatial_region_size) {
    // otherwise the bitwise stuff won't work
    assert(
            (spatial_region_size & (spatial_region_size - 1)) == 0
                    && spatial_region_size != 0);
}

unsigned int sms::get_spatial_region_shift(address spatial_region_size) {
    check_pot(spatial_region_size);
    unsigned int spatial_region_shift = 0;
    while (spatial_region_size >>= 1) {
        spatial_region_shift++;
    }
    return spatial_region_shift;
}

sequence sms::get_sequence(pc pc, address address) const {
    pst::const_iterator it = m_pst.find(
            construct_pc_offset_tag(pc, get_spatial_region_offset(address)));
    if (it != m_pst.end()) {
        return it->second;
    } else {
        return sequence();
    }
}

void sms::inform_eviction(address address) {
    spatial_region_tag tag = get_spatial_region_tag(address);
    spatial_region_offset region_offset = get_spatial_region_offset(address);
    agt::const_iterator it = m_agt.find(tag); // find the address in the AGT
    if (it != m_agt.end()) {
        const generation& entry = it->second;
        if (entry.m_trigger_offset == region_offset) {
            // Also commit if the trigger was evicted.
            commit_to_pst(tag);
            return;
        }
        for (const sequence_element& elem : entry.m_sequence) {
            if (((offset) region_offset) - ((offset) entry.m_trigger_offset)
                    == elem.m_offset) {
                commit_to_pst(tag);
                return;
            }
        }
    }
}

bool sms::is_trigger_access(address address) {
    return m_agt.find(get_spatial_region_tag(address)) == m_agt.end();
}

void sms::inform_access(pc pc, address address, access_count access_count) {
    spatial_region_tag tag = get_spatial_region_tag(address);
    spatial_region_offset region_offset = get_spatial_region_offset(address);
    if (is_trigger_access(address)) {
        // trigger access
        m_agt[tag] = {pc, region_offset, access_count, sequence()};
        // do NOT store the trigger access when used with STeMS
    } else {
        generation& entry = m_agt[tag];
        entry.m_sequence.push_back( {((offset) region_offset)
                    - ((offset) entry.m_trigger_offset), access_count - 1
                    - entry.m_last_count
                });
        entry.m_last_count = access_count;
    }
}

spatial_region_offset sms::get_spatial_region_offset(address address) const {
    // get lower-order bits
    return address & m_spatial_region_mask;
}

spatial_region_tag sms::get_spatial_region_tag(address address) const {
    // get higher-order bits
    return address >> m_spatial_region_shift;
}

pc_offset sms::construct_pc_offset_tag(pc pc,
        spatial_region_offset offset) const {
    return (pc << m_spatial_region_shift) | offset;
}

}
}
