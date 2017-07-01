/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_STREAM_QUEUE_H
#define PREFETCHER_STEMS_STREAM_QUEUE_H

#include <deque>

#include "tms/tms.h"

namespace stems {

class stream_queue: public std::deque<address> {
public:
	stream_queue(); // indicates spatial-only stream

	stream_queue(tms::rmob::index_type origin_rmob_index);

	bool m_new_stream;

	bool m_useful_stream;

	tms::rmob::index_type m_next_rmob_index;

	const bool m_spatial_only_stream;
};

}

#endif
