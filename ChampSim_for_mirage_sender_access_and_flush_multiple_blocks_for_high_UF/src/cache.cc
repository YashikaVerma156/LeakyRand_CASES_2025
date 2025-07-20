#include "cache.h"
#include "set.h"
#include "prince_ref.hh"
#include "iostream"
#include <vector>
#include <cassert>
#include <bits/stdc++.h>
#include "mtrand.h"
#include "ooo_cpu.h"
#include "champsim.h"

uint64_t l2pf_access = 0;

// atomically enforcing inclusion
int CACHE::make_inclusive(CACHE &cache, uint64_t address, uint64_t instr_id)
{ 
	  if(cache.MSHR.occupancy != 0)
	  {
		    for (uint32_t i=0; i<cache.MSHR.SIZE; i++)
			    if (cache.MSHR.entry[i].address == address /*&& cache.MSHR.entry[i].returned == COMPLETED*/) //Entry is waiting to be filled in cache.
			    {  
              //cout<<"to be invalidated is set 1"<< " triggered from cache_type: "<<(int)cache_type <<" triggered into cache type: "<<(int)cache.cache_type<< endl;
              cache.MSHR.entry[i].to_be_invalidated = 1;
			    }
	  }

    if(cache.cache_type == IS_L2C && cache.WQ.occupancy != 0)
    {
            for (uint32_t i=0; i<cache.WQ.SIZE; i++)
                if (cache.WQ.entry[i].address == address) // An update is present in WQ.
                {
                    //XXX We are stalling the cache here but with an assumption that such cases are very few. This needs to be looked at if the cases increase.
                    return 0; // Stall the eviction.
                }
    }

	  int set = cache.get_set(address);
	  for(unsigned int way = 0; way < cache.NUM_WAY; way++)
		    if(cache.block[set][way].valid && cache.block[set][way].address == address)
		    {
			     if(cache.block[set][way].dirty)
			     {
				        if (cache.lower_level)
				        {
										 if (cache.lower_level->get_occupancy(2, cache.block[set][way].address) == cache.lower_level->get_size(2, cache.block[set][way].address))      
										 {

												cache.lower_level->increment_WQ_FULL(cache.block[set][way].address);
												return 0; // Stall the eviction.
										 }
										 else 
										 {
												 cache.block[set][way].valid = 0;
											
												 // This return type is to identify that a dirty block is found.
												 return 2;
									 }
				       }
			     }
			     else
			     {
			        cache.block[set][way].valid = 0;
	            break;
			     }
		    }
	return 1;
}


//Provides uint64_t in little-endian format (converting from big-endian output of  PRINCE)
uint64_t CACHE::bytevec_to_uint64(uint8_t input[8])
{
  uint64_t output;
    uint8_t* output_le = (uint8_t*)&output;
    for(int i=0; i<8; i++)
  {
      output_le[i] = input[7-i];
    }
    return output;
}

//Provides bytevec in big-endian format (MSB first as needed by PRINCE)
void CACHE::uint64_to_bytevec(uint64_t  input, uint8_t output[8])
{
  uint8_t* input_le = (uint8_t*)&input;
    for(int i=0; i<8; i++)
  {
      output[i] = input_le[7-i];
    }
}

uint32_t CACHE::extractSet(uint64_t addr, uint32_t skew_id) // This is just address, don't remove extra bits ?
{

    #if(! PRINCE_INDEX)
        return (uint32_t) (addr & ((1 << lg2(NUM_SET)) - 1));
   #endif    

		//return (uint32_t) (addr & ((1 << lg2(NUM_SET)) - 1));
	
  // Get physical line address and line number
 // int64_t phy_lineaddress = addr >> setShift; //addr >> setShift; //setShift = log(blksize)
  int64_t physical_line_num = addr % lines_in_mem;

  //cout << "Physical line address is: "<< phy_lineaddress << endl;
  //cout << "Plain address set is: " << (physical_line_num & setMask) << endl;

  // Get skew-id from the way & encryption table
  //cout << way << endl;
  //int skew_id = way / NUM_WAYS_PER_SKEW;
  vector<int64_t>* m_ela_table = &rand_table_vec[skew_id];
  // Lookup table to get encrypted line number
  int64_t encrypted_line_num = (*m_ela_table)[physical_line_num];
 // cout << "Encrypted line number is: " << (encrypted_line_num) << endl;
  //cout << "Encrypted address is: " << (encrypted_line_num << setShift) << endl;

  // Cacheset is modulo setMask (num_sets -1)
  int64_t cacheset = encrypted_line_num & (setMask-1); //@Yashika, setMask to be defined.
  // DPRINTF(Cache, "extractSet=> Skew:%d, Address %lld, Line-Addr: %lld, PLN %lld, ELN %lld, Set %lld \n",skew_id, addr,phy_lineaddress, physical_line_num,encrypted_line_num,cacheset);
  //cout << "cacheset returned for skew id: "<<skew_id << " is: " << cacheset <<endl;
  return  cacheset;
}

// [Skewed Randomized Cache]: Randomized Indexing (IDF) Using PRINCE Cipher-based Hashing
uint64_t CACHE::calcPRINCE64(uint64_t phy_line_num,uint64_t seed){

  //Variables for keys, input, output.
  uint8_t plaintext[8];
  uint8_t ciphertext[8];
  uint8_t key[16];

  //Key Values  
  uint64_t k0 = 0x0011223344556677;
  uint64_t k1 = 0x8899AABBCCDDEEFF;
  //Add seed to keys for each skew.
  k0 = k0 + seed;
  k1 = k1 + seed;

  //Set up keys, plaintext
  uint64_to_bytevec(k0,key);
  uint64_to_bytevec(k1,key+8);
  uint64_to_bytevec(phy_line_num,plaintext);

  //TEST PLAINTEXT
  //uint64_t test_plaintext = 0x0123456789ABCDEF; 
  //uint64_to_bytevec(test_plaintext,plaintext);

  //Prince encrypt.
  prince_encrypt(plaintext,key,ciphertext);

  //Convert output
  uint64_t enc64_hash = bytevec_to_uint64(ciphertext);

  //PRINT TEST
  //cprintf("PT: %llx, K0: %llx, K1 %llx, CT:%llx\n", /*test_plaintext*/phy_line_num, k0,k1,enc64_hash);

  return enc64_hash;
}

void CACHE::gen_rand_table (vector<int64_t>  &m_ela_table,
                                      uint64_t num_lines_in_mem,    \
                                      int seed_rand)
{
  m_ela_table.resize(0);
  m_ela_table.resize(num_lines_in_mem,-1);

  //Ensure the size of uint64 used by gem5 & QARMA is the same (needed only if QARMA used)
  //assert(sizeof(uint64_t) == sizeof(u64));

  //Encr-Index = Index(PRINCE64(PLN))
  //Encr-Tag   = PLN
  //PLN = Encr-Tag
  for (uint64_t i=0;i <num_lines_in_mem; i++){
    m_ela_table[i] = calcPRINCE64(i,seed_rand) % num_lines_in_mem ;
  }
}

void CACHE::init_cache_indexing(int ways_per_tag_set, int sets_per_skew, int num_skews, int block_size)
{
  //[NOTE]: mem_size is calculated as per tag array and not data array
  mem_size = num_skews*sets_per_skew*ways_per_tag_set*block_size;
  NUM_SKEWING_FUNCTIONS = num_skews;
  setShift = log2(block_size);
  setMask = sets_per_skew;
  // Set the max-physical cachelines in memory
  lines_in_mem = (mem_size >> setShift);
  //DPRINTF(Cache,"For LLC: Mem_Size %ld, MaxLines in Memory %ld",mem_size, lines_in_mem);
    cout << "Configurations set in init_cache_indexing\n "<<"mem_size: "<<mem_size <<"  NUM_SKEWING_FUNCTIONS: "<<NUM_SKEWING_FUNCTIONS<<"  setShift: " <<setShift<< "  setMask: "<<setMask <<endl;

  assert(lines_in_mem > 0 && mem_size > 0 && NUM_SKEWING_FUNCTIONS > 0 && setShift > 0 && setMask > 0);

  // Allocate size for the rand-table vector.
 // assert((skewedCache || (NUM_SKEWING_FUNCTIONS == 1) ) &&   "Either a explicitly skewedCache, or numSkews should be 1");
 // assert(randomizedIndexing && "Randomized Indexing should be enabled.");
  rand_table_vec.resize(NUM_SKEWING_FUNCTIONS);

  // Generate skewing functions
  //srand(42);
  for (int i=0; i< NUM_SKEWING_FUNCTIONS; i++){
    //gen_rand_table(rand_table_vec[i],lines_in_mem,rand());  // What is this function rand();   mtrand->randInt(NUM_BUCKETS*BALLS_PER_BUCKET -1)
    gen_rand_table(rand_table_vec[i],lines_in_mem,mtrand->randInt(/*189983456332*/ UINT64_MAX));  // TODO can it create any problem ?
  }
}

// Way and set of data_array block is passed
int CACHE::back_invalidate_indirection(uint32_t set, uint32_t way)
{
		//cout << "=================== Inside block invalidation ================="<< "set: "<<set<<" way: "<<way << endl;

		int partition = -1; 

		if(set != 0) // as data_array is fully-associative
				assert(0);
		 
		if(/*set == -1 || way == -1 || */set > side_level->NUM_SET || way > side_level->NUM_WAY || cache_type != IS_LLC)
    	assert(0);

		// sanity check that reverse and forward pointers are maintained.
		if(set != block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].set_ptr || way != block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].way_ptr)
		{
			cout << side_level->block[set][way].set_ptr << " " << block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].set_ptr <<" " << side_level->block[set][way].way_ptr << " " <<block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].way_ptr << endl;			
      assert(0); 
    }

		for(uint32_t part = 0; part < NUM_SKEW; part++)
    {
        if(side_level->block[set][way].way_ptr < NUM_WAY/NUM_SKEW*(part+1) && side_level->block[set][way].way_ptr >= NUM_WAY/NUM_SKEW*(part))
        		partition = part;
    }	
	
		// If block is not dirty, directly remove the indirection.
		if(side_level->block[set][way].dirty == 0) 
    {
      block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].valid = 0;
      side_level->block[set][way].valid = 0;
      block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].set_ptr = -1;
      block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].way_ptr = -1;
      side_level->block[set][way].set_ptr = -1;
      side_level->block[set][way].way_ptr = -1;

			//cout << "=================== block invalidation is complete ================="<< endl;

			return 1;
    }    
		else if(side_level->block[set][way].dirty == 1)
		{
	      // check if the lower level WQ has enough room to keep this writeback request
                if (lower_level->get_occupancy(2, side_level->block[set][way].address) == lower_level->get_size(2, block[set][way].address)) {
										
                    // lower level WQ is full, cannot replace this victim
                    lower_level->increment_WQ_FULL(side_level->block[set][way].address);
                 //   STALL[side_level->block[set][way].type]++;
								//cout << "=================== block invalidation is incomplete ================="<< endl;
										return 0;
                }
                else 
								{
                    PACKET writeback_packet;

                    writeback_packet.fill_level = fill_level << 1;
                    writeback_packet.cpu = side_level->block[set][way].cpu; //fill_cpu;
                    writeback_packet.address = side_level->block[set][way].address;
                    writeback_packet.full_addr = side_level->block[set][way].full_addr;
                    writeback_packet.data = side_level->block[set][way].data;
                    writeback_packet.instr_id = side_level->block[set][way].instr_id; //MSHR.entry[mshr_index].instr_id;
                    writeback_packet.ip = 0; // writeback does not have ip
                    writeback_packet.type = WRITEBACK;
                    writeback_packet.event_cycle = current_core_cycle[side_level->block[set][way].cpu];

                    lower_level->add_wq(&writeback_packet);
									  cout << "found dirty, inserted into lower queue wq. 269" <<" instr_id: "<<side_level->block[set][way].instr_id<<" full_addr: "<<side_level->block[set][way].full_addr<< endl;	
				//						llc_update_replacement_state_inv_block(side_level->block[set][way].cpu, side_level->block[set][way].set_ptr, side_level->block[set][way].way_ptr, side_level->block[set][way].full_addr, partition/*side_level->block[set][way].ip, 0, side_level->block[set][way].type, 0*/);
										block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].valid = 0;
                    side_level->block[set][way].valid = 0;
                    block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].dirty = 0;
                    side_level->block[set][way].dirty = 0;
                    block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].set_ptr = -1;
     								block[side_level->block[set][way].set_ptr][side_level->block[set][way].way_ptr].way_ptr = -1;
      							side_level->block[set][way].set_ptr = -1;
                    side_level->block[set][way].way_ptr = -1;
										//cout << "=================== block invalidation is complete ================="<< endl;
										return 1;
                }
		}
		else
      assert(0); // We should not reach here.
}

uint32_t CACHE::load_balanced_skew_selection(uint32_t set_per_partition[/*NUM_SKEW*/])
{
		int invalid_blocks[NUM_SKEW] = {0};
		for(uint32_t part = 0; part < NUM_SKEW; part++)
		{
				for(uint32_t way = part*NUM_WAY/NUM_SKEW; way < (NUM_WAY/NUM_SKEW)*(part+1); way++)
				{
						if(block[set_per_partition[part]][way].valid == 0)
								invalid_blocks[part] ++;	
				}
		}

		int partition = -1, max_inv_blocks = -1;

		for(uint32_t part = 0; part < NUM_SKEW; part++)
    {
				if(invalid_blocks[part] > max_inv_blocks)
						{	
								max_inv_blocks = invalid_blocks[part];
								if(max_inv_blocks == 0)
									spills ++;
								partition = part;
						}
		}

		return partition;   // return skew number with maximum invalid blocks, or return skew 0 if both skews have same invalid blocks
}


