/*
 * Implementation author: Arka Majumdar
 */

#include "rmob.h"

namespace stems {
namespace tms {

rmob::reference rmob::operator[](index_type pos) {
	if (pos < 0) {
		throw std::out_of_range(
				"index_type passed to stems_prefetcher::tms::rmob::operator[] is invalid");
	}
	return super::operator[](static_cast<size_type>(pos) % size());
}

rmob::const_reference rmob::operator[](index_type pos) const {
	if (pos < 0) {
		throw std::out_of_range(
				"index_type passed to stems_prefetcher::tms::rmob::operator[] is invalid");
	}
	return super::operator[](static_cast<size_type>(pos) % size());
}

rmob::rmob(size_type n) :
		super(n) {
}

}
}
