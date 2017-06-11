#include "dram_controller.h"

// initialized in main.cc
uint32_t DRAM_MTPS, DRAM_ABUS_REQUEST_TIME, DRAM_DBUS_RETURN_TIME,
         tRP, tRCD, tCAS;

void MEMORY_CONTROLLER::reset_remain_requests(PACKET_QUEUE *queue)
{
    for (uint32_t i=0; i<queue->SIZE; i++) {
        if (queue->entry[i].scheduled) {

            uint64_t op_addr = queue->entry[i].address;
            uint32_t op_cpu = queue->entry[i].cpu,
                     op_channel = dram_get_channel(op_addr), 
                     op_rank = dram_get_rank(op_addr), 
                     op_bank = dram_get_bank(op_addr), 
                     op_row = dram_get_row(op_addr);

#ifdef DEBUG_PRINT
            //uint32_t op_column = dram_get_column(op_addr);
#endif

            // update open row
            if ((bank_request[op_channel][op_rank][op_bank].cycle_available - tCAS) <= current_core_cycle[op_cpu])
                bank_request[op_channel][op_rank][op_bank].open_row = op_row;
            else
                bank_request[op_channel][op_rank][op_bank].open_row = UINT32_MAX;

            // this bank is ready for another DRAM request
            bank_request[op_channel][op_rank][op_bank].request_index = -1;
            bank_request[op_channel][op_rank][op_bank].row_buffer_hit = 0;
            bank_request[op_channel][op_rank][op_bank].working = 0;
            bank_request[op_channel][op_rank][op_bank].cycle_available = current_core_cycle[op_cpu];
            if (bank_request[op_channel][op_rank][op_bank].is_write) {
                scheduled_writes--;
                bank_request[op_channel][op_rank][op_bank].is_write = 0;
            }
            else if (bank_request[op_channel][op_rank][op_bank].is_read) {
                scheduled_reads--;
                bank_request[op_channel][op_rank][op_bank].is_read = 0;
            }

            queue->entry[i].scheduled = 0;
            queue->entry[i].event_cycle = current_core_cycle[op_cpu];

            DP ( if (warmup_complete[op_cpu]) {
            cout << queue->NAME << " instr_id: " << queue->entry[i].instr_id << " swrites: " << scheduled_writes << " sreads: " << scheduled_reads << endl; });

        }
    }
    
    update_schedule_cycle(&RQ);
    update_schedule_cycle(&WQ);
    update_process_cycle(&RQ);
    update_process_cycle(&WQ);

#ifdef SANITY_CHECK
    if (queue->is_WQ) {
        if (scheduled_writes != 0)
            assert(0);
    }
    else {
        if (scheduled_reads != 0)
            assert(0);
    }
#endif
}

void MEMORY_CONTROLLER::operate()
{
    if (do_write == 0) {
        if (WQ.occupancy >= DRAM_WRITE_HIGH_WM) {
            do_write = 1;
            processed_writes = 0;

            // reset scheduled RQ requests
            reset_remain_requests(&RQ);

            for (uint32_t i=0; i<DRAM_CHANNELS; i++) {
                // add data bus turnaround time
                dbus_cycle_available[i] += DRAM_DBUS_TURN_AROUND_TIME;
            }
        }
    }
    else {
        if (WQ.occupancy == 0)
            do_write = 0;
        else if (RQ.occupancy && (WQ.occupancy < DRAM_WRITE_LOW_WM))
            do_write = 0;
        else if (RQ.occupancy && (MIN_DRAM_WRITES_PER_SWITCH <= processed_writes))
            do_write = 0;

        if (do_write == 0) {
            processed_writes = 0;

            // reset scheduled WQ requests
            reset_remain_requests(&WQ);

            for (uint32_t i=0; i<DRAM_CHANNELS; i++) {
                // add data bus turnaround time
                dbus_cycle_available[i] += DRAM_DBUS_TURN_AROUND_TIME;
            }
        }
    }

    // do FR-FCFS scheduling

    // handle write
    // schedule new entry
    if (do_write && (WQ.next_schedule_index < WQ.SIZE)) {
        if (WQ.next_schedule_cycle <= current_core_cycle[WQ.entry[WQ.next_schedule_index].cpu])
            schedule(&WQ);
    }

    // process DRAM requests
    if (do_write && (WQ.next_process_index < WQ.SIZE)) {
        if (WQ.next_process_cycle <= current_core_cycle[WQ.entry[WQ.next_process_index].cpu])
            process(&WQ);
    }

    // handle read
    // schedule new entry
    if ((do_write == 0) && (RQ.next_schedule_index < RQ.SIZE)) {
        if (RQ.next_schedule_cycle <= current_core_cycle[RQ.entry[RQ.next_schedule_index].cpu])
            schedule(&RQ);
    }

    // process DRAM requests
    if ((do_write == 0) && (RQ.next_process_index < RQ.SIZE)) {
        if (RQ.next_process_cycle <= current_core_cycle[RQ.entry[RQ.next_process_index].cpu])
            process(&RQ);
    }
}

