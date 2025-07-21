#ifndef CACHE_H
#define CACHE_H
#include <vector>
#include "mtrand.h"
#include "memory_class.h"

// PAGE
extern uint32_t PAGE_TABLE_LATENCY, SWAP_LATENCY;

// CACHE TYPE
#define IS_ITLB 0
#define IS_DTLB 1
#define IS_STLB 2
#define IS_L1I  3
#define IS_L1D  4
#define IS_L2C  5
#define IS_LLC  6
#define IS_DATA_LLC 7

// INSTRUCTION TLB
#define ITLB_SET 16
#define ITLB_WAY 4
#define ITLB_RQ_SIZE 16
#define ITLB_WQ_SIZE 16
#define ITLB_PQ_SIZE 0
#define ITLB_MSHR_SIZE 8
#define ITLB_LATENCY 1 // cacti calculated

// DATA TLB
#define DTLB_SET 16
#define DTLB_WAY 4
#define DTLB_RQ_SIZE 16
#define DTLB_WQ_SIZE 16
#define DTLB_PQ_SIZE 0
#define DTLB_MSHR_SIZE 8
#define DTLB_LATENCY 1 // cacti calculated

// SECOND LEVEL TLB
#define STLB_SET 128
#define STLB_WAY 12
#define STLB_RQ_SIZE 32
#define STLB_WQ_SIZE 32
#define STLB_PQ_SIZE 0
#define STLB_MSHR_SIZE 16
#define STLB_LATENCY 2  // cacti calculated

// L1 INSTRUCTION CACHE //32KB capacity
#define L1I_SET 64
#define L1I_WAY 8
#define L1I_RQ_SIZE 64
#define L1I_WQ_SIZE 64 
#define L1I_PQ_SIZE 64
#define L1I_MSHR_SIZE 8
#define L1I_LATENCY 2 //cacti calculated

// L1 DATA CACHE //32KB capacity
#define L1D_SET 64
#define L1D_WAY 8
#define L1D_RQ_SIZE 64
#define L1D_WQ_SIZE 64 
#define L1D_PQ_SIZE 64
#define L1D_MSHR_SIZE 32 //8
#define L1D_LATENCY 2 // Cacti calculated 

// L2 CACHE //256KB capacity
#define L2C_SET 512
#define L2C_WAY 8
#define L2C_RQ_SIZE 32
#define L2C_WQ_SIZE 32
#define L2C_PQ_SIZE 32
#define L2C_MSHR_SIZE 32 //16
#define L2C_LATENCY 3 // cacti calculated // 4 (L1I or L1D) + 8 = 12 cycles

// LAST LEVEL CACHE  //2MB capacity
#define LLC_SET 1*2048 //NUM_CPUS*2048 Assuming sliced LLC
#define LLC_WAY 28 //28 //16
#define LLC_RQ_SIZE NUM_CPUS*L2C_MSHR_SIZE //48
#define LLC_WQ_SIZE NUM_CPUS*L2C_MSHR_SIZE //48
#define LLC_PQ_SIZE NUM_CPUS*L2C_MSHR_SIZE //48
#define LLC_MSHR_SIZE 32
#define LLC_LATENCY 15  // ( cacti calculated 11(direct mapped data store and set-associative tag store) + 4 cycles of Mirage)  // 4 (L1I or L1D) + 8 + 20 = 32 cycles

/*

#define LLC_SET 8*2048
#define LLC_WAY 16
#define LLC_RQ_SIZE 8*L2C_MSHR_SIZE //48
#define LLC_WQ_SIZE 8*L2C_MSHR_SIZE //48
#define LLC_PQ_SIZE 8*L2C_MSHR_SIZE //48
#define LLC_MSHR_SIZE 32
#define LLC_LATENCY 20  // 4 (L1I or L1D) + 8 + 20 = 32 cycles

*/

