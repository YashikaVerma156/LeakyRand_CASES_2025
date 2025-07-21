#define _BSD_SOURCE

#include <getopt.h>
#include "ooo_cpu.h"
#include "uncore.h"
#include <fstream>
#include "config.h"

uint64_t FIRST_WAIT_OF_SENDER = 990600, // In set-up period, sender waits for longer time.
         SENDER_WAIT_PERIOD = 303500, // Sender wait period, once transmission begins.
         RECEIVER_WAIT_PERIOD = 800; // Receiver wait period, once transmission period begins.

//================= Error analysis for covert channel variables starts. ========================//
int EVICTED_CPU = -1, // values can be -1(no eviction), 0(block of cpu0 is evicted), or 1(block of cpu1 is evicted).
    dont_fill_empty_slot = 0,
    sender_seen_wait_once = 0, 
    rdtsc_timer_is_on = 0,
    inside_rdtsc = 0;

// int countoo=0;

int CURR_ERR_CORR_ITR = 0, MISSES_OBSERVED = 0, ERROR_CORRECTION_PHASE_LAST = 0, BIT_REVEALED_LAST = 0, CURR_LLC_OCC_CHK_ITR = 0, ITR_TO_FILL_HOLE = 0, bit_number_received = -1;
uint32_t IS_EVICTED_BLOCK_INSTR = 0, receiver_observed_miss = 0, correctly_transmitted_zero = 0; // value can be 0 or 1.
uint64_t EVICTED_IP = 0,  // IP of the block evicted by the sender. 
         SENDER_ACTIVE_IP_START = 0, // Start IP of sender active blocks.
         SENDER_ACTIVE_IP_END = 0; // End IP of sender active blocks.
uint64_t sender_evicted_own_active_block = 0, sender_evicted_own_inactive_block = 0, sender_evicted_receiver_inactive_block = 0, sender_evicted_receiver_unrecognizable_active_block = 0, sender_evicted_receiver_recognizable_active_block = 0, sender_filled_up_empty_block = 0;
//================= Error analysis for covert channel variables ends. ========================//

uint32_t HOLE_FILLING_BIT = 0, LLC_OCC_CHK_BIT = 0;

uint8_t SETUP_DONE[NUM_CPUS]={0}, FIRST_TIME_CACHE_SETUP_DONE=0, seen_llc_occ_chk=0, LLC_OCC_CHK_PHASE;
uint32_t LLC_OCCUPANCY_CHECK_ONGOING =0, seen_error_correction=0, ERROR_CORRECTION_PHASE=0, DOUBLE_PROBE=0, HOLE_FILLING_PROCESS_ONGOING=0, CACHE_FILL_DONE=0;
uint64_t differ_sig[DIFFERENTIAL_SIGNALLING], repeat_code[REPETETION_CODING], LLC_MISS_COUNT[NUM_CPUS]={0};

FILE * fptr =  NULL;

// Calculate LLC occupancy for initial set-up.
uint64_t FIRST_FENCE = 0, range_start = -1 /*1243133*/ /*296440  304127*/, range_end = -1 /*1243133*/ /*296440  304129*/;
int Calculate_LLC_occupancy = 0, DONT_INC_ARR = 0;
// Calculate LLC occupancy for initial set-up.

uint8_t warmup_complete[NUM_CPUS], 
        simulation_complete[NUM_CPUS], 
        all_warmup_complete = 0, 
        all_simulation_complete = 0,
        MAX_INSTR_DESTINATIONS = NUM_INSTR_DESTINATIONS,
        knob_cloudsuite = 0,
        knob_low_bandwidth = 0;

uint64_t warmup_instructions     = 1000000,
         simulation_instructions = 10000000,
         champsim_seed;

time_t start_time;
uint8_t cpu_on_wait[NUM_CPUS], fence_called_on_cpu[NUM_CPUS], LLC_FILL_PENALTY=0;
int TURN=-1;    

int64_t CENTRAL_CLOCK[NUM_CPUS], WAIT_CYCLES[NUM_CPUS], WAIT_PERIOD[3]={0};

uint64_t spills = 0;
int receiver_checks_for_a_miss = 0, bit_revealed_is = 0, CENTRAL_CLOCK_ALL_COMPLETE = 0;
uint64_t prev_rdtsc_counter = 0, prev_rdtsc_id = 0;

uint64_t CHECK_IP = 94066950308703;

uint64_t clflush_on_cpu[NUM_CPUS][MAX_CLFLUSH_POSSIBLE];
int cycles_to_wait_for_before_triggering_clflush = 20;
uint64_t cpu_cycle_to_trigger_clflush[NUM_CPUS][MAX_CLFLUSH_POSSIBLE] = {0};

uint64_t fence_instr[NUM_CPUS], recorded_rdtsc_value[NUM_CPUS], hit_block = 0;

ofstream llc_access_stream_file;  // To generate llc access stream

int start_count=0, start_count_1=0, stop_count=0, flushed_sender_blocks_once =0;

// PAGE TABLE
uint32_t PAGE_TABLE_LATENCY = 0, SWAP_LATENCY = 0;
queue <uint64_t > page_queue;
map <uint64_t, uint64_t> page_table, inverse_table, recent_page, unique_cl[NUM_CPUS];
uint64_t previous_ppage, num_adjacent_page, num_cl[NUM_CPUS], allocated_pages, num_page[NUM_CPUS], minor_fault[NUM_CPUS], major_fault[NUM_CPUS];

void record_error_stats()
{
    // Incrementing stats for errors.
		 if(EVICTED_CPU == 1)
		 {
				 if(EVICTED_IP <= SENDER_ACTIVE_IP_START && EVICTED_IP <= SENDER_ACTIVE_IP_END)
				 {
						 sender_evicted_own_active_block += 1;
				 }
				 else
				 {
						 sender_evicted_own_inactive_block += 1;
				 }
         EVICTED_IP = 0;
         EVICTED_CPU = -1;
		 }
     else if(EVICTED_CPU == 0)
     {
         if(receiver_observed_miss && inside_rdtsc)
         {
             sender_evicted_receiver_recognizable_active_block ++;
             receiver_observed_miss =0;
             inside_rdtsc = 0;
         }
         else if(receiver_observed_miss && !(inside_rdtsc))
         {
             sender_evicted_receiver_unrecognizable_active_block ++;
             receiver_observed_miss = 0;
             inside_rdtsc = 0;
         }
         else
         {
             sender_evicted_receiver_inactive_block ++;
         }
         EVICTED_IP = 0;
         EVICTED_CPU = -1;
     }
     else if(EVICTED_CPU == -1)
     {
         // If bit sent was zero.
         if(!(receiver_observed_miss))
         {
             correctly_transmitted_zero += 1;
         }
     }

}

void calculate_llc_occupancy_for_specific_cpu(uint64_t FIRST_FENCE_CPU_0)
{
		uint64_t cnt1 = 0, cnt = 0;
		cout<<"Calculated LLC occupancy for cpu 0 and cpu 1 is: ";
		for(uint32_t i1=0; i1 < uncore.LLC.NUM_SET; i1++)
		{
			for(uint32_t j1=0; j1 < uncore.LLC.NUM_WAY; j1++)
			{
					if(uncore.LLC.block[i1][j1].cpu == 0 && uncore.LLC.block[i1][j1].valid == 1 && uncore.LLC.block[i1][j1].instr_id > FIRST_FENCE_CPU_0 )
								cnt ++;
					else if(uncore.LLC.block[i1][j1].valid == 1 && uncore.LLC.block[i1][j1].cpu == 1 )
								cnt1 ++;
			}
		}
		cout << cnt <<" "<< cnt1<< endl;
}

// Verification check
void check_inclusive()
{
  for(int i=0; i<NUM_CPUS; i++)
  {
      int counter = 0;
      int counter1 = 0;
  //L1I data should be present at L2C
    for(int l1iset=0; l1iset<L1I_SET; l1iset++)
      for(int l1iway=0; l1iway<L1I_WAY; l1iway++)
        if(ooo_cpu[i].L1I.block[l1iset][l1iway].valid){
          int match = -1;
          int l2cset = ooo_cpu[i].L2C.get_set(ooo_cpu[i].L1I.block[l1iset][l1iway].address);

          for(int l2cway=0;l2cway< L2C_WAY;l2cway++)
            {
              if(ooo_cpu[i].L2C.block[l2cset][l2cway].tag == ooo_cpu[i].L1I.block[l1iset][l1iway].tag &&
                 ooo_cpu[i].L2C.block[l2cset][l2cway].valid ==1){
                match = 1 ;
              }
            }
        #if INCLUSIVE_PRINT
          if(match == -1)
          {
            counter++;
            //cout<<"L1I address not in L2C:"<<hex<<ooo_cpu[i].L1I.block[l1iset][l1iway].address<<dec<<" instr_id: "<<ooo_cpu[i].L1I.block[l1iset][l1iway].instr_id<<endl;
          }
        #endif

          //assert(match==1);
          match = -1;

  //L1I data should be in LLC
          //int llcset = uncore.LLC.get_set(ooo_cpu[i].L1I.block[l1iset][l1iway].address);
          for(int llcset=0;llcset< LLC_SET;llcset++)
            for(int llcway=0;llcway< LLC_WAY;llcway++)
              if(ooo_cpu[i].L1I.block[l1iset][l1iway].tag == uncore.LLC.block[llcset][llcway].tag &&
                  uncore.LLC.block[llcset][llcway].valid == 1){
                match = 1 ;
              }
        #if INCLUSIVE_PRINT
          if(match == -1)
          {
            counter1++;
            //cout<<"L1I address not in LLC:"<<hex<<ooo_cpu[i].L1I.block[l1iset][l1iway].address<<dec<<" instr_id: "<<ooo_cpu[i].L1I.block[l1iset][l1iway].instr_id<<endl;
          }
        #endif

          //assert(match==1);
        }
    if(counter)
      cout<<"l1I data not in l2c: "<<counter<<endl;
    if(counter1)
      cout<<"l1I data not in llc: "<<counter1<<endl;
    counter=0;
    counter1=0;

    //L1D data should be present at L2C
      for(int l1dset=0;l1dset<L1D_SET;l1dset++)
        for(int l1dway=0;l1dway<L1D_WAY;l1dway++)
          if(ooo_cpu[i].L1D.block[l1dset][l1dway].valid == 1){

            int match = -1;
            int l2cset = ooo_cpu[i].L2C.get_set(ooo_cpu[i].L1D.block[l1dset][l1dway].address);

            for(int l2cway=0;l2cway< L2C_WAY;l2cway++){
                if(ooo_cpu[i].L2C.block[l2cset][l2cway].tag == ooo_cpu[i].L1D.block[l1dset][l1dway].tag &&
                    ooo_cpu[i].L2C.block[l2cset][l2cway].valid ==1)
                  match = 1 ;
                }
        #if INCLUSIVE_PRINT
            if(match == -1)
            {
              counter++;
              if(ooo_cpu[i].L1D.block[l1dset][l1dway].dirty)
                cout<<"L1D dirty block not in L2C"<<endl;
              //cout<<"L1D address not in L2C:"<<hex<<ooo_cpu[i].L1D.block[l1dset][l1dway].address<<dec<<endl;
            }
        #endif

            //assert(match==1);
            match = -1;

    //L1D data should be present at LLC
       //     int llcset = uncore.LLC.get_set(ooo_cpu[i].L1D.block[l1dset][l1dway].address);
            for(int llcset=0;llcset< LLC_SET;llcset++)
              for(int llcway=0;llcway< LLC_WAY;llcway++)
                if(ooo_cpu[i].L1D.block[l1dset][l1dway].tag == uncore.LLC.block[llcset][llcway].tag &&
                    uncore.LLC.block[llcset][llcway].valid == 1){
                  match = 1 ;
                }

        #if INCLUSIVE_PRINT
            if(match == -1)
            {
              counter1++;
              if(ooo_cpu[i].L1D.block[l1dset][l1dway].dirty)
                                                                cout<<"L1D dirty block not in LLC"<<endl;
              //cout<<"L1D address not in LLC:"<<hex<<ooo_cpu[i].L1D.block[l1dset][l1dway].address<<dec<<endl;
            }
        #endif
            //assert(match==1);
          }

      if(counter)
        cout<<"l1D data not in l2c: "<<counter<<endl;
      if(counter1)
        cout<<"l1D data not in llc: "<<dec<<counter1<<endl;

      counter=0;
      counter1=0;

      //L2C data should be present in LLC
      for(int l2cset=0;l2cset<L2C_SET;l2cset++)
        for(int l2cway=0;l2cway<L2C_WAY;l2cway++)
          if(ooo_cpu[i].L2C.block[l2cset][l2cway].valid == 1){
            int match = -1;
            //int llcset = uncore.LLC.get_set(ooo_cpu[i].L2C.block[l2cset][l2cway].address);
            for(int llcset=0;llcset< LLC_SET;llcset++)
              for(int llcway=0;llcway< LLC_WAY;llcway++)
                if(ooo_cpu[i].L2C.block[l2cset][l2cway].tag == uncore.LLC.block[llcset][llcway].tag &&
                    uncore.LLC.block[llcset][llcway].valid == 1){
                  match = 1 ;
                }
        #if INCLUSIVE_PRINT
            if(match == -1)
            {
              counter++;
              if(ooo_cpu[i].L2C.block[l2cset][l2cway].dirty)
                counter1++;
              //cout<<"L2C address not in LLC:"<<hex<<ooo_cpu[i].L2C.block[l2cset][l2cway].address<<dec<<endl;
            }
        #endif
            //assert(match==1);
          }

      if(counter)
        cout<<"l2C data not in llc: "<<dec<<counter<<endl;
      if(counter && counter1)
        cout<<"l2c dirty block not in llc:"<<counter1<<endl;
}
}	