void CACHE::handle_forwarding()
{
	int returned_data_count = 0;
	for (uint32_t index=0; index<MSHR_SIZE; index++) 
	{     
        if (MSHR.next_fill_index == index && MSHR.entry[index].returned == COMPLETED && MSHR.entry[index].forwarded_to_next_cache_level == 0) 
				{
            //if(MSHR.entry[index].instr_id == 7458) cout <<"stall_cycle of cpu: "<<stall_cycle[MSHR.entry[index].cpu]<< " returned from cache_type: " << (int)cache_type<<" core cycle: "<<current_core_cycle[cpu]<<endl;
						int fill_cpu = MSHR.entry[index].cpu;
						// check fill level
            if (MSHR.entry[index].fill_level < fill_level) {

								MSHR.entry[index].forwarded_to_next_cache_level = 1;

                if (MSHR.entry[index].instruction)
                    upper_level_icache[fill_cpu]->return_data(&MSHR.entry[index]);
                else // data
                    upper_level_dcache[fill_cpu]->return_data(&MSHR.entry[index]);
								
								returned_data_count ++;
            }
						else if(MSHR.entry[index].fill_level == fill_level)
							{
									MSHR.entry[index].forwarded_to_next_cache_level = 1; 
							}
						
						else
								assert(0); // We should not reach here.

            // update processed packets
            if (cache_type == IS_ITLB) {

								if(MSHR.entry[index].data == 0) assert(0); //Sanity check

                MSHR.entry[index].instruction_pa = MSHR.entry[index].data; // Transferring physical address.
								if(DETAILED_PRINT || (MSHR.entry[index].instr_id >= range_start && MSHR.entry[index].instr_id <= range_end))
								cout << "transferred to ITLB processed queue: " <<" instr_id: "<<MSHR.entry[index].instr_id<<" IP: " << MSHR.entry[index].ip<<" current_core_cycle[0]: "<<current_core_cycle[0] << endl;
                if (PROCESSED.occupancy < PROCESSED.SIZE)
                    PROCESSED.add_queue(&MSHR.entry[index]);
            }
            else if (cache_type == IS_DTLB) {

								if(MSHR.entry[index].data == 0) assert(0); //Sanity check
							
              	if(DETAILED_PRINT || (MSHR.entry[index].instr_id >= range_start && MSHR.entry[index].instr_id <= range_end))
								cout << "transferred to DTLB processed queue: " <<" instr_id: "<<MSHR.entry[index].instr_id<<" IP: " << MSHR.entry[index].ip<<" current_core_cycle[0]: "<<current_core_cycle[0] << endl;

                MSHR.entry[index].data_pa = MSHR.entry[index].data; // Transferring physical address.
                if (PROCESSED.occupancy < PROCESSED.SIZE)
                    PROCESSED.add_queue(&MSHR.entry[index]);
            }
            else if (cache_type == IS_L1I) {
                if (PROCESSED.occupancy < PROCESSED.SIZE){
										if(DETAILED_PRINT || (MSHR.entry[index].instr_id >= range_start && MSHR.entry[index].instr_id <= range_end) )
										cout << "transferred to L1I processed queue: " <<" instr_id: "<<MSHR.entry[index].instr_id<<" IP: "<< MSHR.entry[index].ip << " current_core_cycle[0]: "<<current_core_cycle[0]<<endl;
                    PROCESSED.add_queue(&MSHR.entry[index]); }
            }
            else if ((cache_type == IS_L1D) && (MSHR.entry[index].type != PREFETCH)) { // Prefetcher requests are not forwarded to core, unless it is demanded.
                if (PROCESSED.occupancy < PROCESSED.SIZE)
                    {  
											PROCESSED.add_queue(&MSHR.entry[index]); 
 
                     if(DETAILED_PRINT || (MSHR.entry[index].instr_id >= range_start && MSHR.entry[index].instr_id <= range_end) )
                    cout << "transferred to L1D processed queue: " <<" instr_id: "<<MSHR.entry[index].instr_id<<" IP: "<< MSHR.entry[index].ip << " current_core_cycle[0]: "<<current_core_cycle[0]<<endl;        
 }   }

						if(returned_data_count > 0)
						{
								if(returned_data_count > 1)
										assert(0); // Current implementation should return only one packet in one cycle.

								return;		
						}
				}
	}
}

int CACHE::back_invalidate_for_inclusion(uint32_t cpu, uint64_t address, uint8_t is_llc, uint64_t instr_id)
{        
         int ret = 1, ret1 = 1, ret2 = 1;

	       // ---------------check in L1I
				 ret = make_inclusive(ooo_cpu[cpu].L1I, address, instr_id);
         if(ret == 0)
				 {
             return 0; //Can't proceed further.
				 }
				if(ret == 2) assert(0); // We expect this to never happen.
				//---------------- check in L1D
				ret1 = make_inclusive(ooo_cpu[cpu].L1D, address, instr_id);
        if(ret == 0)
				{
            return 0; //Can't proceed further.
				}

        if(is_llc == 1) 
        {
							// ---------------------check in L2C
							ret2 = make_inclusive(ooo_cpu[cpu].L2C, address, instr_id);
              if(ret == 0)
							{
									return 0; //Can't proceed further.
							}
				}
    if(ret1 == 2 || ret2 == 2)// Assuming L1I will never found a dirty block.
        return 2;
    else
        return 1;
}

void CACHE::handle_fill()
{
    // handle fill
    uint32_t fill_cpu = (MSHR.next_fill_index == MSHR_SIZE) ? NUM_CPUS : MSHR.entry[MSHR.next_fill_index].cpu;
    if (fill_cpu == NUM_CPUS)
        return;

    // Don't wait for cache filling to return data to next cache level.
		handle_forwarding();
    if (MSHR.next_fill_cycle <= current_core_cycle[fill_cpu]) {

#ifdef SANITY_CHECK
        if (MSHR.next_fill_index >= MSHR.SIZE)
            assert(0);
#endif
        uint32_t mshr_index = MSHR.next_fill_index;

   //if(current_core_cycle[0] > 32705 && cache_type==IS_L2C)
     //cout<<"I am in L2C"<<" instr_id: "<<MSHR.entry[mshr_index].instr_id<<endl;
   
     // find victim
        uint32_t set = get_set(MSHR.entry[mshr_index].address), way, data_set = -1, data_way = -1, part=-1;
        if (cache_type == IS_LLC) 
				{
        	// Victim selection for MIRAGE
					#if MIRAGE
          		uint32_t set_per_partition[NUM_SKEW];
							for(uint32_t partition = 0; partition < NUM_SKEW; partition++)
							{
									set_per_partition[partition] = extractSet(MSHR.entry[mshr_index].address, partition);								
							}
							part = load_balanced_skew_selection(set_per_partition);
							set = set_per_partition[part]; //Overwrite set for mirage LLC
							way = llc_find_victim_mirage(fill_cpu, MSHR.entry[mshr_index].instr_id, set, block[set], MSHR.entry[mshr_index].ip, 
																						MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].type, part);

          #else
          		way = llc_find_victim(fill_cpu, MSHR.entry[mshr_index].instr_id, set, block[set], MSHR.entry[mshr_index].ip, 
																		MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].type);
					#endif

					#if MIRAGE
							if(block[set][way].set_ptr == UINT32_MAX || block[set][way].way_ptr == UINT32_MAX)
							{
									data_set = side_level->get_set(MSHR.entry[mshr_index].address);
            			data_way = side_level->llc_find_victim(fill_cpu, MSHR.entry[mshr_index].instr_id, data_set, side_level->block[data_set], MSHR.entry[mshr_index].ip, 
																													MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].type);
                
                  if(side_level->block[data_set][data_way].set_ptr != UINT32_MAX || side_level->block[data_set][data_way].way_ptr != UINT32_MAX)
                  {
                     if(fill_cpu == 1) // Storing this information only for cpu 1, i.e. sender cpu.
                     {
												 EVICTED_IP = side_level->block[data_set][data_way].ip;
												 EVICTED_CPU = side_level->block[data_set][data_way].cpu;
												 IS_EVICTED_BLOCK_INSTR = side_level->block[data_set][data_way].instruction;
                     }
                     //cout << "Details of evicted block, instr_id: "<<side_level->block[data_set][data_way].instr_id<<" cpu: "<<side_level->block[data_set][data_way].cpu << " full_addr: " <<side_level->block[data_set][data_way].full_addr<<" ip: "<<side_level->block[data_set][data_way].ip <<" instr_type: "<<(int)side_level->block[data_set][data_way].instruction<<" "<<IS_EVICTED_BLOCK_INSTR<<endl;  
                     // Back invalidate due to invalidating indirection.
				  #if INCLUSIVE 
                    // When eviction is done at LLC Level
                   if(cache_type == IS_LLC && side_level->block[data_set][data_way].valid) //In Mirage LLC, back-invalidation maintains inclusivity.
                   { 
                       int ret = back_invalidate_for_inclusion(side_level->block[data_set][data_way].cpu, side_level->block[data_set][data_way].address, 1, side_level->block[data_set][data_way].instr_id); // calling back invalidation function for maintaining inclusion.

				               if(ret == 2)
                       {   //Magically copying the dirty value to LLC block in the same cpu cycle.
                           side_level->block[data_set][data_way].dirty = 1; // Make data block dirty.
                           //Make tag block dirty.
                           block[side_level->block[data_set][data_way].set_ptr][side_level->block[data_set][data_way].way_ptr].dirty = 1;
                       }
                       if(ret == 0) // Can't process the invalidation in this cycle.
                       {
                          STALL[MSHR.entry[mshr_index].type]++;
                          return;
                       }
                          // If ret is 1, everything is fine.
                   }
          #endif
							        int ret = back_invalidate_indirection(data_set, data_way); // Invalidating indirection.
                      if(ret == 0)
                      {
                         STALL[MSHR.entry[mshr_index].type]++;
                         return;
                      }
                  }
                  else
                  {   
                      if(FIRST_TIME_CACHE_SETUP_DONE == 1 && TURN == 1)
                          sender_filled_up_empty_block ++;
                  }
              }
              else
              {
									cout<< "Block in tag array is not empty"<<endl;
               		data_set = block[set][way].set_ptr;
               		data_way = block[set][way].way_ptr;
                  assert(0);
              }
         #endif
        }
        else
            way = find_victim(fill_cpu, MSHR.entry[mshr_index].instr_id, set, block[set], MSHR.entry[mshr_index].ip, 
															MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].type);

#ifdef LLC_BYPASS
        if ((cache_type == IS_LLC) && (way == LLC_WAY)) { // this is a bypass that does not fill the LLC

            // update replacement policy
            if (cache_type == IS_LLC) {
                llc_update_replacement_state(fill_cpu, set, way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, 0, MSHR.entry[mshr_index].type, 0);
								#if MIRAGE
                  side_level->llc_update_replacement_state(fill_cpu, data_set, data_way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, 0, MSHR.entry[mshr_index].type, 0);
							  #endif
            }
            else
                update_replacement_state(fill_cpu, set, way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, 0, MSHR.entry[mshr_index].type, 0);

            // COLLECT STATS
						//cout << "handle_fill: cache type: "<<(int)cache_type << " "<< MSHR.entry[mshr_index].address<< " core cycle: "<<current_core_cycle[0] << endl;
            sim_miss[fill_cpu][MSHR.entry[mshr_index].type]++;
            sim_access[fill_cpu][MSHR.entry[mshr_index].type]++;

						// for MIRAGE
						#if MIRAGE
							side_level->sim_miss[fill_cpu][MSHR.entry[mshr_index].type]++;
            	side_level->sim_access[fill_cpu][MSHR.entry[mshr_index].type]++;
						#endif

            MSHR.remove_queue(&MSHR.entry[mshr_index]);
            MSHR.num_returned--;

            update_fill_cycle();

            return; // return here, no need to process further in this function
        }
#endif

        uint8_t  do_fill = 1;

#ifdef INCLUSIVE
    // When eviction is done at LLC Level
	 if(cache_type == IS_LLC && block[set][way].valid) 
	 {
			 int ret = back_invalidate_for_inclusion(block[set][way].cpu, block[set][way].address, 1, block[set][way].instr_id); // calling back invalidation function for maintaining inclusion.
			 if(ret == 2)
			 {   //XXX Magically copying the dirty value to LLC block in the same cpu cycle.
					 block[set][way].dirty = 1; //make tag block dirty 
			 }
			if(ret == 0) // Can't process the invalidation in this cycle.
			{
				 STALL[MSHR.entry[mshr_index].type]++;
				 return;
			}
					// If ret is 1, everything is fine.
	 }
    // When eviction is done at L2C Level
   if(cache_type == IS_L2C && block[set][way].valid)
   {
       int ret = back_invalidate_for_inclusion(block[set][way].cpu, block[set][way].address, 0, block[set][way].instr_id); // calling back invalidation function for maintaining inclusion.
       if(ret == 2)
       {   //XXX Magically copying the dirty value to L2C block in the same cpu cycle.
           block[set][way].dirty = 1; //make tag block dirty 
       }
      if(ret == 0) // Can't process the invalidation in this cycle.
      {
         STALL[MSHR.entry[mshr_index].type]++;
         return;
      }
          // If ret is 1, everything is fine.
   }

