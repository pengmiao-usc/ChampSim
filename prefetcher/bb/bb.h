/*
 * Author: Arka Majumdar
 */

#ifndef PREFETCHER_BB_BB_H
#define PREFETCHER_BB_BB_H

#include <map>
#include <unordered_map>

#include "bb_types.h"
#include "ghb.h"

namespace bb {

class bb_prefetcher {
    using prefetch_line_func = std::function<bool(pc pc, address base_address, block prefetch_block)>;

public:
    bb_prefetcher(prefetch_line_func prefetch_line);

    void operate(address address, pc pc, bool cache_hit,
            cache_access_type type);

    void inform_branch(bool branch_taken);

    const std::map<std::string, stat>& stats() const;

private:
    std::map<std::string, stat> m_stats;

    ghb m_ghb;

    using index_table = std::unordered_map<block, ghb::size_type>;

    index_table m_branch_taken_index_table, m_branch_not_taken_index_table;

    bool m_watching_for_trigger;

    bool m_last_branch_taken;

    ghb::size_type m_current_ghb_entry_index;

    ghb::size_type m_degree;

    ghb::size_type m_skip_degree;

    prefetch_line_func m_prefetch_line;

    index_table& index_table_for(bool branch_taken);
};

}

#endif