// data LAST LEVEL CACHE
#define data_LLC_SET 1                   // NUM_CPUS*2048
#define data_LLC_WAY 16*2048*1 //16*2048*NUM_CPUS  // 16 Assuming sliced LLC
#define data_LLC_RQ_SIZE 0 //48    
#define data_LLC_WQ_SIZE 0 //48
#define data_LLC_PQ_SIZE 0 //48
#define data_LLC_MSHR_SIZE 0
#define data_LLC_LATENCY 0  //XXX For now it is not visible to rest of the Champsim hence all queues size is zero and latency is also zero.


class CACHE : public MEMORY {
  public:
    CACHE *side_level;
    uint32_t cpu;
    const string NAME;
    const uint32_t NUM_SET, NUM_WAY, NUM_LINE, WQ_SIZE, RQ_SIZE, PQ_SIZE, MSHR_SIZE;
    uint32_t LATENCY;
    BLOCK **block;
    int fill_level;
    uint32_t MAX_READ, MAX_FILL;
    uint8_t cache_type;

		MTRand *mtrand=new MTRand();
		
		//mtrand->seed(myseed);

		//Randomization parameters
		int lines_in_mem, //2048*16; // For 2MB slice // What is this value ? // This should be according to SPILL_THRESHOLD
		    mem_size, //2*1024*1024*8; // This is to be identified that what this value will be ?
		    setMask, //2048 - 1; // For 2MB slice #NUM_BUCKETS or NUM_BUCKETS_PER_SKEW
		    setShift, //6; /* log2(block size) */ LINE_SZ_BYTES 
		    NUM_SKEWING_FUNCTIONS, 
				NUM_WAYS_PER_SKEW;
	  uint32_t  NUM_SKEW;
    vector<vector<int64_t>> rand_table_vec;

    // prefetch stats
    uint64_t pf_requested,
             pf_issued,
             pf_useful,
             pf_useless,
             pf_fill;

    // queues
    PACKET_QUEUE WQ{NAME + "_WQ", WQ_SIZE}, // write queue
                 RQ{NAME + "_RQ", RQ_SIZE}, // read queue
                 PQ{NAME + "_PQ", PQ_SIZE}, // prefetch queue
                 MSHR{NAME + "_MSHR", MSHR_SIZE}, // MSHR
                 PROCESSED{NAME + "_PROCESSED", ROB_SIZE}; // processed queue

    uint64_t sim_access[NUM_CPUS][NUM_TYPES],
             sim_hit[NUM_CPUS][NUM_TYPES],
             sim_miss[NUM_CPUS][NUM_TYPES],
             roi_access[NUM_CPUS][NUM_TYPES],
             roi_hit[NUM_CPUS][NUM_TYPES],
             roi_miss[NUM_CPUS][NUM_TYPES];
    
    // constructor
    CACHE(string v1, uint32_t v2, int v3, uint32_t v4, uint32_t v5, uint32_t v6, uint32_t v7, uint32_t v8) 
        : NAME(v1), NUM_SET(v2), NUM_WAY(v3), NUM_LINE(v4), WQ_SIZE(v5), RQ_SIZE(v6), PQ_SIZE(v7), MSHR_SIZE(v8) {

        LATENCY = 0;

        // cache block
        block = new BLOCK* [NUM_SET];
        for (uint32_t i=0; i<NUM_SET; i++) {
            block[i] = new BLOCK[NUM_WAY]; 

            for (uint32_t j=0; j<NUM_WAY; j++) {
                block[i][j].lru = j;
            }
        }

        for (uint32_t i=0; i<NUM_CPUS; i++) {
            upper_level_icache[i] = NULL;
            upper_level_dcache[i] = NULL;

            for (uint32_t j=0; j<NUM_TYPES; j++) {
                sim_access[i][j] = 0;
                sim_hit[i][j] = 0;
                sim_miss[i][j] = 0;
                roi_access[i][j] = 0;
                roi_hit[i][j] = 0;
                roi_miss[i][j] = 0;
            }
        }

        lower_level = NULL;
				side_level = NULL;  // For mirage
        extra_interface = NULL;
        fill_level = -1;
        MAX_READ = 1;
        MAX_FILL = 1;

        pf_requested = 0;
        pf_issued = 0;
        pf_useful = 0;
        pf_useless = 0;
        pf_fill = 0;

				//Randomization parameters
        lines_in_mem = -1, //2048*16; // For 2MB slice // What is this value ? // This should be according to SPILL_THRESHOLD
        mem_size = -1, //2*1024*1024*8; // This is to be identified that what this value will be ?
        setMask = -1, //2048 - 1; // For 2MB slice #NUM_BUCKETS or NUM_BUCKETS_PER_SKEW
        setShift = -1, //6; /* log2(block size) */ LINE_SZ_BYTES 
        NUM_SKEWING_FUNCTIONS = -1,
        NUM_SKEW = 1,
        NUM_WAYS_PER_SKEW = NUM_WAY/NUM_SKEW;

    };