#endif

        // is this dirty?
        if (block[set][way].dirty && block[set][way].valid) {

						if(cache_type == IS_LLC)
						{
							#if MIRAGE
								if( side_level->block[block[set][way].set_ptr][block[set][way].way_ptr].dirty == 0)
                { 
                    // Sanity check
										cout <<"block[set][way].valid: " << (int)block[set][way].valid << " set: "<<set << " way: "<<way << " block[set][way].set_ptr: "
                         <<block[set][way].set_ptr<< " block[set][way].way_ptr: " << block[set][way].way_ptr<< endl;
                		assert(0); 
                }
              #endif
            }

            // check if the lower level WQ has enough room to keep this writeback request
            if (lower_level) {
                if (lower_level->get_occupancy(2, block[set][way].address) == lower_level->get_size(2, block[set][way].address)) {

                    // lower level WQ is full, cannot replace this victim
                    do_fill = 0;
                    lower_level->increment_WQ_FULL(block[set][way].address);
                    STALL[MSHR.entry[mshr_index].type]++;

                    DP ( if (warmup_complete[fill_cpu]) {
                    cout << "[" << NAME << "] " << __func__ << "do_fill: " << +do_fill;
                    cout << " lower level wq is full!" << " fill_addr: " << hex << MSHR.entry[mshr_index].address;
                    cout << " victim_addr: " << block[set][way].tag << dec << endl; });
                     }
                else {
                    PACKET writeback_packet;

                    writeback_packet.fill_level = fill_level << 1;
                    writeback_packet.cpu = fill_cpu;
                    writeback_packet.address = block[set][way].address;
                    writeback_packet.full_addr = block[set][way].full_addr;
                    writeback_packet.data = block[set][way].data;
                    writeback_packet.instr_id = MSHR.entry[mshr_index].instr_id;
                    writeback_packet.ip = 0; // writeback does not have ip
                    writeback_packet.type = WRITEBACK;
                    writeback_packet.event_cycle = current_core_cycle[fill_cpu];

                    lower_level->add_wq(&writeback_packet);
                }
            }
#ifdef SANITY_CHECK
            else {
                // sanity check
                if (cache_type != IS_STLB)
                    assert(0);
            }
#endif
        }


        if (do_fill) {
            // update prefetcher
            if (cache_type == IS_L1D)
                l1d_prefetcher_cache_fill(MSHR.entry[mshr_index].full_addr, set, way, (MSHR.entry[mshr_index].type == PREFETCH) ? 1 : 0, block[set][way].full_addr);
            if  (cache_type == IS_L2C)
                l2c_prefetcher_cache_fill(MSHR.entry[mshr_index].full_addr, set, way, (MSHR.entry[mshr_index].type == PREFETCH) ? 1 : 0, block[set][way].full_addr);

            // update replacement policy
            if (cache_type == IS_LLC) 
						{
						#if MIRAGE
								llc_update_replacement_state_mirage(fill_cpu, set, way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, block[set][way].full_addr, MSHR.entry[mshr_index].type, 0, part);
              	side_level->llc_update_replacement_state(fill_cpu, data_set, data_way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, side_level->block[data_set][data_way].full_addr, MSHR.entry[mshr_index].type, 0);
						#else
								llc_update_replacement_state(fill_cpu, set, way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, block[set][way].full_addr, MSHR.entry[mshr_index].type, 0);
            #endif

            }
            else
                update_replacement_state(fill_cpu, set, way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, block[set][way].full_addr, MSHR.entry[mshr_index].type, 0);

            // COLLECT STATS
            sim_miss[fill_cpu][MSHR.entry[mshr_index].type]++;
            sim_access[fill_cpu][MSHR.entry[mshr_index].type]++;

						fill_cache(set, way, &MSHR.entry[mshr_index]); // fill_cache() should be called first and then update the stats. As full_addr is previous value. If it was used in replacement status. Then it is a problem.
						if(cache_type == IS_LLC) {
						#if MIRAGE
            	side_level->sim_miss[fill_cpu][MSHR.entry[mshr_index].type]++;
            	side_level->sim_access[fill_cpu][MSHR.entry[mshr_index].type]++;
              side_level->fill_cache(data_set, data_way, &MSHR.entry[mshr_index]);

              // Updating Indirection
              block[set][way].set_ptr = data_set;
              block[set][way].way_ptr = data_way;
              side_level->block[data_set][data_way].set_ptr = set;
              side_level->block[data_set][data_way].way_ptr = way;
            #endif
           }

            // RFO marks cache line dirty
            if (cache_type == IS_L1D) {
                if (MSHR.entry[mshr_index].type == RFO)
                    block[set][way].dirty = 1;
            }

            MSHR.remove_queue(&MSHR.entry[mshr_index]);
            MSHR.num_returned--;

            update_fill_cycle();
        }
    }
}

void CACHE::handle_writeback()
{
    // handle write
    uint32_t writeback_cpu = WQ.entry[WQ.head].cpu;
    //cout << "cache_type: "<< (int) cache_type << " inside handle_writeback " <<endl;
    if (writeback_cpu == NUM_CPUS)
        return;

    // handle the oldest entry
    if ((WQ.entry[WQ.head].event_cycle <= current_core_cycle[writeback_cpu]) && (WQ.occupancy > 0)) {
        int index = WQ.head;


        
        //if(cache_type == IS_L1I)
      //  cout << "cache_type: "<< (int) cache_type << " inside handle_writeback " <<endl;

        // access cache
        uint32_t set = get_set(WQ.entry[index].address);
        uint32_t way = check_hit(&WQ.entry[index]);
        int data_set = -1, data_way = -1, part=-1;
        
        if (way >= 0 && way < NUM_WAY) { // writeback hit (or RFO hit for L1D)

            if (cache_type == IS_LLC) 
						{
                #if MIRAGE
									uint32_t set_per_partition[NUM_SKEW];
              		for(uint32_t partition = 0; partition < NUM_SKEW; partition++)
              		{
                  		set_per_partition[partition] = extractSet(WQ.entry[index].address, partition);
											if(way >= (NUM_WAY/NUM_SKEW)*partition && way < (NUM_WAY/NUM_SKEW)*(partition+1))
											{
													part = partition;
													set = set_per_partition[partition];  // Changing the set value for mirage LLC.
                      }
              		}
                   data_set = block[set][way].set_ptr;
                   data_way = block[set][way].way_ptr;
                   side_level->llc_update_replacement_state(writeback_cpu, data_set, data_way, side_level->block[data_set][data_way].full_addr, WQ.entry[index].ip, 0, WQ.entry[index].type, 1);
									 llc_update_replacement_state_mirage(writeback_cpu, set, way, block[set][way].full_addr, WQ.entry[index].ip, 0, WQ.entry[index].type, 1, part);
								#else
									llc_update_replacement_state(writeback_cpu, set, way, block[set][way].full_addr, WQ.entry[index].ip, 0, WQ.entry[index].type, 1);
								#endif
            }
            else
                update_replacement_state(writeback_cpu, set, way, block[set][way].full_addr, WQ.entry[index].ip, 0, WQ.entry[index].type, 1);

            // COLLECT STATS
            sim_hit[writeback_cpu][WQ.entry[index].type]++;
            sim_access[writeback_cpu][WQ.entry[index].type]++;

            // mark dirty
            block[set][way].dirty = 1;

            if (cache_type == IS_LLC) 
            {
							#if MIRAGE
                   side_level->sim_hit[writeback_cpu][WQ.entry[index].type]++;
                   side_level->sim_access[writeback_cpu][WQ.entry[index].type]++;
                   side_level->block[data_set][data_way].dirty = 1;
            	#endif
            }
            if (cache_type == IS_ITLB)
                WQ.entry[index].instruction_pa = block[set][way].data;  //@Question, Not clear about this and below 2 conditions.
            else if (cache_type == IS_DTLB)
                WQ.entry[index].data_pa = block[set][way].data;
            else if (cache_type == IS_STLB)
                WQ.entry[index].data = block[set][way].data;

            // check fill level
            if (WQ.entry[index].fill_level < fill_level) {   // @Question, will this ever be the case? Is this unnecessary. fill_level will never be less for WQ pkt.

                if (WQ.entry[index].instruction) 
                    upper_level_icache[writeback_cpu]->return_data(&WQ.entry[index]);
                else // data
                    upper_level_dcache[writeback_cpu]->return_data(&WQ.entry[index]);
            }

            HIT[WQ.entry[index].type]++;
            ACCESS[WQ.entry[index].type]++;

						if (cache_type == IS_LLC)
            {
              #if MIRAGE
                   side_level->HIT[WQ.entry[index].type]++;
                   side_level->ACCESS[WQ.entry[index].type]++;
              #endif
            }						

            // remove this entry from WQ
            WQ.remove_queue(&WQ.entry[index]);
        }
        else { // writeback miss (or RFO miss for L1D)
            
            DP ( if (warmup_complete[writeback_cpu]) {
            cout << "[" << NAME << "] " << __func__ << " type: " << +WQ.entry[index].type << " miss";
            cout << " instr_id: " << WQ.entry[index].instr_id << " address: " << hex << WQ.entry[index].address;
            cout << " full_addr: " << WQ.entry[index].full_addr << dec;
            cout << " cycle: " << WQ.entry[index].event_cycle << endl; });

            if (cache_type == IS_L1D) { // RFO miss

                // check mshr
                uint8_t miss_handled = 1;
                int mshr_index = check_mshr(&WQ.entry[index]);

                if ((mshr_index == -1) && (MSHR.occupancy < MSHR_SIZE)) { // this is a new miss

                    // add it to mshr (RFO miss)
                    add_mshr(&WQ.entry[index]);

                    // add it to the next level's read queue
                    //if (lower_level) // L1D always has a lower level cache
                        lower_level->add_rq(&WQ.entry[index]);
                }
                else {
                    if ((mshr_index == -1) && (MSHR.occupancy == MSHR_SIZE)) { // not enough MSHR resource
                        
                        // cannot handle miss request until one of MSHRs is available
                        miss_handled = 0;
                        STALL[WQ.entry[index].type]++;
                    }
                    else if (mshr_index != -1) { // already in-flight miss

                        // update fill_level // @Question, Why is this needed? L1D has the least fill_level i.e. FILL_L1, this condition will never be true.
                        if (WQ.entry[index].fill_level < MSHR.entry[mshr_index].fill_level)
                            MSHR.entry[mshr_index].fill_level = WQ.entry[index].fill_level;

                        // update request // @Question, what is this ?
                        if (MSHR.entry[mshr_index].type == PREFETCH) {
                            uint8_t  prior_returned = MSHR.entry[mshr_index].returned;
                            uint64_t prior_event_cycle = MSHR.entry[mshr_index].event_cycle;
			    MSHR.entry[mshr_index] = WQ.entry[index];

                            // in case request is already returned, we should keep event_cycle and retunred variables
                            MSHR.entry[mshr_index].returned = prior_returned; // @Question, what is this ?
                            MSHR.entry[mshr_index].event_cycle = prior_event_cycle;
                        }

                        MSHR_MERGED[WQ.entry[index].type]++;

                        DP ( if (warmup_complete[writeback_cpu]) {
                        cout << "[" << NAME << "] " << __func__ << " mshr merged";
                        cout << " instr_id: " << WQ.entry[index].instr_id << " prior_id: " << MSHR.entry[mshr_index].instr_id; 
                        cout << " address: " << hex << WQ.entry[index].address;
                        cout << " full_addr: " << WQ.entry[index].full_addr << dec;
                        cout << " cycle: " << WQ.entry[index].event_cycle << endl; });
                    }
                    else { // WE SHOULD NOT REACH HERE
                        cerr << "[" << NAME << "] MSHR errors" << endl;
                        assert(0);
                    }
                }

                if (miss_handled) {

                    MISS[WQ.entry[index].type]++;
                    ACCESS[WQ.entry[index].type]++;

                    // remove this entry from WQ
                    WQ.remove_queue(&WQ.entry[index]);
                }

            }
            else {
                // find victim
                uint32_t set = get_set(WQ.entry[index].address), way=-1, part=-1;
                uint32_t data_set = -1, data_way = -1;              

                if (cache_type == IS_LLC) 
                {
										// Victim selection for MIRAGE
          					#if MIRAGE
              					uint32_t set_per_partition[NUM_SKEW];
              					for(uint32_t partition = 0; partition < NUM_SKEW; partition++)
              					{
                  					set_per_partition[partition] = extractSet(WQ.entry[index].address, partition);
              					}
              					part = load_balanced_skew_selection(set_per_partition);
              					set = set_per_partition[part];

              					way = llc_find_victim_mirage(writeback_cpu, WQ.entry[index].instr_id, set, block[set], WQ.entry[index].ip, 
															WQ.entry[index].full_addr, WQ.entry[index].type, part);

          					#else
                    		way = llc_find_victim(writeback_cpu, WQ.entry[index].instr_id, set, block[set], WQ.entry[index].ip, 
															WQ.entry[index].full_addr, WQ.entry[index].type);
										#endif
										#if MIRAGE
                        if(block[set][way].set_ptr == UINT32_MAX || block[set][way].way_ptr == UINT32_MAX) // Victim is not linked with any data LLC block.
                        {
                            data_set = side_level->get_set(WQ.entry[index].address);    if(data_set != 0) assert(0); // Sanity check
                            data_way = side_level->llc_find_victim(writeback_cpu, WQ.entry[index].instr_id, data_set, block[data_set], WQ.entry[index].ip, 
																																	WQ.entry[index].full_addr, WQ.entry[index].type);
                            
														if(side_level->block[data_set][data_way].set_ptr != UINT32_MAX || side_level->block[data_set][data_way].way_ptr != UINT32_MAX)
                            {    
                                // Back invalidate due to invalidating indirection.
							#if INCLUSIVE 
																 if(side_level->block[data_set][data_way].valid) //In Mirage LLC, back-invalidation maintains inclusivity.
																 {
																		 int ret = back_invalidate_for_inclusion(side_level->block[data_set][data_way].cpu, side_level->block[data_set][data_way].address, 1, side_level->block[data_set][data_way].instr_id); // calling back invalidation function for maintaining inclusion.

																		 if(ret == 2)
																		 {   //XXX Magically copying the dirty value to LLC block in the same cpu cycle.
																				 side_level->block[data_set][data_way].dirty = 1; // Make data block dirty.
																				 //Make tag block dirty.
																				 block[side_level->block[data_set][data_way].set_ptr][side_level->block[data_set][data_way].way_ptr].dirty = 1;
																		 }
																		 if(ret == 0) // Can't process the invalidation in this cycle.
																		 {
																				STALL[WQ.entry[index].type]++;
																				return;
																		 }
																				// If ret is 1, everything is fine.
																 }
							#endif

																 int ret = back_invalidate_indirection( data_set, data_way);
																 if(ret == 0)
																 {
																   	STALL[WQ.entry[index].type]++;  
																		return; 
																 }
													  }
											  }
											  else         // Victim is already linked, so use it.
											  {
                           cout<< "Block in tag array is not empty"<<endl;
													 data_set = block[set][way].set_ptr;
													 data_way = block[set][way].way_ptr;
                           assert(0);
											  }
					 	#endif
										}
										else
												way = find_victim(writeback_cpu, WQ.entry[index].instr_id, set, block[set], WQ.entry[index].ip, WQ.entry[index].full_addr, WQ.entry[index].type);

		#ifdef LLC_BYPASS
										if ((cache_type == IS_LLC) && (way == LLC_WAY)) {
												cerr << "LLC bypassing for writebacks is not allowed!" << endl;
												assert(0);
										}
		#endif

										uint8_t  do_fill = 1;

#ifdef INCLUSIVE
							// When eviction is done at LLC Level
						 if(cache_type == IS_LLC && block[set][way].valid)
						 {
								 int ret = back_invalidate_for_inclusion(block[set][way].cpu, block[set][way].address, 1, block[set][way].instr_id); // calling back invalidation function for inclusion.
								 if(ret == 2)
								 {   //XXX Magically copying the dirty value to LLC block in the same cpu cycle.
										 block[set][way].dirty = 1; //make tag block dirty 
								 }
								if(ret == 0) // Can't process the invalidation in this cycle.
								{
									 STALL[WQ.entry[index].type]++;
									 return;
								}
										// If ret is 1, everything is fine.
						 }
							// When eviction is done at L2C Level
						 if(cache_type == IS_L2C && block[set][way].valid)
						 {
								 int ret = back_invalidate_for_inclusion(block[set][way].cpu, block[set][way].address, 0, block[set][way].instr_id); // calling back invalidation function for inclusion.
								 if(ret == 2)
								 {   //XXX Magically copying the dirty value to L2C block in the same cpu cycle.
										 block[set][way].dirty = 1; //make tag block dirty 
								 }
								if(ret == 0) // Can't process the invalidation in this cycle.
								{
									 STALL[WQ.entry[index].type]++;
									 return;
								}
										// If ret is 1, everything is fine.
						 }

#endif
										// is this dirty?
										if (block[set][way].dirty && block[set][way].valid) {

												// check if the lower level WQ has enough room to keep this writeback request
												if (lower_level) { 
														if (lower_level->get_occupancy(2, block[set][way].address) == lower_level->get_size(2, block[set][way].address)) {

																// lower level WQ is full, cannot replace this victim
																do_fill = 0;
																lower_level->increment_WQ_FULL(block[set][way].address);
																STALL[WQ.entry[index].type]++;

																DP ( if (warmup_complete[writeback_cpu]) {
																cout << "[" << NAME << "] " << __func__ << "do_fill: " << +do_fill;
																cout << " lower level wq is full!" << " fill_addr: " << hex << WQ.entry[index].address;
																cout << " victim_addr: " << block[set][way].tag << dec << endl; });
														}
														else { 
																PACKET writeback_packet;

																writeback_packet.fill_level = fill_level << 1;
																writeback_packet.cpu = writeback_cpu;
																writeback_packet.address = block[set][way].address;
																writeback_packet.full_addr = block[set][way].full_addr;
																writeback_packet.data = block[set][way].data;
																writeback_packet.instr_id = WQ.entry[index].instr_id;
																writeback_packet.ip = 0;
																writeback_packet.type = WRITEBACK;
																writeback_packet.event_cycle = current_core_cycle[writeback_cpu];

																lower_level->add_wq(&writeback_packet);
														}
												}
		#ifdef SANITY_CHECK
												else {
														// sanity check
														if (cache_type != IS_STLB) //@Question, how is it handled for STLB? What is a dirty block in TLB ?
																assert(0);
												}
		#endif
										}

										if (do_fill) {
												// update prefetcher
												if (cache_type == IS_L1D)
														l1d_prefetcher_cache_fill(WQ.entry[index].full_addr, set, way, 0, block[set][way].full_addr);
												else if (cache_type == IS_L2C)
														l2c_prefetcher_cache_fill(WQ.entry[index].full_addr, set, way, 0, block[set][way].full_addr);

												// update replacement policy
												if (cache_type == IS_LLC) 
												{
													#if MIRAGE  
															side_level->llc_update_replacement_state(writeback_cpu, data_set, data_way, WQ.entry[index].full_addr, WQ.entry[index].ip, 
																																			side_level->block[data_set][data_way].full_addr, WQ.entry[index].type, 0);
															llc_update_replacement_state_mirage(writeback_cpu, set, way, WQ.entry[index].full_addr, WQ.entry[index].ip, 
																																	block[set][way].full_addr, WQ.entry[index].type, 0, part);
													#else
															llc_update_replacement_state(writeback_cpu, set, way, WQ.entry[index].full_addr, WQ.entry[index].ip, 
																													block[set][way].full_addr, WQ.entry[index].type, 0);
													#endif
												}
												else
														update_replacement_state(writeback_cpu, set, way, WQ.entry[index].full_addr, WQ.entry[index].ip, block[set][way].full_addr, 
																											WQ.entry[index].type, 0);

												// COLLECT STATS
												sim_miss[writeback_cpu][WQ.entry[index].type]++;
												sim_access[writeback_cpu][WQ.entry[index].type]++;

												fill_cache(set, way, &WQ.entry[index]);
												 
												if (cache_type == IS_LLC) {
												
												#if MIRAGE
													side_level->sim_miss[writeback_cpu][WQ.entry[index].type]++;
													side_level->sim_access[writeback_cpu][WQ.entry[index].type]++;
													side_level->fill_cache(data_set, data_way, &WQ.entry[index]);

													 // Updating Indirection
													block[set][way].set_ptr = data_set;
													block[set][way].way_ptr = data_way;
													side_level->block[data_set][data_way].set_ptr = set;
													side_level->block[data_set][data_way].way_ptr = way;

													side_level->block[data_set][data_way].dirty = 1;

												#endif
												}

												// mark dirty
												block[set][way].dirty = 1; 

												// check fill level
												if (WQ.entry[index].fill_level < fill_level) {

														if (WQ.entry[index].instruction) 
																upper_level_icache[writeback_cpu]->return_data(&WQ.entry[index]);
														else // data
																upper_level_dcache[writeback_cpu]->return_data(&WQ.entry[index]);
												}

												MISS[WQ.entry[index].type]++;
												ACCESS[WQ.entry[index].type]++;

												if (cache_type == IS_LLC)
												{
													 #if MIRAGE  
															side_level->MISS[WQ.entry[index].type]++;
															side_level->ACCESS[WQ.entry[index].type]++;
													 #endif
												}

												// remove this entry from WQ
												WQ.remove_queue(&WQ.entry[index]);
										}
								}
						}
				}
		}

