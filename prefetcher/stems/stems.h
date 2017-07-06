/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_STEMS_H
#define PREFETCHER_STEMS_STEMS_H

#include <vector>

#include "../../inc/block.h"
#include "../../inc/cache.h"

#include "stems_types.h"
#include "lru_queue.h"
#include "reconstruction_buffer.h"
#include "streaming_engine.h"
#include "svb.h"

#include "sms/sms.h"
#include "tms/tms.h"

namespace stems {

class stems_prefetcher {
public:
    /**
     * Initializes the STeMS prefetcher and its parts from the configuration options.
     */
    stems_prefetcher(CACHE* l1d);

    void operate(address address, pc pc, bool cache_hit,
            cache_access_type type);

    void inform_eviction(address address);

    bool access_svb(cache_access_type type, PACKET* packet);

    const std::map<std::string, stat>& stats() const;

private:
    CACHE* m_l1d;

    std::map<std::string, stat> m_stats;

    access_count m_access_count;

    reconstruction_buffer m_reconstruction_buffer;

    sms::sms m_spatial_prefetcher;

    tms::tms m_temporal_prefetcher;

    streaming_engine m_streaming_engine;

    stream_queue::size_type m_reconstruction_resume_threshold;

    svb m_svb;

    void fetch_to_svb(address address, stream_queue_id origin,
            bool unlock_cache_cntlr);

    void move_reconstruction_to_queue(stream_queue& queue);

    /**
     * To quote the TMS paper: "a constant number of blocks outstanding in the SVB for each active stream"
     */
    svb::size_type m_stream_lookahead;

    /**
     * The core of the paper: reconstruction.
     * This will add on a reconstructed stream of values to the reconstruction buffer given the temporally prefetched spatial triggers. It will also conduct spatial lookups for these triggers and interleave them with the overall sequence.
     *
     * (See Figure 3 and 5 from the STeMS paper).
     *
     * @param temporal_rmob_index Passed by reference, is advanced to the position after the last trigger fetched.
     */
    void reconstruct(tms::rmob::index_type& temporal_rmob_index);

    /**
     * Spatial-only "reconstruction".
     */
    void reconstruct(address trigger_address, const sms::sequence& sequence);

    PACKET* read_dram(address address);
};

}

#endif
