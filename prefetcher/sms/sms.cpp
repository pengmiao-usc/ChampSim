/*
 * Implementation of the Spatial Memory Streaming (SMS) Prefetcher.
 *
 * Originally written by the Computer Architecture Laboratory at Carnegie Mellon University and by the Parallel Systems Architecture Lab at Ecole Polytechnique Federale de Lausanne for the The 1st JILP Data Prefetching Championship.
 *
 * Adapted for the Marssx86 simulator: Akanksha Jain at the University of Texas at Austin
 *
 * Then adapted for the ChampSim simulator: Arka Majumdar
 */

#include <map>
#include <string>
#include <cassert>

#include "sms.h"

struct SMS_Stats
{
    typedef std::map<std::string,long long> tCounters;
    tCounters theCounters;
    ~SMS_Stats() {
        for(tCounters::iterator i=theCounters.begin();i!=theCounters.end();++i) {
            std::cerr << i->first << "," << i->second << std::endl;
        }
    }
} theStats;

#define C(n) do { ++(theStats.theCounters[(#n)]); } while(0);
#define S(n,v) do { (theStats.theCounters[(#n)])+=(v); } while(0);

pattern_t SMS_prefetcher_t::rotate(int aBitIndex,int anOffset) {
    //debug(buffer,"rotate(%d,%d [%d]) : ",aBitIndex,anOffset,theBlocksPerRegion);
    pattern_t res = 1ULL<<((aBitIndex + anOffset) % theBlocksPerRegion);
    //debug(buffer,"%llx > %llx\n",1ULL<<aBitIndex,res);
    assert(!(res & (res-1)));
    return res;
}

