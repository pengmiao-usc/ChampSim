/*
 * Implementation author: Arka Majumdar
 */

#include "streaming_engine.h"

namespace stems {

streaming_engine::streaming_engine(size_type num_stream_queues, stats& stats) :
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

template<typename StreamQueue>
void streaming_engine::push_front(StreamQueue&& new_queue) {
	super::push_front(
			std::pair<stream_queue_id, stream_queue>(++m_stream_count,
					std::forward<StreamQueue>(new_queue)));
}

void streaming_engine::pre_eviction(const value_type& value) {
	m_stats["streaming-engine-evictions"]++;
}

}
