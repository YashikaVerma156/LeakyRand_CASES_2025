#ifndef CHAMPSIM_H   
#define CHAMPSIM_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>

#include <iostream>
#include <queue>
#include <map>
#include <random>
#include <string>
#include <iomanip>
#include <fstream>
#include "config.h"

// USEFUL MACROS
//#define DEBUG_PRINT
#define SANITY_CHECK
#define LLC_BYPASS
#define DRC_BYPASS
#define NO_CRC2_COMPILE

#ifdef DEBUG_PRINT
#define DP(x) x
#else
#define DP(x)
#endif

// Manual seed number.
#define SEED_NUMBER 0 //1700


#define INCLUSIVE 1
#define INCLUSIVE_PRINT 1
#define DETAILED_PRINT 0
#define CHANNEL_BW_ERR_PRINT 0
#define LR_NO_ERR_CORR 0
#define LR_ERR_CORR_BOTH_ALGO 0

// CPU
#define NUM_CPUS 1
#define CPU_FREQ 4000
#define DRAM_IO_FREQ 800
#define PAGE_SIZE 4096
#define LOG2_PAGE_SIZE 12

// CACHE
#define BLOCK_SIZE 64
#define LOG2_BLOCK_SIZE 6
#define MAX_READ_PER_CYCLE 4
#define MAX_FILL_PER_CYCLE 1

#define INFLIGHT 1
#define COMPLETED 2

#define FILL_L1    1
#define FILL_L2    2
#define FILL_LLC   4
#define FILL_DRC   8
#define FILL_DRAM 16

// DRAM
#define DRAM_CHANNELS 1      // default: assuming one DIMM per one channel 4GB * 1 => 4GB off-chip memory
#define LOG2_DRAM_CHANNELS 0
#define DRAM_RANKS 8         // 512MB * 8 ranks => 4GB per DIMM
#define LOG2_DRAM_RANKS 3
#define DRAM_BANKS 8         // 64MB * 8 banks => 512MB per rank
#define LOG2_DRAM_BANKS 3
#define DRAM_ROWS 32768      // 2KB * 32K rows => 64MB per bank
#define LOG2_DRAM_ROWS 15
#define DRAM_COLUMNS 32      // 64B * 32 column chunks (Assuming 1B DRAM cell * 8 chips * 8 transactions = 64B size of column chunks) => 2KB per row
#define LOG2_DRAM_COLUMNS 5
#define DRAM_ROW_SIZE (BLOCK_SIZE*DRAM_COLUMNS/1024)

#define DRAM_SIZE (DRAM_CHANNELS*DRAM_RANKS*DRAM_BANKS*DRAM_ROWS*DRAM_ROW_SIZE/1024) 
#define DRAM_PAGES ((DRAM_SIZE<<10)>>2) 
//#define DRAM_PAGES 10

#define generate_llc_traces 0
#define MIRAGE 1
#define PRINCE_INDEX 1

#define HIT_MISS_THRESHOLD 0    //134     // 141 as hit latency for 32unrolling factor is seen.

#define HIT_MISS_THRESHOLD_LLC_OCC_CHK 0   //134

#define HIT_MISS_THRESHOLD_SINGLE_ARR_ACCESS 50

#define UNROLLING_FACTOR 0

#define UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0 

#define WAIT_IMPLEMENTATION 0

#define IEEE_SEED 0  //Make it explicitly 1 

#define CLEAR_CACHE 0

#define MAX_EXTRA_BLOCKS_POSSIBLE 1

#define PRIORITIZE_INVALID_BLOCKS 0

using namespace std;

//================= Error analysis for covert channel variables starts. ========================//
extern int EVICTED_CPU, // values can be -1(no eviction), 0(block of cpu0 is evicted), or 1(block of cpu1 is evicted).
           sender_seen_wait_once,
           dont_fill_empty_slot,
           rdtsc_timer_is_on,
           inside_rdtsc;
extern int CURR_ERR_CORR_ITR, MISSES_OBSERVED, CURR_LLC_OCC_CHK_ITR;
extern uint32_t IS_EVICTED_BLOCK_INSTR; // value can be 0 or 1.
extern uint64_t EVICTED_IP,  // IP of the block evicted by the sender. 
                SENDER_ACTIVE_IP_START, // Start IP of sender active blocks.
                SENDER_ACTIVE_IP_END; // End IP of sender active blocks.