int count_LLC_valid_blocks()
{
    uint32_t count_valid1 = 0, count_dirty1 = 0, count_invalid1 = 0, count_valid_cpu0 = 0, count_dirty_cpu1 = 0, count_valid_cpu1 = 0, count_dirty_cpu0 = 0;
      for(uint32_t i1=0; i1 < uncore.LLC.NUM_SET; i1++)
      {
        for(uint32_t j1=0; j1 < uncore.LLC.NUM_WAY; j1++)
        {
            if(uncore.LLC.block[i1][j1].valid == 1)
              {
                  count_valid1 ++;

                  if(uncore.LLC.block[i1][j1].cpu == 0)
                      count_valid_cpu0++;
                  else
                  {
                      count_valid_cpu1++;
                      if(DETAILED_PRINT)
                           cout << " CPU 1 address:  "<<(int)uncore.LLC.block[i1][j1].address << " instr_id: " << (int)uncore.LLC.block[i1][j1].instr_id <<endl;
                  }
              }

            if(uncore.LLC.block[i1][j1].dirty == 1)
              {count_dirty1 ++; /*cout<<" Dirty instr_id: "<<uncore.LLC.block[i1][j1].instr_id<<endl;*/
                if(uncore.LLC.block[i1][j1].cpu == 0)count_dirty_cpu0++; else count_dirty_cpu1++;  }

            if(uncore.LLC.block[i1][j1].valid == 0)
              count_invalid1 ++;
        }
      }
    if(DETAILED_PRINT || ((LR_NO_ERR_CORR || LR_ERR_CORR_BOTH_ALGO) && NUM_CPUS < 2) || IEEE_SEED)
    cout << "Tag LLC Valid blocks: "<< count_valid1 << " Dirty blocks: "<<count_dirty1 << " Invalid blocks: "<< count_invalid1 << "cpu 0 blocks "<< " valid: "<< count_valid_cpu0<< " dirty: "<<count_dirty_cpu0 << " cpu 1 blocks->"<< " valid: "<< count_valid_cpu1<< " dirty: "<<count_dirty_cpu1 << endl;

    count_valid1 = 0, count_dirty1 = 0, count_invalid1 = 0, count_valid_cpu0 = 0, count_dirty_cpu1 = 0, count_valid_cpu1 = 0, count_dirty_cpu0 = 0;

    for(uint32_t i1=0; i1 < uncore.data_LLC.NUM_SET; i1++)
      {
        for(uint32_t j1=0; j1 < uncore.data_LLC.NUM_WAY; j1++)
        {
            if(uncore.data_LLC.block[i1][j1].valid == 1)
            { count_valid1 ++; if(uncore.data_LLC.block[i1][j1].cpu == 0)count_valid_cpu0++; else count_valid_cpu1++;  }

            if(uncore.data_LLC.block[i1][j1].dirty == 1)
            { count_dirty1 ++; if(uncore.data_LLC.block[i1][j1].cpu == 0)count_dirty_cpu0++; else count_dirty_cpu1++;  }

            if(uncore.data_LLC.block[i1][j1].valid == 0)
              count_invalid1 ++;
        }
      }
   if(DETAILED_PRINT || CHANNEL_BW_ERR_PRINT || IEEE_SEED)
   cout << "Data LLC Valid blocks: "<< count_valid1 << " Dirty blocks: "<<count_dirty1 << " Invalid blocks: "<< count_invalid1 << "cpu 0 blocks->"<< " valid: "<< count_valid_cpu0<< " dirty: "<<count_dirty_cpu0<< " cpu 1 blocks->"<< " valid: "<< count_valid_cpu1<< " dirty: "<<count_dirty_cpu1 << endl;

 return count_valid1;
}

int check_mshr_for_fills(PACKET_QUEUE *queue, uint64_t address)
{
    int count = 0;
    cout << endl << queue->NAME << " Entry" << " queue->size"<<queue->SIZE << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        //if(address == queue->entry[j].address){ // check block address
							cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
							cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
							cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index<<" returned: "<<(int)queue->entry[j].returned<<" forwarded to next level: "<<(int)queue->entry[j].forwarded_to_next_cache_level << endl;
            count ++;
       // }
    }
    return count ;
}


void cache_line_flush(int cpu, int clflush_address_num)
{
  uint64_t physical_address;
  int match_way = -1, num = clflush_address_num; 
	int set = ooo_cpu[cpu].DTLB.get_set(clflush_on_cpu[cpu][num]>>12);
  if(DETAILED_PRINT || CHANNEL_BW_ERR_PRINT || LR_NO_ERR_CORR || LR_ERR_CORR_BOTH_ALGO)
  {
      cout<<"/////////////////// Cache state before clflush /////////////////////" <<endl;
      cout << "clflush called on cpu: "<< cpu<<" cpu_cycle: " <<current_core_cycle[cpu]<< " address: "<<hex<<clflush_on_cpu[cpu][num] <<" address>>12: "<<(clflush_on_cpu[cpu][num]>>12)<<" address>>6: "<<(clflush_on_cpu[cpu][num]>>6)<<" "<<endl;
      cout <<dec<< "clflush called on cpu: "<< cpu<<" cpu_cycle: " <<current_core_cycle[cpu]<< " address: "<<clflush_on_cpu[cpu][num] <<" address>>12: "<<(clflush_on_cpu[cpu][num]>>12)<<" address>>6: "<<(clflush_on_cpu[cpu][num]>>6)<<" "<<endl;
  }
  for (uint32_t way=0; way<ooo_cpu[cpu].DTLB.NUM_WAY; way++)
            {  
            //cout <<"DTLB tag: " <<ooo_cpu[cpu].DTLB.block[set][way].tag << " address: "<<ooo_cpu[cpu].DTLB.block[set][way].address<<" set: "<<set <<endl;
                if (ooo_cpu[cpu].DTLB.block[set][way].valid && (ooo_cpu[cpu].DTLB.block[set][way].tag == (clflush_on_cpu[cpu][num]>>12)))
                {
                    physical_address = (ooo_cpu[cpu].DTLB.block[set][way].data << 12) | (clflush_on_cpu[cpu][num] & ((1 << LOG2_PAGE_SIZE) - 1));
                    match_way = way;
                    break;
                }
            }
  if(match_way == -1)
  {
      set = ooo_cpu[cpu].STLB.get_set(clflush_on_cpu[cpu][num]>>12);
      for (uint32_t way=0; way<ooo_cpu[cpu].STLB.NUM_WAY; way++)
      {  //cout <<"STLB tag: " <<ooo_cpu[cpu].STLB.block[set][way].tag << " address: "<<ooo_cpu[cpu].STLB.block[set][way].address<<" set: "<<set <<endl;
            if (ooo_cpu[cpu].STLB.block[set][way].valid && (ooo_cpu[cpu].STLB.block[set][way].tag == (clflush_on_cpu[cpu][num]>>12)))
            {
                match_way = way;
                physical_address = (ooo_cpu[cpu].STLB.block[set][way].data << 12) | (clflush_on_cpu[cpu][num] & ((1 << LOG2_PAGE_SIZE) - 1));
                break;
            }
      }

  }
  if(match_way == -1) assert(0); // The translation should be found, as per current implementation of sender code and receiver code.
 
  clflush_on_cpu[cpu][num] = 0; // Resetting back to normal.

/* clflushing_blocks_from_all_cache */
      if(DETAILED_PRINT || CHANNEL_BW_ERR_PRINT || LR_NO_ERR_CORR || LR_ERR_CORR_BOTH_ALGO)
          cout << " physical address: "<< physical_address <<" ";

		    set = ooo_cpu[cpu].L1D.get_set(physical_address>>6);
        for(uint32_t j1=0; j1 < ooo_cpu[cpu].L1D.NUM_WAY; j1++)
        {
            //cout <<"tag: " <<ooo_cpu[cpu].L1D.block[set][j1].tag << " address: "<<ooo_cpu[cpu].L1D.block[set][j1].address<<" set: "<<set <<endl;
            if(ooo_cpu[cpu].L1D.block[set][j1].valid && (ooo_cpu[cpu].L1D.block[set][j1].tag == (physical_address>>6)))
            { ooo_cpu[cpu].L1D.block[set][j1].dirty = 0;  ooo_cpu[cpu].L1D.block[set][j1].valid = 0; }
        }
        set = ooo_cpu[cpu].L2C.get_set(physical_address>>6);
        for(uint32_t j1=0; j1 < ooo_cpu[cpu].L2C.NUM_WAY; j1++)
        {
            //cout <<"tag: " <<ooo_cpu[cpu].L2C.block[set][j1].tag << " address: "<<ooo_cpu[cpu].L2C.block[set][j1].address<<" set: "<<set <<endl;
            if(ooo_cpu[cpu].L2C.block[set][j1].valid && (ooo_cpu[cpu].L2C.block[set][j1].tag == (physical_address>>6)))
            { ooo_cpu[cpu].L2C.block[set][j1].dirty = 0;  ooo_cpu[cpu].L2C.block[set][j1].valid = 0; }
        }
// ================ For LLC =================
        int flag = 0;
				uint32_t set_per_partition[uncore.LLC.NUM_SKEW];
        for(uint32_t partition = 0; partition < uncore.LLC.NUM_SKEW; partition++)
        { 
            set_per_partition[partition] = uncore.LLC.extractSet(physical_address>>6, partition); 
        }

        for(uint32_t part = 0; part < uncore.LLC.NUM_SKEW; part++)
        {
            for(uint32_t way = part*(uncore.LLC.NUM_WAY/uncore.LLC.NUM_SKEW); way < (uncore.LLC.NUM_WAY/uncore.LLC.NUM_SKEW)*(part+1); way++)
            {
                if(uncore.LLC.block[set_per_partition[part]][way].valid && uncore.LLC.block[set_per_partition[part]][way].tag == physical_address>>6)
                {
                    if( !(uncore.data_LLC.block[uncore.LLC.block[set_per_partition[part]][way].set_ptr][uncore.LLC.block[set_per_partition[part]][way].way_ptr].valid))
                        assert(0);
                    uncore.LLC.block[set_per_partition[part]][way].valid = 0;
                    uncore.data_LLC.block[uncore.LLC.block[set_per_partition[part]][way].set_ptr][uncore.LLC.block[set_per_partition[part]][way].way_ptr].valid = 0;
                    uncore.LLC.block[set_per_partition[part]][way].dirty = 0;
                    uncore.data_LLC.block[uncore.LLC.block[set_per_partition[part]][way].set_ptr][uncore.LLC.block[set_per_partition[part]][way].way_ptr].dirty = 0;
                    uncore.data_LLC.block[uncore.LLC.block[set_per_partition[part]][way].set_ptr][uncore.LLC.block[set_per_partition[part]][way].way_ptr].set_ptr = -1;
                    uncore.data_LLC.block[uncore.LLC.block[set_per_partition[part]][way].set_ptr][uncore.LLC.block[set_per_partition[part]][way].way_ptr].way_ptr = -1;

                    if(DETAILED_PRINT || CHANNEL_BW_ERR_PRINT || LR_NO_ERR_CORR || LR_ERR_CORR_BOTH_ALGO)
								        cout << "Removed sender block from LLC, address:  "<<(int)uncore.LLC.block[set_per_partition[part]][way].address << " instruction_id: " << (int)uncore.LLC.block[set_per_partition[part]][way].instr_id <<" way pointer: " <<uncore.LLC.block[set_per_partition[part]][way].way_ptr <<" set pointer: " <<uncore.LLC.block[set_per_partition[part]][way].set_ptr << endl;
                    uncore.LLC.block[set_per_partition[part]][way].set_ptr = -1;
                    uncore.LLC.block[set_per_partition[part]][way].way_ptr = -1; 
                    flag = 1;
                    break;
                } 
            }
            if(flag == 1) break;
        }
        //int nums = check_mshr_for_fills(&uncore.LLC.MSHR, physical_address>>6);
        //cout <<"entries found: "<< nums<<endl;
        //if(flag != 1) assert(0); // sanity check // check not required.
   
        for(uint32_t i1=0; i1 < uncore.data_LLC.NUM_SET; i1++) // Verification check, that data_LLC does not have the flushed block as valid.
        {
            for(uint32_t j1=0; j1 < uncore.data_LLC.NUM_WAY; j1++)
            {
                if((uncore.data_LLC.block[i1][j1].tag == physical_address >> 6) && (uncore.data_LLC.block[i1][j1].valid == 1)) //The sender's cache block.
                    assert(0);
            }  
        }
}