void CACHE::handle_read()
{
				// handle read
				uint32_t read_cpu = RQ.entry[RQ.head].cpu;

        //if(cache_type == IS_L1I)
        //cout << "cache_type: "<< (int) cache_type << " MAX_READ: "<<MAX_READ <<endl;

				if (read_cpu == NUM_CPUS)
						return;

				for (uint32_t i=0; i<MAX_READ; i++) {
             
                //if(cache_type == IS_L1I)
                  //  cout << "cache_type: "<< (int) cache_type << " i: "<<i <<endl;
						// handle the oldest entry
						if ((RQ.entry[RQ.head].event_cycle <= current_core_cycle[read_cpu]) && (RQ.occupancy > 0)) {
								int index = RQ.head;

                //if( RQ.entry[index].ip  == 4200542 )
                 //cout <<"Hi I am here"<<endl;
								// access cache
                //if(cache_type == IS_L1I)
                    //cout <<"i: "<<i<<" entry address: "<<RQ.entry[index].address<<" instr_id: "<<RQ.entry[index].instr_id<<endl;
								uint32_t set = get_set(RQ.entry[index].address), data_set = -1;
								uint32_t way = check_hit(&RQ.entry[index]), data_way = -1, part=-1;
								int ret_index = check_mshr_for_hits(&RQ.entry[index]); 
								if(ret_index != -1)
								{
										// update prefetcher on load instruction
										if (RQ.entry[index].type == LOAD) {
												if (cache_type == IS_L1D)
														l1d_prefetcher_operate(RQ.entry[index].full_addr, RQ.entry[index].ip, 1, RQ.entry[index].type);
												else if (cache_type == IS_L2C)
														l2c_prefetcher_operate(RQ.entry[index].full_addr, RQ.entry[index].ip, 1, RQ.entry[index].type);
										}

										// remove this entry from RQ
										RQ.remove_queue(&RQ.entry[index]);
		 
								}
								else if (way >= 0 && way < NUM_WAY) { // read hit

										if (cache_type == IS_ITLB) {
												if(DETAILED_PRINT || (RQ.entry[index].instr_id >= range_start && RQ.entry[index].instr_id <= range_end) )
														cout << "transferred to ITLB processed queue: " << RQ.entry[index].ip<<" current_core_cycle[0]: "<<current_core_cycle[0] << endl;
												RQ.entry[index].instruction_pa = block[set][way].data;
												if (PROCESSED.occupancy < PROCESSED.SIZE)
														PROCESSED.add_queue(&RQ.entry[index]);
										}
										else if (cache_type == IS_DTLB) {
												if(DETAILED_PRINT || (RQ.entry[index].instr_id >= range_start && RQ.entry[index].instr_id <= range_end) )
														cout << "transferred to DTLB processed queue: " << RQ.entry[index].ip<<" current_core_cycle[0]: "<<current_core_cycle[0] << endl;
												RQ.entry[index].data_pa = block[set][way].data;
												if (PROCESSED.occupancy < PROCESSED.SIZE)
														PROCESSED.add_queue(&RQ.entry[index]);
										}
										else if (cache_type == IS_STLB) 
												RQ.entry[index].data = block[set][way].data;

										else if (cache_type == IS_L1I) {
												if (PROCESSED.occupancy < PROCESSED.SIZE)
														PROCESSED.add_queue(&RQ.entry[index]);
											if(DETAILED_PRINT || (RQ.entry[index].instr_id >= range_start && RQ.entry[index].instr_id <= range_end) )
											cout << "transferred to L1I processed queue: " << RQ.entry[index].ip<<" current_core_cycle[0]: "<<current_core_cycle[0] << endl;
										}
		 
										else if ((cache_type == IS_L1D) && (RQ.entry[index].type != PREFETCH)) {
												if (PROCESSED.occupancy < PROCESSED.SIZE)
														PROCESSED.add_queue(&RQ.entry[index]);
                        if(DETAILED_PRINT || (RQ.entry[index].instr_id >= range_start && RQ.entry[index].instr_id <= range_end) )
                      cout << "transferred to L1D processed queue: " << RQ.entry[index].ip<<" current_core_cycle[0]: "<<current_core_cycle[0] << endl;
										}

										// update prefetcher on load instruction
										if (RQ.entry[index].type == LOAD) {
												if (cache_type == IS_L1D) 
														l1d_prefetcher_operate(block[set][way].full_addr, RQ.entry[index].ip, 1, RQ.entry[index].type);
												else if (cache_type == IS_L2C)
														l2c_prefetcher_operate(block[set][way].full_addr, RQ.entry[index].ip, 1, RQ.entry[index].type);
										}

										// update replacement policy
										if (cache_type == IS_LLC) 
										 {
											#if MIRAGE
													uint32_t set_per_partition[NUM_SKEW];

													for(uint32_t partition = 0; partition < NUM_SKEW; partition++)
													{
															set_per_partition[partition] = extractSet(RQ.entry[index].address, partition);
															if(way >= (NUM_WAY/NUM_SKEW)*partition && way < (NUM_WAY/NUM_SKEW)*(partition+1)) // If (way > 7) then 2nd skew, if (way < 8) then 1st skew.
															{	
																	part = partition;
																	set = set_per_partition[partition];  // Changing the set value for mirage LLC.
																	break;
															}
													}
		 
													data_set = block[set][way].set_ptr; 
													data_way = block[set][way].way_ptr;
													side_level->llc_update_replacement_state(read_cpu, data_set, data_way, side_level->block[data_set][data_way].full_addr, 
																																	 RQ.entry[index].ip, 0, RQ.entry[index].type, 1);
													llc_update_replacement_state_mirage(read_cpu, set, way, block[set][way].full_addr, RQ.entry[index].ip, 0, RQ.entry[index].type, 1, part);
											#else
													llc_update_replacement_state(read_cpu, set, way, block[set][way].full_addr, RQ.entry[index].ip, 0, RQ.entry[index].type, 1);
											#endif
										 }
										else
												update_replacement_state(read_cpu, set, way, block[set][way].full_addr, RQ.entry[index].ip, 0, RQ.entry[index].type, 1);

										// COLLECT STATS
										sim_hit[read_cpu][RQ.entry[index].type]++;
										sim_access[read_cpu][RQ.entry[index].type]++;

										// check fill level
										if (RQ.entry[index].fill_level < fill_level) {

												if (RQ.entry[index].instruction) 
														upper_level_icache[read_cpu]->return_data(&RQ.entry[index]);
												else // data
														upper_level_dcache[read_cpu]->return_data(&RQ.entry[index]);
										}

										// update prefetch stats and reset prefetch bit
										if (block[set][way].prefetch) {
												pf_useful++;
												block[set][way].prefetch = 0;
										}
										block[set][way].used = 1;

										if (cache_type == IS_LLC) {
												#if MIRAGE  
														
												if (side_level->block[data_set][data_way].prefetch) 
												{
														side_level->pf_useful++;
														side_level->block[data_set][data_way].prefetch = 0;
												}

														
														side_level->sim_hit[read_cpu][RQ.entry[index].type]++;
														side_level->sim_access[read_cpu][RQ.entry[index].type]++;
														side_level->block[data_set][data_way].used = 1;
														side_level->HIT[RQ.entry[index].type]++;
														side_level->ACCESS[RQ.entry[index].type]++;
												#endif
										}

										HIT[RQ.entry[index].type]++;
										ACCESS[RQ.entry[index].type]++;
										
										// remove this entry from RQ
										RQ.remove_queue(&RQ.entry[index]);
								}
								else { // read miss
										DP ( if (warmup_complete[read_cpu]) {
										cout << "[" << NAME << "] " << __func__ << " read miss";
										cout << " instr_id: " << RQ.entry[index].instr_id << " address: " << hex << RQ.entry[index].address;
										cout << " full_addr: " << RQ.entry[index].full_addr << dec;
										cout << " cycle: " << RQ.entry[index].event_cycle << endl; });

										// check mshr
										uint8_t miss_handled = 1;
										int mshr_index = check_mshr(&RQ.entry[index]);

										if ((mshr_index == -1) && (MSHR.occupancy < MSHR_SIZE)) { // this is a new miss

							// Write the address and packet_type in the file
							#if generate_llc_traces
								if(cache_type == IS_L2C && all_warmup_complete > NUM_CPUS)
									llc_access_stream_file << RQ.entry[index].full_addr<< " " << (int)RQ.entry[index].type<<endl;
							#endif


							// add it to mshr (read miss)
												add_mshr(&RQ.entry[index]);

												// add it to the next level's read queue
												if (lower_level)
														lower_level->add_rq(&RQ.entry[index]);
												else { // this is the last level
														if (cache_type == IS_STLB) {
																// TODO: need to differentiate page table walk and actual swap

																// emulate page table walk
																uint64_t pa = va_to_pa(read_cpu, RQ.entry[index].instr_id, RQ.entry[index].full_addr, RQ.entry[index].address);

																RQ.entry[index].data = pa >> LOG2_PAGE_SIZE; 
																RQ.entry[index].event_cycle = current_core_cycle[read_cpu];
																return_data(&RQ.entry[index]);
														}
												}
										}
										else {
												if ((mshr_index == -1) && (MSHR.occupancy == MSHR_SIZE)) { // not enough MSHR resource
														
														// cannot handle miss request until one of MSHRs is available
														miss_handled = 0;
														STALL[RQ.entry[index].type]++;
												}
												else if (mshr_index != -1) { // already in-flight miss

														// mark merged consumer
														if (RQ.entry[index].type == RFO) {

                            if (RQ.entry[index].tlb_access) { // @Question, what is tlb_access, sq_index ?
                                uint32_t sq_index = RQ.entry[index].sq_index;
                                MSHR.entry[mshr_index].store_merged = 1;
                                MSHR.entry[mshr_index].sq_index_depend_on_me.insert (sq_index);
				MSHR.entry[mshr_index].sq_index_depend_on_me.join (RQ.entry[index].sq_index_depend_on_me, SQ_SIZE);
                            }

                            if (RQ.entry[index].load_merged) {
                                //uint32_t lq_index = RQ.entry[index].lq_index; 
                                MSHR.entry[mshr_index].load_merged = 1;
                                //MSHR.entry[mshr_index].lq_index_depend_on_me[lq_index] = 1;
				MSHR.entry[mshr_index].lq_index_depend_on_me.join (RQ.entry[index].lq_index_depend_on_me, LQ_SIZE);
                            }
                        }
                        else {
                            if (RQ.entry[index].instruction) {
                                uint32_t rob_index = RQ.entry[index].rob_index;
                                MSHR.entry[mshr_index].instr_merged = 1;
                                MSHR.entry[mshr_index].rob_index_depend_on_me.insert (rob_index);

                                DP (if (warmup_complete[MSHR.entry[mshr_index].cpu]) {
                                cout << "[INSTR_MERGED] " << __func__ << " cpu: " << MSHR.entry[mshr_index].cpu << " instr_id: " << MSHR.entry[mshr_index].instr_id;
                                cout << " merged rob_index: " << rob_index << " instr_id: " << RQ.entry[index].instr_id << endl; });

                                if (RQ.entry[index].instr_merged) {
				    MSHR.entry[mshr_index].rob_index_depend_on_me.join (RQ.entry[index].rob_index_depend_on_me, ROB_SIZE);
                                    DP (if (warmup_complete[MSHR.entry[mshr_index].cpu]) {
                                    cout << "[INSTR_MERGED] " << __func__ << " cpu: " << MSHR.entry[mshr_index].cpu << " instr_id: " << MSHR.entry[mshr_index].instr_id;
                                    cout << " merged rob_index: " << i << " instr_id: N/A" << endl; });
                                }
                            }
                            else 
                            {
                                uint32_t lq_index = RQ.entry[index].lq_index;
                                MSHR.entry[mshr_index].load_merged = 1;
                                MSHR.entry[mshr_index].lq_index_depend_on_me.insert (lq_index);

                                DP (if (warmup_complete[read_cpu]) {
                                cout << "[DATA_MERGED] " << __func__ << " cpu: " << read_cpu << " instr_id: " << RQ.entry[index].instr_id;
                                cout << " merged rob_index: " << RQ.entry[index].rob_index << " instr_id: " << RQ.entry[index].instr_id << " lq_index: " << RQ.entry[index].lq_index << endl; });
				MSHR.entry[mshr_index].lq_index_depend_on_me.join (RQ.entry[index].lq_index_depend_on_me, LQ_SIZE);
                                if (RQ.entry[index].store_merged) {
                                    MSHR.entry[mshr_index].store_merged = 1;
				    MSHR.entry[mshr_index].sq_index_depend_on_me.join (RQ.entry[index].sq_index_depend_on_me, SQ_SIZE);
                                }
                            }
                        }

                        // update fill_level, if this request has originated from even lower levels.
                        if (RQ.entry[index].fill_level < MSHR.entry[mshr_index].fill_level)
                            MSHR.entry[mshr_index].fill_level = RQ.entry[index].fill_level;

                        // update request
                        if (MSHR.entry[mshr_index].type == PREFETCH) {
                            uint8_t  prior_returned = MSHR.entry[mshr_index].returned;
                            uint64_t prior_event_cycle = MSHR.entry[mshr_index].event_cycle;
                            MSHR.entry[mshr_index] = RQ.entry[index];
                            
                            // in case request is already returned, we should keep event_cycle and retunred variables
                            MSHR.entry[mshr_index].returned = prior_returned;  // @Answer, this will take care of merged mshr entries.
                            MSHR.entry[mshr_index].event_cycle = prior_event_cycle;
                        }

                        MSHR_MERGED[RQ.entry[index].type]++;

                        DP ( if (warmup_complete[read_cpu]) {
                        cout << "[" << NAME << "] " << __func__ << " mshr merged";
                        cout << " instr_id: " << RQ.entry[index].instr_id << " prior_id: " << MSHR.entry[mshr_index].instr_id; 
                        cout << " address: " << hex << RQ.entry[index].address;
                        cout << " full_addr: " << RQ.entry[index].full_addr << dec;
                        cout << " cycle: " << RQ.entry[index].event_cycle << endl; });
                    }
                    else { // WE SHOULD NOT REACH HERE
                        cerr << "[" << NAME << "] MSHR errors" << endl;
                        assert(0);
                    }
                }

                if (miss_handled) {
                    // update prefetcher on load instruction
                    if (RQ.entry[index].type == LOAD) {
                        if (cache_type == IS_L1D) 
                            l1d_prefetcher_operate(RQ.entry[index].full_addr, RQ.entry[index].ip, 0, RQ.entry[index].type);
                        if (cache_type == IS_L2C)
                            l2c_prefetcher_operate(RQ.entry[index].full_addr, RQ.entry[index].ip, 0, RQ.entry[index].type);
                    }

                    MISS[RQ.entry[index].type]++;
                    ACCESS[RQ.entry[index].type]++;

										if(cache_type == IS_LLC)
										{
											#if MIRAGE
                        side_level->MISS[RQ.entry[index].type]++;
                        side_level->ACCESS[RQ.entry[index].type]++;
                      #endif
										}

                    // remove this entry from RQ
                    RQ.remove_queue(&RQ.entry[index]);
                }
            }
        }
    }
}

