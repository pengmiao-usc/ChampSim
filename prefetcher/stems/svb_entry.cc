/*
 * Implementation author: Arka Majumdar
 */

#include "svb_entry.h"

namespace stems {

svb_entry::svb_entry() :
		m_address(0), m_data(nullptr), m_data_length(0), m_origin(0) {
}

svb_entry::svb_entry(address address, size_t data_length,
		stream_queue_id origin) :
		m_address(address), m_data_length(data_length), m_data(
				std::unique_ptr<Byte[]>(new Byte[data_length]())), m_origin(
				origin) {
}

}
