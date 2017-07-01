/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_SVB_ENTRY_H
#define PREFETCHER_STEMS_SVB_ENTRY_H

#include "stems_types.h"

namespace stems {

/**
 * Streamed Value Buffer (SVB).
 * Holds a reconstruction for a while before actually loading it to avoid polluting the cache in case the prediction was wrong. (I think?)
 */
class svb_entry {
public:
	address m_address;

	size_t m_data_length;

	std::unique_ptr<Byte[]> m_data;

	/**
	 * Reference to the stream queue that this entry was prefetched from.
	 */
	stream_queue_id m_origin;

	svb_entry();

	svb_entry(address address, size_t data_length, stream_queue_id origin);
};

}

#endif
