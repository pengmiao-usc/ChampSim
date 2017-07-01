/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_TMS_RMOB_H
#define PREFETCHER_STEMS_TMS_RMOB_H

#include <vector>

#include "sequence_element.h"

namespace stems {
namespace tms {

/**
 * The Region Miss Order Buffer -- basically a GHB.
 */
class rmob: public std::vector<sequence_element> {
	using super = std::vector<sequence_element>;

public:
	using index_type = long int;

	reference operator[](index_type pos);

	const_reference operator[](index_type pos) const;

private:
	rmob(size_type n);

	friend class tms;
};

}
}

#endif
