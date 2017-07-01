/*
 * Implementation author: Arka Majumdar
 */

#include "reconstruction_buffer.h"

namespace stems {

reconstruction_buffer::reconstruction_buffer(size_type size,
		stats& stats) :
		super(size), m_stats(stats) {
}

bool reconstruction_buffer::place(size_type index,
		address value) {
	if (index >= size()) {
		return false;
	}
	if (!at(index)) { // check if the existing index is empty
		at(index) = value; // nothing more to do, set the value at the given index and return success
		return true;
	} else {
		m_stats["reconstruction-buffer-reorderings"]++;
		long int long_int_index = static_cast<long int>(index); // cast to long int to allow possible negatives that will be removed later
		// try checking one spot forwards and backwards, then two spots forwards and backwards
		// (yes, this is specifically requested somehow in the STeMS paper. see section 4.1: Reconstruction, page 6 of the paper)
		long int indices[] = { long_int_index + 1, long_int_index - 1,
				long_int_index + 2, long_int_index - 2 };
		for (int i : indices) {
			if (i >= 0 && i < static_cast<long int>(size()) && !at(i) // check if empty
					) {
				at(i) = value; // and set
				return true; // return success
			}
		}
	}
	m_stats["reconstruction-buffer-rejections"]++;
	return false; // nothing worked, return failure
}

void reconstruction_buffer::clear() {
	for (address& address : *this) {
		address = 0;
	}
}

}