void MEMORY_CONTROLLER::schedule(PACKET_QUEUE *queue)
{
    uint64_t read_addr;
    uint32_t read_channel, read_rank, read_bank, read_row;
    uint8_t  row_buffer_hit = 0;

    int oldest_index = -1;
    uint64_t oldest_cycle = UINT64_MAX;

    // first, search for the oldest open row hit
    for (uint32_t i=0; i<queue->SIZE; i++) {

        // already scheduled
        if (queue->entry[i].scheduled) 
            continue;

        // empty entry
        read_addr = queue->entry[i].address;
        if (read_addr == 0) 
            continue;

        read_channel = dram_get_channel(read_addr);
        read_rank = dram_get_rank(read_addr);
        read_bank = dram_get_bank(read_addr);

        // bank is busy
        if (bank_request[read_channel][read_rank][read_bank].working) { // should we check this or not? how do we know if bank is busy or not for all requests in the queue?

            //DP ( if (warmup_complete[0]) {
            //cout << queue->NAME << " " << __func__ << " instr_id: " << queue->entry[i].instr_id << " bank is busy";
            //cout << " swrites: " << scheduled_writes << " sreads: " << scheduled_reads;
            //cout << " write: " << +bank_request[read_channel][read_rank][read_bank].is_write << " read: " << +bank_request[read_channel][read_rank][read_bank].is_read << hex;
            //cout << " address: " << queue->entry[i].address << dec << " channel: " << read_channel << " rank: " << read_rank << " bank: " << read_bank << endl; });

            continue;
        }

        read_row = dram_get_row(read_addr);
        //read_column = dram_get_column(read_addr);

        // check open row
        if (bank_request[read_channel][read_rank][read_bank].open_row != read_row) {

            /*
            DP ( if (warmup_complete[0]) {
            cout << queue->NAME << " " << __func__ << " instr_id: " << queue->entry[i].instr_id << " row is inactive";
            cout << " swrites: " << scheduled_writes << " sreads: " << scheduled_reads;
            cout << " write: " << +bank_request[read_channel][read_rank][read_bank].is_write << " read: " << +bank_request[read_channel][read_rank][read_bank].is_read << hex;
            cout << " address: " << queue->entry[i].address << dec << " channel: " << read_channel << " rank: " << read_rank << " bank: " << read_bank << endl; });
            */

            continue;
        }

        // select the oldest entry
        if (queue->entry[i].event_cycle < oldest_cycle) {
            oldest_cycle = queue->entry[i].event_cycle;
            oldest_index = i;
            row_buffer_hit = 1;
        }	  
    }

    if (oldest_index == -1) { // no matching open_row (row buffer miss)

        oldest_cycle = UINT64_MAX;
        for (uint32_t i=0; i<queue->SIZE; i++) {

            // already scheduled
            if (queue->entry[i].scheduled)
                continue;

            // empty entry
            read_addr = queue->entry[i].address;
            if (read_addr == 0) 
                continue;

            // bank is busy
            read_channel = dram_get_channel(read_addr);
            read_rank = dram_get_rank(read_addr);
            read_bank = dram_get_bank(read_addr);
            if (bank_request[read_channel][read_rank][read_bank].working) 
                continue;

            //read_row = dram_get_row(read_addr);
            //read_column = dram_get_column(read_addr);

            // select the oldest entry
            if (queue->entry[i].event_cycle < oldest_cycle) {
                oldest_cycle = queue->entry[i].event_cycle;
                oldest_index = i;
            }
        }
    }

    // at this point, the scheduler knows which bank to access and if the request is a row buffer hit or miss
    if (oldest_index != -1) { // scheduler might not find anything if all requests are already scheduled or all banks are busy

        uint64_t LATENCY = 0;
        if (row_buffer_hit)  
            LATENCY = tCAS;
        else 
            LATENCY = tRP + tRCD + tCAS;

        uint64_t op_addr = queue->entry[oldest_index].address;
        uint32_t op_cpu = queue->entry[oldest_index].cpu,
                 op_channel = dram_get_channel(op_addr), 
                 op_rank = dram_get_rank(op_addr), 
                 op_bank = dram_get_bank(op_addr), 
                 op_row = dram_get_row(op_addr);
#ifdef DEBUG_PRINT
        uint32_t op_column = dram_get_column(op_addr);
#endif

        // this bank is now busy
        bank_request[op_channel][op_rank][op_bank].working = 1;
        bank_request[op_channel][op_rank][op_bank].cycle_available = current_core_cycle[op_cpu] + LATENCY;

        bank_request[op_channel][op_rank][op_bank].request_index = oldest_index;
        bank_request[op_channel][op_rank][op_bank].row_buffer_hit = row_buffer_hit;
        if (queue->is_WQ) {
            bank_request[op_channel][op_rank][op_bank].is_write = 1;
            bank_request[op_channel][op_rank][op_bank].is_read = 0;
            scheduled_writes++;
        }
        else {
            bank_request[op_channel][op_rank][op_bank].is_write = 0;
            bank_request[op_channel][op_rank][op_bank].is_read = 1;
            scheduled_reads++;
        }

        // update open row
        bank_request[op_channel][op_rank][op_bank].open_row = op_row;

        queue->entry[oldest_index].scheduled = 1;
        queue->entry[oldest_index].event_cycle = current_core_cycle[op_cpu] + LATENCY;

        update_schedule_cycle(queue);
        update_process_cycle(queue);

        DP (if (warmup_complete[op_cpu]) {
        cout << "[" << queue->NAME << "] " <<  __func__ << " instr_id: " << queue->entry[oldest_index].instr_id;
        cout << " row buffer: " << (row_buffer_hit ? (int)bank_request[op_channel][op_rank][op_bank].open_row : -1) << hex;
        cout << " address: " << queue->entry[oldest_index].address << " full_addr: " << queue->entry[oldest_index].full_addr << dec;
        cout << " index: " << oldest_index << " occupancy: " << queue->occupancy;
        cout << " ch: " << op_channel << " rank: " << op_rank << " bank: " << op_bank; // wrong from here
        cout << " row: " << op_row << " col: " << op_column;
        cout << " current: " << current_core_cycle[op_cpu] << " event: " << queue->entry[oldest_index].event_cycle << endl; });
    }
}

