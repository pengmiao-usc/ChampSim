/**
 * Define some types here so that the rest of the code is more simulator-independent and readable.
 *
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_BB_BB_PREFTETCHER_TYPES_H
#define PREFETCHER_BB_BB_PREFTETCHER_TYPES_H

#include <map>
#include <string>
#include <functional>
#include <cstdint>

namespace bb {

/*
 * Integer type used for program counter values.
 */
using pc = uint64_t;

/*
 * Integer type used to represent memory addresses in the simulator.
 */
using address = uint64_t;

using block = uint64_t;

using cache_access_type = uint8_t;

using stat = uint64_t;

using delta = int64_t;

}

#endif
