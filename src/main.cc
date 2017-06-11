#define _BSD_SOURCE

#include <getopt.h>
#include "ooo_cpu.h"
#include "uncore.h"

uint8_t warmup_complete[NUM_CPUS], 
        simulation_complete[NUM_CPUS], 
        all_warmup_complete = 0, 
        all_simulation_complete = 0,
        knob_low_bandwidth = 0;
#ifdef CRC2_COMPILE
uint64_t warmup_instructions     = 100000,
         simulation_instructions = 1000000;
#else
uint64_t warmup_instructions     = 1000000,
         simulation_instructions = 10000000;
#endif
time_t start_time;

void record_roi_stats(uint32_t cpu, CACHE *cache)
{
    for (uint32_t i=0; i<NUM_TYPES; i++) {
        cache->roi_access[cpu][i] = cache->sim_access[cpu][i];
        cache->roi_hit[cpu][i] = cache->sim_hit[cpu][i];
        cache->roi_miss[cpu][i] = cache->sim_miss[cpu][i];
    }
}

void print_roi_stats(uint32_t cpu, CACHE *cache)
{
    uint64_t TOTAL_ACCESS = 0, TOTAL_HIT = 0, TOTAL_MISS = 0;

    for (uint32_t i=0; i<NUM_TYPES; i++) {
        TOTAL_ACCESS += cache->roi_access[cpu][i];
        TOTAL_HIT += cache->roi_hit[cpu][i];
        TOTAL_MISS += cache->roi_miss[cpu][i];
    }

    cout << cache->NAME;
    cout << " TOTAL     ACCESS: " << setw(10) << TOTAL_ACCESS << "  HIT: " << setw(10) << TOTAL_HIT << "  MISS: " << setw(10) << TOTAL_MISS << endl;

    cout << cache->NAME;
    cout << " LOAD      ACCESS: " << setw(10) << cache->roi_access[cpu][LOAD];
    cout << "  HIT: " << setw(10) << cache->roi_hit[cpu][LOAD];
    cout << "  MISS: " << setw(10) << cache->roi_miss[cpu][LOAD] << endl;

    cout << cache->NAME;
    cout << " RFO       ACCESS: " << setw(10) << cache->roi_access[cpu][RFO];
    cout << "  HIT: " << setw(10) << cache->roi_hit[cpu][RFO];
    cout << "  MISS: " << setw(10) << cache->roi_miss[cpu][RFO] << endl;

    cout << cache->NAME;
    cout << " PREFETCH  ACCESS: " << setw(10) << cache->roi_access[cpu][PREFETCH];
    cout << "  HIT: " << setw(10) << cache->roi_hit[cpu][PREFETCH];
    cout << "  MISS: " << setw(10) << cache->roi_miss[cpu][PREFETCH] << endl;

    cout << cache->NAME;
    cout << " WRITEBACK ACCESS: " << setw(10) << cache->roi_access[cpu][WRITEBACK];
    cout << "  HIT: " << setw(10) << cache->roi_hit[cpu][WRITEBACK];
    cout << "  MISS: " << setw(10) << cache->roi_miss[cpu][WRITEBACK] << endl;
}

void print_sim_stats(uint32_t cpu, CACHE *cache)
{
    uint64_t TOTAL_ACCESS = 0, TOTAL_HIT = 0, TOTAL_MISS = 0;

    for (uint32_t i=0; i<NUM_TYPES; i++) {
        TOTAL_ACCESS += cache->sim_access[cpu][i];
        TOTAL_HIT += cache->sim_hit[cpu][i];
        TOTAL_MISS += cache->sim_miss[cpu][i];
    }

    cout << cache->NAME;
    cout << " TOTAL     ACCESS: " << setw(10) << TOTAL_ACCESS << "  HIT: " << setw(10) << TOTAL_HIT << "  MISS: " << setw(10) << TOTAL_MISS << endl;

    cout << cache->NAME;
    cout << " LOAD      ACCESS: " << setw(10) << cache->sim_access[cpu][LOAD];
    cout << "  HIT: " << setw(10) << cache->sim_hit[cpu][LOAD];
    cout << "  MISS: " << setw(10) << cache->sim_miss[cpu][LOAD] << endl;

    cout << cache->NAME;
    cout << " RFO       ACCESS: " << setw(10) << cache->sim_access[cpu][RFO];
    cout << "  HIT: " << setw(10) << cache->sim_hit[cpu][RFO];
    cout << "  MISS: " << setw(10) << cache->sim_miss[cpu][RFO] << endl;

    cout << cache->NAME;
    cout << " PREFETCH  ACCESS: " << setw(10) << cache->sim_access[cpu][PREFETCH];
    cout << "  HIT: " << setw(10) << cache->sim_hit[cpu][PREFETCH];
    cout << "  MISS: " << setw(10) << cache->sim_miss[cpu][PREFETCH] << endl;

    cout << cache->NAME;
    cout << " WRITEBACK ACCESS: " << setw(10) << cache->sim_access[cpu][WRITEBACK];
    cout << "  HIT: " << setw(10) << cache->sim_hit[cpu][WRITEBACK];
    cout << "  MISS: " << setw(10) << cache->sim_miss[cpu][WRITEBACK] << endl;
}