void MEMORY_CONTROLLER::process(PACKET_QUEUE *queue)
{
    uint32_t request_index = queue->next_process_index;

    // sanity check
    if (request_index == queue->SIZE)
        assert(0);

    uint64_t op_addr = queue->entry[request_index].address;
    uint32_t op_cpu = queue->entry[request_index].cpu,
             op_channel = dram_get_channel(op_addr), 
             op_rank = dram_get_rank(op_addr), 
             op_bank = dram_get_bank(op_addr);
#ifdef DEBUG_PRINT
    uint32_t op_row = dram_get_row(op_addr), 
             op_column = dram_get_column(op_addr);
#endif

    // sanity check
    if (bank_request[op_channel][op_rank][op_bank].request_index != (int)request_index) {
        assert(0);
    }

    // paid all DRAM access latency, data is ready to be processed
    if (bank_request[op_channel][op_rank][op_bank].cycle_available <= current_core_cycle[op_cpu]) {

        // check if data bus is available
        if (dbus_cycle_available[op_channel] <= current_core_cycle[op_cpu]) {

            if (queue->is_WQ) {
                if (bank_request[op_channel][op_rank][op_bank].row_buffer_hit)
                    queue->ROW_BUFFER_HIT++;
                else
                    queue->ROW_BUFFER_MISS++;

                // this bank is ready for another DRAM request
                bank_request[op_channel][op_rank][op_bank].request_index = -1;
                bank_request[op_channel][op_rank][op_bank].row_buffer_hit = 0;
                bank_request[op_channel][op_rank][op_bank].working = false;
                bank_request[op_channel][op_rank][op_bank].is_write = 0;
                bank_request[op_channel][op_rank][op_bank].is_read = 0;

                scheduled_writes--;
            }
            else {
#ifdef DRAM_CACHE
                // DRAM cannot return data if DRC WQ is full
                if (upper_level_dcache[op_cpu]->get_occupancy(2) == upper_level_dcache[op_cpu]->get_size(2))
                    return;
#endif
                // update data bus cycle time
                dbus_cycle_available[op_channel] = current_core_cycle[op_cpu] + DRAM_DBUS_RETURN_TIME;
                queue->entry[request_index].event_cycle = dbus_cycle_available[op_channel]; 

                DP ( if (warmup_complete[op_cpu]) {
                cout << "[" << queue->NAME << "] " <<  __func__ << " return data" << hex;
                cout << " address: " << queue->entry[request_index].address << " full_addr: " << queue->entry[request_index].full_addr << dec;
                cout << " occupancy: " << queue->occupancy << " channel: " << op_channel << " rank: " << op_rank << " bank: " << op_bank;
                cout << " row: " << op_row << " column: " << op_column;
                cout << " current_cycle: " << current_core_cycle[op_cpu] << " event_cycle: " << queue->entry[request_index].event_cycle << endl; });

                // send data back to the core cache hierarchy
                upper_level_dcache[op_cpu]->return_data(&queue->entry[request_index]);

                if (bank_request[op_channel][op_rank][op_bank].row_buffer_hit)
                    queue->ROW_BUFFER_HIT++;
                else
                    queue->ROW_BUFFER_MISS++;

                // this bank is ready for another DRAM request
                bank_request[op_channel][op_rank][op_bank].request_index = -1;
                bank_request[op_channel][op_rank][op_bank].row_buffer_hit = 0;
                bank_request[op_channel][op_rank][op_bank].working = false;
                bank_request[op_channel][op_rank][op_bank].is_write = 0;
                bank_request[op_channel][op_rank][op_bank].is_read = 0;

                scheduled_reads--;
            }

            // remove the oldest entry
            queue->remove_queue(&queue->entry[request_index]);
            update_process_cycle(queue);
        }
        else { // data bus is busy, the available bank cycle time is fast-forwarded for faster simulation
            bank_request[op_channel][op_rank][op_bank].cycle_available = dbus_cycle_available[op_channel];
            dbus_congested++;

            DP ( if (warmup_complete[op_cpu]) {
            cout << "[" << queue->NAME << "] " <<  __func__ << " dbus_occupied" << hex;
            cout << " address: " << queue->entry[request_index].address << " full_addr: " << queue->entry[request_index].full_addr << dec;
            cout << " occupancy: " << queue->occupancy << " channel: " << op_channel << " rank: " << op_rank << " bank: " << op_bank;
            cout << " row: " << op_row << " column: " << op_column;
            cout << " current_cycle: " << current_core_cycle[op_cpu] << " event_cycle: " << bank_request[op_channel][op_rank][op_bank].cycle_available << endl; });
        }
    }

    // TODO: Maybe we can handle the second oldest request something like that? 
    // If the bank is busy.. Was there some kind of job before? (Check DRAM scheduling paper)
    // How many times do we stuck by "waited enough but bank is still busy" scenario?

    // TODO: sometime channel is idel since tCAS takes 18 cycles and data transfer across channel takes only 16 cycles
    // if we use a wider channel the channel idle problem becomes more critical
}