extern uint32_t receiver_observed_miss, correctly_transmitted_zero;
extern uint64_t sender_evicted_own_active_block, sender_evicted_own_inactive_block, sender_evicted_receiver_inactive_block, sender_evicted_receiver_nonrecognizable_active_block, sender_evicted_receiver_recognizable_active_block, sender_filled_up_empty_block;
//================= Error analysis for covert channel variables ends. ========================//

// IEEE SEED variables
extern uint64_t differ_sig[DIFFERENTIAL_SIGNALLING], repeat_code[REPETETION_CODING];

extern uint64_t FIRST_FENCE, LLC_MISS_COUNT[NUM_CPUS]; // Used for calculating initial-setup overhead.
extern int Calculate_LLC_occupancy, DONT_INC_ARR;

//extern int countoo;

extern uint32_t seen_error_correction, ERROR_CORRECTION_PHASE, DOUBLE_PROBE, HOLE_FILLING_PROCESS_ONGOING;
extern uint32_t LLC_OCCUPANCY_CHECK_ONGOING, CACHE_FILL_DONE;

extern uint8_t seen_llc_occ_chk, LLC_OCC_CHK_PHASE;

extern uint64_t FIRST_WAIT_OF_SENDER, // In set-up period, sender waits for longer time.
                SENDER_WAIT_PERIOD, // Sender wait period, once transmission begins.
                RECEIVER_WAIT_PERIOD; // Receiver wait period, once transmission period begins.

extern uint8_t SETUP_DONE[NUM_CPUS], FIRST_TIME_CACHE_SETUP_DONE;

extern int64_t CENTRAL_CLOCK[NUM_CPUS], WAIT_PERIOD[3]; 

extern uint64_t spills, range_start, range_end;
extern int receiver_checks_for_a_miss, bit_revealed_is;
extern uint8_t warmup_complete[NUM_CPUS], 
               simulation_complete[NUM_CPUS], 
               all_warmup_complete, 
               all_simulation_complete,
               MAX_INSTR_DESTINATIONS,
               knob_cloudsuite,
               knob_low_bandwidth;

extern ofstream llc_access_stream_file;

extern uint64_t current_core_cycle[NUM_CPUS], 
                stall_cycle[NUM_CPUS], 
                last_drc_read_mode, 
                last_drc_write_mode,
                drc_blocks;

extern uint64_t prev_rdtsc_counter, prev_rdtsc_id;
extern int start_count_1, start_count, stop_count, TURN, flushed_sender_blocks_once;
extern int cycles_to_wait_for_before_triggering_clflush;
extern uint64_t cpu_cycle_to_trigger_clflush[NUM_CPUS][MAX_CLFLUSH_POSSIBLE];
extern uint8_t cpu_on_wait[NUM_CPUS], fence_called_on_cpu[NUM_CPUS]; // For covert channel
extern uint64_t fence_instr[NUM_CPUS], recorded_rdtsc_value[NUM_CPUS], hit_block; // For covert channel

// variables for IOcalls invalidations starts.
extern uint64_t counting_instr_for_ipc, start_IO_cycle, Total_IO_instr, IO_instruction_end, IO_instruction_start, Total_cycles_spent_in_IO, seen_fence_once, IO_miss, Total_IO_miss;
// variables for IOcalls invalidations ends.
extern queue <uint64_t> page_queue;
extern map <uint64_t, uint64_t> page_table, inverse_table, recent_page, unique_cl[NUM_CPUS];
extern uint64_t previous_ppage, num_adjacent_page, num_cl[NUM_CPUS], allocated_pages, num_page[NUM_CPUS], minor_fault[NUM_CPUS], major_fault[NUM_CPUS];

extern uint64_t clflush_on_cpu[NUM_CPUS][MAX_CLFLUSH_POSSIBLE];
extern uint64_t CHECK_IP;

void print_stats();
uint64_t rotl64 (uint64_t n, unsigned int c),
         rotr64 (uint64_t n, unsigned int c),
         va_to_pa(uint32_t cpu, uint64_t instr_id, uint64_t va, uint64_t unique_vpage);

extern FILE * fptr;

// log base 2 function from efectiu
int lg2(int n);

// smart random number generator
class RANDOM {
  public:
    std::random_device rd;
    std::mt19937_64 engine{rd()};
    std::uniform_int_distribution<uint64_t> dist{0, 0xFFFFFFFFF}; // used to generate random physical page numbers

    RANDOM (uint64_t seed) {
        engine.seed(seed);
    }

    uint64_t draw_rand() {
        return dist(engine);
    };
};
extern uint64_t champsim_seed;
#endif
