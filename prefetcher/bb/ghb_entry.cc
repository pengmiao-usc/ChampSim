/*
 * Author: Arka Majumdar
 */

#include "ghb_entry.h"

namespace bb {

ghb_entry::ghb_entry() :
        m_trigger(0), m_spatial(), m_valid(false) {
}

ghb_entry::ghb_entry(address trigger) :
        m_trigger(trigger), m_spatial(), m_valid(true) {
}

}
