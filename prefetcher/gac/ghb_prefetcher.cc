/*
 * Implementation of the Global History Buffer (G/AC) prefetcher.
 *
 * Originally written for the Sniper simulator.
 *
 * Adapted for the ChampSim simulator: Arka Majumdar
 */

#include "ghb_config.h"
#include "ghb_prefetcher.h"

#include <algorithm>

GhbPrefetcher::GhbPrefetcher()
   : m_prefetchWidth(GHB_WIDTH)
   , m_prefetchDepth(GHB_DEPTH)
   , m_lastAddress(INVALID_ADDRESS)
   , m_ghbSize(GHB_SIZE)
   , m_ghbHead(0)
   , m_generation(0)
   , m_ghb(m_ghbSize)
   , m_tableSize(GHB_TABLE_SIZE)
   , m_tableHead(0)
   , m_ghbTable(m_tableSize)
{
}

GhbPrefetcher::~GhbPrefetcher()
{
}

std::vector<uint64_t>
GhbPrefetcher::getNextAddress(uint64_t pc, uint64_t currentAddress, bool cache_hit)
{
   std::vector<uint64_t> prefetchList;

   //deal with prefether initialization
   if (m_lastAddress == INVALID_ADDRESS)
   {
      m_lastAddress = currentAddress;
      return prefetchList;
   }

   //determine the delta with the last address
   int64_t delta = currentAddress - m_lastAddress;
   m_lastAddress = currentAddress;

   //look for the current delta in the table
   uint32_t i = 0;
   while (i < m_tableSize &&
          m_ghbTable[i].delta != INVALID_DELTA &&
          m_ghbTable[i].delta != delta)
      ++i;

   if (i != m_tableSize &&
       m_ghbTable[i].delta == delta &&
       m_ghbTable[i].generation == m_ghb[m_ghbTable[i].ghbIndex].generation) //check if the table still points to the current GHB 'generation'
   {
      uint32_t width = 0;
      uint32_t ghbIndex = m_ghbTable[i].ghbIndex;

      while(width < m_prefetchWidth && ghbIndex != INVALID_INDEX)
      {
         uint32_t depth = 0;

         uint64_t newAddress = currentAddress;

         while (depth < m_prefetchDepth &&
                m_ghb[(ghbIndex + depth)%m_ghbSize].delta != INVALID_DELTA)
         {
            newAddress += m_ghb[(ghbIndex + depth)%m_ghbSize].delta;

            //add address to the list if it wasn't in there already
            if (find(prefetchList.begin(), prefetchList.end(), newAddress) == prefetchList.end())
               prefetchList.push_back(newAddress);

            ++depth;
         }

         ++width;

         uint32_t nextIndex = m_ghb[ghbIndex].nextIndex;
         if ((nextIndex > ghbIndex ||   //if we circle the GHB
              ghbIndex > m_ghbHead) &&  //OR we were already larger than the head pointer
              nextIndex < m_ghbHead)    //AND the nextIndex has been overwritten by new entries
            ghbIndex = INVALID_INDEX;
         else
            ghbIndex = nextIndex;
      }
   }

   //add new delta to the ghb and table

   m_ghb[m_ghbHead].delta = delta;
   m_ghb[m_ghbHead].nextIndex = INVALID_INDEX;
   m_ghb[m_ghbHead].generation = m_generation;

   uint32_t prevHead = m_ghbHead > 0 ? m_ghbHead - 1 : m_ghbSize - 1;
   int64_t prevDelta = m_ghb[prevHead].delta;

   if (prevDelta != INVALID_DELTA)
   {
      i = 0;
      while (i < m_tableSize &&
             m_ghbTable[i].delta != INVALID_DELTA &&
             m_ghbTable[i].delta != prevDelta)
         ++i;

      if (i != m_tableSize &&
          m_ghbTable[i].delta == prevDelta)
      { //update existing entry

         //if the current table entry refers to a live ghb entry,
         //have the new entry link to the live entry
         //
         //otherwise, refer to INVALID_INDEX
         if (m_ghbTable[i].generation == m_ghb[m_ghbTable[i].ghbIndex].generation)
            m_ghb[m_ghbHead].nextIndex = m_ghbTable[i].ghbIndex;
         else
            m_ghb[m_ghbHead].nextIndex = INVALID_INDEX;

         m_ghbTable[i].ghbIndex = m_ghbHead;
         m_ghbTable[i].generation = m_generation;
      }
      else
      { //prevDelta not found ==> add entry to table
         m_ghbTable[m_tableHead].delta = prevDelta;
         m_ghbTable[m_tableHead].ghbIndex = m_ghbHead;
         m_ghbTable[m_tableHead].generation = m_generation;

         m_tableHead = (m_tableHead + 1) % m_tableSize;
      }

   }

   ++m_ghbHead;
   if (m_ghbHead == m_ghbSize)
   {
      m_ghbHead = 0;
      m_generation = (m_generation + 1) % 4;
   }

   return prefetchList;
}
