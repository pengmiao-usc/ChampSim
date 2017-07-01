/*
 * Implementation author: Arka Majumdar
 */

#ifndef PREFETCHER_STEMS_SMS_SEQUENCE_ELEMENT_H
#define PREFETCHER_STEMS_SMS_SEQUENCE_ELEMENT_H

#include "sms_types.h"

namespace stems {
namespace sms {

/**
 * An offset-delta pair in the sequence of SMS accesses.
 */
struct sequence_element {
	offset m_offset;

	reconstruction_delta m_delta;
};

}
}

#endif