void CACHE::handle_prefetch()
{
    // handle prefetch
    uint32_t prefetch_cpu = PQ.entry[PQ.head].cpu;
    if (prefetch_cpu == NUM_CPUS)
        return;

    for (uint32_t i=0; i<MAX_READ; i++) {

        // handle the oldest entry
        if ((PQ.entry[PQ.head].event_cycle <= current_core_cycle[prefetch_cpu]) && (PQ.occupancy > 0)) {
            int index = PQ.head;

            // access cache
            uint32_t set = get_set(PQ.entry[index].address), data_set=-1;
            uint32_t way = check_hit(&PQ.entry[index]), data_way=-1, part=-1;
            
            if (way >= 0 && way < NUM_WAY) { // prefetch hit

                // update replacement policy
                if (cache_type == IS_LLC) {
										#if MIRAGE

												uint32_t set_per_partition[NUM_SKEW];
                  			for(uint32_t partition = 0; partition < NUM_SKEW; partition++)
                  			{  
                      			set_per_partition[partition] = extractSet(PQ.entry[index].address, partition);
                      			if(way >= (NUM_WAY/NUM_SKEW)*partition && way < (NUM_WAY/NUM_SKEW)*(partition+1))
                          	{		
																part = partition;
																set = set_per_partition[partition];  // Changing the set value for mirage LLC.
														}
                  			}												

                      	data_set = block[set][way].set_ptr;
                      	data_way = block[set][way].way_ptr;
                      	side_level->llc_update_replacement_state(prefetch_cpu, data_set, data_way, side_level->block[data_set][data_way].full_addr, 
                                                               PQ.entry[index].ip, 0, PQ.entry[index].type, 1);
												llc_update_replacement_state_mirage(prefetch_cpu, set, way, block[set][way].full_addr, PQ.entry[index].ip, 0, PQ.entry[index].type, 1, part);
                    #else
												llc_update_replacement_state(prefetch_cpu, set, way, block[set][way].full_addr, PQ.entry[index].ip, 0, PQ.entry[index].type, 1);
										#endif
                }
                else
                    update_replacement_state(prefetch_cpu, set, way, block[set][way].full_addr, PQ.entry[index].ip, 0, PQ.entry[index].type, 1);

                // COLLECT STATS
                sim_hit[prefetch_cpu][PQ.entry[index].type]++;
                sim_access[prefetch_cpu][PQ.entry[index].type]++;

                // check fill level
                if (PQ.entry[index].fill_level < fill_level) {

                    if (PQ.entry[index].instruction) 
                        upper_level_icache[prefetch_cpu]->return_data(&PQ.entry[index]);
                    else // data
                        upper_level_dcache[prefetch_cpu]->return_data(&PQ.entry[index]);
                }

                HIT[PQ.entry[index].type]++;
                ACCESS[PQ.entry[index].type]++;

								if (cache_type == IS_LLC)
                 {
                   #if MIRAGE
                    side_level->sim_hit[prefetch_cpu][PQ.entry[index].type]++;
                    side_level->sim_access[prefetch_cpu][PQ.entry[index].type]++;
                    side_level->HIT[PQ.entry[index].type]++;
                    side_level->ACCESS[PQ.entry[index].type]++;
                   #endif
                 }
                
                // remove this entry from PQ
                PQ.remove_queue(&PQ.entry[index]);
            }
            else { // prefetch miss

                DP ( if (warmup_complete[prefetch_cpu]) {
                cout << "[" << NAME << "] " << __func__ << " prefetch miss";
                cout << " instr_id: " << PQ.entry[index].instr_id << " address: " << hex << PQ.entry[index].address;
                cout << " full_addr: " << PQ.entry[index].full_addr << dec << " fill_level: " << PQ.entry[index].fill_level;
                cout << " cycle: " << PQ.entry[index].event_cycle << endl; });

                // check mshr
                uint8_t miss_handled = 1;
                int mshr_index = check_mshr(&PQ.entry[index]);

                if ((mshr_index == -1) && (MSHR.occupancy < MSHR_SIZE)) { // this is a new miss

                    DP ( if (warmup_complete[PQ.entry[index].cpu]) {
                    cout << "[" << NAME << "_PQ] " <<  __func__ << " want to add instr_id: " << PQ.entry[index].instr_id << " address: " << hex << PQ.entry[index].address;
                    cout << " full_addr: " << PQ.entry[index].full_addr << dec;
                    cout << " occupancy: " << lower_level->get_occupancy(3, PQ.entry[index].address) << " SIZE: " << lower_level->get_size(3, PQ.entry[index].address) << endl; });

                    // first check if the lower level PQ is full or not
                    // this is possible since multiple prefetchers can exist at each level of caches
                    if (lower_level) {
                        if (cache_type == IS_LLC) {
                            if (lower_level->get_occupancy(1, PQ.entry[index].address) == lower_level->get_size(1, PQ.entry[index].address))
                                miss_handled = 0;
                            else {
                                // add it to MSHRs if this prefetch miss will be filled to this cache level
                                if (PQ.entry[index].fill_level <= fill_level)
                                    add_mshr(&PQ.entry[index]);
                                
                                lower_level->add_rq(&PQ.entry[index]); // add it to the DRAM RQ
                            }
                        }
                        else {
                            if (lower_level->get_occupancy(3, PQ.entry[index].address) == lower_level->get_size(3, PQ.entry[index].address))
                                miss_handled = 0;
                            else {
                                // add it to MSHRs if this prefetch miss will be filled to this cache level
                                if (PQ.entry[index].fill_level <= fill_level)
                                    add_mshr(&PQ.entry[index]);

                                lower_level->add_pq(&PQ.entry[index]); // add it to the DRAM RQ
                            }
                        }
                    }
                }
                else {
                    if ((mshr_index == -1) && (MSHR.occupancy == MSHR_SIZE)) { // not enough MSHR resource

                        // TODO: should we allow prefetching with lower fill level at this case?
                        
                        // cannot handle miss request until one of MSHRs is available
                        miss_handled = 0;
                        STALL[PQ.entry[index].type]++;
                    }
                    else if (mshr_index != -1) { // already in-flight miss

                        // no need to update request except fill_level
                        // update fill_level
                        if (PQ.entry[index].fill_level < MSHR.entry[mshr_index].fill_level)
                            MSHR.entry[mshr_index].fill_level = PQ.entry[index].fill_level;

                        MSHR_MERGED[PQ.entry[index].type]++;

                        DP ( if (warmup_complete[prefetch_cpu]) {
                        cout << "[" << NAME << "] " << __func__ << " mshr merged";
                        cout << " instr_id: " << PQ.entry[index].instr_id << " prior_id: " << MSHR.entry[mshr_index].instr_id; 
                        cout << " address: " << hex << PQ.entry[index].address;
                        cout << " full_addr: " << PQ.entry[index].full_addr << dec << " fill_level: " << MSHR.entry[mshr_index].fill_level;
                        cout << " cycle: " << MSHR.entry[mshr_index].event_cycle << endl; });
                    }
                    else { // WE SHOULD NOT REACH HERE
                        cerr << "[" << NAME << "] MSHR errors" << endl;
                        assert(0);
                    }
                }

                if (miss_handled) {

                    DP ( if (warmup_complete[prefetch_cpu]) {
                    cout << "[" << NAME << "] " << __func__ << " prefetch miss handled";
                    cout << " instr_id: " << PQ.entry[index].instr_id << " address: " << hex << PQ.entry[index].address;
                    cout << " full_addr: " << PQ.entry[index].full_addr << dec << " fill_level: " << PQ.entry[index].fill_level;
                    cout << " cycle: " << PQ.entry[index].event_cycle << endl; });

                    MISS[PQ.entry[index].type]++;
                    ACCESS[PQ.entry[index].type]++;

										if(cache_type == IS_LLC)
                    {
                      #if MIRAGE
                      	side_level->MISS[PQ.entry[index].type]++;
                        side_level->ACCESS[PQ.entry[index].type]++;
                      #endif
                    }
                    // remove this entry from PQ
                    PQ.remove_queue(&PQ.entry[index]);
                }
            }
        }
    }
}