bool SMS_prefetcher_t::replace( uint64_t addr ) {
    uint64_t region_tag = addr & ~theRegionMask;
    int region_offset = (addr & theRegionMask)>>6;
    AGT::Item agt_evicted;
    //DBG_(Dev, ( << std::dec << theId << "-evict: group=" << std::hex << region_tag << "  offset=" << std::dec << region_offset ) );
    bool erased_something(false);

    AGT::Iter agt_ent = theAGT.find(region_tag);
    if (agt_ent != theAGT.end()) {
        pattern_t new_bit = rotate(region_offset,agt_ent->second.offset);
        if(NoRotation) new_bit = 1ULL << region_offset;
        if (agt_ent->second.pattern & new_bit) {
            agt_evicted = *agt_ent;
            //DBG_(Dev, ( << std::dec << theId << "-end: group=" << std::hex << region_tag << "  key=" << agt_evicted.second.pc << "  " << agt_evicted.second.pattern ) );
            theAGT.erase(region_tag);
            erased_something = true;
        }
    }

    if (agt_evicted.second.pattern) {
        if (thePHT.erase(agt_evicted.second.pc)) C(PHT_erased_previous); // if replacing or if it's a singleton
        if ((agt_evicted.second.pattern-1)&agt_evicted.second.pattern) {// not singleton
     //       debug(buffer,"learned pattern (block eviction) %llx into PHT, pc=%llx\n",agt_evicted.second.pattern,agt_evicted.second.pc);
            thePHT.insert(agt_evicted.second.pc,PHTent(agt_evicted.second.pattern));
        }
    }

    return erased_something;
}
/*
void SMS_prefetcher_t::checkEvictions() {
    for(int n=0;n<theAGT.theHeight;++n) {
invalidated_list:
        AGT::ListType& aList(theAGT.theItems[n]);
        int x=0;
        for(AGT::Iter i=aList.begin();i!=aList.end();i++) {
            //fprintf(buffer,"bla %d n=%d x=%d\n",__LINE__,n,x);
            //fprintf(buffer,"bla %llx\n",i->second.pattern);
            int offset = theBlocksPerRegion-1;
            for(pattern_t pattern = i->second.pattern;pattern;--offset) {
                pattern_t mask = (1ULL<<offset);
                if (!(pattern & mask)) continue;
        //        std::cout << "Region " << (void*)i->second.EVICT_DETECTOR << endl;
        //        std::cout << "  attempt offset=" << offset <<" mask= " <<(void*)mask << " on region_offset=" << i->second.offset << " with pattern " << pattern << endl;
                pattern &= ~mask;
                uint64_t prediction = ((-i->second.offset+offset)*theBlockSize);
                if(NoRotation) prediction = (offset*theBlockSize);
         //       std::cout <<"  prediction = " << (void*)prediction << endl;
                prediction &= theRegionMask;
                uint64_t anAddress = i->second.EVICT_DETECTOR+prediction;
          //      std::cout <<"  address = " << (void*)anAddress << endl;
              //  EVICTdebug(buffer,"  prediction&= %llx\n",prediction);
              //  EVICTdebug(buffer,"  prediction!= %llx\n",i->first + prediction);
                //if (GetPrefetchBit(0,anAddress)==-1) {
                if (!probe_cache(anAddress)) {
           //         std::cout << "Not in cache " << endl;
                    if(probe_mshrs(anAddress)){
            //            std::cout << "In MSHR " << endl;
                    }
                    else{
                        if (replace(anAddress)) goto invalidated_list;
                    }
                   // if (mshrs->inflight(anAddress)) {
                   //     MSHRfprintf(buffer,"block still in flight (n=%d, x=%d), region=%llx offset=%d addr=%llx\n",n,x,i->first,i->second.offset,anAddress);
                    //} else {
                       // MSHRfprintf(buffer,"detected evicted block (n=%d, x=%d), region=%llx offset=%d, evicted addr=%llx\n",n,x,i->first,i->second.offset,anAddress);
                    //}
                }
            }
            ++x;
        }
    }
}
*/
void SMS_prefetcher_t::IssuePrefetchCandidates( uint64_t rip, uint64_t addr, bool mshr_hit, bool hit ){

   // if(!hit)
   //     checkEvictions();

    uint64_t pc(rip);
    assert(pc);
    uint64_t region_tag = addr & ~theRegionMask;
    int region_offset = (addr & theRegionMask)>>6;
    uint64_t key = pc;
    if(NoRotation) key = (pc << (theRegionShift-6)) | region_offset;
    bool miss = (! hit);

    //DBG_(Dev, ( << std::dec << theId << "-access: group=" << std::hex << region_tag << "  key=" << key << "  offset=" << std::dec << region_offset ) );
   // debug(buffer,"region=%llx offset=%d bit=%llx pc=%llx\n",region_tag,region_offset,1ULL<<region_offset,pc);
    //std::cout << "    Trigger " << (void*)addr << " with pc " << (void*)rip <<  " key " << (void*)key << endl;
    //std::cout << "region=" << (void*)region_tag <<  " offset="<<region_offset <<" bit=" << (void*)(1ULL<<region_offset) <<" pc=" << (void*)pc << endl;
    AGT::Iter agt_ent = theAGT.find(region_tag);
    AGT::Item agt_evicted;
    bool new_gen = false;
    if (agt_ent == theAGT.end()) {
        C(AGT_evict_replacement)
            pattern_t new_bit = 1ULL<<(theBlocksPerRegion-1);
        if(!NoRotation) {
            agt_evicted = theAGT.insert(region_tag,AGTent(key,theBlocksPerRegion-region_offset-1,new_bit));
        } else {
            new_bit = 1ULL << region_offset;
            agt_evicted = theAGT.insert(region_tag,AGTent(key,0,new_bit));
        }
        new_gen = true;
       // std::cout << "new pattern (from scratch) new_bit=" << new_bit <<" offset=" << region_offset << " -> " << theBlocksPerRegion-region_offset << endl;
    } else {
        pattern_t new_bit = rotate(region_offset,agt_ent->second.offset);
        if(NoRotation) new_bit = 1ULL << region_offset;
        if ((agt_ent->second.pattern & new_bit) && miss) {
            C(AGT_samebit_replacement)
                // FIXME: is same bit repeating the common case or not?
         //       debug(buffer,"collided on pattern %llx (new_bit=%llx)\n",agt_ent->second.pattern,new_bit);
            /* same-bit ends gen logic
               agt_evicted = *agt_ent;
               if(!NoRotation) {
               agt_ent->second = AGTent(key,theBlocksPerRegion-region_offset-1);
               } else {
               agt_ent->second = AGTent(key,0);
               }
               new_gen = true;
             */
        } else {
            C(AGT_addbit)
        }
        agt_ent->second.pattern |= new_bit;
        //debug(buffer,"update pattern:%llx new_bit=%llx offset=%d\n",agt_ent->second.pattern,new_bit,agt_ent->second.offset);
       // std::cout << "update pattern:" <<agt_ent->second.pattern << " new_bit= "<<new_bit <<" offset=" << agt_ent->second.offset << endl;
    }

   // std::cout << "Evicted AGT entry - key=" <<(void*)agt_evicted.second.pc << " offset " << (void*)agt_evicted.second.offset << " pattern " << (void*)agt_evicted.second.pattern << endl;
    if (agt_evicted.second.pattern) {
        if (thePHT.erase(agt_evicted.second.pc)) C(PHT_erased_previous); // if replacing or if it's a singleton
        if ((agt_evicted.second.pattern-1)&agt_evicted.second.pattern) {
            // not singleton
            //debug(buffer,"learned pattern (AGT eviction) %llx into PHT, pc=%llx\n",agt_evicted.second.pattern,agt_evicted.second.pc);
          //  std::cout << "learned pattern (AGT eviction) " << (void*)(agt_evicted.second.pattern) << " into PHT, pc=" << (void*)agt_evicted.second.pc << endl;
            thePHT.insert(agt_evicted.second.pc,PHTent(agt_evicted.second.pattern));
        }
    }

    if (new_gen) {
        PHT::Iter pht_ent = thePHT.find(key);
        if (pht_ent != thePHT.end()) {
            //DBG_(Dev, ( << std::dec << theId << "-predict: group=" << std::hex << region_tag << "  key=" << key << "  " << pht_ent->second.pattern ) );
            C(L1_Found_Pattern)
                //debug(buffer,"prediction pattern for pc=%llx is %llx and region_offset %d\n",key,pht_ent->second.pattern,region_offset);
            int offset = theBlocksPerRegion-2; // extra -1 to avoid prefetch of trigger
            if(NoRotation) offset += 1;
            //assert((pht_ent->second.pattern-1)&pht_ent->second.pattern);
            int lookahead = config.lookahead;
            uint64_t num_prefetched = 0;
            for(pattern_t pattern = pht_ent->second.pattern;pattern;--offset) {
                //debug(buffer,"  prediction at offset %d, pattern is %llx and region_offset %d\n",offset,pht_ent->second.pattern,region_offset);
                pattern_t mask = (1ULL<<offset);
                //debug(buffer,"  attempt offset=%d mask=%llx on region_offset=%d with pattern %llx\n",offset,mask,region_offset,pattern);
                if (!(pattern & mask)) continue;
                pattern &= ~mask;
                if (NoRotation && (offset == region_offset)) continue;
                //debug(buffer,"  prediction at offset %d, pattern is %llx and region_offset %d\n",offset,pht_ent->second.pattern,region_offset);
                uint64_t prediction = ((region_offset+offset+1)*theBlockSize);
                if(NoRotation) prediction = (offset*theBlockSize);
                //debug(buffer,"  prediction = %llx\n",prediction);
                prediction &= theRegionMask;
                //debug(buffer,"  prediction&= %llx\n",prediction);
                //debug(buffer,"  prediction!= %llx\n",region_tag + prediction);
                if(lookahead == 0){
                    InitPrefetch(region_tag + prediction, 2*num_prefetched);
                    num_prefetched++;
                    if(num_prefetched == config.degree)
                        return;
                }
                else
                    lookahead--;
                //mshrs->allocate(region_tag + prediction, -cycle); only for stats, don't waste MSHRs
                C(L1_Prefetches_Issued)
            }
        }
    }
   // std::cout << endl;

}
