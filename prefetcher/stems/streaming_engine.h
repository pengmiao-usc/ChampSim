/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_STREAMING_ENGINE_H
#define PREFETCHER_STEMS_STREAMING_ENGINE_H

#include "stems_types.h"
#include "lru_queue.h"
#include "stream_queue.h"

namespace stems {

/**
 * STeMS manages multiple stream queues in a method similar to the TSE (Temporal Streaming Engine) from TMS.
 */
class streaming_engine: public lru_queue<
        std::pair<stream_queue_id, stream_queue>> {
    using super = lru_queue<std::pair<stream_queue_id, stream_queue>>;

    stream_queue_id m_stream_count;

    std::map<std::string, stat>& m_stats;

    void pre_eviction(const value_type& value) override;

public:
    streaming_engine(size_type num_stream_queues, std::map<std::string, stat>& stats);

    iterator find(stream_queue_id queue_id);

    template<typename StreamQueue>
    void push_front(StreamQueue&& new_queue) {
        super::push_front(
                std::pair<stream_queue_id, stream_queue>(++m_stream_count,
                        std::forward < StreamQueue > (new_queue)));
    }
};

}

#endif