void CACHE::operate()
{
    handle_fill();
    handle_writeback();
    handle_read();

    if (PQ.occupancy && (RQ.occupancy == 0))
        handle_prefetch();
}

uint32_t CACHE::get_set(uint64_t address)
{
    return (uint32_t) (address & ((1 << lg2(NUM_SET)) - 1)); 
}

uint32_t CACHE::get_way(uint64_t address, uint32_t set)
{
    for (uint32_t way=0; way<NUM_WAY; way++) 
		{
        if (block[set][way].valid && (block[set][way].tag == address)) 
            return way;
    }

    return NUM_WAY;
}

void CACHE::fill_cache(uint32_t set, uint32_t way, PACKET *packet)
{

    if(packet->to_be_invalidated == 1) // don't fill the invalidated packet. Should we copy this field as well.
        return;

#ifdef SANITY_CHECK
    if (cache_type == IS_ITLB) {
        if (packet->data == 0)
            assert(0);
    }

    if (cache_type == IS_DTLB) {
        if (packet->data == 0)
            assert(0);
    }

    if (cache_type == IS_STLB) {
        if (packet->data == 0)
            assert(0);
    }
#endif

//		if(cache_type == IS_LLC)
	//		cout << "filled into cache: "<<(int)cache_type << ", cpu: "<< packet->cpu <<" evicted block of cpu: " <<block[set][way].cpu<< endl;

    	if (block[set][way].prefetch && (block[set][way].used == 0))
      	  pf_useless++;
    
		if (block[set][way].valid == 0)
        block[set][way].valid = 1;
    block[set][way].dirty = 0;
    block[set][way].prefetch = (packet->type == PREFETCH) ? 1 : 0;
    block[set][way].used = 0;

    	if (block[set][way].prefetch)
      	  pf_fill++;
	
    block[set][way].delta = packet->delta;
    block[set][way].depth = packet->depth;
    block[set][way].signature = packet->signature;
    block[set][way].confidence = packet->confidence;

    block[set][way].tag = packet->address;
    block[set][way].address = packet->address;
    block[set][way].full_addr = packet->full_addr;
    block[set][way].data = packet->data;
    block[set][way].cpu = packet->cpu;
    block[set][way].instr_id = packet->instr_id;
    block[set][way].ip = packet->ip;
    if (packet->instruction) 
        block[set][way].instruction = 1;
    else
        block[set][way].instruction = 0;

		if(DETAILED_PRINT || (packet->instr_id >= range_start && packet->instr_id <= range_end))
			cout << "filled into cache: "<<(int)cache_type << ", for cpu: "<< packet->cpu <<" current_core_cycle[0]: " <<current_core_cycle[0] << "  packet->instr_id: "<<packet->instr_id<<" packet->ip: " <<packet->ip<< endl;
    
    //if(packet->instr_id == 7454 /*&& cache_type == IS_LLC*/)
    //cout <<"stall_cycle of cpu: "<<stall_cycle[packet->cpu]<<" 7454 filled into cache"<<(int)cache_type<<" , at cycle: "<< current_core_cycle[cpu]<<" event_cycle: "<<packet->event_cycle<<endl;

    DP ( if (warmup_complete[packet->cpu]) {
    cout << "[" << NAME << "] " << __func__ << " set: " << set << " way: " << way;
    cout << " lru: " << block[set][way].lru << " tag: " << hex << block[set][way].tag << " full_addr: " << block[set][way].full_addr;
    cout << " data: " << block[set][way].data << dec << endl; });
}

uint32_t CACHE::check_hit(PACKET *packet)
{		
    uint32_t set = get_set(packet->address);
    uint32_t match_way = UINT32_MAX;
		int flag = 0;
		if (cache_type == IS_LLC)
        {
          // Victim selection for MIRAGE
          #if MIRAGE
              uint32_t set_per_partition[NUM_SKEW];
              for(uint32_t partition = 0; partition < NUM_SKEW; partition++)
              {
                  set_per_partition[partition] = extractSet(packet->address, partition);
                  
              }
							for(uint32_t part = 0; part < NUM_SKEW; part++)
      				{
        					for(uint32_t way = part*NUM_WAY/NUM_SKEW; way < (NUM_WAY/NUM_SKEW)*(part+1); way++)
        					{
            					if(block[set_per_partition[part]][way].valid && block[set_per_partition[part]][way].tag == packet->address)
                				{	
														match_way = way;
														flag = 1;
														break;
												}
											
        					}
									if(flag == 1) // Exit partition loop
											break;
    					}
					#else
							if (NUM_SET < set)
            	{   
              	  cerr << "[" << NAME << "_ERROR] " << __func__ << " invalid set index: " << set << " NUM_SET: " << NUM_SET;
               		cerr << " address: " << hex << packet->address << " full_addr: " << packet->full_addr << dec;
                	cerr << " event: " << packet->event_cycle << endl;
                	assert(0);
            	}
            // hit
            for (uint32_t way=0; way<NUM_WAY; way++)
            {   
                if (block[set][way].valid && (block[set][way].tag == packet->address))
                {   
                    match_way = way;
                    DP ( if (warmup_complete[packet->cpu]) {
                    cout << "[" << NAME << "] " << __func__ << " instr_id: " << packet->instr_id << " type: " << +packet->type << hex << " addr: " << packet->address;
                    cout << " full_addr: " << packet->full_addr << " tag: " << block[set][way].tag << " data: " << block[set][way].data << dec;
                    cout << " set: " << set << " way: " << way << " lru: " << block[set][way].lru;
                    cout << " event: " << packet->event_cycle << " cycle: " << current_core_cycle[cpu] << endl; });
                break;
            		}
        		}
					#endif
       }
	 else
			{
    				if (NUM_SET < set) 
						{
        				cerr << "[" << NAME << "_ERROR] " << __func__ << " invalid set index: " << set << " NUM_SET: " << NUM_SET;
        				cerr << " address: " << hex << packet->address << " full_addr: " << packet->full_addr << dec;
        				cerr << " event: " << packet->event_cycle << endl;
        				assert(0);
    				}

    				// hit
    				for (uint32_t way=0; way<NUM_WAY; way++) 
						{
        				if (block[set][way].valid && (block[set][way].tag == packet->address)) 
								{
            				match_way = way;

            				DP ( if (warmup_complete[packet->cpu]) {
            				cout << "[" << NAME << "] " << __func__ << " instr_id: " << packet->instr_id << " type: " << +packet->type << hex << " addr: " << packet->address;
            				cout << " full_addr: " << packet->full_addr << " tag: " << block[set][way].tag << " data: " << block[set][way].data << dec;
            				cout << " set: " << set << " way: " << way << " lru: " << block[set][way].lru;
                		cout << " event: " << packet->event_cycle << " cycle: " << current_core_cycle[cpu] << endl; });
                		break;
            		}
        		}
    	}

    if(DETAILED_PRINT || (packet->instr_id >= range_start && packet->instr_id <= range_end))
     {
       if(match_way == UINT32_MAX)
           cout << "it is a miss ";
       else
           cout << "it is a hit ";
      cout << "Inside check_hit, cache: "<<(int)cache_type << ", for cpu: "<< packet->cpu <<" full_addr: "<<packet->full_addr<<" set: "<<set<<" way: "<<match_way<<" current_core_cycle[0]: " <<current_core_cycle[0] << "  packet->instr_id: "<<packet->instr_id<<" packet->ip: " <<packet->ip<< endl;    
     }


    if(cache_type == IS_LLC && /*FIRST_TIME_CACHE_SETUP_DONE &&*/ match_way == UINT32_MAX && (CHANNEL_BW_ERR_PRINT || ((LR_ERR_CORR_BOTH_ALGO || LR_NO_ERR_CORR) && CACHE_FILL_DONE)))
    {
        //Stat for SPP.
        LLC_MISS_COUNT[packet->cpu] ++; 

        if(packet->cpu == 0 && FIRST_TIME_CACHE_SETUP_DONE)
        {
            receiver_observed_miss = 1;
            if(rdtsc_timer_is_on == 1)
                inside_rdtsc = 1;
        }

        cout <<"found a miss for CPU: "<< packet->cpu <<" packet->address: "<<packet->address<< " cycle: " << current_core_cycle[packet->cpu]<<" receiver_checks_for_a_miss "<<receiver_checks_for_a_miss<<" instr_id: "<<packet->instr_id <<endl; 
    }
    //if(cache_type == IS_LLC && current_core_cycle[packet->cpu] > FIRST_TIME_CACHE_SETUP_CYCLES/*1427985 2285913 3921255*/ && receiver_checks_for_a_miss && match_way == UINT32_MAX)
    //{  //bit_revealed_is = 1; 
    //   cout<<"in cache_hit bit revealed is: 1"<< " miss occurred for CPU: "<<packet->cpu<<endl; }
    //if( packet->instr_id == 7454)
    // cout << "7454 cache_type: "<<(int)cache_type<< " match_way: "<<match_way<<" current_core_cycle: "<<current_core_cycle[packet->cpu]<<endl;
		return match_way;
}

int CACHE::invalidate_entry(uint64_t inval_addr)   //TODO invalidate from data LLC as well.
{
    uint32_t set = get_set(inval_addr);
    int match_way = -1;

    if (NUM_SET < set) {
        cerr << "[" << NAME << "_ERROR] " << __func__ << " invalid set index: " << set << " NUM_SET: " << NUM_SET;
        cerr << " inval_addr: " << hex << inval_addr << dec << endl;
        assert(0);
    }

    // invalidate
    for (uint32_t way=0; way<NUM_WAY; way++) {
        if (block[set][way].valid && (block[set][way].tag == inval_addr)) {

            block[set][way].valid = 0;

            match_way = way;

            DP ( if (warmup_complete[cpu]) {
            cout << "[" << NAME << "] " << __func__ << " inval_addr: " << hex << inval_addr;  
            cout << " tag: " << block[set][way].tag << " data: " << block[set][way].data << dec;
            cout << " set: " << set << " way: " << way << " lru: " << block[set][way].lru << " cycle: " << current_core_cycle[cpu] << endl; });

            break;
        }
    }

    return match_way;
}

