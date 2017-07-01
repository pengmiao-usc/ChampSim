/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_TMS_TMS_TYPES_H
#define PREFETCHER_STEMS_TMS_TMS_TYPES_H

#include <unordered_map>

#include "../stems_types.h"

#include "tms_types.h"
#include "rmob.h"

namespace stems {
namespace tms {

using index_table = std::unordered_map<address, rmob::index_type>;

}
}

#endif
