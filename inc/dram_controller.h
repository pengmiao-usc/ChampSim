#ifndef DRAM_H
#define DRAM_H

#include "memory_class.h"

// DRAM configuration
#define DRAM_CHANNEL_WIDTH 8 // 8B
#define DRAM_WQ_SIZE 48
#define DRAM_RQ_SIZE 48

#define tRP_DRAM_CYCLE  11 
#define tRCD_DRAM_CYCLE 11
#define tCAS_DRAM_CYCLE 11

// the data bus must wait this amount of time when switching between reads and writes, and vice versa
#define DRAM_DBUS_TURN_AROUND_TIME ((15*CPU_FREQ)/2000) // 7.5 ns 
extern uint32_t DRAM_MTPS, DRAM_ABUS_REQUEST_TIME, DRAM_DBUS_RETURN_TIME;

// these values control when to send out a burst of writes
#define DRAM_WRITE_HIGH_WM    (DRAM_WQ_SIZE*3/4)
#define DRAM_WRITE_LOW_WM     (DRAM_WQ_SIZE*1/4)
#define MIN_DRAM_WRITES_PER_SWITCH (DRAM_WQ_SIZE*1/4)

// DRAM
class MEMORY_CONTROLLER : public MEMORY {
  public:
    const string NAME;

    DRAM_ARRAY dram_array[DRAM_CHANNELS][DRAM_RANKS][DRAM_BANKS];
    uint64_t dbus_cycle_available[DRAM_CHANNELS], dbus_congested;
    uint64_t bank_cycle_available[DRAM_CHANNELS][DRAM_RANKS][DRAM_BANKS];
    uint8_t  do_write, write_mode[DRAM_CHANNELS]; 
    uint32_t processed_writes, scheduled_reads, scheduled_writes;
    int fill_level;

    BANK_REQUEST bank_request[DRAM_CHANNELS][DRAM_RANKS][DRAM_BANKS];

    // constructor
    MEMORY_CONTROLLER(string v1) : NAME (v1) {
        dbus_congested = 0;
        do_write = 0;
        processed_writes = 0;
        scheduled_reads = 0;
        scheduled_writes = 0;
        for (uint32_t i=0; i<DRAM_CHANNELS; i++) {
            dbus_cycle_available[i] = 0;
            write_mode[i] = 0;

            for (uint32_t j=0; j<DRAM_RANKS; j++) {
                for (uint32_t k=0; k<DRAM_BANKS; k++)
                    bank_cycle_available[i][j][k] = 0;
            }
        }

        fill_level = FILL_DRAM;
    };

    // destructor
    ~MEMORY_CONTROLLER() {

    };

    // queues
    PACKET_QUEUE WQ{NAME + "_WQ", DRAM_WQ_SIZE}, // write queue
                 RQ{NAME + "_RQ", DRAM_RQ_SIZE}; // read queue

    // functions
    virtual int  add_rq(PACKET *packet);
    virtual int  add_wq(PACKET *packet);
    virtual int  add_pq(PACKET *packet);
    virtual void return_data(PACKET *packet);
    virtual void operate();
    virtual void increment_WQ_FULL();
    virtual uint32_t get_occupancy(uint8_t queue_type);
    virtual uint32_t get_size(uint8_t queue_type);

    void schedule(PACKET_QUEUE *queue), process(PACKET_QUEUE *queue),
         update_schedule_cycle(PACKET_QUEUE *queue),
         update_process_cycle(PACKET_QUEUE *queue),
         reset_remain_requests(PACKET_QUEUE *queue);

    uint32_t dram_get_channel(uint64_t address),
             dram_get_rank   (uint64_t address),
             dram_get_bank   (uint64_t address),
             dram_get_row    (uint64_t address),
             dram_get_column (uint64_t address),
             drc_check_hit (uint64_t address, uint32_t cpu, uint32_t channel, uint32_t rank, uint32_t bank, uint32_t row);

    uint64_t get_bank_earliest_cycle();

    int check_dram_queue(PACKET_QUEUE *queue, PACKET *packet);
};

#endif
