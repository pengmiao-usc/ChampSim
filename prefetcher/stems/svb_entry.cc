/*
 * Implementation author: Arka Majumdar
 */

#include "svb_entry.h"

namespace stems {

svb_entry::svb_entry(PACKET* packet, stream_queue_id origin) :
		m_packet(packet), m_origin(origin) {
}

}