void print_branch_stats()
{
    for (uint32_t i=0; i<NUM_CPUS; i++) {
        cout << endl << "CPU " << i << " Branch Prediction Accuracy: ";
        cout << (100.0*(ooo_cpu[i].num_branch - ooo_cpu[i].branch_mispredictions)) / ooo_cpu[i].num_branch;
        cout << "% MPKI: " << (1000.0*ooo_cpu[i].branch_mispredictions)/(ooo_cpu[i].num_retired - ooo_cpu[i].warmup_instructions) << endl;
    }
}

void print_dram_stats()
{
    cout << endl;
    cout << "DRAM Statistics" << endl;
    cout << " RQ ROW_BUFFER_HIT: " << setw(10) << uncore.DRAM.RQ.ROW_BUFFER_HIT << "  ROW_BUFFER_MISS: " << setw(10) << uncore.DRAM.RQ.ROW_BUFFER_MISS;
    cout << "  DBUS_CONGESTED: " << setw(10) << uncore.DRAM.dbus_congested << endl; 
    cout << " WQ ROW_BUFFER_HIT: " << setw(10) << uncore.DRAM.WQ.ROW_BUFFER_HIT << "  ROW_BUFFER_MISS: " << setw(10) << uncore.DRAM.WQ.ROW_BUFFER_MISS;
    cout << "  FULL: " << setw(10) << uncore.DRAM.WQ.FULL << endl; 
}

void reset_cache_stats(uint32_t cpu, CACHE *cache)
{
    for (uint32_t i=0; i<NUM_TYPES; i++) {
        cache->ACCESS[i] = 0;
        cache->HIT[i] = 0;
        cache->MISS[i] = 0;
        cache->MSHR_MERGED[i] = 0;
        cache->STALL[i] = 0;

        cache->sim_access[cpu][i] = 0;
        cache->sim_hit[cpu][i] = 0;
        cache->sim_miss[cpu][i] = 0;
    }

    cache->RQ.ACCESS = 0;
    cache->RQ.MERGED = 0;
    cache->RQ.TO_CACHE = 0;

    cache->WQ.ACCESS = 0;
    cache->WQ.MERGED = 0;
    cache->WQ.TO_CACHE = 0;
    cache->WQ.FORWARD = 0;
    cache->WQ.FULL = 0;
}

