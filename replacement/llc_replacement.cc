#include "cache.h"
#include <vector>

uint64_t accurate_prefetches[NUM_CPUS];
uint64_t total_prefetches[NUM_CPUS];
uint64_t demand_misses[NUM_CPUS];
uint64_t pollution[NUM_CPUS][NUM_CPUS];

struct POLLUTION_ENTRY
{
    uint64_t victim_addr;
    uint32_t cpu;

    POLLUTION_ENTRY(uint64_t _addr, uint32_t _cpu)
    {
        victim_addr = _addr;
        cpu = _cpu;
    }
};

bool prefetched[LLC_SET][LLC_WAY];
vector<POLLUTION_ENTRY> pollution_filter;
#define FILTER_SIZE 4096*NUM_CPUS

// initialize replacement state
void CACHE::llc_initialize_replacement()
{
    for(int i=0; i<NUM_CPUS; i++)
    {
        accurate_prefetches[i] = 0;
        total_prefetches[i] = 0;
        demand_misses[i] = 0;
        for(int j=0; j<NUM_CPUS; j++)
            pollution[i][j] = 0;
    }

    for (int i=0; i<LLC_SET; i++) {
        for (int j=0; j<LLC_WAY; j++) {
            prefetched[i][j]= false;
        }
    }

    pollution_filter.clear();
}

// find replacement victim
uint32_t CACHE::llc_find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    // baseline LRU
    return lru_victim(cpu, instr_id, set, current_set, ip, full_addr, type); 
}

// called on every cache hit and cache fill
void CACHE::llc_update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    string TYPE_NAME;
    if (type == LOAD)
        TYPE_NAME = "LOAD";
    else if (type == RFO)
        TYPE_NAME = "RFO";
    else if (type == PREFETCH)
        TYPE_NAME = "PF";
    else if (type == WRITEBACK)
        TYPE_NAME = "WB";
    else
        assert(0);

    if (hit)
        TYPE_NAME += "_HIT";
    else
        TYPE_NAME += "_MISS";

    if ((type == WRITEBACK) && ip)
        assert(0);

    // uncomment this line to see the LLC accesses
    // cout << "CPU: " << cpu << "  LLC " << setw(9) << TYPE_NAME << " set: " << setw(5) << set << " way: " << setw(2) << way;
    // cout << hex << " paddr: " << setw(12) << paddr << " ip: " << setw(8) << ip << " victim_addr: " << victim_addr << dec << endl;

    // baseline LRU
    if (hit && (type == WRITEBACK)) // writeback hit does not update LRU state
        return;

    victim_addr = (victim_addr >> 6) << 6;
    full_addr = (full_addr >> 6) << 6;

    if(type == PREFETCH)
    {
        if(!hit)
        {
            total_prefetches[cpu]++;
            if((victim_addr != 0) && !(prefetched[set][way])) // Demand victim
            {   
                POLLUTION_ENTRY new_entry(victim_addr, cpu);
                pollution_filter.push_back(new_entry);
                while(pollution_filter.size() > FILTER_SIZE)
                    pollution_filter.erase(pollution_filter.begin());
            }

            for(vector<POLLUTION_ENTRY>::iterator it=pollution_filter.begin(); it != pollution_filter.end(); it++)
            {
                if((*it).victim_addr == full_addr)
                {
                    pollution_filter.erase(it); // We kicked it out, but brought it back in
                    break;
                }
            }

            prefetched[set][way] = true;
        }
    }
    else
    {
        if((hit) && prefetched[set][way])
            accurate_prefetches[cpu]++;

        if(!hit)
        {
            demand_misses[cpu]++;

            for(vector<POLLUTION_ENTRY>::iterator it=pollution_filter.begin(); it != pollution_filter.end(); it++)
            {
                if((*it).victim_addr == full_addr)
                {
                    // Victim Core: cpu; Attack Core: (*it).cpu
                    pollution[(*it).cpu][cpu]++;
                    pollution_filter.erase(it);
                    break;
                }
            }
        }

        prefetched[set][way] = false;
    }

    return lru_update(set, way);
}

void CACHE::llc_replacement_final_stats()
{
    for(int i=0; i<NUM_CPUS; i++)
    {
        cout << "Core " << i << " Accuracy: " << 100*(double)accurate_prefetches[i]/(double)total_prefetches[i] << endl;
        cout << "Core " << i << " Accurate prefetches " << accurate_prefetches[i] << " out of " << total_prefetches[i] << endl;
        cout << "Core " << i << " Coverage: " << 100*(double)accurate_prefetches[i]/(double)(accurate_prefetches[i] + demand_misses[i]) << endl;
        uint64_t pollution_others = 0;
        uint64_t demand_others = 0;
        for(int j=0; j<NUM_CPUS; j++)
        {
            if(i != j)
            {
                pollution_others += pollution[i][j];
                demand_others += demand_misses[j];
            }
            cout << "Core " << i << " pollution on " << j << ": " << 100*(double)pollution[i][j]/(double)demand_misses[j] << " " << pollution[i][j] << " " << demand_misses[j] << endl;
        }

        cout << "Core " << i << " Pollution: " << 100*(double)pollution[i][i]/(double)demand_misses[i] << " " << 100*(double)pollution_others/(double)demand_others << endl;
    }
}
