#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include <stdint.h>
#include <assert.h>
#include <sys/mman.h>
#include <string.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Machine specific parameters
#define CACHE_BLOCK_SIZE 64
#define PAGE_SIZE 4096

// Configurable parameters
#define MAX_NUM_BLOCKS (1 << 14)
#define NUM_TOTAL_ACCESS (1 << 26)

const int SIZE = sizeof(int);

// Block structure for a cache line
typedef struct{
        int a[16];
} Block;


int main(int argc, char* argv[]){

	// set CPU affinity and scheduling priority
    // Pin process to CPU 5
    cpu_set_t set;
	struct sched_param sp;

	int sys = system("echo performance | sudo tee /sys/devices/system/cpu/cpu5/cpufreq/scaling_governor");
	CPU_ZERO(&set);
	CPU_SET(5, &set);

	if (sched_setaffinity(0, sizeof(set), &set) == -1)
		perror("sched_setaffinity");

	sp.sched_priority = 50;
	if(sched_setscheduler(0, SCHED_FIFO, &sp) < 0)	
        perror("sched_setrt");

	// Allocate large number of blocks to access elements from
	Block *A = (Block*)malloc(sizeof(Block)*MAX_NUM_BLOCKS);
	assert(A);

	// internal variables
	uint64_t hit_time, miss_time;
	int i, j, k;
	int *x, y, z=0;
    char buffer[25];    
	uint64_t start, end;
	k=0;

	// open a file to write the data
    int fd = open("data.csv", O_CREAT|O_APPEND|O_RDWR,0666);
    if(fd < 0) perror("data.csv open failed");

	// Touch all blocks in A
	for(int i=0;i<64;i++) for (j=0; j<MAX_NUM_BLOCKS; j+=(PAGE_SIZE/CACHE_BLOCK_SIZE)) A[i+j].a[0] = 0; // Warm up and initialize
	_mm_mfence();

	/*-------------------------------------- Hit Timings ---------------------------------------*/
	while (k <= NUM_TOTAL_ACCESS) {

		// Time-Access all blocks in A sequentially
		// append the time to the file as <time>,0
		for(int i=0; i<64; i++){
			for (j=0; j<MAX_NUM_BLOCKS; j+=(PAGE_SIZE/CACHE_BLOCK_SIZE)) {
				x = (int*)((char*)A + (i+j)*CACHE_BLOCK_SIZE);
				asm volatile("mfence":::"memory");
				asm volatile(
					"rdtsc\n\t"               
					"shl $32, %%rdx\n\t"      
					"or %%rdx, %%rax\n\t"     
					"mov %%rax, %0"           
					: "=r" (start)            
					:                         
					: "%rax", "%rdx"     
				);
				asm volatile("mfence":::"memory");
				y = *x;
				asm volatile("mfence":::"memory");
				asm volatile(
					"rdtsc\n\t"               
					"shl $32, %%rdx\n\t"      
					"or %%rdx, %%rax\n\t"     
					"mov %%rax, %0"           
					: "=r" (end)            
					:                         
					: "%rax", "%rdx"     
				);
				asm volatile("mfence":::"memory");
				hit_time = (end - start);
				asm volatile("mfence":::"memory");
				k++;
				z += y;
				int len = snprintf(buffer, sizeof(buffer), "%ld,0\n", hit_time);
				if(write(fd, buffer, len) < 0){
					perror("Error writing to data4.csv");
					close(fd);
					return 1;
				}
			}
		}
	}


	/*-------------------------------------- Miss Timings ---------------------------------------*/
	k = 0;
	_mm_mfence();
	while (k <= NUM_TOTAL_ACCESS) {

		// Flush a block in A and then time-access it to get the miss time
		// append miss time to the file as <time>,1
		for(int i=0; i<64; i++){
			for (j=0; j<MAX_NUM_BLOCKS; j+=(PAGE_SIZE/CACHE_BLOCK_SIZE)) {
				x = (int*)((char*)A + (i+j)*CACHE_BLOCK_SIZE);
				asm volatile(
					"clflush (%0)"
					:
					: "r" (x)
					: "memory"
				);
				asm volatile("mfence":::"memory");
				asm volatile(
					"rdtsc\n\t"
					"shl $32, %%rdx\n\t"
					"or %%rdx, %%rax\n\t"
					"mov %%rax, %0"
					: "=r" (start)
					:
					: "%rax", "%rdx"
				);
				asm volatile("mfence":::"memory");
				y = *x;
				asm volatile("mfence":::"memory");
				asm volatile(
					"rdtsc\n\t"
					"shl $32, %%rdx\n\t"
					"or %%rdx, %%rax\n\t"
					"mov %%rax, %0"
					: "=r" (end)
					:
					: "%rax", "%rdx"
				);
				asm volatile("mfence":::"memory");
				miss_time = end - start;
				asm volatile("mfence":::"memory");
				k++;
				z += y;
				int len = snprintf(buffer, sizeof(buffer), "%ld,1\n", miss_time);
				if(write(fd, buffer, len) < 0){
					perror("Error writing to data4.csv");
					close(fd);
					return 1;
				}
			}		
		}
	}
		
	printf("%d\n", z);
    return 0;
}