int MEMORY_CONTROLLER::add_rq(PACKET *packet)
{
    // simply return read requests with dummy response before the warmup
    if (all_warmup_complete < NUM_CPUS) {
        if (packet->instruction) 
            upper_level_icache[packet->cpu]->return_data(packet);
        else // data
            upper_level_dcache[packet->cpu]->return_data(packet);

        return -1;
    }

    // check for the latest wirtebacks in the write queue
    //int wq_index = WQ.check_queue(packet);
    int wq_index = check_dram_queue(&WQ, packet);
    if (wq_index != -1) {
        
        // no need to check fill level
        //if (packet->fill_level < fill_level) {

            packet->data = WQ.entry[wq_index].data;
            if (packet->instruction) 
                upper_level_icache[packet->cpu]->return_data(packet);
            else // data
                upper_level_dcache[packet->cpu]->return_data(packet);
        //}

        DP ( if (packet->cpu) {
        cout << "[" << NAME << "_RQ] " << __func__ << " instr_id: " << packet->instr_id << " found recent writebacks";
        cout << hex << " read: " << packet->address << " writeback: " << WQ.entry[wq_index].address << dec << endl; });

        ACCESS[1]++;
        HIT[1]++;

        WQ.FORWARD++;
        RQ.ACCESS++;
        //assert(0);

        return -1;
    }

    // check for duplicates in the read queue
    //int index = RQ.check_queue(packet);
    int index = check_dram_queue(&RQ, packet);
    if (index != -1)
        return index; // merged index

    // search for the empty index
    for (index=0; index<DRAM_RQ_SIZE; index++) {
        if (RQ.entry[index].address == 0) {
            
            memcpy(&RQ.entry[index], packet, sizeof(PACKET));
            RQ.occupancy++;

#ifdef DEBUG_PRINT
            uint32_t channel = dram_get_channel(packet->address),
                     rank = dram_get_rank(packet->address),
                     bank = dram_get_bank(packet->address),
                     row = dram_get_row(packet->address),
                     column = dram_get_column(packet->address); 
#endif

            DP ( if(warmup_complete[packet->cpu]) {
            cout << "[" << NAME << "_RQ] " <<  __func__ << " instr_id: " << packet->instr_id << " address: " << hex << packet->address;
            cout << " full_addr: " << packet->full_addr << dec << " ch: " << channel;
            cout << " rank: " << rank << " bank: " << bank << " row: " << row << " col: " << column;
            cout << " occupancy: " << RQ.occupancy << " current: " << current_core_cycle[packet->cpu] << " event: " << packet->event_cycle << endl; });

            break;
        }
    }

    update_schedule_cycle(&RQ);

    return -1;
}

