#ifndef SMS_H
#define SMS_H

#include <vector>
#include <list>
#include <cassert>
#include <functional>
#include <cstdint>
#include <map>
#include <iostream>

//char buffer[100];
//#define debug sprintf
//#define MSHRfprintf sprintf
//#define EVICTdebug sprintf

#define SIGNED_COUNTER long long

const int theBlockSize(64);

//const bool NoRotation = true;
const bool NoRotation = false;

template<class KeyType,class ItemType>
struct Container
{
    int theHeight,theWidth,theKeyShift;
    unsigned long long theTagMask; // dictates number of bits used to tag elements (includes lower index bits for simulation purposes only)
    unsigned long long theIndexMask; // based only on Height of structure

    typedef std::pair<KeyType,ItemType> Item;
    typedef std::list<Item> ListType;
    std::vector<ListType> theItems;
    typedef typename ListType::iterator Iter;
    Container(
        int aHeight
    ,   int aWidth
    ,   int aKeyShift
    ,   int aTagBits
    )
    :   theHeight(aHeight)
    ,   theWidth(aWidth)
    ,   theKeyShift(aKeyShift)
    ,   theTagMask((1ULL<<aTagBits)-1)
    ,   theIndexMask(theHeight-1)
    {
        assert(!(theHeight & theIndexMask));
        theItems.resize(theHeight);
        for(int i=0;i<theHeight;++i)
            theItems[i].resize(theWidth);
    }

    ~Container() {
        /*for(int i=0;i<theHeight;++i)
            for(int j=0;j<theWidth;++i)
                theItems[i].push_back(new ItemType());*/
    }

    Item insert(KeyType aKey,ItemType anItem) {
        int aKeyIndex(index(aKey));
        ListType& aList(theItems[aKeyIndex]);

        Item anOldItem(*(aList.rbegin()));
        aList.pop_back();
        KeyType aTag = tag(aKey);
        aList.push_front(std::make_pair(aTag,anItem));
        aList.front().second.EVICT_DETECTOR = aKey;
        //fprintf(stderr,"insert %llx [%d:%d/%d]\n",aKey,index(aKey),theHeight,theWidth);
       // cout << "insert "<<(void*)aKey << " at index " << aKeyIndex << endl;
        return anOldItem;
    }

    Iter end() {
        return theItems[0].end();
    }

    static inline unsigned long long inthash(unsigned long long key)
    {
        key += (key << 12);
        key ^= (key >> 22);
        key += (key << 4);
        key ^= (key >> 9);
        key += (key << 10);
        key ^= (key >> 2);
        key += (key << 7);
        key ^= (key >> 12);
        return key;
    }

    int index(KeyType aKey) {
        return (inthash(aKey >> theKeyShift) & theIndexMask);
    }

    int tag(KeyType aKey) {
        return (inthash(aKey >> theKeyShift) & theTagMask);
    }

    Iter find(KeyType aKey) {
        ListType& aList(theItems[index(aKey)]);
        KeyType aTag = tag(aKey);
        for(Iter i=aList.begin();i!=aList.end();i++) {
            //fprintf(stderr,"search for %llx in %llx [%d:%d/%d]\n",aKey,i->first,index(aKey),theHeight,theWidth);
            if (i->first == aTag) {
                aList.push_front(*i);
                aList.erase(i);
                return aList.begin();
            }
        }
        return end();
    }

    bool erase(KeyType aKey) {
        ListType& aList(theItems[index(aKey)]);
        KeyType aTag = tag(aKey);
        for(Iter i=aList.begin();i!=aList.end();i++) {
            if (i->first == aTag) {
                aList.erase(i);
                aList.resize(theWidth);
                return true;
            }
        }
        return false;
    }

};

typedef unsigned long long pattern_t;

struct SMS_prefetcher_t
{
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

    uint64_t theRegionShift,theRegionSize,theRegionMask,theBlocksPerRegion;
    typedef std::function<bool(uint64_t pc, uint64_t base_addr, uint64_t addr, int delay)> InitPrefetch;
    InitPrefetch theInitPrefetch;
    
    struct AGTent {
        uint64_t pc;
        int offset;
        pattern_t pattern;
        AGTent(uint64_t aPC = 0ULL,int aOffset = 0,pattern_t aPattern = 0ULL) // @suppress("Class members should be properly initialized")
        :   pc(aPC)
        ,   offset(aOffset)
        ,   pattern(aPattern)
        { }
        uint64_t EVICT_DETECTOR;
    };
    struct PHTent {
        pattern_t pattern;
        PHTent(pattern_t aPattern = 0ULL):pattern(aPattern) { } // @suppress("Class members should be properly initialized")
        uint64_t EVICT_DETECTOR;
    };
    typedef Container<uint64_t,AGTent> AGT; AGT theAGT;
    typedef Container<uint64_t,PHTent> PHT; PHT thePHT;

    pattern_t rotate(int aBitIndex,int anOffset);
    bool replace(  uint64_t addr );
    void checkEvictions( );

    public :
    SMS_prefetcher_t(InitPrefetch theInitPrefetch) :
          theStats()
        , theRegionShift(9) // shift: 12 = 4KB region, 10 = 1KB region
        , theRegionSize(1<<theRegionShift)
        , theRegionMask(theRegionSize-1)
        , theBlocksPerRegion(theRegionSize/theBlockSize)
        , theInitPrefetch(theInitPrefetch)
//        , theAGT(4,16,theRegionShift,27-theRegionShift)
        , theAGT(16,16,theRegionShift,27-theRegionShift)
//        , thePHT(256,8,NoRotation?0:2,14)
        , thePHT(1024,8,NoRotation?0:2,14)
    {}

    void IssuePrefetchCandidates( uint64_t key, uint64_t addr, bool mshr_hit, bool hit );

};

#endif