void finish_warmup()
{
    uint64_t elapsed_second = (uint64_t)(time(NULL) - start_time),
             elapsed_minute = elapsed_second / 60,
             elapsed_hour = elapsed_minute / 60;
    elapsed_minute -= elapsed_hour*60;
    elapsed_second -= (elapsed_hour*3600 + elapsed_minute*60);

    // reset core latency
    SCHEDULING_LATENCY = 6;
    EXEC_LATENCY = 1;
    PAGE_TABLE_LATENCY = 100;
    SWAP_LATENCY = 100000;

    cout << endl;
    for (uint32_t i=0; i<NUM_CPUS; i++) {
        cout << "Warmup complete CPU " << i << " instructions: " << ooo_cpu[i].num_retired << " cycles: " << current_core_cycle[i];
        cout << " (Simulation time: " << elapsed_hour << " hr " << elapsed_minute << " min " << elapsed_second << " sec) " << endl;

        ooo_cpu[i].begin_sim_cycle = current_core_cycle[i]; 
        ooo_cpu[i].begin_sim_instr = ooo_cpu[i].num_retired;

        // reset branch stats
        ooo_cpu[i].num_branch = 0;
        ooo_cpu[i].branch_mispredictions = 0;

        reset_cache_stats(i, &ooo_cpu[i].L1I);
        reset_cache_stats(i, &ooo_cpu[i].L1D);
        reset_cache_stats(i, &ooo_cpu[i].L2C);
        reset_cache_stats(i, &uncore.LLC);
    }
    cout << endl;

    // reset DRAM stats
    uncore.DRAM.RQ.ROW_BUFFER_HIT = 0;
    uncore.DRAM.RQ.ROW_BUFFER_MISS = 0;
    uncore.DRAM.WQ.ROW_BUFFER_HIT = 0;
    uncore.DRAM.WQ.ROW_BUFFER_MISS = 0;

    // set actual cache latency
    for (uint32_t i=0; i<NUM_CPUS; i++) {
        ooo_cpu[i].ITLB.LATENCY = ITLB_LATENCY;
        ooo_cpu[i].DTLB.LATENCY = DTLB_LATENCY;
        ooo_cpu[i].STLB.LATENCY = STLB_LATENCY;
        ooo_cpu[i].L1I.LATENCY  = L1I_LATENCY;
        ooo_cpu[i].L1D.LATENCY  = L1D_LATENCY;
        ooo_cpu[i].L2C.LATENCY  = L2C_LATENCY;
    }
    uncore.LLC.LATENCY = LLC_LATENCY;
}

void print_deadlock(uint32_t i)
{
    cout << "DEADLOCK! CPU " << i << " instr_id: " << ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id;
    cout << " translated: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].translated;
    cout << " fetched: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].fetched;
    cout << " scheduled: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].scheduled;
    cout << " executed: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].executed;
    cout << " is_memory: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].is_memory;
    cout << " event: " << ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].event_cycle;
    cout << " current: " << current_core_cycle[i] << endl;

    // print LQ entry
    cout << endl << "Load Queue Entry" << endl;
    for (uint32_t j=0; j<LQ_SIZE; j++) {
        cout << "[LQ] entry: " << j << " instr_id: " << ooo_cpu[i].LQ.entry[j].instr_id << " address: " << hex << ooo_cpu[i].LQ.entry[j].physical_address << dec << " translated: " << +ooo_cpu[i].LQ.entry[j].translated << " fetched: " << +ooo_cpu[i].LQ.entry[i].fetched << endl;
    }

    // print SQ entry
    cout << endl << "Store Queue Entry" << endl;
    for (uint32_t j=0; j<SQ_SIZE; j++) {
        cout << "[SQ] entry: " << j << " instr_id: " << ooo_cpu[i].SQ.entry[j].instr_id << " address: " << hex << ooo_cpu[i].SQ.entry[j].physical_address << dec << " translated: " << +ooo_cpu[i].SQ.entry[j].translated << " fetched: " << +ooo_cpu[i].SQ.entry[i].fetched << endl;
    }

    // print L1D MSHR entry
    PACKET_QUEUE *queue;
    queue = &ooo_cpu[i].L1D.MSHR;
    cout << endl << queue->NAME << " Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl; 
    }

    assert(0);
}

void signal_handler(int signal) 
{
	cout << "Caught signal: " << signal << endl;
	exit(1);
}