int MEMORY_CONTROLLER::add_wq(PACKET *packet)
{
    // simply drop write requests before the warmup
    if (all_warmup_complete < NUM_CPUS)
        return -1;

    // check for duplicates in the write queue
    //int index = WQ.check_queue(packet);
    int index = check_dram_queue(&WQ, packet);
    if (index != -1)
        return index; // merged index

    // search for the empty index
    for (index=0; index<DRAM_WQ_SIZE; index++) {
        if (WQ.entry[index].address == 0) {
            
            memcpy(&WQ.entry[index], packet, sizeof(PACKET));
            WQ.occupancy++;

#ifdef DEBUG_PRINT
            uint32_t channel = dram_get_channel(packet->address),
                     rank = dram_get_rank(packet->address),
                     bank = dram_get_bank(packet->address),
                     row = dram_get_row(packet->address),
                     column = dram_get_column(packet->address); 
#endif

            DP ( if(warmup_complete[packet->cpu]) {
            cout << "[" << NAME << "_WQ] " <<  __func__ << " instr_id: " << packet->instr_id << " address: " << hex << packet->address;
            cout << " full_addr: " << packet->full_addr << dec << " ch: " << channel;
            cout << " rank: " << rank << " bank: " << bank << " row: " << row << " col: " << column;
            cout << " occupancy: " << WQ.occupancy << " current: " << current_core_cycle[packet->cpu] << " event: " << packet->event_cycle << endl; });

            break;
        }
    }

    /*
    //if (WQ.occupancy == 1) { // queue was empty and we are adding the first entry

        // update event time
        WQ.event_cycle = packet->event_cycle;
        WQ.event_cpu = packet->cpu;

        //if (warmup_complete[packet->cpu]) 
        //    DP(cout << "[" << NAME << "_WQ] " << __func__ << " update event_cpu: " << WQ.event_cpu << " event_cycle: " << WQ.event_cycle << endl;)
    //}
    */
    update_schedule_cycle(&WQ);

    return -1;
}