    // destructor
    ~CACHE() {
        for (uint32_t i=0; i<NUM_SET; i++)
            delete[] block[i];
        delete[] block;
    };

    // functions
    int  add_rq(PACKET *packet),
         add_wq(PACKET *packet),
         add_pq(PACKET *packet);

    void return_data(PACKET *packet),
         operate(),
         increment_WQ_FULL(uint64_t address);

    uint32_t get_occupancy(uint8_t queue_type, uint64_t address),
             get_size(uint8_t queue_type, uint64_t address);

    uint32_t check_hit(PACKET *packet);
    int invalidate_entry(uint64_t inval_addr),
         check_mshr(PACKET *packet),
         prefetch_line(uint64_t ip, uint64_t base_addr, uint64_t pf_addr, int fill_level),
         kpc_prefetch_line(uint64_t base_addr, uint64_t pf_addr, int fill_level, int delta, int depth, int signature, int confidence);

		int back_invalidate_indirection(uint32_t set, uint32_t way),
        back_invalidate_for_inclusion(uint32_t cpu, uint64_t address, uint8_t is_llc, uint64_t instr_id);
    void handle_fill(),
         handle_writeback(),
         handle_read(),
         handle_prefetch(),
				 handle_forwarding();

    int check_mshr_for_hits( PACKET *packet);

    void add_mshr(PACKET *packet),
         update_fill_cycle(),
         llc_initialize_replacement(),
         update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit),
         llc_update_replacement_state_mirage(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit, int skew_number),
				 llc_update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit),
         lru_update(uint32_t set, uint32_t way),
         fill_cache(uint32_t set, uint32_t way, PACKET *packet),
         replacement_final_stats(),
         llc_replacement_final_stats(),
         //prefetcher_initialize(),
         l1d_prefetcher_initialize(),
         l2c_prefetcher_initialize(),
         prefetcher_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type),
         l1d_prefetcher_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type),
         l2c_prefetcher_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type),
         prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr),
         l1d_prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr),
         l2c_prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr),
         //prefetcher_final_stats(),
         l1d_prefetcher_final_stats(),
         l2c_prefetcher_final_stats();

    uint32_t get_set(uint64_t address),
             get_way(uint64_t address, uint32_t set),
             find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type),
             llc_find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type),
						 llc_find_victim_mirage(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type, int skew_number),
             lru_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type);

		//void llc_update_replacement_state_inv_block(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, int partition);
				
				// PRINCE functions 
	void init_cache_indexing(int ways_per_tag_set, int sets_per_skew, int num_skews, int block_size), uint64_to_bytevec(uint64_t  input, uint8_t output[8]), 
			 gen_rand_table(vector<int64_t>  &m_ela_table, uint64_t num_lines_in_mem, int seed_rand); 

  uint64_t bytevec_to_uint64(uint8_t input[8]), calcPRINCE64(uint64_t phy_line_num,uint64_t seed);
	uint32_t extractSet(uint64_t addr, uint32_t way), load_balanced_skew_selection(uint32_t set_per_partition[/*NUM_SKEW*/]);

  int make_inclusive(CACHE &cache,uint64_t address, uint64_t instr_id);

};

#endif