int CACHE::add_rq(PACKET *packet)
{
    // check for the latest wirtebacks in the write queue
    int wq_index = WQ.check_queue(packet);
    if (wq_index != -1) {
        
        // check fill level
        if (packet->fill_level < fill_level) {

            packet->data = WQ.entry[wq_index].data;
            if (packet->instruction) 
                upper_level_icache[packet->cpu]->return_data(packet);
            else // data
                upper_level_dcache[packet->cpu]->return_data(packet);
        }

#ifdef SANITY_CHECK
        if (cache_type == IS_ITLB)
            assert(0);
        else if (cache_type == IS_DTLB)
            assert(0);
        else if (cache_type == IS_L1I)
            assert(0);
#endif
        // update processed packets
        if ((cache_type == IS_L1D) && (packet->type != PREFETCH)) {
            if (PROCESSED.occupancy < PROCESSED.SIZE)
                PROCESSED.add_queue(packet);

            DP ( if (warmup_complete[packet->cpu]) {
            cout << "[" << NAME << "_RQ] " << __func__ << " instr_id: " << packet->instr_id << " found recent writebacks";
            cout << hex << " read: " << packet->address << " writeback: " << WQ.entry[wq_index].address << dec;
            cout << " index: " << MAX_READ << " rob_signal: " << packet->rob_signal << endl; });
        }

        HIT[packet->type]++;
        ACCESS[packet->type]++;

        WQ.FORWARD++; //What are these ?
        RQ.ACCESS++; // Do we need upper and this variable for MSHR in check_mshr_for_hit()?

        return -1;
    }


    // New changes: Don't add to rq, if a request to the same block has returned in MSHR(not yet filled into cache).
    int mshr_index = check_mshr_for_hits(packet);
    if(mshr_index != -1)
    {
       // update prefetcher on load instruction
       if (packet->type == LOAD) 
        {
           if (cache_type == IS_L1D)
                  l1d_prefetcher_operate(packet->full_addr, packet->ip, 1, packet->type);
           else if (cache_type == IS_L2C)
                  l2c_prefetcher_operate(packet->full_addr, packet->ip, 1, packet->type);
        }

       // Copy the details of packet to MSHR entry
        return -1;  
    }

    // check for duplicates in the read queue
    int index = RQ.check_queue(packet);
    if (index != -1) {
        
        if (packet->instruction) {
            uint32_t rob_index = packet->rob_index;
            RQ.entry[index].rob_index_depend_on_me.insert (rob_index);
            RQ.entry[index].instr_merged = 1;

            DP (if (warmup_complete[packet->cpu]) {
            cout << "[INSTR_MERGED] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << RQ.entry[index].instr_id;
            cout << " merged rob_index: " << rob_index << " instr_id: " << packet->instr_id << endl; });
        }
        else 
        {
            // mark merged consumer
            if (packet->type == RFO) {

                uint32_t sq_index = packet->sq_index;
                RQ.entry[index].sq_index_depend_on_me.insert (sq_index);
                RQ.entry[index].store_merged = 1;
            }
            else {
                uint32_t lq_index = packet->lq_index; 
                RQ.entry[index].lq_index_depend_on_me.insert (lq_index);
                RQ.entry[index].load_merged = 1;

                DP (if (warmup_complete[packet->cpu]) {
                cout << "[DATA_MERGED] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << RQ.entry[index].instr_id;
                cout << " merged rob_index: " << packet->rob_index << " instr_id: " << packet->instr_id << " lq_index: " << packet->lq_index << endl; });
            }
        }

        RQ.MERGED++;
        RQ.ACCESS++;

        return index; // merged index
    }

    // check occupancy
    if (RQ.occupancy == RQ_SIZE) {
        RQ.FULL++;

        return -2; // cannot handle this request
    }

    // if there is no duplicate, add it to RQ
    index = RQ.tail;

#ifdef SANITY_CHECK
    if (RQ.entry[index].address != 0) {
        cerr << "[" << NAME << "_ERROR] " << __func__ << " is not empty index: " << index;
        cerr << " address: " << hex << RQ.entry[index].address;
        cerr << " full_addr: " << RQ.entry[index].full_addr << dec << endl;
        assert(0);
    }
#endif

    RQ.entry[index] = *packet;

    // ADD LATENCY
    if (RQ.entry[index].event_cycle < current_core_cycle[packet->cpu])
        RQ.entry[index].event_cycle = current_core_cycle[packet->cpu] + LATENCY;
    else
        RQ.entry[index].event_cycle += LATENCY;

    RQ.occupancy++;
    RQ.tail++;
    if (RQ.tail >= RQ.SIZE)
        RQ.tail = 0;

    DP ( if (warmup_complete[RQ.entry[index].cpu]) {
    cout << "[" << NAME << "_RQ] " <<  __func__ << " instr_id: " << RQ.entry[index].instr_id << " address: " << hex << RQ.entry[index].address;
    cout << " full_addr: " << RQ.entry[index].full_addr << dec;
    cout << " type: " << +RQ.entry[index].type << " head: " << RQ.head << " tail: " << RQ.tail << " occupancy: " << RQ.occupancy;
    cout << " event: " << RQ.entry[index].event_cycle << " current: " << current_core_cycle[RQ.entry[index].cpu] << endl; });

    if (packet->address == 0)
        assert(0);

    RQ.TO_CACHE++;
    RQ.ACCESS++;

    //if(DETAILED_PRINT || (packet->instr_id >= range_start && packet->instr_id <= range_end))
    //    cout << "Added to rq: "<<(int)cache_type << ", for cpu: "<< packet->cpu <<" current_core_cycle[0]: " <<current_core_cycle[0] << "  packet->instr_id: "<<packet->instr_id<<" packet->ip: " <<packet->ip<<" rq occupancy: " <<RQ.occupancy << endl;

    return -1;
}

int CACHE::add_wq(PACKET *packet)
{
    // check for duplicates in the write queue
    int index = WQ.check_queue(packet);
    if (index != -1) {

        WQ.MERGED++;
        WQ.ACCESS++;

        return index; // merged index
    }

    // sanity check
    if (WQ.occupancy >= WQ.SIZE)
        assert(0);

    // if there is no duplicate, add it to the write queue
    index = WQ.tail;
    if (WQ.entry[index].address != 0) {
        cerr << "[" << NAME << "_ERROR] " << __func__ << " is not empty index: " << index;
        cerr << " address: " << hex << WQ.entry[index].address;
        cerr << " full_addr: " << WQ.entry[index].full_addr << dec << endl;
        assert(0);
    }

    WQ.entry[index] = *packet;

    // ADD LATENCY
    if (WQ.entry[index].event_cycle < current_core_cycle[packet->cpu])
        WQ.entry[index].event_cycle = current_core_cycle[packet->cpu] + LATENCY;
    else
        WQ.entry[index].event_cycle += LATENCY;

    WQ.occupancy++;
    WQ.tail++;
    if (WQ.tail >= WQ.SIZE)
        WQ.tail = 0;

    DP (if (warmup_complete[WQ.entry[index].cpu]) {
    cout << "[" << NAME << "_WQ] " <<  __func__ << " instr_id: " << WQ.entry[index].instr_id << " address: " << hex << WQ.entry[index].address;
    cout << " full_addr: " << WQ.entry[index].full_addr << dec;
    cout << " head: " << WQ.head << " tail: " << WQ.tail << " occupancy: " << WQ.occupancy;
    cout << " data: " << hex << WQ.entry[index].data << dec;
    cout << " event: " << WQ.entry[index].event_cycle << " current: " << current_core_cycle[WQ.entry[index].cpu] << endl; });

    WQ.TO_CACHE++;
    WQ.ACCESS++;

    return -1;
}

int CACHE::prefetch_line(uint64_t ip, uint64_t base_addr, uint64_t pf_addr, int fill_level)
{
    pf_requested++;

    if (PQ.occupancy < PQ.SIZE) {
        if ((base_addr>>LOG2_PAGE_SIZE) == (pf_addr>>LOG2_PAGE_SIZE)) {
            
            PACKET pf_packet;
            pf_packet.fill_level = fill_level;
            pf_packet.cpu = cpu;
            //pf_packet.data_index = LQ.entry[lq_index].data_index;
            //pf_packet.lq_index = lq_index;
            pf_packet.address = pf_addr >> LOG2_BLOCK_SIZE;
            pf_packet.full_addr = pf_addr;
            //pf_packet.instr_id = LQ.entry[lq_index].instr_id;
            //pf_packet.rob_index = LQ.entry[lq_index].rob_index;
            pf_packet.ip = ip;
            pf_packet.type = PREFETCH;
            pf_packet.event_cycle = current_core_cycle[cpu];

            // give a dummy 0 as the IP of a prefetch
            add_pq(&pf_packet);

            pf_issued++;

            return 1;
        }
    }

    return 0;
}

int CACHE::kpc_prefetch_line(uint64_t base_addr, uint64_t pf_addr, int fill_level, int delta, int depth, int signature, int confidence)
{
    if (PQ.occupancy < PQ.SIZE) {
        if ((base_addr>>LOG2_PAGE_SIZE) == (pf_addr>>LOG2_PAGE_SIZE)) {
            
            PACKET pf_packet;
            pf_packet.fill_level = fill_level;
            pf_packet.cpu = cpu;
            //pf_packet.data_index = LQ.entry[lq_index].data_index;
            //pf_packet.lq_index = lq_index;
            pf_packet.address = pf_addr >> LOG2_BLOCK_SIZE;
            pf_packet.full_addr = pf_addr;
            //pf_packet.instr_id = LQ.entry[lq_index].instr_id;
            //pf_packet.rob_index = LQ.entry[lq_index].rob_index;
            pf_packet.ip = 0;
            pf_packet.type = PREFETCH;
            pf_packet.delta = delta;
            pf_packet.depth = depth;
            pf_packet.signature = signature;
            pf_packet.confidence = confidence;
            pf_packet.event_cycle = current_core_cycle[cpu];

            // give a dummy 0 as the IP of a prefetch
            add_pq(&pf_packet);

            pf_issued++;

            return 1;
        }
    }

    return 0;
}

int CACHE::add_pq(PACKET *packet)
{
    // check for the latest wirtebacks in the write queue
    int wq_index = WQ.check_queue(packet);
    if (wq_index != -1) {
        
        // check fill level
        if (packet->fill_level < fill_level) {

            packet->data = WQ.entry[wq_index].data;
            if (packet->instruction) 
                upper_level_icache[packet->cpu]->return_data(packet);
            else // data
                upper_level_dcache[packet->cpu]->return_data(packet);
        }

        HIT[packet->type]++;
        ACCESS[packet->type]++;

        WQ.FORWARD++;
        PQ.ACCESS++;

        return -1;
    }

    // check for duplicates in the PQ
    int index = PQ.check_queue(packet);
    if (index != -1) {
        if (packet->fill_level < PQ.entry[index].fill_level)
            PQ.entry[index].fill_level = packet->fill_level;

        PQ.MERGED++;
        PQ.ACCESS++;

        return index; // merged index
    }

    // check occupancy
    if (PQ.occupancy == PQ_SIZE) {
        PQ.FULL++;

        DP ( if (warmup_complete[packet->cpu]) {
        cout << "[" << NAME << "] cannot process add_pq since it is full" << endl; });
        return -2; // cannot handle this request
    }

    // if there is no duplicate, add it to PQ
    index = PQ.tail;

#ifdef SANITY_CHECK
    if (PQ.entry[index].address != 0) {
        cerr << "[" << NAME << "_ERROR] " << __func__ << " is not empty index: " << index;
        cerr << " address: " << hex << PQ.entry[index].address;
        cerr << " full_addr: " << PQ.entry[index].full_addr << dec << endl;
        assert(0);
    }
#endif

    PQ.entry[index] = *packet;

    // ADD LATENCY
    if (PQ.entry[index].event_cycle < current_core_cycle[packet->cpu])
        PQ.entry[index].event_cycle = current_core_cycle[packet->cpu] + LATENCY;
    else
        PQ.entry[index].event_cycle += LATENCY;

    PQ.occupancy++;
    PQ.tail++;
    if (PQ.tail >= PQ.SIZE)
        PQ.tail = 0;

    DP ( if (warmup_complete[PQ.entry[index].cpu]) {
    cout << "[" << NAME << "_PQ] " <<  __func__ << " instr_id: " << PQ.entry[index].instr_id << " address: " << hex << PQ.entry[index].address;
    cout << " full_addr: " << PQ.entry[index].full_addr << dec;
    cout << " type: " << +PQ.entry[index].type << " head: " << PQ.head << " tail: " << PQ.tail << " occupancy: " << PQ.occupancy;
    cout << " event: " << PQ.entry[index].event_cycle << " current: " << current_core_cycle[PQ.entry[index].cpu] << endl; });

    if (packet->address == 0)
        assert(0);

    PQ.TO_CACHE++;
    PQ.ACCESS++;

    return -1;
}