int MEMORY_CONTROLLER::add_pq(PACKET *packet)
{
    return -1;
}

void MEMORY_CONTROLLER::return_data(PACKET *packet)
{

}

void MEMORY_CONTROLLER::update_schedule_cycle(PACKET_QUEUE *queue)
{
    // update next_schedule_cycle
    uint64_t min_cycle = UINT64_MAX;
    uint32_t min_index = queue->SIZE;
    for (uint32_t i=0; i<queue->SIZE; i++) {
        /*
        DP (if (warmup_complete[queue->entry[min_index].cpu]) {
        cout << "[" << queue->NAME << "] " <<  __func__ << " instr_id: " << queue->entry[i].instr_id;
        cout << " index: " << i << " address: " << hex << queue->entry[i].address << dec << " scheduled: " << +queue->entry[i].scheduled;
        cout << " event: " << queue->entry[i].event_cycle << " min_cycle: " << min_cycle << endl;
        });
        */

        if (queue->entry[i].address && (queue->entry[i].scheduled == 0) && (queue->entry[i].event_cycle < min_cycle)) {
            min_cycle = queue->entry[i].event_cycle;
            min_index = i;
        }
    }
    
    queue->next_schedule_cycle = min_cycle;
    queue->next_schedule_index = min_index;
    if (min_index < queue->SIZE) {

        DP (if (warmup_complete[queue->entry[min_index].cpu]) {
        cout << "[" << queue->NAME << "] " <<  __func__ << " instr_id: " << queue->entry[min_index].instr_id;
        cout << " address: " << hex << queue->entry[min_index].address << " full_addr: " << queue->entry[min_index].full_addr;
        cout << " data: " << queue->entry[min_index].data << dec;
        cout << " event: " << queue->entry[min_index].event_cycle << " current: " << current_core_cycle[queue->entry[min_index].cpu] << " next: " << queue->next_schedule_cycle << endl; });
    }
}

void MEMORY_CONTROLLER::update_process_cycle(PACKET_QUEUE *queue)
{
    // update next_process_cycle
    uint64_t min_cycle = UINT64_MAX;
    uint32_t min_index = queue->SIZE;
    for (uint32_t i=0; i<queue->SIZE; i++) {
        if (queue->entry[i].scheduled && (queue->entry[i].event_cycle < min_cycle)) {
            min_cycle = queue->entry[i].event_cycle;
            min_index = i;
        }
    }
    
    queue->next_process_cycle = min_cycle;
    queue->next_process_index = min_index;
    if (min_index < queue->SIZE) {

        DP (if (warmup_complete[queue->entry[min_index].cpu]) {
        cout << "[" << queue->NAME << "] " <<  __func__ << " instr_id: " << queue->entry[min_index].instr_id;
        cout << " address: " << hex << queue->entry[min_index].address << " full_addr: " << queue->entry[min_index].full_addr;
        cout << " data: " << queue->entry[min_index].data << dec << " num_returned: " << queue->num_returned;
        cout << " event: " << queue->entry[min_index].event_cycle << " current: " << current_core_cycle[queue->entry[min_index].cpu] << " next: " << queue->next_process_cycle << endl; });
    }
}

