/*
 * Implementation author: Arka Majumdar
 */

#include "streaming_engine.h"

namespace stems {

streaming_engine::streaming_engine(size_type num_stream_queues, std::map<std::string, stat>& stats) :
        super(num_stream_queues), m_stream_count(0), m_stats(stats) {
}

streaming_engine::iterator streaming_engine::find(stream_queue_id queue_id) {
    for (iterator it = begin(); it != end(); ++it) {
        if (it->first == queue_id) {
            return it;
        }
    }
    return end();
}

void streaming_engine::pre_eviction(const value_type& value) {
    m_stats["streaming-engine-evictions"]++;
}

}
