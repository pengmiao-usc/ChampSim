/*
 * Author: Arka Majumdar
 */

#include "ghb.h"

namespace bb {

ghb::ghb(size_type n) :
        super(n), m_counter(0) {
}

ghb::const_reference ghb::operator[](size_type pos) const {
    return super::operator[](pos % size());
}

ghb::reference ghb::operator[](size_type pos) {
    return const_cast<reference>(static_cast<const ghb&>(*this).operator[](pos));
}

template<typename GHBEntry>
ghb::size_type ghb::append(GHBEntry&& entry) {
    operator[](m_counter) = std::forward<GHBEntry>(entry);
    size_type old_counter = m_counter;
    m_counter++;
    m_counter %= size();
    return old_counter;
}
template ghb::size_type ghb::append(const ghb_entry& entry);
template ghb::size_type ghb::append(ghb_entry&& entry);

}
