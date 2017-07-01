/*
 * Implementation author: Arka Majumdar
 */

#include "stems.h"
#include "stems_config.h"

//#define ENABLE_HARDCODED_TESTING

namespace stems {

stems_prefetcher::stems_prefetcher() :
		// Initialize the map of statistics.
		m_stats(),
		// Set the initial access count to 0 because no accesses have been made yet.
		m_access_count(0),
		// Initialize the reconstruction buffer with the size given by "stems_config.h".
		// Also give it access to add/increment statistics.
		m_reconstruction_buffer(STEMS_RECONSTRUCTION_BUFFER_SIZE, m_stats),
		// Initialize the reconstruction buffer with the size given by "stems_config.h".
		// Also give it access to add/increment statistics.
		m_spatial_prefetcher(STEMS_SPATIAL_REGION_SIZE, STEMS_PST_SIZE,
		STEMS_AGT_SIZE, m_stats),
		//
		m_temporal_prefetcher(STEMS_RMOB_SIZE),
		//
		m_streaming_engine(STEMS_STREAMING_ENGINE_NUM_STREAM_QUEUES, m_stats),
		//
		m_reconstruction_resume_threshold(
		STEMS_RECONSTRUCTION_RESUME_THRESHOLD),
		//
		m_svb(STEMS_SVB_SIZE, m_stats),
		//
		m_stream_lookahead(STEMS_SVB_STREAM_LOOKAHEAD) {
#ifdef ENABLE_HARDCODED_TESTING
	address a = 1000, b = 2000, c = 3000, d = 4000;
	/*
	 * Testing example from STeMS paper, Fig. 3.
	 */
	getNextAddress(1, a, false, 0);
	getNextAddress(1, a + 4, false, 0);
	getNextAddress(2, b, false, 0);
	getNextAddress(1, a + 2, false, 0);
	getNextAddress(2, b + 6, false, 0);
	getNextAddress(1, a - 1, false, 0);
	getNextAddress(3, c, false, 0);
	getNextAddress(4, d, false, 0);
	getNextAddress(4, d + 1, false, 0);
	getNextAddress(4, d + 2, false, 0);
	/*
	 * Testing SMS AGT -> PST behavior.
	 */
	inform_eviction(a + 2);
	inform_eviction(b);
	inform_eviction(c + 1);
	inform_eviction(d);
	/*
	 * Testing example from STeMS paper, Fig. 5.
	 */
	getNextAddress(1, a, false, 0);
	/*
	 * Testing spatial-only stream behavior.
	 */
	getNextAddress(1, 488, false,
			0); // 488 has the same spatial-region offset as 1000 (value of a) when the spatial region size is 32.
	/*
	 * Testing access_svb behavior.
	 */
	Byte test_buffer[m_cache_cntlr->getCacheBlockSize()];
	access_svb(Core::READ, 1, a, 0, test_buffer);
	/*
	 * Testing refill behavior on useful streams.
	 */
	getNextAddress(1, a + 4, false, 0);
#endif
#ifdef ENABLE_HARDCODED_TESTING
	std::cout << "Hardcoded tests completed. Exiting..." << std::endl;
	std::exit(0);
#endif
}

// FIXME Port from Sniper.
void stems_prefetcher::operate(address current_address, pc pc, bool cache_hit,
		cache_access_type type) {
	m_stats["getNextAddress-received"]++;
	m_access_count++; // Keep a running count of the current number of accesses in order to calculate deltas.
	if (!cache_hit) {
		m_stats["cache-misses-received"]++;
		/*
		 * STeMS section 4.2 "High Level Operation: Streaming"
		 */
		sms::sequence spatial_sequence = m_spatial_prefetcher.get_sequence(pc,
				current_address);
		if (spatial_sequence.empty()) {
			m_stats["spatial-misses"]++;
		}
		// Look up temporal prefetcher for latest access.
		tms::rmob::index_type temporal_rmob_index =
				m_temporal_prefetcher.get_rmob_index(current_address);
		if (temporal_rmob_index < 0) {
			m_stats["temporal-misses"]++;
			// Temporal miss.
			// Check spatial part for hit as spatial-only stream.
			if (spatial_sequence.empty()) {
				m_stats["uncovered-misses"]++;
			} else {
				// Create new spatial-only stream.
				stream_queue new_queue {/* no rmob index implies spatial-only */};
				m_stats["spatial-only-streams"]++;
				// Spatial-only reconstruction.
				reconstruct(current_address, spatial_sequence);
				move_reconstruction_to_queue(new_queue);
				m_streaming_engine.push_front(std::move(new_queue));
			}
		} else {
			m_stats["normal-streams"]++;
			// Create new, normal, spatio-temporal stream.
			stream_queue new_queue(temporal_rmob_index);
			// Reconstruct.
			reconstruct(new_queue.m_next_rmob_index);
			move_reconstruction_to_queue(new_queue);
			m_streaming_engine.push_front(std::move(new_queue));
		}
		// (Streaming first, training second, otherwise streaming will use just-trained data rather than previous history).
		/*
		 * STeMS section 4.1 "High Level Operation: Training"
		 */
		// If this was a miss, inform the temporal prefetcher.
		// But first, make sure it is a trigger and can't be predicted spatially
		if (spatial_sequence.empty()
				&& m_spatial_prefetcher.is_trigger_access(current_address)) {
			m_stats["temporal-triggers-recorded"]++;
			m_temporal_prefetcher.inform_miss(pc, current_address,
					m_access_count);
		}
	}
	// Always train the spatial prefetcher.
	m_spatial_prefetcher.inform_access(pc, current_address, m_access_count);
	// Start fetching stuff.
	// This loop just tries to maintain the stream lookahead and adds items from new stream queues
	// (as opposed to stems_prefetcher::access_svb, which is where the main SVB handling code is).
	for (std::pair<stream_queue_id, stream_queue>& pair : m_streaming_engine) {
		stream_queue& queue = pair.second;
		svb::size_type num_fetch = 0;
		if (queue.m_new_stream) {
			// Prefetch only one at the beginning of new streams.
			num_fetch = 1;
			queue.m_new_stream = false;
		} else if (queue.m_useful_stream) {
			// Maintain stream lookahead for all streams.
			num_fetch =
					std::min(m_svb.max_size() - m_svb.size(),
							m_stream_lookahead
									- m_svb.m_const_current_lookahead.find(
											pair.first)->second);
			// Should I prefetch more if there is space? Is there a designated single "active stream"?
			// Vague in the STeMS paper.
		}
		for (svb::size_type i = 0; i < num_fetch && !queue.empty(); i++) {
			fetch_to_svb(queue.front(), pair.first, true);
			queue.pop_front();
		}
	}
	// The TMS paper mentions groups of FIFOs per stream queue, but STeMS makes no mention of them, and they don't really make sense in STeMS: should they be implemented?
	// Refill stream queues.
	for (std::pair<stream_queue_id, stream_queue>& pair : m_streaming_engine) {
		stream_queue& queue = pair.second;
		if (!queue.m_spatial_only_stream
				&& queue.size() < m_reconstruction_resume_threshold) {
			// Reconstruct.
			reconstruct(queue.m_next_rmob_index);
			move_reconstruction_to_queue(queue);
		}
	}
}

// This method (in contrast to the second loop in stems_prefetcher::getNextAddress) handles invalidations on writes to the SVB,
// reading from the SVB into buffers, and fetching new blocks into the SVB for useful stream queues.
bool stems_prefetcher::access_svb(Core::mem_op_t mem_op_type, pc pc,
		address address, block_offset offset, Byte* data_buffer) {
	m_stats["access_svb-receieved"]++;
	for (svb::iterator it = m_svb.begin(); it != m_svb.end();) {
		if (it->m_address == address) {
			m_stats["svb-hits"]++;
			switch (mem_op_type) {
			case Core::READ: {
				m_stats["svb-reads"]++;
				// I don't know why sniper gives me a null data_buffer sometimes...
				if (data_buffer) {
					// copy into result buffer
					std::memcpy(data_buffer, it->m_data.get(),
							m_cache_cntlr->getCacheBlockSize());
				}
				// Move block into L1 on SVB hit.
				insert_cache_block(pc, address, it->m_data.get());
				// Remember to capture the origin stream before removing the entry from the SVB.
				streaming_engine::iterator origin_stream_it =
						m_streaming_engine.find(it->m_origin);
				// Unspecified: LRU policy for SVB doesn't even make any sense because blocks are moved out of the SVB immediately upon usage - LRU just becomes a plain queue.
				// Make some space for the (possible) next element from the originating queue.
				// Remove from SVB, destructor will take care of memory because of unique_ptr.
				it = m_svb.erase(it);
				if (origin_stream_it == m_streaming_engine.end()) {
					m_stats["stale-svb-hits"]++;
					// Unspecified: what happens when the stream queue is gone by the time a block that originated from it is an SVB hit?
				} else {
					m_stats["useful-streams"]++;
					stream_queue& queue = origin_stream_it->second;
					if (!queue.empty()) {
						fetch_to_svb(queue.front(), origin_stream_it->first,
								false);
						queue.pop_front();
					}
					queue.m_useful_stream = true;
					m_streaming_engine.use(origin_stream_it);
				}
				return true;
			}
			case Core::READ_EX:
			case Core::WRITE:
				m_stats["svb-invalidates"]++;
				// Invalidate block on write.
				// No "valid bit" in my implementation; just remove the entry.
				it = m_svb.erase(it);
				// Actually do the real write in the cache.
				return false;
			default:
				return false;
			}
		} else {
			++it;
		}
	}
	m_stats["svb-misses"]++;
	return false;
}

void stems_prefetcher::inform_eviction(address address) {
	m_stats["evictions-received"]++;
	// Pass on to the spatial part.
	m_spatial_prefetcher.inform_eviction(address);
}

void stems_prefetcher::fetch_to_svb(address address, stream_queue_id origin,
		bool unlock_cache_cntlr) {
	svb_entry new_entry(address, m_cache_cntlr->getCacheBlockSize(), origin);
	read_dram(address, new_entry.m_data.get(), unlock_cache_cntlr);
	m_svb.push_front(std::move(new_entry));
}

void stems_prefetcher::move_reconstruction_to_queue(stream_queue& queue) {
	for (address& elem : m_reconstruction_buffer) {
		if (elem) { // Don't allow null addresses (which are gaps left by reconstruction).
			queue.push_back(std::move(elem));
		}
	}
}

void stems_prefetcher::reconstruct(tms::rmob::index_type& temporal_rmob_index) {
	m_reconstruction_buffer.clear();
	bool first = true;
	for (reconstruction_buffer::size_type temporal_position = 0;
			temporal_position < m_reconstruction_buffer.size();
			temporal_rmob_index++) {
		const tms::sequence_element& temporal_elem =
				m_temporal_prefetcher.m_const_rmob[temporal_rmob_index];
		if (!temporal_elem.m_valid) {
			break;
		}
		if (!first) {
			// (Ignore delta on first element).
			// Skip as many elements as required by delta.
			temporal_position += temporal_elem.m_delta;
		} else {
			first = false;
		}
		// Attempt to place in desired location.
		m_reconstruction_buffer.place(temporal_position,
				temporal_elem.m_address);
		// This address was predicted temporally.
		m_stats["temporal-predicts"]++;
		// Set the next location to be the one after the one that was just set.
		temporal_position++;
		// Set the first spatial position equal to the position after the temporal address.
		reconstruction_buffer::size_type spatial_position = temporal_position;
		// Go through every entry of the spatial pattern and do a spatial lookup based on the temporally predicted trigger.
		for (const sms::sequence_element& spatial_elem : m_spatial_prefetcher.get_sequence(
				temporal_elem.m_pc, temporal_elem.m_address)) {
			// Skip as many elements as required by delta.
			spatial_position += spatial_elem.m_delta;
			// Attempt to place in desired location.
			m_reconstruction_buffer.place(spatial_position,
					temporal_elem.m_address + spatial_elem.m_offset);
			// Set the next location to be the one after the one that was just set.
			spatial_position++;
			// (This address was predicted spatially).
			m_stats["spatial-predicts"]++;
		}
	}
	m_stats["reconstructions"]++;
}

void stems_prefetcher::reconstruct(address trigger_address,
		const sms::sequence& sequence) {
	m_reconstruction_buffer.clear();
	// Initially set to start of buffer.
	reconstruction_buffer::size_type i = 0;
	for (auto it = sequence.begin(); it != sequence.end(); ++it, ++i) {
		m_reconstruction_buffer.place(i, trigger_address + it->m_offset);
		m_stats["spatial-predicts"]++;
	}
	m_stats["reconstructions"]++;
}

void stems_prefetcher::read_dram(address address, Byte* data_buffer,
		bool unlock_cache_cntlr) {
	// TODO
	m_stats["dram-reads"]++;
}

void stems_prefetcher::insert_cache_block(pc pc, address address,
		Byte* data_buffer) {
	// TODO
	m_stats["cache-block-inserts"]++;
}

}
