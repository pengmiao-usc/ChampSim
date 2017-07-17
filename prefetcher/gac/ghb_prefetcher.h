#ifndef __GHB_PREFETCHER_H
#define __GHB_PREFETCHER_H

#include <cstdint>
#include <vector>

#define INVALID_ADDRESS 0

class GhbPrefetcher
{
   public:
      GhbPrefetcher();
      std::vector<uint64_t> getNextAddress(uint64_t pc, uint64_t currentAddress, bool cache_hit);

      ~GhbPrefetcher();

   private:
      static const int64_t INVALID_DELTA = INT64_MAX;
      static const uint32_t INVALID_INDEX = UINT32_MAX;

      struct GHBEntry
      {
         uint32_t nextIndex; //index of the next entry belonging to the same list
         int64_t delta; //delta between last address and current address
         uint32_t generation;
         GHBEntry() : nextIndex(INVALID_INDEX), delta(INVALID_DELTA), generation(0) {}
      };

      struct TableEntry
      {
         uint32_t ghbIndex;
         int64_t delta;
         uint32_t generation;
         TableEntry() : ghbIndex(INVALID_INDEX), delta(INVALID_DELTA), generation(0) {}
      };

      uint32_t m_prefetchWidth;
      uint32_t m_prefetchDepth;

      uint64_t m_lastAddress;

      //circular global history buffer
      uint32_t m_ghbSize;
      uint32_t m_ghbHead;
      uint32_t m_generation;
      std::vector<GHBEntry> m_ghb;

      uint32_t m_tableSize;
      uint32_t m_tableHead; //next table position to be overwritten (in lack of a better replacement policy at the moment)
      std::vector<TableEntry> m_ghbTable;
};

#endif // __GHB_PREFETCHER_H