/*void count_LLC_valid_blocks()
{ 
    cout<<"current core cycle: " << current_core_cycle[0]<< endl;
    uint32_t count_valid1 = 0, count_dirty1 = 0, count_invalid1 = 0, count_valid_cpu0 = 0, count_dirty_cpu1 = 0, count_valid_cpu1 = 0, count_dirty_cpu0 = 0;
      for(uint32_t i1=0; i1 < uncore.LLC.NUM_SET; i1++)
      {
        for(uint32_t j1=0; j1 < uncore.LLC.NUM_WAY; j1++)
        {
            if(uncore.LLC.block[i1][j1].valid == 1)
              {
                  count_valid1 ++; 
              
                  if(uncore.LLC.block[i1][j1].cpu == 0)
                      count_valid_cpu0++; 
                  else 
                  {
                      count_valid_cpu1++; 
                      if(DETAILED_PRINT)
                           cout << " CPU 1 address:  "<<(int)uncore.LLC.block[i1][j1].address << " instr_id: " << (int)uncore.LLC.block[i1][j1].instr_id <<endl;
                  }
              }

            if(uncore.LLC.block[i1][j1].dirty == 1)
              {count_dirty1 ++; //cout<<" Dirty instr_id: "<<uncore.LLC.block[i1][j1].instr_id<<endl;
                if(uncore.LLC.block[i1][j1].cpu == 0)count_dirty_cpu0++; else count_dirty_cpu1++;  }

            if(uncore.LLC.block[i1][j1].valid == 0)
              count_invalid1 ++;
        }
      }

//		if(DETAILED_PRINT)
    cout << "Tag LLC Valid blocks: "<< count_valid1 << " Dirty blocks: "<<count_dirty1 << " Invalid blocks: "<< count_invalid1 << "cpu 0 blocks "<< " valid: "<< count_valid_cpu0<< " dirty: "<<count_dirty_cpu0 << " cpu 1 blocks->"<< " valid: "<< count_valid_cpu1<< " dirty: "<<count_dirty_cpu1 << endl;

    count_valid1 = 0, count_dirty1 = 0, count_invalid1 = 0, count_valid_cpu0 = 0, count_dirty_cpu1 = 0, count_valid_cpu1 = 0, count_dirty_cpu0 = 0;

    for(uint32_t i1=0; i1 < uncore.data_LLC.NUM_SET; i1++)
      {
        for(uint32_t j1=0; j1 < uncore.data_LLC.NUM_WAY; j1++)

            if(uncore.data_LLC.block[i1][j1].valid == 1)
            { count_valid1 ++; if(uncore.data_LLC.block[i1][j1].cpu == 0)count_valid_cpu0++; else count_valid_cpu1++;  }

            if(uncore.data_LLC.block[i1][j1].dirty == 1)
            { count_dirty1 ++; if(uncore.data_LLC.block[i1][j1].cpu == 0)count_dirty_cpu0++; else count_dirty_cpu1++;  }

            if(uncore.data_LLC.block[i1][j1].valid == 0)
              count_invalid1 ++;
        }
      
//	 if(DETAILED_PRINT)
   cout << "Data LLC Valid blocks: "<< count_valid1 << " Dirty blocks: "<<count_dirty1 << " Invalid blocks: "<< count_invalid1 << "cpu 0 blocks->"<< " valid: "<< count_valid_cpu0<< " dirty: "<<count_dirty_cpu0<< " cpu 1 blocks->"<< " valid: "<< count_valid_cpu1<< " dirty: "<<count_dirty_cpu1 << endl;
} */

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
    cout << " LOAD      ACCESS: " << setw(10) << cache->roi_access[cpu][0] << "  HIT: " << setw(10) << cache->roi_hit[cpu][0] << "  MISS: " << setw(10) << cache->roi_miss[cpu][0] << endl;

    cout << cache->NAME;
    cout << " RFO       ACCESS: " << setw(10) << cache->roi_access[cpu][1] << "  HIT: " << setw(10) << cache->roi_hit[cpu][1] << "  MISS: " << setw(10) << cache->roi_miss[cpu][1] << endl;

    cout << cache->NAME;
    cout << " PREFETCH  ACCESS: " << setw(10) << cache->roi_access[cpu][2] << "  HIT: " << setw(10) << cache->roi_hit[cpu][2] << "  MISS: " << setw(10) << cache->roi_miss[cpu][2] << endl;

    cout << cache->NAME;
    cout << " WRITEBACK ACCESS: " << setw(10) << cache->roi_access[cpu][3] << "  HIT: " << setw(10) << cache->roi_hit[cpu][3] << "  MISS: " << setw(10) << cache->roi_miss[cpu][3] << endl;

    cout << cache->NAME;
    cout << " PREFETCH  REQUESTED: " << setw(10) << cache->pf_requested << "  ISSUED: " << setw(10) << cache->pf_issued;
    cout << "  USEFUL: " << setw(10) << cache->pf_useful << "  USELESS: " << setw(10) << cache->pf_useless << endl;
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
    cout << " LOAD      ACCESS: " << setw(10) << cache->sim_access[cpu][0] << "  HIT: " << setw(10) << cache->sim_hit[cpu][0] << "  MISS: " << setw(10) << cache->sim_miss[cpu][0] << endl;

    cout << cache->NAME;
    cout << " RFO       ACCESS: " << setw(10) << cache->sim_access[cpu][1] << "  HIT: " << setw(10) << cache->sim_hit[cpu][1] << "  MISS: " << setw(10) << cache->sim_miss[cpu][1] << endl;

    cout << cache->NAME;
    cout << " PREFETCH  ACCESS: " << setw(10) << cache->sim_access[cpu][2] << "  HIT: " << setw(10) << cache->sim_hit[cpu][2] << "  MISS: " << setw(10) << cache->sim_miss[cpu][2] << endl;

    cout << cache->NAME;
    cout << " WRITEBACK ACCESS: " << setw(10) << cache->sim_access[cpu][3] << "  HIT: " << setw(10) << cache->sim_hit[cpu][3] << "  MISS: " << setw(10) << cache->sim_miss[cpu][3] << endl;
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
    for (uint32_t i=0; i<DRAM_CHANNELS; i++) {
        cout << " CHANNEL " << i << endl;
        cout << " RQ ROW_BUFFER_HIT: " << setw(10) << uncore.DRAM.RQ[i].ROW_BUFFER_HIT << "  ROW_BUFFER_MISS: " << setw(10) << uncore.DRAM.RQ[i].ROW_BUFFER_MISS << endl;
        cout << " DBUS_CONGESTED: " << setw(10) << uncore.DRAM.dbus_congested[NUM_TYPES][NUM_TYPES] << endl; 
        cout << " WQ ROW_BUFFER_HIT: " << setw(10) << uncore.DRAM.WQ[i].ROW_BUFFER_HIT << "  ROW_BUFFER_MISS: " << setw(10) << uncore.DRAM.WQ[i].ROW_BUFFER_MISS;
        cout << "  FULL: " << setw(10) << uncore.DRAM.WQ[i].FULL << endl; 
        cout << endl;
    }

    uint64_t total_congested_cycle = 0;

    for (uint32_t i=0; i<DRAM_CHANNELS; i++)
        {total_congested_cycle += uncore.DRAM.dbus_cycle_congested[i];
		cout << uncore.DRAM.dbus_cycle_congested[i] << " " << uncore.DRAM.dbus_congested[NUM_TYPES][NUM_TYPES] << endl;}
    if(uncore.DRAM.dbus_congested[NUM_TYPES][NUM_TYPES] > 0)
		cout << " AVG_CONGESTED_CYCLE: " << (total_congested_cycle / uncore.DRAM.dbus_congested[NUM_TYPES][NUM_TYPES]) << endl;
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
        reset_cache_stats(i, &uncore.data_LLC);
    }
    cout << endl;

    // reset DRAM stats
    for (uint32_t i=0; i<DRAM_CHANNELS; i++) {
        uncore.DRAM.RQ[i].ROW_BUFFER_HIT = 0;
        uncore.DRAM.RQ[i].ROW_BUFFER_MISS = 0;
        uncore.DRAM.WQ[i].ROW_BUFFER_HIT = 0;
        uncore.DRAM.WQ[i].ROW_BUFFER_MISS = 0;
    }

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

    // print ROB status
    cout << endl << "ROB Queue Entries" << endl;

		for (uint32_t j=0; j<ROB_SIZE; j++) {
        cout << "[ROB] entry: " << j << " instr_id: " << ooo_cpu[i].ROB.entry[j].instr_id << " address: " << hex << ooo_cpu[i].ROB.entry[j].physical_address << dec << " translated: " << +ooo_cpu[i].ROB.entry[j].translated << " fetched: " << +ooo_cpu[i].ROB.entry[j].fetched << " scheduled: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].scheduled<< " executed: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].executed<< " is_memory: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].is_memory<<endl;
    }


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
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index <<" returned: "<<(int)queue->entry[j].returned<<" forwarded to next level: "<<(int)queue->entry[j].forwarded_to_next_cache_level << endl; 
    }

    // print L2C MSHR entry
    queue = &ooo_cpu[i].L2C.MSHR;
    cout << endl << queue->NAME << " Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index<<" returned: "<<(int)queue->entry[j].returned<<" forwarded to next level: "<<(int)queue->entry[j].forwarded_to_next_cache_level << endl;
    }

    // print LLC MSHR entry
    queue = &uncore.LLC.MSHR;
    cout << endl << queue->NAME << " Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index<<" returned: "<<(int)queue->entry[j].returned<<" forwarded to next level: "<<(int)queue->entry[j].forwarded_to_next_cache_level << endl;
    }

    assert(0);
}