void CACHE::return_data(PACKET *packet)
{
    // check MSHR information
    int mshr_index = check_mshr(packet);

    // sanity check
    if (mshr_index == -1) {
        cerr << "[" << NAME << "_MSHR] " << __func__ << " instr_id: " << packet->instr_id << " cannot find a matching entry!";
        cerr << " full_addr: " << hex << packet->full_addr;
        cerr << " address: " << packet->address << dec;
        cerr << " event: " << packet->event_cycle << " current: " << current_core_cycle[packet->cpu] << endl;
        assert(0);
    }

    //if(packet->instr_id == 3671259)
    //cout << "returned to cache level: "<<(int)cache_type<< " current_core_cycle: "<<current_core_cycle[packet->cpu]<<endl;

    // MSHR holds the most updated information about this request
    // no need to do memcpy
    MSHR.num_returned++;
    MSHR.entry[mshr_index].returned = COMPLETED;
    MSHR.entry[mshr_index].data = packet->data;

    if(packet->to_be_invalidated == 1) 
    {   
        MSHR.entry[mshr_index].to_be_invalidated = packet->to_be_invalidated;
    }

    // ADD LATENCY
    if (MSHR.entry[mshr_index].event_cycle < current_core_cycle[packet->cpu])
        MSHR.entry[mshr_index].event_cycle = current_core_cycle[packet->cpu] + LATENCY;
    else
        MSHR.entry[mshr_index].event_cycle += LATENCY;

    update_fill_cycle();

    DP (if (warmup_complete[packet->cpu]) {
    cout << "[" << NAME << "_MSHR] " <<  __func__ << " instr_id: " << MSHR.entry[mshr_index].instr_id;
    cout << " address: " << hex << MSHR.entry[mshr_index].address << " full_addr: " << MSHR.entry[mshr_index].full_addr;
    cout << " data: " << MSHR.entry[mshr_index].data << dec << " num_returned: " << MSHR.num_returned;
    cout << " index: " << mshr_index << " occupancy: " << MSHR.occupancy;
    cout << " event: " << MSHR.entry[mshr_index].event_cycle << " current: " << current_core_cycle[packet->cpu] << " next: " << MSHR.next_fill_cycle << endl; });
}

void CACHE::update_fill_cycle()
{
    // update next_fill_cycle
    uint64_t min_cycle = UINT64_MAX;
    uint32_t min_index = MSHR.SIZE;
    for (uint32_t i=0; i<MSHR.SIZE; i++) {
        if ((MSHR.entry[i].returned == COMPLETED) && (MSHR.entry[i].event_cycle < min_cycle)) {
            min_cycle = MSHR.entry[i].event_cycle;
            min_index = i;
        }

        DP (if (warmup_complete[MSHR.entry[i].cpu]) {
        cout << "[" << NAME << "_MSHR] " <<  __func__ << " checking instr_id: " << MSHR.entry[i].instr_id;
        cout << " address: " << hex << MSHR.entry[i].address << " full_addr: " << MSHR.entry[i].full_addr;
        cout << " data: " << MSHR.entry[i].data << dec << " returned: " << +MSHR.entry[i].returned << " fill_level: " << MSHR.entry[i].fill_level;
        cout << " index: " << i << " occupancy: " << MSHR.occupancy;
        cout << " event: " << MSHR.entry[i].event_cycle << " current: " << current_core_cycle[MSHR.entry[i].cpu] << " next: " << MSHR.next_fill_cycle << endl; });
    }
    
    MSHR.next_fill_cycle = min_cycle;
    MSHR.next_fill_index = min_index;
    if (min_index < MSHR.SIZE) {

        DP (if (warmup_complete[MSHR.entry[min_index].cpu]) {
        cout << "[" << NAME << "_MSHR] " <<  __func__ << " instr_id: " << MSHR.entry[min_index].instr_id;
        cout << " address: " << hex << MSHR.entry[min_index].address << " full_addr: " << MSHR.entry[min_index].full_addr;
        cout << " data: " << MSHR.entry[min_index].data << dec << " num_returned: " << MSHR.num_returned;
        cout << " event: " << MSHR.entry[min_index].event_cycle << " current: " << current_core_cycle[MSHR.entry[min_index].cpu] << " next: " << MSHR.next_fill_cycle << endl; });
    }
}

// New changes
int CACHE::check_mshr_for_hits( PACKET *packet)
{
    for(uint32_t index=0; index<MSHR_SIZE; index++) 
    {
       if(MSHR.entry[index].address == packet->address /*&& MSHR.entry[index].forwarded_to_next_cache_level == 1*/ && MSHR.entry[index].returned == COMPLETED)
       { 
           //Don't merge, instead forward to next level as per the fill level or send to the process queue
           // check fill level
        if (packet->fill_level < fill_level) {

            packet->data = MSHR.entry[index].data;
            if (packet->instruction)
                upper_level_icache[packet->cpu]->return_data(packet);
            else // data
                upper_level_dcache[packet->cpu]->return_data(packet);
        }
                    // update processed packets
            if (cache_type == IS_ITLB) {

                if(MSHR.entry[index].data == 0) assert(0); //Sanity check

                packet->instruction_pa = MSHR.entry[index].data; // Transferring physical address.
                if (PROCESSED.occupancy < PROCESSED.SIZE)
                    PROCESSED.add_queue(packet);
            }
            else if (cache_type == IS_DTLB) {

                if(MSHR.entry[index].data == 0) assert(0); //Sanity check

                packet->data_pa = MSHR.entry[index].data; // Transferring physical address.
                if (PROCESSED.occupancy < PROCESSED.SIZE)
                    PROCESSED.add_queue(packet);
            }
            else if ((cache_type == IS_L1I) && (packet->type != PREFETCH)) {
                if (PROCESSED.occupancy < PROCESSED.SIZE){
                    PROCESSED.add_queue(packet); }
            }                                            // This should be the packet type instead.
            else if ((cache_type == IS_L1D) && (packet->type != PREFETCH)) { // Prefetcher requests are not forwarded to core, unless it is demanded.
                if (PROCESSED.occupancy < PROCESSED.SIZE)
                    {
                      PROCESSED.add_queue(packet);
                    }
       }
       if (MSHR.entry[index].type == PREFETCH) 
       {
          pf_useful++; //Q. is it counted even if a prefetch request serves for another prefetch request ?
          if(packet->type != PREFETCH)   
              MSHR.entry[index].type = packet->type; // This ensures, type of a prefetch request is changed before filling it into cache.
          int mshr_entry_fill_level = MSHR.entry[index].fill_level;
          uint8_t  prior_returned = MSHR.entry[index].returned, forwarding_to_next_level = MSHR.entry[index].forwarded_to_next_cache_level;
          uint64_t prior_event_cycle = MSHR.entry[index].event_cycle;
          MSHR.entry[index] = RQ.entry[index];

          // in case request is already returned, we should keep event_cycle and retunred variables
          MSHR.entry[index].fill_level = mshr_entry_fill_level;
          MSHR.entry[index].returned = prior_returned;
          MSHR.entry[index].forwarded_to_next_cache_level = forwarding_to_next_level;
          MSHR.entry[index].event_cycle = prior_event_cycle;
       }
       // COLLECT STATS
       sim_hit[packet->cpu][packet->type]++;
       sim_access[packet->cpu][packet->type]++;

       HIT[packet->type]++;
       ACCESS[packet->type]++;
       return index;
    }
  }
  return -1;
}

int CACHE::check_mshr(PACKET *packet)
{
    // search mshr
    for (uint32_t index=0; index<MSHR_SIZE; index++) {
        if (MSHR.entry[index].address == packet->address && (int)MSHR.entry[index].forwarded_to_next_cache_level == 0) {
            
            DP ( if (warmup_complete[packet->cpu]) {
            cout << "[" << NAME << "_MSHR] " << __func__ << " same entry instr_id: " << packet->instr_id << " prior_id: " << MSHR.entry[index].instr_id;
            cout << " address: " << hex << packet->address;
            cout << " full_addr: " << packet->full_addr << dec << endl; });

            return index;
        }
    }

    DP ( if (warmup_complete[packet->cpu]) {
    cout << "[" << NAME << "_MSHR] " << __func__ << " new address: " << hex << packet->address;
    cout << " full_addr: " << packet->full_addr << dec << endl; });

    DP ( if (warmup_complete[packet->cpu] && (MSHR.occupancy == MSHR_SIZE)) { 
    cout << "[" << NAME << "_MSHR] " << __func__ << " mshr is full";
    cout << " instr_id: " << packet->instr_id << " mshr occupancy: " << MSHR.occupancy;
    cout << " address: " << hex << packet->address;
    cout << " full_addr: " << packet->full_addr << dec;
    cout << " cycle: " << current_core_cycle[packet->cpu] << endl; });

    return -1;
}

void CACHE::add_mshr(PACKET *packet)
{
    uint32_t index = 0;

    // search mshr
    for (index=0; index<MSHR_SIZE; index++) {
        if (MSHR.entry[index].address == 0) {
            
            MSHR.entry[index] = *packet;
            MSHR.entry[index].returned = INFLIGHT;
            MSHR.occupancy++;

            DP ( if (warmup_complete[packet->cpu]) {
            cout << "[" << NAME << "_MSHR] " << __func__ << " instr_id: " << packet->instr_id;
            cout << " address: " << hex << packet->address << " full_addr: " << packet->full_addr << dec;
            cout << " index: " << index << " occupancy: " << MSHR.occupancy << endl; });

            break;
        }
    }
}

uint32_t CACHE::get_occupancy(uint8_t queue_type, uint64_t address)
{
    if (queue_type == 0)
        return MSHR.occupancy;
    else if (queue_type == 1)
        return RQ.occupancy;
    else if (queue_type == 2)
        return WQ.occupancy;
    else if (queue_type == 3)
        return PQ.occupancy;

    return 0;
}

uint32_t CACHE::get_size(uint8_t queue_type, uint64_t address)
{
    if (queue_type == 0)
        return MSHR.SIZE;
    else if (queue_type == 1)
        return RQ.SIZE;
    else if (queue_type == 2)
        return WQ.SIZE;
    else if (queue_type == 3)
        return PQ.SIZE;

    return 0;
}

void CACHE::increment_WQ_FULL(uint64_t address)
{
    WQ.FULL++;
}

// Verification check
/*void CACHE::check_inclusive(){

	for(int i=0; i<NUM_CPUS; i++)
  {
	    int counter = 0;
	    int counter1 = 0;
	//L1I data should be present L2C
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
				#ifdef INCLUSIVE_PRINT
					if(match == -1)
					{
						counter++;
						cout<<"L1I address not in L2C:"<<hex<<ooo_cpu[i].L1I.block[l1iset][l1iway].address<<dec<<endl;
					}
				#endif

					assert(match==1);
					match = -1;
	//L1I data should be in LLC
					int llcset = uncore.LLC.get_set(ooo_cpu[i].L1I.block[l1iset][l1iway].address);
						for(int llcway=0;llcway< LLC_WAY;llcway++)
							if(ooo_cpu[i].L1I.block[l1iset][l1iway].tag == uncore.LLC.block[llcset][llcway].tag &&
									uncore.LLC.block[llcset][llcway].valid == 1){
								match = 1 ;
							}
				#ifdef INCLUSIVE_PRINT
					if(match == -1)
					{
						counter1++;
						cout<<"L1I address not in LLC:"<<hex<<ooo_cpu[i].L1I.block[l1iset][l1iway].address<<dec<<endl;
					}
				#endif

					assert(match==1);
				}
		if(counter)
			cout<<"l1I data not in l2c: "<<counter<<endl;
		if(counter1)
			cout<<"l1I data not in llc: "<<counter1<<endl;
		counter=0;
		counter1=0;
		//L1D data should be present L2C
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
				#ifdef INCLUSIVE_PRINT
						if(match == -1)
						{
							counter++;
							if(ooo_cpu[i].L1D.block[l1dset][l1dway].dirty)
								cout<<"L1D dirty block not in L2C"<<endl;
							cout<<"L1D address not in L2C:"<<hex<<ooo_cpu[i].L1D.block[l1dset][l1dway].address<<dec<<endl;
						}
				#endif

						assert(match==1);
						match = -1;
		//L1D data should be present in LLC
						int llcset = uncore.LLC.get_set(ooo_cpu[i].L1D.block[l1dset][l1dway].address);
							for(int llcway=0;llcway< LLC_WAY;llcway++)
								if(ooo_cpu[i].L1D.block[l1dset][l1dway].tag == uncore.LLC.block[llcset][llcway].tag &&
										uncore.LLC.block[llcset][llcway].valid == 1){
									match = 1 ;
								}

				#ifdef INCLUSIVE_PRINT
						if(match == -1)
						{
							counter1++;
							if(ooo_cpu[i].L1D.block[l1dset][l1dway].dirty)
                                                                cout<<"L1D dirty block not in LLC"<<endl;
							cout<<"L1D address not in LLC:"<<hex<<ooo_cpu[i].L1D.block[l1dset][l1dway].address<<dec<<endl;
						}
				#endif
						assert(match==1);
					}

			if(counter)
				cout<<"l1D data not in l2c: "<<counter<<endl;
			if(counter1)
				cout<<"l1D data not in llc: "<<counter1<<endl;

			counter=0;
			counter1=0;

			//L2C data should be present in LLC
			for(int l2cset=0;l2cset<L2C_SET;l2cset++)
				for(int l2cway=0;l2cway<L2C_WAY;l2cway++)
					if(ooo_cpu[i].L2C.block[l2cset][l2cway].valid == 1){
						int match = -1;
						int llcset = uncore.LLC.get_set(ooo_cpu[i].L2C.block[l2cset][l2cway].address);
							for(int llcway=0;llcway< LLC_WAY;llcway++)
								if(ooo_cpu[i].L2C.block[l2cset][l2cway].tag == uncore.LLC.block[llcset][llcway].tag &&
										uncore.LLC.block[llcset][llcway].valid == 1){
									match = 1 ;
								}
				#ifdef INCLUSIVE_PRINT
						if(match == -1)
						{
							counter++;
							if(ooo_cpu[i].L2C.block[l2cset][l2cway].dirty)
								counter1++;
							cout<<"L2C address not in LLC:"<<hex<<ooo_cpu[i].L2C.block[l2cset][l2cway].address<<dec<<endl;
						}
				#endif
						assert(match==1);
					}

			if(counter)
				cout<<"l2C data not in llc: "<<counter<<endl;
			if(counter && counter1)
				cout<<"l2c dirty block not in llc:"<<counter1<<endl;
				} 
} */