int MEMORY_CONTROLLER::check_dram_queue(PACKET_QUEUE *queue, PACKET *packet)
{
    // search write queue
    for (uint32_t index=0; index<queue->SIZE; index++) {
        if (queue->entry[index].address == packet->address) {
            
            DP ( if (warmup_complete[packet->cpu]) {
            cout << "[" << queue->NAME << "] " << __func__ << " same entry instr_id: " << packet->instr_id << " prior_id: " << queue->entry[index].instr_id;
            cout << " address: " << hex << packet->address << " full_addr: " << packet->full_addr << dec << endl; });

            return index;
        }
    }

    DP ( if (warmup_complete[packet->cpu]) {
    cout << "[" << queue->NAME << "] " << __func__ << " new address: " << hex << packet->address;
    cout << " full_addr: " << packet->full_addr << dec << endl; });

    DP ( if (warmup_complete[packet->cpu] && (queue->occupancy == queue->SIZE)) {
    cout << "[" << queue->NAME << "] " << __func__ << " mshr is full";
    cout << " instr_id: " << packet->instr_id << " mshr occupancy: " << queue->occupancy;
    cout << " address: " << hex << packet->address;
    cout << " full_addr: " << packet->full_addr << dec;
    cout << " cycle: " << current_core_cycle[packet->cpu] << endl; });

    return -1;
}

uint32_t MEMORY_CONTROLLER::dram_get_column(uint64_t address)
{
    if (LOG2_DRAM_COLUMNS == 0)
        return 0;

    int shift = 0;

    return (uint32_t) (address >> shift) & (DRAM_COLUMNS - 1);
}

uint32_t MEMORY_CONTROLLER::dram_get_channel(uint64_t address)
{
    if (LOG2_DRAM_CHANNELS == 0)
        return 0;

    int shift = LOG2_DRAM_COLUMNS;

    return (uint32_t) (address >> shift) & (DRAM_CHANNELS - 1);
}

uint32_t MEMORY_CONTROLLER::dram_get_bank(uint64_t address)
{
    if (LOG2_DRAM_BANKS == 0)
        return 0;

    int shift = LOG2_DRAM_COLUMNS + LOG2_DRAM_CHANNELS;

    return (uint32_t) (address >> shift) & (DRAM_BANKS - 1);
}

uint32_t MEMORY_CONTROLLER::dram_get_rank(uint64_t address)
{
    if (LOG2_DRAM_RANKS == 0)
        return 0;

    int shift = LOG2_DRAM_COLUMNS + LOG2_DRAM_CHANNELS + LOG2_DRAM_BANKS;

    return (uint32_t) (address >> shift) & (DRAM_RANKS - 1);
}

uint32_t MEMORY_CONTROLLER::dram_get_row(uint64_t address)
{
    if (LOG2_DRAM_ROWS == 0)
        return 0;

    int shift = LOG2_DRAM_COLUMNS + LOG2_DRAM_CHANNELS + LOG2_DRAM_BANKS + LOG2_DRAM_RANKS;

    return (uint32_t) (address >> shift) & (DRAM_ROWS - 1);
}

uint32_t MEMORY_CONTROLLER::get_occupancy(uint8_t queue_type)
{
    if (queue_type == 1)
        return RQ.occupancy;
    else if (queue_type == 2)
        return WQ.occupancy;

    return 0;
}

uint32_t MEMORY_CONTROLLER::get_size(uint8_t queue_type)
{
    if (queue_type == 1)
        return RQ.SIZE;
    else if (queue_type == 2)
        return WQ.SIZE;

    return 0;
}

void MEMORY_CONTROLLER::increment_WQ_FULL()
{
    WQ.FULL++;
}