void signal_handler(int signal) 
{
	cout << "Caught signal: " << signal << endl;
	exit(1);
}

// log base 2 function from efectiu
int lg2(int n)
{
    int i, m = n, c = -1;
    for (i=0; m; i++) {
        m /= 2;
        c++;
    }
    return c;
}

uint64_t rotl64 (uint64_t n, unsigned int c)
{
    const unsigned int mask = (CHAR_BIT*sizeof(n)-1);

    assert ( (c<=mask) &&"rotate by type width or more");
    c &= mask;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
    return (n<<c) | (n>>( (-c)&mask ));
}

uint64_t rotr64 (uint64_t n, unsigned int c)
{
    const unsigned int mask = (CHAR_BIT*sizeof(n)-1);

    assert ( (c<=mask) &&"rotate by type width or more");
    c &= mask;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
    return (n>>c) | (n<<( (-c)&mask ));
}

RANDOM champsim_rand(champsim_seed);
uint64_t va_to_pa(uint32_t cpu, uint64_t instr_id, uint64_t va, uint64_t unique_vpage)
{
#ifdef SANITY_CHECK
    if (va == 0) 
        assert(0);
#endif

    uint8_t  swap = 0;
    uint64_t high_bit_mask = rotr64(cpu, lg2(NUM_CPUS)),
             unique_va = va | high_bit_mask;
    //uint64_t vpage = unique_va >> LOG2_PAGE_SIZE,
    uint64_t vpage = unique_vpage | high_bit_mask,
             voffset = unique_va & ((1<<LOG2_PAGE_SIZE) - 1);

    // smart random number generator
    uint64_t random_ppage;

    map <uint64_t, uint64_t>::iterator pr = page_table.begin();
    map <uint64_t, uint64_t>::iterator ppage_check = inverse_table.begin();

    // check unique cache line footprint
    map <uint64_t, uint64_t>::iterator cl_check = unique_cl[cpu].find(unique_va >> LOG2_BLOCK_SIZE);
    if (cl_check == unique_cl[cpu].end()) { // we've never seen this cache line before
        unique_cl[cpu].insert(make_pair(unique_va >> LOG2_BLOCK_SIZE, 0));
        num_cl[cpu]++;
    }
    else
        cl_check->second++;

    pr = page_table.find(vpage);
    if (pr == page_table.end()) { // no VA => PA translation found 

        if (allocated_pages >= DRAM_PAGES) { // not enough memory

            // TODO: elaborate page replacement algorithm
            // here, ChampSim randomly selects a page that is not recently used and we only track 32K recently accessed pages
            uint8_t  found_NRU = 0;
            uint64_t NRU_vpage = 0; // implement it
            //map <uint64_t, uint64_t>::iterator pr2 = recent_page.begin();
            for (pr = page_table.begin(); pr != page_table.end(); pr++) {

                NRU_vpage = pr->first;
                if (recent_page.find(NRU_vpage) == recent_page.end()) {
                    found_NRU = 1;
                    break;
                }
            }
#ifdef SANITY_CHECK
            if (found_NRU == 0)
                assert(0);

            if (pr == page_table.end())
                assert(0);
#endif
            DP ( if (warmup_complete[cpu]) {
            cout << "[SWAP] update page table NRU_vpage: " << hex << pr->first << " new_vpage: " << vpage << " ppage: " << pr->second << dec << endl; });

            // update page table with new VA => PA mapping
            // since we cannot change the key value already inserted in a map structure, we need to erase the old node and add a new node
            uint64_t mapped_ppage = pr->second;
            page_table.erase(pr);
            page_table.insert(make_pair(vpage, mapped_ppage));

            // update inverse table with new PA => VA mapping
            ppage_check = inverse_table.find(mapped_ppage);
#ifdef SANITY_CHECK
            if (ppage_check == inverse_table.end())
                assert(0);
#endif
            ppage_check->second = vpage;

            DP ( if (warmup_complete[cpu]) {
            cout << "[SWAP] update inverse table NRU_vpage: " << hex << NRU_vpage << " new_vpage: ";
            cout << ppage_check->second << " ppage: " << ppage_check->first << dec << endl; });

            // update page_queue
            page_queue.pop();
            page_queue.push(vpage);

            // invalidate corresponding vpage and ppage from the cache hierarchy
            ooo_cpu[cpu].ITLB.invalidate_entry(NRU_vpage);
            ooo_cpu[cpu].DTLB.invalidate_entry(NRU_vpage);
            ooo_cpu[cpu].STLB.invalidate_entry(NRU_vpage);
            for (uint32_t i=0; i<BLOCK_SIZE; i++) {
                uint64_t cl_addr = (mapped_ppage << 6) | i;
                ooo_cpu[cpu].L1I.invalidate_entry(cl_addr);
                ooo_cpu[cpu].L1D.invalidate_entry(cl_addr);
                ooo_cpu[cpu].L2C.invalidate_entry(cl_addr);
                uncore.LLC.invalidate_entry(cl_addr);
            }

            // swap complete
            swap = 1;
        } else {
            uint8_t fragmented = 0;
            if (num_adjacent_page > 0)
                random_ppage = ( (++previous_ppage) % DRAM_PAGES ) + 1;
            else {
                random_ppage = (( champsim_rand.draw_rand() ) % DRAM_PAGES) + 1;
                fragmented = 1;
            }

            // encoding cpu number 
            // this allows ChampSim to run homogeneous multi-programmed workloads without VA => PA aliasing
            // (e.g., cpu0: astar  cpu1: astar  cpu2: astar  cpu3: astar...)
            //random_ppage &= (~((NUM_CPUS-1)<< (32-LOG2_PAGE_SIZE)));
            //random_ppage |= (cpu<<(32-LOG2_PAGE_SIZE)); 

            while (1) { // try to find an empty physical page number
                ppage_check = inverse_table.find(random_ppage); // check if this page can be allocated 
                if (ppage_check != inverse_table.end()) { // random_ppage is not available
                    DP ( if (warmup_complete[cpu]) {
                    cout << "vpage: " << hex << ppage_check->first << " is already mapped to ppage: " << random_ppage << dec << endl; }); 
                    
                    if (num_adjacent_page > 0)
                        fragmented = 1;

                    // try one more time
                    random_ppage = ( (champsim_rand.draw_rand() ) % DRAM_PAGES) + 1;
                    
                    // encoding cpu number 
                    //random_ppage &= (~((NUM_CPUS-1)<<(32-LOG2_PAGE_SIZE)));
                    //random_ppage |= (cpu<<(32-LOG2_PAGE_SIZE)); 
                }
                else
                    break;
            }

            // insert translation to page tables
            //printf("Insert  num_adjacent_page: %u  vpage: %lx  ppage: %lx\n", num_adjacent_page, vpage, random_ppage);
            page_table.insert(make_pair(vpage, random_ppage));
            inverse_table.insert(make_pair(random_ppage, vpage));
            page_queue.push(vpage);
            previous_ppage = random_ppage;
            num_adjacent_page--;
            num_page[cpu]++;
            allocated_pages++;

            // try to allocate pages contiguously
            if (fragmented) {
                num_adjacent_page = 1 << (rand() % 10);
                DP ( if (warmup_complete[cpu]) {
                cout << "Recalculate num_adjacent_page: " << num_adjacent_page << endl; });
            }
        }

        if (swap)
            major_fault[cpu]++;
        else
            minor_fault[cpu]++;
    }
    else {
        //printf("Found  vpage: %lx  random_ppage: %lx\n", vpage, pr->second);
    }

    pr = page_table.find(vpage);
#ifdef SANITY_CHECK
    if (pr == page_table.end())
        assert(0);
#endif
    uint64_t ppage = pr->second;

    uint64_t pa = ppage << LOG2_PAGE_SIZE;
    pa |= voffset;

    DP ( if (warmup_complete[cpu]) {
    cout << "[PAGE_TABLE] instr_id: " << instr_id << " vpage: " << hex << vpage;
    cout << " => ppage: " << (pa >> LOG2_PAGE_SIZE) << " vadress: " << unique_va << " paddress: " << pa << dec << endl; });

    if (swap)
        stall_cycle[cpu] = current_core_cycle[cpu] + SWAP_LATENCY;
    else
        stall_cycle[cpu] = current_core_cycle[cpu] + PAGE_TABLE_LATENCY;

    //cout << "cpu: " << cpu << " allocated unique_vpage: " << hex << unique_vpage << " to ppage: " << ppage << dec << endl;
	  //cout << hex << "LOG2_PAGE_SIZE: "<<LOG2_PAGE_SIZE <<" ppage: "<< ppage << " pa: " << pa<<" va: " << va << endl;

    return pa;
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
    uint64_t random_seed = 0;
    // check to see if knobs changed using getopt_long()
    int c;
    while (1) {
        static struct option long_options[] =
        {
            {"warmup_instructions", required_argument, 0, 'w'},
            {"simulation_instructions", required_argument, 0, 'i'},
            {"hide_heartbeat", no_argument, 0, 'h'},
            {"cloudsuite", no_argument, 0, 'c'},
            {"low_bandwidth",  no_argument, 0, 'b'},
            {"traces",  no_argument, 0, 't'},
            {"random_seed",  required_argument, 0, 'r'},
            {0, 0, 0, 0}      
        };

        int option_index = 0;

        c = getopt_long_only(argc, argv, "wihsbr", long_options, &option_index);

        // no more option characters
        if (c == -1)
            break;

        int traces_encountered = 0;

        switch(c) {
            case 'w':
                warmup_instructions = atol(optarg);
                break;
            case 'i':
                simulation_instructions = atol(optarg);
                break;
            case 'h':
                show_heartbeat = 0;
                break;
            case 'c':
                knob_cloudsuite = 1;
                MAX_INSTR_DESTINATIONS = NUM_INSTR_DESTINATIONS_SPARC;
                break;
            case 'b':
                knob_low_bandwidth = 1;
                break;
            case 't':
                traces_encountered = 1;
                break;
            case 'r':
                random_seed = atol(optarg);
                //printf("Random_seed: %ld\n", random_seed);
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
    //cout << "Scramble Loads: " << (knob_scramble_loads ? "ture" : "false") << endl;
    cout << "Number of CPUs: " << NUM_CPUS << endl;

    cout << "L1D sets: " << L1D_SET << endl;
    cout << "L1D ways: " << L1D_WAY << endl;
    cout << "L1D MSHR: " << L1D_MSHR_SIZE << endl;

    cout << "L2C sets: " << L2C_SET << endl;
    cout << "L2C ways: " << L2C_WAY << endl;
    cout << "L2C MSHR: " << L2C_MSHR_SIZE << endl; 

    cout << "LLC sets: " << LLC_SET << endl;
    cout << "LLC ways: " << LLC_WAY << endl;
    cout << "LLC MSHR: " << LLC_MSHR_SIZE << endl;

	  cout << "data LLC sets: " << data_LLC_SET << endl;
	  cout << "data LLC ways: " << data_LLC_WAY << endl;

    if (knob_low_bandwidth)
        DRAM_MTPS = 400;
    else
        DRAM_MTPS = 1600;

    // DRAM access latency
    tRP  = tRP_DRAM_CYCLE  * (CPU_FREQ / DRAM_IO_FREQ); 
    tRCD = tRCD_DRAM_CYCLE * (CPU_FREQ / DRAM_IO_FREQ); 
    tCAS = tCAS_DRAM_CYCLE * (CPU_FREQ / DRAM_IO_FREQ); 

    // default: 16 = (64 / 8) * (3200 / 1600)
    // it takes 16 CPU cycles to tranfser 64B cache block on a 8B (64-bit) bus 
    // note that dram burst length = BLOCK_SIZE/DRAM_CHANNEL_WIDTH
    DRAM_DBUS_RETURN_TIME = (BLOCK_SIZE / DRAM_CHANNEL_WIDTH) * (CPU_FREQ / DRAM_MTPS);

    printf("Off-chip DRAM Size: %u MB Channels: %u Width: %u-bit Data Rate: %u MT/s\n",
            DRAM_SIZE, DRAM_CHANNELS, 8*DRAM_CHANNEL_WIDTH, DRAM_MTPS);

    cout <<" DRAM column: "<<DRAM_COLUMNS<< "DRAM RANKS: "<<DRAM_RANKS<< " DRAM BANKS: "<<DRAM_BANKS<< " DRAM_ROWS: "<<DRAM_ROWS<< " DRAM_ROW_SIZE: "<<DRAM_ROW_SIZE<< endl;

    // end consequence of knobs

   cout <<"REPETETION_CODING: "<<REPETETION_CODING<< endl;
   cout <<"DIFFERENTIAL_SIGNALLING: "<<DIFFERENTIAL_SIGNALLING<< endl;
   cout <<"EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK: "<<EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK<< endl;
   cout <<"EXTRA_ARR_SIZE: "<<EXTRA_ARR_SIZE<< endl;
   cout <<"EXTRA_ARR_MAX_ADDR: "<<EXTRA_ARR_MAX_ADDR<< endl;
   cout <<"ARR_START_ADDR: "<<ARR_START_ADDR<< endl;
   cout <<"ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<< endl;
   cout <<"MAX_CLFLUSH_POSSIBLE: "<<MAX_CLFLUSH_POSSIBLE<< endl;
   cout <<"MAX_ARR_INC: "<<MAX_ARR_INC<< endl;
   cout <<"NUM_ACCESS_FOR_BIT1: "<<NUM_ACCESS_FOR_BIT1<< endl;

    // search through the argv for "-traces"
    int found_traces = 0;
    int count_traces = 0;
    cout << endl;
    for (int i=0; i<argc; i++) {
        if (found_traces) {
            printf("CPU %d runs %s\n", count_traces, argv[i]);
			

			//=================================  Generate LLC access stream ========================================== //
    		#if generate_llc_traces
				string running_trace = argv[i];
    			int start = 0;
    			string del = ".champsimtrace";
    			int end = running_trace.find(del);
   				string trace_name = running_trace.substr(start, end - start) + ".txt";
    			llc_access_stream_file.open(trace_name);
				if(NUM_CPUS > 1)
				{	
					cout << "This code is only tested for single cpu system." << endl;
					assert(0);
				}
			#endif
			//=================================  Generate LLC access stream ========================================== //
			
            sprintf(ooo_cpu[count_traces].trace_string, "%s", argv[i]);
            string trace_path_str=argv[i];
            size_t lastSlash = trace_path_str.rfind('/');
            string trace_path_str_split = trace_path_str.substr(0, lastSlash + 1);
            string new_string_path1 = trace_path_str_split+"inc_arr_len.trace.gz";
            string new_string_path2 = trace_path_str_split+"flush_file.trace.gz";
            string new_string_path3 = trace_path_str_split+"flush_file_llc_occ.trace.gz";
            string new_string_path4 = trace_path_str_split+"fence_file.trace.gz";
            string new_string_path5 = trace_path_str_split+"fence_rdtsc.trace.gz";
            string new_string_path6 = trace_path_str_split+"arr_access_file.trace.gz";

            //std::cout <<"==================" <<lastSlash <<"==============="<<new_string_path1 <<std::endl;

            // Check for receiver trace file.    
            if(count_traces == 0)
            {
								sprintf(ooo_cpu[count_traces].trace_string_inc, "%s", new_string_path1.c_str());
								sprintf(ooo_cpu[count_traces].trace_string_fl, "%s", new_string_path2.c_str());
								sprintf(ooo_cpu[count_traces].trace_string_fl_llc_occ, "%s", new_string_path3.c_str());
								sprintf(ooo_cpu[count_traces].trace_string_fen, "%s", new_string_path4.c_str());
								sprintf(ooo_cpu[count_traces].trace_string_fen_rd, "%s", new_string_path5.c_str());
								sprintf(ooo_cpu[count_traces].trace_string_arr_access, "%s", new_string_path6.c_str());
            }
            //sprintf(ooo_cpu[count_traces].trace_string_inc, "%s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/inc_arr_len.trace.gz");
            //sprintf(ooo_cpu[count_traces].trace_string_fl, "%s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/flush_file.trace.gz");
            //sprintf(ooo_cpu[count_traces].trace_string_fl_llc_occ, "%s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/flush_file_llc_occ.trace.gz");
            //sprintf(ooo_cpu[count_traces].trace_string_fen, "%s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/fence_file.trace.gz");
            //sprintf(ooo_cpu[count_traces].trace_string_fen_rd, "%s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/fence_rdtsc.trace.gz");
            //sprintf(ooo_cpu[count_traces].trace_string_arr_access, "%s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/arr_access_file.trace.gz");

            char *full_name = ooo_cpu[count_traces].trace_string,
                 *last_dot = strrchr(ooo_cpu[count_traces].trace_string, '.');

			ifstream test_file(full_name);
			if(!test_file.good()){
        cout<<" full_name: "<< full_name << endl;
				printf("TRACE FILE DOES NOT EXIST\n");
				assert(false);
			}
            // Check for receiver trace file.    
            if(count_traces == 0)
            {
                sprintf(ooo_cpu[count_traces].gunzip_command_inc, "gunzip -c %s", new_string_path1.c_str());
                sprintf(ooo_cpu[count_traces].gunzip_command_fl, "gunzip -c %s", new_string_path2.c_str());
                sprintf(ooo_cpu[count_traces].gunzip_command_fl_llc_occ, "gunzip -c %s", new_string_path3.c_str());
                sprintf(ooo_cpu[count_traces].gunzip_command_fen, "gunzip -c %s", new_string_path4.c_str());
                sprintf(ooo_cpu[count_traces].gunzip_command_fen_rd, "gunzip -c %s", new_string_path5.c_str());
                sprintf(ooo_cpu[count_traces].gunzip_command_arr_access, "gunzip -c %s", new_string_path6.c_str());
            }
                //sprintf(ooo_cpu[count_traces].gunzip_command_inc, "gunzip -c %s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/inc_arr_len.trace.gz");
                //sprintf(ooo_cpu[count_traces].gunzip_command_fl, "gunzip -c %s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/flush_file.trace.gz");
                //sprintf(ooo_cpu[count_traces].gunzip_command_fl_llc_occ, "gunzip -c %s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/flush_file_llc_occ.trace.gz");
                //sprintf(ooo_cpu[count_traces].gunzip_command_fen, "gunzip -c %s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/fence_file.trace.gz");
                //sprintf(ooo_cpu[count_traces].gunzip_command_fen_rd, "gunzip -c %s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/fence_rdtsc.trace.gz");
                //sprintf(ooo_cpu[count_traces].gunzip_command_arr_access, "gunzip -c %s", "/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23/arr_access_file.trace.gz");

                
            if (full_name[last_dot - full_name + 1] == 'g') // gzip format
                sprintf(ooo_cpu[count_traces].gunzip_command, "gunzip -c %s", argv[i]);
            else if (full_name[last_dot - full_name + 1] == 'x') // xz
                sprintf(ooo_cpu[count_traces].gunzip_command, "xz -dc %s", argv[i]);
            else {
                cout << "ChampSim does not support traces other than gz or xz compression!" << endl; 
                assert(0);
            }

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
            // Check for receiver trace file.    
            if(count_traces == 0)
            {
                ooo_cpu[count_traces].trace_file_inc = popen(ooo_cpu[count_traces].gunzip_command_inc, "r");
                ooo_cpu[count_traces].trace_file_fl = popen(ooo_cpu[count_traces].gunzip_command_fl, "r");
                ooo_cpu[count_traces].trace_file_fl_llc_occ = popen(ooo_cpu[count_traces].gunzip_command_fl_llc_occ, "r");
                ooo_cpu[count_traces].trace_file_fen = popen(ooo_cpu[count_traces].gunzip_command_fen, "r");
                ooo_cpu[count_traces].trace_file_fen_rd = popen(ooo_cpu[count_traces].gunzip_command_fen_rd, "r");
                ooo_cpu[count_traces].trace_file_arr_access = popen(ooo_cpu[count_traces].gunzip_command_arr_access, "r");
                //ooo_cpu[count_traces].trace_file_arr_access = fopen(ooo_cpu[count_traces].gunzip_command_arr_access, "r");
            
								cout <<"count_traces: "<<count_traces<< " trace_file is: "<<ooo_cpu[count_traces].trace_file<<" inc: "<<ooo_cpu[count_traces].trace_file_inc<<" fl_llc_occ: "<<ooo_cpu[count_traces].trace_file_fl_llc_occ<<" fl: "<<ooo_cpu[count_traces].trace_file_fl<<" fen: "<<ooo_cpu[count_traces].trace_file_fen<<" fen_rd: "<<ooo_cpu[count_traces].trace_file_fen_rd<<" arr_access: "<<ooo_cpu[count_traces].trace_file_arr_access<<endl;

								if (ooo_cpu[count_traces].trace_file == NULL || ooo_cpu[count_traces].trace_file_inc == NULL || ooo_cpu[count_traces].trace_file_fl_llc_occ == NULL || ooo_cpu[count_traces].trace_file_fl == NULL || ooo_cpu[count_traces].trace_file_fen_rd == NULL || ooo_cpu[count_traces].trace_file_fen == NULL) 
								{
										printf("\n*** Trace file not found: %s ***\n\n", argv[i]);
										assert(0);
								}
            }
            // Check for sender trace file.    
						if (ooo_cpu[count_traces].trace_file == NULL) 
						{
								printf("\n*** Trace file not found: %s ***\n\n", argv[i]);
								assert(0);
						}
            //cout << "fseek done"<<endl;
            //fseek(ooo_cpu[count_traces].trace_file, sizeof(input_instr)*5, SEEK_SET); // Updating the location of pointer in file. 
            count_traces++;
            if (count_traces > NUM_CPUS) {
                printf("\n*** Too many traces for the configured number of cores ***\n\n");
                assert(0);
            }
        }
        else if(strcmp(argv[i],"-traces") == 0) {
            found_traces = 1;
        }
    }

    if (count_traces != NUM_CPUS) {
        printf("\n*** Not enough traces for the configured number of cores ***\n\n");
        cout << count_traces<<" "<< NUM_CPUS<<endl;
        assert(0);
    }
    // end trace file setup
    
// TODO: can we initialize these variables from the class constructor?
    seed_number = random_seed; //SEED_NUMBER ; //50; //100;
    srand(seed_number);
    champsim_seed = seed_number;
    cout << "seed_number is: "<<seed_number<<endl;
    //Updating wait period
    WAIT_PERIOD[0] = FIRST_WAIT_OF_SENDER; // In set-up period, sender waits for longer time.
    WAIT_PERIOD[1] = SENDER_WAIT_PERIOD;  // Sender wait period, once transmission begins.
    WAIT_PERIOD[2] = RECEIVER_WAIT_PERIOD;


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

        ooo_cpu[i].flush_more_address = 0;
        // SHARED CACHE
        uncore.LLC.cache_type = IS_LLC;
        uncore.LLC.fill_level = FILL_LLC;
        uncore.LLC.upper_level_icache[i] = &ooo_cpu[i].L2C;
        uncore.LLC.upper_level_dcache[i] = &ooo_cpu[i].L2C;
        uncore.LLC.lower_level = &uncore.DRAM;


        CENTRAL_CLOCK[i] = WAIT_PERIOD[0];

        cout << "LLC: " << uncore.LLC.lower_level << endl;

				uncore.LLC.NUM_SKEW = 2;

				// For Mirage
        uncore.data_LLC.cache_type = IS_DATA_LLC;
				uncore.LLC.side_level = &uncore.data_LLC;  
				uncore.data_LLC.side_level = &uncore.LLC;

        // OFF-CHIP DRAM
        uncore.DRAM.fill_level = FILL_DRAM;
        uncore.DRAM.upper_level_icache[i] = &uncore.LLC;
        uncore.DRAM.upper_level_dcache[i] = &uncore.LLC;
        for (uint32_t i=0; i<DRAM_CHANNELS; i++) {
            uncore.DRAM.RQ[i].is_RQ = 1;
            uncore.DRAM.WQ[i].is_WQ = 1;
        }

        warmup_complete[i] = 0;
        //all_warmup_complete = NUM_CPUS;
        simulation_complete[i] = 0;
        current_core_cycle[i] = 0;
        stall_cycle[i] = 0;
        
        previous_ppage = 0;
        num_adjacent_page = 0;
        num_cl[i] = 0;
        allocated_pages = 0;
        num_page[i] = 0;
        minor_fault[i] = 0;
        major_fault[i] = 0;
				cpu_on_wait[i] = 0;                                                      
        fence_called_on_cpu[i] = 0;                                             
        recorded_rdtsc_value[i] = 0;                                             
        fence_instr[i] = 0;
        for(int num =0; num < NUM_ACCESS_FOR_BIT1; num++)
        clflush_on_cpu[i][num] = 0;

        WAIT_CYCLES[i] = 0;
    }

    if(DIFFERENTIAL_SIGNALLING != 0)
    {
        for(int differ=0; differ < DIFFERENTIAL_SIGNALLING; differ++)
				    differ_sig[differ] = 0; 
            
    }
    if(REPETETION_CODING != 0)
    {
        for(int differ=0; differ < REPETETION_CODING; differ++)
            repeat_code[differ] = 0;
    }


    // Initialize the reduced_array_size for receiver core.
    ooo_cpu[0].REDUCED_ARR_SIZE = (ARR_MAX_LIMIT - (EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK * 64 ));
    cout << "ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<<" EXTRA_ARR_SIZE: "<<EXTRA_ARR_SIZE<<" ooo_cpu[0].REDUCED_ARR_SIZE: "<<ooo_cpu[0].REDUCED_ARR_SIZE<<" EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK: "<<EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK<<endl;
    uncore.LLC.llc_initialize_replacement();
    uncore.data_LLC.llc_initialize_replacement();

		cout << "MIRAGE: "<< MIRAGE << endl;
		cout << "PRINCE: "<< PRINCE_INDEX << endl;
		cout <<"LLC NUM way: "<< uncore.LLC.NUM_WAY<<" sets per skew: "<< uncore.LLC.NUM_SET<<" NUM skew: " << uncore.LLC.NUM_SKEW <<" block size: "<< BLOCK_SIZE << endl;
    cout <<"NUM_SENDER_DISTURBANCES: "<<NUM_SENDER_DISTURBANCES<<endl;
    cout <<"LLC_LATENCY: "<< LLC_LATENCY << endl;
    cout <<"HIT_MISS_THRESHOLD: " << HIT_MISS_THRESHOLD << endl;
    cout <<"HIT_MISS_THRESHOLD_LLC_OCC_CHK: " << HIT_MISS_THRESHOLD_LLC_OCC_CHK <<endl;
    cout <<"UNROLLING_FACTOR: " <<UNROLLING_FACTOR<<endl;
    cout <<"UNROLLING_FACTOR_FOR_LLC_OCC_CHK: "<<UNROLLING_FACTOR_FOR_LLC_OCC_CHK<<endl;


		#if(PRINCE_INDEX)
      uncore.LLC.mtrand->seed(1);
			uncore.LLC.init_cache_indexing(uncore.LLC.NUM_WAY/uncore.LLC.NUM_SKEW, uncore.LLC.NUM_SET, uncore.LLC.NUM_SKEW, BLOCK_SIZE); //TODO: mem_size is still not clear.
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
						//count_LLC_valid_blocks();
            // proceed one cycle
            current_core_cycle[i]++;

            
// =========== Calculate LLC occupancy for initial set-up.

        if((Calculate_LLC_occupancy == 1 /*|| Calculate_LLC_occupancy == 3*/) && ooo_cpu[i].num_retired >= fence_instr[i])
        {
            //Calculate_LLC_occupancy ++; 
            Calculate_LLC_occupancy =0; 
            uint64_t cnt = 0;  
            //cout<<"FIRST_FENCE: "<<FIRST_FENCE<<" Calculated LLC occupancy is : ";
						for(uint32_t i1=0; i1 < uncore.LLC.NUM_SET; i1++)
						{
							for(uint32_t j1=0; j1 < uncore.LLC.NUM_WAY; j1++)
							{
									if(uncore.LLC.block[i1][j1].valid == 1 && uncore.LLC.block[i1][j1].instr_id > FIRST_FENCE )
								  {
												cnt ++;
									}
							}
						}
            //cout << cnt << endl;
        }

// =========== Calculate LLC occupancy for initial set-up.

						
						// check for warmup
            // warmup complete
            if ((warmup_complete[i] == 0) && (ooo_cpu[i].num_retired >= warmup_instructions)) {
                warmup_complete[i] = 1;
                all_warmup_complete++;
            }
            if (all_warmup_complete == NUM_CPUS) { // this part is called only once when all cores are warmed up
                all_warmup_complete++;
                finish_warmup();
            }

            // Trigger flush instructions, if it is the time.
            for(int num=0; num<MAX_CLFLUSH_POSSIBLE; num++)
						{   if(clflush_on_cpu[i][num] != 0)
                {
                    if(cpu_cycle_to_trigger_clflush[i][num] <= current_core_cycle[i])
                       { 
                           cout << "inside clflush check" << endl;
                           cache_line_flush(i, num);  
                           cpu_cycle_to_trigger_clflush[i][num] = 0;
                       }
                }  
            }
                // Decrement CENTRAL_CLOCK outside stall condition.
                CENTRAL_CLOCK[i] --;
                //cout << " Reduction" << CENTRAL_CLOCK[i]<<" cpu: "<<i << endl;
            /*    if(cpu_on_wait[0]==1 && cpu_on_wait[1]==1)
                    cout << "We have finished execution and waiting for clock to complete. CENTRAL_CLOCK: "<< CENTRAL_CLOCK[i]<<" cpu: "<<i<<" current core cycle: "<<current_core_cycle[i]<< endl;  */

            
            if(seen_llc_occ_chk == 1 && NUM_CPUS > 1)
            { 
                // Tracking the currently running error-correction iteration number.
                CURR_LLC_OCC_CHK_ITR ++;

                // Switch-on the phase
                LLC_OCC_CHK_PHASE = 1;
                cout <<" received Switched on LLC occpancy check phase"<<endl;
                if(CURR_LLC_OCC_CHK_ITR == 1)
							      cout <<" cycle: "<<current_core_cycle[i]<< " bit received is: "<< bit_revealed_is << " before llc_occ_chk, CURR_LLC_OCC_CHK_ITR: "<<CURR_LLC_OCC_CHK_ITR << endl; 
                LLC_OCCUPANCY_CHECK_ONGOING = 1;
                // Reset the flag counter.
                seen_llc_occ_chk --;//This ensure variable value oscillate betwn 2 and 1, as there is a redundant call to LLC_occupancy_check in the beginning to bring instructions and data blocks of this call to cache.
							  //receiver_checks_for_a_miss = 0; 
							  //bit_revealed_is=0; 
            }            

            if(seen_error_correction == 2 && NUM_CPUS > 1)
            { 
                // Tracking the currently running error-correction iteration number.
                CURR_ERR_CORR_ITR ++;
		cout <<"received, CURR_ERR_CORR_ITR: "<< CURR_ERR_CORR_ITR<<endl;

                if(CURR_ERR_CORR_ITR == 1)
                {
                    // Switch-on the phase
                    if(bit_revealed_is != 1 || MISSES_OBSERVED != NUM_SENDER_DISTURBANCES) //The marker bit at error_correction is always 1 and MISSES_OBSERVED should be 2. MISSES_OBSERVED can be 1 if both misses belong to the same chunk consists of #UNROLLING_FACTOR array accesses.
                    {    ERROR_CORRECTION_PHASE = 1;
			 ITR_TO_FILL_HOLE ++;
                         // Used to decide the status of next ERROR_CORRECTION_PHASE value.
                         ERROR_CORRECTION_PHASE_LAST = ERROR_CORRECTION_PHASE;
                         BIT_REVEALED_LAST = bit_revealed_is;
			 cout <<" received, MISSES_OBSERVED: "<<MISSES_OBSERVED<<" bit_revealed_is: "<<bit_revealed_is<<" ERROR_CORRECTION_PHASE_LAST: "<<ERROR_CORRECTION_PHASE_LAST<<" ITR_TO_FILL_HOLE: "<<ITR_TO_FILL_HOLE<<endl;
                    }
                    else
                    {
                        DONT_INC_ARR = 1;
                        BIT_REVEALED_LAST = bit_revealed_is;
                        ERROR_CORRECTION_PHASE_LAST = ERROR_CORRECTION_PHASE;
                        if(ERROR_CORRECTION_PHASE_LAST != 0)
                        {   cout <<"ERROR_CORRECTION_PHASE_LAST is not set properly" <<endl;
                            assert(0);}
			 cout <<" received, DONT_INC_ARR, MISSES_OBSERVED: "<<MISSES_OBSERVED<<" bit_revealed_is: "<<bit_revealed_is<<endl;
                    }
                }
                else if(CURR_ERR_CORR_ITR > 1)
                {
	            int temp_var= NUM_SENDER_DISTURBANCES - MISSES_OBSERVED;
                    // Switch-on the phase
                    if(bit_revealed_is == 0 && ERROR_CORRECTION_PHASE_LAST == 1 && ITR_TO_FILL_HOLE < temp_var) //The marker bit at error_correction is always 1 and MISSES_OBSERVED should be 2. MISSES_OBSERVED can be 1 if both misses belong to the same chunk consists of #UNROLLING_FACTOR array accesses.
                    {    ERROR_CORRECTION_PHASE = 1;
			 ITR_TO_FILL_HOLE ++;
                         ERROR_CORRECTION_PHASE_LAST = ERROR_CORRECTION_PHASE;
                         // Used to decide the status of next ERROR_CORRECTION_PHASE value.
		         cout <<" received, MISSES_OBSERVED: "<<MISSES_OBSERVED<<" bit_revealed_is: "<<bit_revealed_is<<" ERROR_CORRECTION_PHASE_LAST: "<<ERROR_CORRECTION_PHASE_LAST<<" ITR_TO_FILL_HOLE: "<<ITR_TO_FILL_HOLE<<" temp_var: "<<temp_var<<endl;
                    }
                    else
                    {
                        DONT_INC_ARR = 1;
                        ERROR_CORRECTION_PHASE_LAST = ERROR_CORRECTION_PHASE;
		        cout<<"received DONT_INC_ARR MISSES_OBSERVED: "<<MISSES_OBSERVED<<" bit_revealed_is: "<<bit_revealed_is<<" ERROR_CORRECTION_PHASE_LAST: "<<ERROR_CORRECTION_PHASE_LAST<<" temp_var: "<<temp_var<<endl;
                    }
		    BIT_REVEALED_LAST = bit_revealed_is;
                }
                else if(CURR_ERR_CORR_ITR > NUM_SENDER_DISTURBANCES)
                {
                    printf("The value for CURR_ERR_CORR_ITR has exceeded than the NUM_SENDER_DISTURBANCES");
                    assert(0);
                }
                HOLE_FILLING_PROCESS_ONGOING = 1;
                // Reset the flag counter.
                seen_error_correction --;//This ensure variable value oscillate betwn 2 and 1, as there is a redundant call to error_correction in the beginning to bring instructions and data blocks to cache.
                if(CURR_ERR_CORR_ITR == 1)
		    cout <<" cycle: "<<current_core_cycle[i]<< " bit received is: "<< bit_revealed_is <<" , before error_correction, hence error_correction phase is: "<<(unsigned)ERROR_CORRECTION_PHASE<<" err corr itr: "<<CURR_ERR_CORR_ITR <<" Misses_observed: "<<MISSES_OBSERVED<< endl; 
		//receiver_checks_for_a_miss = 0; 
		bit_revealed_is=0; 
            }            
             
            // core might be stalled due to page fault or branch misprediction
            if (stall_cycle[i] <= current_core_cycle[i]) {

#if !(WAIT_IMPLEMENTATION)
          if (LR_NO_ERR_CORR || LR_ERR_CORR_BOTH_ALGO) //For LeakyRand with no error correction.
          {     if(NUM_CPUS > 1 && SETUP_DONE[0] == 1 && SETUP_DONE[1] == 1) //XXX Signify that receiver is done with cache prime and sender is ready to send first bit.  
               {
                        if(CACHE_FILL_DONE == 0)
                            cout<<"=========cache fill is done========"<<endl;
                        CACHE_FILL_DONE = 1;
               }
          }
               //Receiver i.e. cpu 0 has seen wait once and sender has seen wait twice. Implementation dependent.
                if(NUM_CPUS > 1 && SETUP_DONE[0] == 1 && SETUP_DONE[1] == 2) //XXX Signify that receiver is done with cache prime and sender is done with sending the first bit.	
                    {
                        if(FIRST_TIME_CACHE_SETUP_DONE == 0)
														cout<<"=========setup is done========"<<endl;
                        FIRST_TIME_CACHE_SETUP_DONE = 1;
                    }

                if(NUM_CPUS > 1 && ((i==0 && TURN==0 && cpu_on_wait[i] == 1 && cpu_on_wait[1] == 1) || (i==1 && TURN==1 && cpu_on_wait[i] == 1 && cpu_on_wait[0] == 1)))
                { 

                          cpu_on_wait[i] = 0; //Cpu's wait has ended.
                          count_LLC_valid_blocks();
                          if(FIRST_TIME_CACHE_SETUP_DONE)
                          {  
                              if(i == 1)
                              {
                                   if( HOLE_FILLING_PROCESS_ONGOING == 0 && LLC_OCCUPANCY_CHECK_ONGOING != 1)
                                   {   
                                       cout <<" cycle: "<<current_core_cycle[i]<< " bit received is: "<< bit_revealed_is << endl; 
                                   }
                                   receiver_checks_for_a_miss = 0; 
                                   bit_revealed_is=0;
                                   HOLE_FILLING_PROCESS_ONGOING = 0;
                                   LLC_OCCUPANCY_CHECK_ONGOING = 0;
                                   dont_fill_empty_slot = 0;
                                   CURR_ERR_CORR_ITR = 0;
                                   MISSES_OBSERVED = 0;
			           ITR_TO_FILL_HOLE = 0;
                                   CURR_LLC_OCC_CHK_ITR = 0;
                              }
                              else
                              {
                                   receiver_checks_for_a_miss = 1; 
                                   bit_revealed_is=0;  
                                   cout <<" receiver_checks_for_a_miss is set to 1 " <<endl;
                              }
                              // Record error stats after receiver has completed its execution.
                              if(i == 1)
                              record_error_stats(); 
                          }
                          cout <<endl<<endl<< "+++++++++++++++++++++++++++++++++++++++++++++++++"<<endl<< "SSSSSSSS cpu awakened from sleep is: "<<i 
                               << " current cycle: "<<current_core_cycle[i]<< " instr unique_id: "<<ooo_cpu[i].instr_unique_id
                               <<" num_retired:  "<<ooo_cpu[i].num_retired << endl;     
                                                    
                          #if(IEEE_SEED)
                          // This code is only for 2-core system. i is cpu number awakened from sleep.
                          if(i==1)
                          {
                              cout <<"Misses observed for cpu0 and cpu1 are: "<<LLC_MISS_COUNT[0]<<" "<<LLC_MISS_COUNT[1]<<endl;
                              LLC_MISS_COUNT[1] = 0;
                              LLC_MISS_COUNT[0] = 0;
                          } 
                          // calculate llc occupancy, specifically meant for IEEE SEED. 
                          if(FIRST_FENCE != 0)
                          {
                              if(i == 0) // CPU awakened from sleep is 0, count occupancy of CPU 1.
                                  calculate_llc_occupancy_for_specific_cpu(FIRST_FENCE);
                              else if(i == 1)// CPU awakened from sleep is 1, count occupancy of CPU 0.
                                  calculate_llc_occupancy_for_specific_cpu(FIRST_FENCE);
                          }
                          #endif
                }
                else if( NUM_CPUS == 1 && cpu_on_wait[i] == 1)
                {
                  cpu_on_wait[i] = 0;
                  cout << "cpu 0 is awakened from sleep" << endl;
                }
#endif
                if( (fence_called_on_cpu[i] == 1) && (ooo_cpu[i].num_retired >= fence_instr[i]) )
                {
                  fence_called_on_cpu[i] = 0;
		               if(DETAILED_PRINT || IEEE_SEED)
                       cout << "FFFFFFFFence is removed for cpu: "<<i<<" ooo_cpu[i].num_retired: "<<ooo_cpu[i].num_retired<< " fence_instr[i]: " << fence_instr[i] <<" core cycle: "<< current_core_cycle[i]<< " instr unique_id: "<<ooo_cpu[i].instr_unique_id<< endl;
									count_LLC_valid_blocks();  //cout << endl <<endl;    
                }

#if (WAIT_IMPLEMENTATION == 1)

                int counter = 0;

                for(int ii=0; ii<NUM_CPUS; ii++)
                { 
                    if(CENTRAL_CLOCK[ii] == 0)
                        counter ++;

                    if(counter == (NUM_CPUS))
                        CENTRAL_CLOCK_ALL_COMPLETE = 1;
                }
                if(CENTRAL_CLOCK_ALL_COMPLETE)
                {   
                    CENTRAL_CLOCK_ALL_COMPLETE = 0;
                    if(TURN == 1)
										{
													 cout <<" cycle: "<<current_core_cycle[i]<< " bit received is: "<< bit_revealed_is<< endl;
													 receiver_checks_for_a_miss = 0;
													 bit_revealed_is=0;
										}
										else
										{
													 receiver_checks_for_a_miss = 1;
													 bit_revealed_is=0;  
													 cout <<" receiver_checks_for_a_miss is set to 1 " <<endl;
										}
                    cpu_on_wait[TURN] = 0;

                    for(int ii=0; ii<NUM_CPUS; ii++)
                        CENTRAL_CLOCK[ii] = WAIT_PERIOD[TURN+1];

                    cout << "central clock has been reset to: "<< CENTRAL_CLOCK[i] << " TURN: "<<TURN <<" current_core_cycle: "<<current_core_cycle[TURN] << endl;
                    cout <<endl<<endl<< "+++++++++++++++++++++++++++++++++++++++++++++++++"<<endl<< "SSSSSSSS cpu awakened from sleep is: "<<TURN
                               << " current cycle: "<<current_core_cycle[TURN]<< " instr unique_id: "<<ooo_cpu[TURN].instr_unique_id
                               <<" num_retired:  "<<ooo_cpu[TURN].num_retired << endl;
                }
               /* else
                {
                    CENTRAL_CLOCK[i] --;
                    cout << " Reduction" << CENTRAL_CLOCK[i]<<" cpu: "<<i << endl;
                    if(cpu_on_wait[0]==1 && cpu_on_wait[1]==1)
                        cout << "We have finished execution and waiting for clock to complete. CENTRAL_CLOCK: "<< CENTRAL_CLOCK[i]<<" cpu: "<<i<<" current core cycle: "<<current_core_cycle[i]<< endl;
                }*/ 
#endif
                // fetch unit
                if (ooo_cpu[i].ROB.occupancy < ooo_cpu[i].ROB.SIZE  && cpu_on_wait[i] == 0 && fence_called_on_cpu[i] == 0) //cpu_on_wait is 1 when core is on sleep
                {
										//cout << "ooo_cpu[i].num_retired: "<<ooo_cpu[i].num_retired << endl;
                    // handle branch
                    if (ooo_cpu[i].fetch_stall == 0) 
                        ooo_cpu[i].handle_branch();
                }

                // fetch
                ooo_cpu[i].fetch_instruction();


                // schedule (including decode latency)
                uint32_t schedule_index = ooo_cpu[i].ROB.next_schedule;
                if ((ooo_cpu[i].ROB.entry[schedule_index].scheduled == 0) && (ooo_cpu[i].ROB.entry[schedule_index].event_cycle <= current_core_cycle[i]))
                    { ooo_cpu[i].schedule_instruction();
											//if(i==0 && current_core_cycle[0] > 7646428 && current_core_cycle[0] < 7646477)
												//			cout << "schedule_index: " << schedule_index << " current_core_cycle[0]: " << current_core_cycle[0]<<" ip: " <<ooo_cpu[i].ROB.entry[schedule_index].ip<< endl;
                    }

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
                
                //count_LLC_valid_blocks();

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
            }

            // check for deadlock
            if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].ip && (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].event_cycle + DEADLOCK_CYCLE) <= current_core_cycle[i])
                print_deadlock(i);

          /*  // check for warmup
            // warmup complete
            if ((warmup_complete[i] == 0) && (ooo_cpu[i].num_retired > warmup_instructions)) {
                warmup_complete[i] = 1;
                all_warmup_complete++;
            }
            if (all_warmup_complete == NUM_CPUS) { // this part is called only once when all cores are warmed up
                all_warmup_complete++;
                finish_warmup();
            } */

            /*
            if (all_warmup_complete == 0) { 
                all_warmup_complete = 1;
                finish_warmup();
            }
            if (ooo_cpu[1].num_retired > 0)
                warmup_complete[1] = 1;
            */
            
            // simulation complete
            if ((all_warmup_complete > NUM_CPUS) && (simulation_complete[i] == 0) && (ooo_cpu[i].num_retired >= (ooo_cpu[i].begin_sim_instr + ooo_cpu[i].simulation_instructions))) {
                simulation_complete[i] = 1;
                ooo_cpu[i].finish_sim_instr = ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr;
                ooo_cpu[i].finish_sim_cycle = current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle;

                cout << "Finished CPU " << i << " instructions: " << ooo_cpu[i].finish_sim_instr << " cycles: " << ooo_cpu[i].finish_sim_cycle;
                cout << " cumulative IPC: " << ((float) ooo_cpu[i].finish_sim_instr / ooo_cpu[i].finish_sim_cycle);
                cout << " (Simulation time: " << elapsed_hour << " hr " << elapsed_minute << " min " << elapsed_second << " sec) " << endl;

                record_roi_stats(i, &ooo_cpu[i].L1D);
                record_roi_stats(i, &ooo_cpu[i].L1I);
                record_roi_stats(i, &ooo_cpu[i].L2C);
                record_roi_stats(i, &uncore.LLC);
                record_roi_stats(i, &uncore.data_LLC);

                all_simulation_complete++;
            }

            if (all_simulation_complete == NUM_CPUS)
                run_simulation = 0;
        }

        // TODO: should it be backward?
        uncore.LLC.operate();
        uncore.DRAM.operate();
    }
	//Closing the file;
	llc_access_stream_file.close();

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
        cout << endl << "Total Simulation Statistics (not including warmup)" << endl;
        for (uint32_t i=0; i<NUM_CPUS; i++) {
            cout << endl << "CPU " << i << " cumulative IPC: " << (float) (ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr) / (current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle); 
            cout << " instructions: " << ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr << " cycles: " << current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle << endl;
#ifndef CRC2_COMPILE
            print_sim_stats(i, &ooo_cpu[i].L1D);
            print_sim_stats(i, &ooo_cpu[i].L1I);
            print_sim_stats(i, &ooo_cpu[i].L2C);
            ooo_cpu[i].L1D.l1d_prefetcher_final_stats();
            ooo_cpu[i].L2C.l2c_prefetcher_final_stats();
#endif
            print_sim_stats(i, &uncore.LLC);
            print_sim_stats(i, &uncore.data_LLC);
        }
    }

    cout << endl << "Region of Interest Statistics" << endl;
    for (uint32_t i=0; i<NUM_CPUS; i++) {
        cout << endl << "CPU " << i << " cumulative IPC: " << ((float) ooo_cpu[i].finish_sim_instr / ooo_cpu[i].finish_sim_cycle); 
        cout << " instructions: " << ooo_cpu[i].finish_sim_instr << " cycles: " << ooo_cpu[i].finish_sim_cycle << endl;
#ifndef CRC2_COMPILE
        print_roi_stats(i, &ooo_cpu[i].L1D);
        print_roi_stats(i, &ooo_cpu[i].L1I);
        print_roi_stats(i, &ooo_cpu[i].L2C);
#endif
        print_roi_stats(i, &uncore.LLC);
        print_roi_stats(i, &uncore.data_LLC);
        cout << "Major fault: " << major_fault[i] << " Minor fault: " << minor_fault[i] << endl;
    }

    for (uint32_t i=0; i<NUM_CPUS; i++) {
        ooo_cpu[i].L1D.l1d_prefetcher_final_stats();
        ooo_cpu[i].L2C.l2c_prefetcher_final_stats();
    }

		uint32_t count_valid = 0, count_dirty = 0, count_invalid = 0;

		for(uint32_t i=0; i < uncore.LLC.NUM_SET; i++)
			{
				for(uint32_t j=0; j < uncore.LLC.NUM_WAY; j++)
      	{
						if(uncore.LLC.block[i][j].valid == 1)
              count_valid ++;

            if(uncore.LLC.block[i][j].dirty == 1)
              count_dirty ++;

            if(uncore.LLC.block[i][j].valid == 0)
              count_invalid ++;
      	}
			}

		cout << "Tag LLC Valid blocks: "<< count_valid << " Dirty blocks: "<<count_dirty << " Invalid blocks: "<< count_invalid << endl;			

		count_valid = 0, count_dirty = 0, count_invalid = 0;

    for(uint32_t i=0; i < uncore.data_LLC.NUM_SET; i++)
      {
        for(uint32_t j=0; j < uncore.data_LLC.NUM_WAY; j++)
        {
            if(uncore.data_LLC.block[i][j].valid == 1)
              count_valid ++;

            if(uncore.data_LLC.block[i][j].dirty == 1)
              count_dirty ++;

            if(uncore.data_LLC.block[i][j].valid == 0)
              count_invalid ++;
        }
      }
   cout << "Data LLC Valid blocks: "<< count_valid << " Dirty blocks: "<<count_dirty << " Invalid blocks: "<< count_invalid << endl;