int main(int argc, char** argv)
{
	// interrupt signal hanlder
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = signal_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

    cout << endl << "*** ChampSim Multicore Out-of-Order Simulator ***" << endl << endl;

    // initialize knobs
    uint8_t show_heartbeat = 1;

    uint32_t seed_number = 0;

    // FIXME
    // how to parse options?
    // set latency as zero during warmup

    // check to see if knobs changed using getopt_long()
    int c;
    while (1) {
        static struct option long_options[] =
        {
            {"warmup_instructions", required_argument, 0, 'w'},
            {"simulation_instructions", required_argument, 0, 'i'},
            {"hide_heartbeat", no_argument, 0, 'h'},
            {"scramble_loads", no_argument, 0, 's'},
            {"low_bandwidth",  no_argument, 0, 'b'},
            {"traces",  no_argument, 0, 't'},
            {0, 0, 0, 0}      
        };

        int option_index = 0;

        c = getopt_long_only(argc, argv, "wihsb", long_options, &option_index);

        // no more option characters
        if (c == -1)
            break;

        int traces_encountered = 0;

        switch(c) {
            case 'w':
                warmup_instructions = atoi(optarg);
                break;
            case 'i':
                simulation_instructions = atoi(optarg);
                break;
            case 'h':
                show_heartbeat = 0;
                break;
            //case 's':
            //    knob_scramble_loads = 1;
            //    break;
            case 'b':
                knob_low_bandwidth = 1;
                break;
            case 't':
                traces_encountered = 1;
                break;
            default:
                abort();
        }

        if (traces_encountered == 1)
            break;
    }

    // consequences of knobs
    cout << "Warmup Instructions: " << warmup_instructions << endl;
    cout << "Simulation Instructions: " << simulation_instructions << endl;
    cout << "Configuration: " << get_config_number() << endl;
    cout << "Number of CPUs: " << NUM_CPUS << endl;
    cout << "LLC sets: " << LLC_SET << endl;
    cout << "LLC ways: " << LLC_WAY << endl;

    if (knob_low_bandwidth) {
        DRAM_MTPS = 400;
    }
    else {
        DRAM_MTPS = 1600;
    }

    // DRAM access latency
    tRP  = tRP_DRAM_CYCLE  * (CPU_FREQ / DRAM_IO_FREQ); 
    tRCD = tRCD_DRAM_CYCLE * (CPU_FREQ / DRAM_IO_FREQ); 
    tCAS = tCAS_DRAM_CYCLE * (CPU_FREQ / DRAM_IO_FREQ); 

    // default: assuming 8B data request packet that takes a single transfer
    DRAM_ABUS_REQUEST_TIME = CPU_FREQ / DRAM_MTPS;

    // default: 16 = (64 / 8) * (3200 / 1600)
    // it takes 16 CPU cycles to tranfser 64B cache block on a 8B (64-bit) bus 
    // note that dram burst length = BLOCK_SIZE/DRAM_CHANNEL_WIDTH
    DRAM_DBUS_RETURN_TIME = (BLOCK_SIZE / DRAM_CHANNEL_WIDTH) * (CPU_FREQ / DRAM_MTPS);

    //printf("Off-chip DRAM Size: %u MB Channels: %u Width: %u-bit Data Rate: %u MT/s\n",
    //        DRAM_SIZE, DRAM_CHANNELS, 8*DRAM_CHANNEL_WIDTH, DRAM_MTPS);
    printf("Off-chip DRAM Channels: %u Width: %u-bit Data Rate: %u MT/s\n",
            DRAM_CHANNELS, 8*DRAM_CHANNEL_WIDTH, DRAM_MTPS);

    // end consequence of knobs

    // search through the argv for "-traces"
    int found_traces = 0;
    int count_traces = 0;
    cout << endl;
    for (int i=0; i<argc; i++) {
        if (found_traces) {
            printf("CPU %d runs %s\n", count_traces, argv[i]);

            sprintf(ooo_cpu[count_traces].trace_string, "%s", argv[i]);
            sprintf(ooo_cpu[count_traces].gunzip_command, "gunzip -c %s", argv[i]);

            char *pch[100];
            int count_str = 0;
            pch[0] = strtok (argv[i], " /,.-");
            while (pch[count_str] != NULL) {
                //printf ("%s %d\n", pch[count_str], count_str);
                count_str++;
                pch[count_str] = strtok (NULL, " /,.-");
            }

            //printf("max count_str: %d\n", count_str);
            //printf("application: %s\n", pch[count_str-3]);

            int j = 0;
            while (pch[count_str-3][j] != '\0') {
                seed_number += pch[count_str-3][j];
                //printf("%c %d %d\n", pch[count_str-3][j], j, seed_number);
                j++;
            }

            ooo_cpu[count_traces].trace_file = popen(ooo_cpu[count_traces].gunzip_command, "r");
            if(ooo_cpu[count_traces].trace_file == NULL) {
                printf("\n*** Trace file not found: %s ***\n\n", argv[i]);
                assert(0);
            }

            count_traces++;
            if (count_traces > NUM_CPUS) {
                eprintf("\n*** Too many traces for the configured number of cores ***\n\n");
                assert(0);
            }
        }
        else if(strcmp(argv[i],"-traces") == 0) {
            found_traces = 1;
        }
    }

    if (count_traces != NUM_CPUS) {
        eprintf("\n*** Not enough traces for the configured number of cores ***\n\n");
        assert(0);
    }
    // end trace file setup

    // TODO: can we initialize these variables from the class constructor?
    srand(seed_number);
    for (int i=0; i<NUM_CPUS; i++) {

        ooo_cpu[i].cpu = i; 
        ooo_cpu[i].warmup_instructions = warmup_instructions;
        ooo_cpu[i].simulation_instructions = simulation_instructions;
        ooo_cpu[i].begin_sim_cycle = 0; 
        ooo_cpu[i].begin_sim_instr = warmup_instructions;

        // ROB
        ooo_cpu[i].ROB.cpu = i;

        // BRANCH PREDICTOR
        ooo_cpu[i].initialize_branch_predictor();

        // TLBs
        ooo_cpu[i].ITLB.cpu = i;
        ooo_cpu[i].ITLB.cache_type = IS_ITLB;
        ooo_cpu[i].ITLB.fill_level = FILL_L1;
        ooo_cpu[i].ITLB.extra_interface = &ooo_cpu[i].L1I;
        ooo_cpu[i].ITLB.lower_level = &ooo_cpu[i].STLB; 

        ooo_cpu[i].DTLB.cpu = i;
        ooo_cpu[i].DTLB.cache_type = IS_DTLB;
        ooo_cpu[i].DTLB.MAX_READ = (2 > MAX_READ_PER_CYCLE) ? MAX_READ_PER_CYCLE : 2;
        ooo_cpu[i].DTLB.fill_level = FILL_L1;
        ooo_cpu[i].DTLB.extra_interface = &ooo_cpu[i].L1D;
        ooo_cpu[i].DTLB.lower_level = &ooo_cpu[i].STLB;

        ooo_cpu[i].STLB.cpu = i;
        ooo_cpu[i].STLB.cache_type = IS_STLB;
        ooo_cpu[i].STLB.fill_level = FILL_L2;
        ooo_cpu[i].STLB.upper_level_icache[i] = &ooo_cpu[i].ITLB;
        ooo_cpu[i].STLB.upper_level_dcache[i] = &ooo_cpu[i].DTLB;

        // PRIVATE CACHE
        ooo_cpu[i].L1I.cpu = i;
        ooo_cpu[i].L1I.cache_type = IS_L1I;
        ooo_cpu[i].L1I.MAX_READ = (FETCH_WIDTH > MAX_READ_PER_CYCLE) ? MAX_READ_PER_CYCLE : FETCH_WIDTH;
        ooo_cpu[i].L1I.fill_level = FILL_L1;
        ooo_cpu[i].L1I.lower_level = &ooo_cpu[i].L2C; 

        ooo_cpu[i].L1D.cpu = i;
        ooo_cpu[i].L1D.cache_type = IS_L1D;
        ooo_cpu[i].L1D.MAX_READ = (2 > MAX_READ_PER_CYCLE) ? MAX_READ_PER_CYCLE : 2;
        ooo_cpu[i].L1D.fill_level = FILL_L1;
        ooo_cpu[i].L1D.lower_level = &ooo_cpu[i].L2C; 
        ooo_cpu[i].L1D.l1d_prefetcher_initialize();

        ooo_cpu[i].L2C.cpu = i;
        ooo_cpu[i].L2C.cache_type = IS_L2C;
        ooo_cpu[i].L2C.fill_level = FILL_L2;
        ooo_cpu[i].L2C.upper_level_icache[i] = &ooo_cpu[i].L1I;
        ooo_cpu[i].L2C.upper_level_dcache[i] = &ooo_cpu[i].L1D;
        ooo_cpu[i].L2C.lower_level = &uncore.LLC;
        ooo_cpu[i].L2C.l2c_prefetcher_initialize();

        // SHARED CACHE
        uncore.LLC.cache_type = IS_LLC;
        uncore.LLC.fill_level = FILL_LLC;
        uncore.LLC.upper_level_icache[i] = &ooo_cpu[i].L2C;
        uncore.LLC.upper_level_dcache[i] = &ooo_cpu[i].L2C;
        uncore.LLC.lower_level = &uncore.DRAM;

        // OFF-CHIP DRAM
        uncore.DRAM.fill_level = FILL_DRAM;
        uncore.DRAM.upper_level_icache[i] = &uncore.LLC;
        uncore.DRAM.upper_level_dcache[i] = &uncore.LLC;
        uncore.DRAM.RQ.is_RQ = 1;
        uncore.DRAM.WQ.is_WQ = 1;

        warmup_complete[i] = 0;
        //all_warmup_complete = NUM_CPUS;
        simulation_complete[i] = 0;
        current_core_cycle[i] = 0;
        stall_cycle[i] = 0;

        generator[i].seed(seed_number);
    }

#ifdef CRC2_COMPILE
    InitReplacementState(),
#else
    uncore.LLC.llc_initialize_replacement();
#endif

    // simulation entry point
    start_time = time(NULL);
    uint8_t run_simulation = 1;
    while (run_simulation) {

        uint64_t elapsed_second = (uint64_t)(time(NULL) - start_time),
                 elapsed_minute = elapsed_second / 60,
                 elapsed_hour = elapsed_minute / 60;
        elapsed_minute -= elapsed_hour*60;
        elapsed_second -= (elapsed_hour*3600 + elapsed_minute*60);

        for (int i=0; i<NUM_CPUS; i++) {
            // proceed one cycle
            current_core_cycle[i]++;

            //cout << "Trying to process instr_id: " << ooo_cpu[i].instr_unique_id << " fetch_stall: " << +ooo_cpu[i].fetch_stall;
            //cout << " stall_cycle: " << stall_cycle[i] << " current: " << current_core_cycle[i] << endl;

            // core might be stalled due to page fault or branch misprediction
            if (stall_cycle[i] <= current_core_cycle[i]) {

                // fetch unit
                if (ooo_cpu[i].ROB.occupancy < ooo_cpu[i].ROB.SIZE) {
                    // handle branch
                    if (ooo_cpu[i].fetch_stall == 0)
                        ooo_cpu[i].handle_branch();

                    // fetch
                    ooo_cpu[i].fetch_instruction();
                }


                // schedule (including decode latency)
                uint32_t schedule_index = ooo_cpu[i].ROB.next_schedule;
                if ((ooo_cpu[i].ROB.entry[schedule_index].scheduled == 0) && (ooo_cpu[i].ROB.entry[schedule_index].event_cycle <= current_core_cycle[i]))
                    ooo_cpu[i].schedule_instruction();

                // execute
                ooo_cpu[i].execute_instruction();

                // memory operation
                ooo_cpu[i].schedule_memory_instruction();
                ooo_cpu[i].execute_memory_instruction();

                // complete 
                ooo_cpu[i].update_rob();

                // retire
                if ((ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].executed == COMPLETED) && (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].event_cycle <= current_core_cycle[i]))
                    ooo_cpu[i].retire_rob();
            }

            // heartbeat information
            if (show_heartbeat && (ooo_cpu[i].num_retired >= ooo_cpu[i].next_print_instruction)) {
                float cumulative_ipc;
                if (warmup_complete[i])
                    cumulative_ipc = (1.0*(ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr)) / (current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle);
                else
                    cumulative_ipc = (1.0*ooo_cpu[i].num_retired) / current_core_cycle[i];
                float heartbeat_ipc = (1.0*ooo_cpu[i].num_retired - ooo_cpu[i].last_sim_instr) / (current_core_cycle[i] - ooo_cpu[i].last_sim_cycle);

                cout << "Heartbeat CPU " << i << " instructions: " << ooo_cpu[i].num_retired << " cycles: " << current_core_cycle[i];
                cout << " heartbeat IPC: " << heartbeat_ipc << " cumulative IPC: " << cumulative_ipc; 
                cout << " (Simulation time: " << elapsed_hour << " hr " << elapsed_minute << " min " << elapsed_second << " sec) " << endl;
                ooo_cpu[i].next_print_instruction += STAT_PRINTING_PERIOD;

                ooo_cpu[i].last_sim_instr = ooo_cpu[i].num_retired;
                ooo_cpu[i].last_sim_cycle = current_core_cycle[i];

#ifdef CRC2_COMPILE
                PrintStats_Heartbeat();
#endif
            }

            // check for deadlock
            if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].ip && (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].event_cycle + DEADLOCK_CYCLE) <= current_core_cycle[i])
                print_deadlock(i);

            // warmup complete
            if ((warmup_complete[i] == 0) && (ooo_cpu[i].num_retired > warmup_instructions)) {
                warmup_complete[i] = 1;
                all_warmup_complete++;
            }
            if (all_warmup_complete == NUM_CPUS) { // this part is called only once when all cores are warmed up
                all_warmup_complete++;
                finish_warmup();
            }
            /*
            if ((all_warmup_complete == 0) && (ooo_cpu[i].num_retired > 10000000)) {
                all_warmup_complete = 1;
                finish_warmup();
            }
            if (ooo_cpu[0].num_retired > 10001300)
                warmup_complete[0] = 1;
            */
            
            // simulation complete
            if ((all_warmup_complete > NUM_CPUS) && (simulation_complete[i] == 0) && (ooo_cpu[i].num_retired >= (ooo_cpu[i].begin_sim_instr + ooo_cpu[i].simulation_instructions))) {
                simulation_complete[i] = 1;
                ooo_cpu[i].finish_sim_instr = ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr;
                ooo_cpu[i].finish_sim_cycle = current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle;

                cout << "Finished CPU " << i << " instructions: " << ooo_cpu[i].finish_sim_instr << " cycles: " << ooo_cpu[i].finish_sim_cycle;
                cout << " cummulative IPC: " << ((float) ooo_cpu[i].finish_sim_instr / ooo_cpu[i].finish_sim_cycle);
                cout << " (Simulation time: " << elapsed_hour << " hr " << elapsed_minute << " min " << elapsed_second << " sec) " << endl;

                record_roi_stats(i, &ooo_cpu[i].L1D);
                record_roi_stats(i, &ooo_cpu[i].L1I);
                record_roi_stats(i, &ooo_cpu[i].L2C);
                record_roi_stats(i, &uncore.LLC);

                all_simulation_complete++;
            }

            if (all_simulation_complete == NUM_CPUS)
                run_simulation = 0;
        }

        // TODO: should it be backward?
        uncore.LLC.operate();
        uncore.DRAM.operate();
    }

