/*
 * Author: Arka Majumdar
 */

#include "bb.h"
#include "bb_config.h"

using namespace std;

namespace bb {

bb_prefetcher::bb_prefetcher(prefetch_line_func prefetch_line) :
        //
        m_stats(),
        //
        m_ghb(BB_GHB_SIZE),
        //
        m_branch_taken_index_table(),
        //
        m_branch_not_taken_index_table(),
        //
        m_watching_for_trigger(false),
        //
        m_last_branch_taken(true),
        //
        m_current_ghb_entry_index(0),
        //
        m_degree(BB_DEGREE),
        //
        m_prefetch_line(prefetch_line) {
}

void bb_prefetcher::operate(address address, pc pc, bool cache_hit,
        cache_access_type type) {
    m_stats["operations"]++;
    index_table& index_table = index_table_for(m_last_branch_taken);
    if (m_watching_for_trigger) {
        m_stats["triggers"]++;
        // First access after a branch.
        index_table::iterator it = index_table.find(address);
        if (it == index_table.end()) {
            m_stats["new_triggers"]++;
            // New trigger.
            // Add to GHB.
            m_current_ghb_entry_index = m_ghb.append(ghb_entry(address));
            index_table[address] = m_current_ghb_entry_index;
        } else {
            m_stats["hits"]++;
            // Found previously.
            // Start prefetching.
            m_current_ghb_entry_index = it->second;
            bool first = true;
            for (ghb::size_type i = 0; i < m_degree; i++) {
                const ghb_entry& entry = m_ghb[m_current_ghb_entry_index + i];
                if (!entry.m_valid) {
                    continue;
                }
                if (!first) {
                    m_stats["prefetches"]++;
                    // Prefetch the trigger access if it is not the first (which is the trigger we received).
                    m_prefetch_line(pc, address, entry.m_trigger);
                } else {
                    first = false;
                }
                for (delta delta : entry.m_spatial.deltas()) {
                    m_stats["prefetches"]++;
                    m_prefetch_line(pc, address, entry.m_trigger + delta);
                }
            }
        }
    } else {
        m_stats["spatials"]++;
        ghb_entry& entry = m_ghb[m_current_ghb_entry_index];
        if (!entry.m_valid) {
            return;
        }
        delta spatial_delta = ((delta) address) - ((delta) entry.m_trigger);
        m_stats["spatial_delta:" + to_string(spatial_delta)]++;
        if (entry.m_spatial.in_range(spatial_delta)) {
            m_stats["in_spatial_range"]++;
            entry.m_spatial[spatial_delta] = true;
        } else {
            m_stats["out_of_spatial_range"]++;
            // Out of spatial range: make it a temporal trigger but don't list it in the index table.
            m_ghb.append(ghb_entry(address));
        }
    }
}

void bb_prefetcher::inform_branch(bool branch_taken) {
    m_stats["total_branches"]++;
    if (branch_taken) {
        m_stats["total_taken_branches"]++;
    } else {
        m_stats["total_not_taken_branches"]++;
    }
    m_watching_for_trigger = true;
    m_last_branch_taken = branch_taken;
}

bb_prefetcher::index_table& bb_prefetcher::index_table_for(
        bool branch_taken) {
    if (branch_taken) {
        return m_branch_taken_index_table;
    } else {
        return m_branch_not_taken_index_table;
    }
}

const map<string, stat>& bb_prefetcher::stats() const {
    return m_stats;
}

}
