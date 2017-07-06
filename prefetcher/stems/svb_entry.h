/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_SVB_ENTRY_H
#define PREFETCHER_STEMS_SVB_ENTRY_H

#include <memory>

#include "../../inc/block.h"

#include "stems_types.h"

namespace stems {

/**
 * Streamed Value Buffer (SVB).
 * Holds a reconstruction for a while before actually loading it to avoid polluting the cache in case the prediction was wrong. (I think?)
 */
class svb_entry {
public:
	unique_ptr<PACKET> m_packet;

	/**
	 * Reference to the stream queue that this entry was prefetched from.
	 */
	stream_queue_id m_origin;

	svb_entry(PACKET* packet, stream_queue_id origin);
};

}

#endif