cout << "Spills: "<< spills << endl;

// Printing stats for errors.
cout << "sender_evicted_own_active_block: "<< sender_evicted_own_active_block <<endl << 
"sender_evicted_own_inactive_block: "<< sender_evicted_own_inactive_block <<endl<<
"sender_evicted_receiver_inactive_block: " <<sender_evicted_receiver_inactive_block <<endl<<
"sender_evicted_receiver_unrecognizable_active_block: "<<sender_evicted_receiver_unrecognizable_active_block << endl << 
"sender_evicted_receiver_recognizable_active_block: "<<sender_evicted_receiver_recognizable_active_block <<endl<<
"sender_filled_up_empty_block: "<< sender_filled_up_empty_block << endl<<
"correctly_transmitted_zero: "<<correctly_transmitted_zero << endl;

/* ==================================== PRINT page table =========================================== */
/*    for (auto pr = page_table.begin(); pr != page_table.end(); pr++) {
             cout<<hex <<"vpage: " <<pr->first<<" ppage: "<<pr->second<<endl;

    }*/

   check_inclusive();

#ifndef CRC2_COMPILE
    uncore.LLC.llc_replacement_final_stats();
    print_dram_stats();
#endif

/*  for(uint32_t i=0; i < ooo_cpu[0].L1D.NUM_SET; i++)
      {
        for(uint32_t j=0; j < ooo_cpu[0].L1D.NUM_WAY; j++)
        {
          cout <<"L1D: i: "<<i<<" j: "<<j<<" "<<"block address: "<<hex<< ooo_cpu[0].L1D.block[i][j].address<<" tag: "<<ooo_cpu[0].L1D.block[i][j].tag <<" full_addr: "<<ooo_cpu[0].L1D.block[i][j].full_addr<<dec<<"  valid: " <<(int)ooo_cpu[0].L1D.block[i][j].valid<<"  dirty: "<< (int)ooo_cpu[0].L1D.block[i][j].dirty<<"  instr_id: "<< ooo_cpu[0].L1D.block[i][j].instr_id<< endl;
} }
cout <<"====================================================================="<<endl;
*/
/*  for(uint32_t i=0; i < ooo_cpu[1].L1D.NUM_SET; i++)
      {
        for(uint32_t j=0; j < ooo_cpu[1].L1D.NUM_WAY; j++)
        {
          cout <<"L1D: i: "<<i<<" j: "<<j<<" "<<"block address: "<<hex<< ooo_cpu[1].L1D.block[i][j].address<<" tag: "<<ooo_cpu[1].L1D.block[i][j].tag <<" full_addr: "<<ooo_cpu[1].L1D.block[i][j].full_addr<<dec<<"  valid: " <<(int)ooo_cpu[1].L1D.block[i][j].valid<<"  dirty: "<< (int)ooo_cpu[1].L1D.block[i][j].dirty<<"  instr_id: "<< ooo_cpu[1].L1D.block[i][j].instr_id<< endl;
} }
cout <<"====================================================================="<<endl;
*/

