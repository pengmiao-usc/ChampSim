/*
 * Implementation author: Arka Majumdar
 */

#include "stream_queue.h"

namespace stems {

stream_queue::stream_queue() :
		m_new_stream(true), m_useful_stream(false), m_next_rmob_index(-1), m_spatial_only_stream(
				true) {
}

stream_queue::stream_queue(tms::rmob::index_type origin_rmob_index) :
		m_new_stream(true), m_useful_stream(false), m_next_rmob_index(
				origin_rmob_index), m_spatial_only_stream(false) {
}

}
