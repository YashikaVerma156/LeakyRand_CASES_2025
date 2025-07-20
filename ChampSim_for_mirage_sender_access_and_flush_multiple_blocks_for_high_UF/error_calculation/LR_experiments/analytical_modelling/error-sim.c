#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
   int cache_block_count, occupied_count, num_sender_blocks, bit_string_length, num_trials, i, k, j, x, bit, error_count = 0, hole_count = 0, index;

   if (argc != 6) {
      printf("Usage: ./error-sim cache_block_count occupied_count num_sender_blocks bit_string_length num_trials\nAborting...\n");
      exit(0);
   }

   // Capture the parameters
   cache_block_count = atoi(argv[1]);  // Number of cache blocks
   occupied_count = atoi(argv[2]);     // Number of occupied cache blocks
   num_sender_blocks = atoi(argv[3]);  // Number of sender blocks
   bit_string_length = atoi(argv[4]);  // Length of error correction trigger interval (e.g., 127, 63, 31, 15, 7)
   num_trials = atoi(argv[5]);         // Number of error correction trigger intervals to experiment with

   // Sanity-check parameters
   assert(cache_block_count > 0);
   assert((occupied_count >= 0) && (occupied_count <= cache_block_count));
   assert(num_sender_blocks > 0);
   assert(bit_string_length > 0);
   assert(num_trials > 0);
   
   // Start the experiments
   for (k=0; k<num_trials; k++) {
      // For each trial
      hole_count = 0;        // No hole in unoccupied region to start with
      for (i=0; i<bit_string_length; i++) {
         // For each bit to transmit
         bit = random() % 2;
         if (bit) {  // If bit is 0, there is nothing to do
            // If bit is 1, need to simulate hole creation and error counting
            x = num_sender_blocks-hole_count; // Number of residual evictions
            for (j=0; j<x; j++) { // Need to simulate only the residual number of evictions
               index = random() % cache_block_count;         // Generate an array index
               if (index >= occupied_count) hole_count++;    // If it is in unoccupied region, increase hole count
            }
	    assert(hole_count <= num_sender_blocks);
            if (hole_count == num_sender_blocks) error_count++;  // If there are already enough holes, this is an error
         }
      }
   }
   printf("Error count: %d, error rate per bit: %f\n", error_count, ((float)error_count)/((float)num_trials*bit_string_length));
   return 0;
}