#ifndef CRC2_COMPILE
    print_branch_stats();
#endif
    uint64_t elapsed_second = (uint64_t)(time(NULL) - start_time),
             elapsed_minute = elapsed_second / 60,
             elapsed_hour = elapsed_minute / 60;
    elapsed_minute -= elapsed_hour*60;
    elapsed_second -= (elapsed_hour*3600 + elapsed_minute*60);
    
    cout << endl << "ChampSim completed all CPUs" << endl;
    if (NUM_CPUS > 1) {
        cout << endl << "Total Simulation Statistics" << endl;
        for (uint32_t i=0; i<NUM_CPUS; i++) {
            cout << endl << "CPU " << i << " cummulative IPC: " << (float) (ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr) / (current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle); 
            cout << " instructions: " << ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr << " cycles: " << current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle << endl;
#ifndef CRC2_COMPILE
            print_sim_stats(i, &ooo_cpu[i].L1D);
            print_sim_stats(i, &ooo_cpu[i].L1I);
            print_sim_stats(i, &ooo_cpu[i].L2C);
            ooo_cpu[i].L1D.l1d_prefetcher_final_stats();
            ooo_cpu[i].L2C.l2c_prefetcher_final_stats();
#endif
            print_sim_stats(i, &uncore.LLC);
        }
    }

    cout << endl << "Region of Interest Statistics" << endl;
    for (uint32_t i=0; i<NUM_CPUS; i++) {
        cout << endl << "CPU " << i << " cummulative IPC: " << ((float) ooo_cpu[i].finish_sim_instr / ooo_cpu[i].finish_sim_cycle); 
        cout << " instructions: " << ooo_cpu[i].finish_sim_instr << " cycles: " << ooo_cpu[i].finish_sim_cycle << endl;
#ifndef CRC2_COMPILE
        print_roi_stats(i, &ooo_cpu[i].L1D);
        print_roi_stats(i, &ooo_cpu[i].L1I);
        print_roi_stats(i, &ooo_cpu[i].L2C);
#endif
        print_roi_stats(i, &uncore.LLC);
    }

#ifdef CRC2_COMPILE
     PrintStats();
#endif

#ifndef CRC2_COMPILE
    print_dram_stats();
#endif

    return 0;
}