/*
for(uint32_t i=0; i < ooo_cpu[0].L1I.NUM_SET; i++)
      {
        for(uint32_t j=0; j < ooo_cpu[0].L1I.NUM_WAY; j++)
        {
          cout <<"L1I: i: "<<i<<" j: "<<j<<" "<<"block address: "<<hex<< ooo_cpu[0].L1I.block[i][j].address<<" tag: "<<ooo_cpu[0].L1I.block[i][j].tag <<" full_addr: "<<ooo_cpu[0].L1I.block[i][j].full_addr<<dec<<"  valid: " <<(int)ooo_cpu[0].L1I.block[i][j].valid<<"  dirty: "<< (int)ooo_cpu[0].L1I.block[i][j].dirty<<"  instr_id: "<< ooo_cpu[0].L1I.block[i][j].instr_id<< endl;
} }

cout <<"====================================================================="<<endl;

for(uint32_t i=0; i < ooo_cpu[0].L2C.NUM_SET; i++)
      {
        for(uint32_t j=0; j < ooo_cpu[0].L2C.NUM_WAY; j++)
        {
          cout<<"L2C: i: "<<i<<" j: "<<j<<" "<<"block address: "<<hex<< ooo_cpu[0].L2C.block[i][j].address <<" tag: "<<ooo_cpu[0].L2C.block[i][j].tag<<" full_addr: "<<ooo_cpu[0].L2C.block[i][j].full_addr <<dec<<"  valid: " <<(int)ooo_cpu[0].L2C.block[i][j].valid<<"  dirty: "<< (int)ooo_cpu[0].L2C.block[i][j].dirty<<"  instr_id: "<< ooo_cpu[0].L2C.block[i][j].instr_id<< endl;
} }
cout <<"====================================================================="<<endl;
  for(uint32_t i=0; i < uncore.LLC.NUM_SET; i++)
      {
        for(uint32_t j=0; j < uncore.LLC.NUM_WAY; j++)
        {
          cout <<"LLC: i: "<<i<<" j: "<<j<<" "<<"block address: "<<hex<< uncore.LLC.block[i][j].address<<" tag: "<<uncore.LLC.block[i][j].tag <<" full_addr: "<< uncore.LLC.block[i][j].full_addr <<dec<<"  valid: "<< (int)uncore.LLC.block[i][j].valid<<"  dirty: "<< (int)uncore.LLC.block[i][j].dirty<<"  instr_id: "<< uncore.LLC.block[i][j].instr_id<< endl;
} } 
*/

    print_branch_stats();
    return 0;
}
