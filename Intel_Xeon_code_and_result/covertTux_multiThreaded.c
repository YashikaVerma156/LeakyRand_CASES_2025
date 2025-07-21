#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <x86intrin.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sched.h>
#include <limits.h>

// Machine specific parameters
#define CACHE_CAPACITY_MB_LEAST_POWER_OF_TWO 16
#define CACHE_BLOCK_SIZE 64
#define PAGE_SIZE 4096
#define THRESHOLD 200
#define OUTLIER 10000
#define ASSOC 16
#define CLOCK_FREQ 3.4e9


// Parameters for the experiment
#define TEST_SUIT_LEN 500
#define TRAIN_SUIT_LEN 250
#define STRLEN 512
#define DS 4        // disturbance set size
#define UNROLLING_FACTOR 1
#define OCC_SET_SIZE1 ASSOC
#define OCC_SET_SIZE2 (int)(ceil(ASSOC * 1.1))
#define OCC_SET_SIZE (int)(ceil(ASSOC * 1.2))
#define CRFILL_ITERS1 1
#define CRFILL_ITERS2 2
#define CRFILL_ITERS3 6

// Other parameters
#define MAX_NUM_BLOCKS ((CACHE_CAPACITY_MB_LEAST_POWER_OF_TWO << 21) / CACHE_BLOCK_SIZE)
#define MAX_NUM_BLOCKS_B MAX_NUM_BLOCKS
#define MIN_NUM_TESTS 50
#define MAX_NUM_TESTS 1000
#define TESTING_PROB 0.8
#define ISOLATED_TESTING_PROB 0.9
#define SUCCESS_PROB 0.99
#define EXPLORATION_BATCH_SIZE 256
#define STRING0 0x0
#define STRING1 0xffffffff
#define CHECK_STRLEN 1000000
#define VALIDATION_ERROR_THRESHOLD 6e-4
#define MAX_TRIALS_PER_RUN 500
#define MAX_RUNS 10
#define NUM_DATA_POINTS 1000000


// Block structure for a cache line
typedef struct{
    int a[16];
} Block;

// Function to flush cache line
inline void flush(void *ptr){
    asm volatile("mfence" ::: "memory");
    asm volatile(
        "clflush (%0)"
        :
        : "r"(ptr)
        : "memory");
    asm volatile("mfence" ::: "memory");
}

// Macro to read time stamp counter
#define RDTSCFENCE(t)                        \
    do{                                      \
        asm volatile("mfence" ::: "memory"); \
        asm volatile(                        \
            "rdtsc\n\t"                      \
            "shl $32, %%rdx\n\t"             \
            "or %%rdx, %%rax\n\t"            \
            "mov %%rax, %0"                  \
            : "=r"(t)                        \
            :                                \
            : "%rax", "%rdx", "memory");     \
        asm volatile("mfence" ::: "memory"); \
    } while (0)

// Macro to check the number of trials
#define CHECK_NUM_TRIAL \
do { \
    if (trial > MAX_TRIALS_PER_RUN) { \
        if (execv(argv[0], argv)) { \
            perror("Exec failed"); \
        } \
    } \
} while (0)

// Global variables
int x = 0, trial = 1, run = 1;
Block *A, *B;
int *index_arrayA, *index_arrayB, *array;
int **sender_array, **receiver_array;
volatile int flag = 0; // flag = 0 -> recv working, sender idle; flag = 1 -> sndr working, recv idle 
volatile int validate = 1;
int occ = 0, num_nz;

unsigned int idx1=0, idx2=NUM_DATA_POINTS-1;
unsigned long glo_min=LONG_MAX, glo_max = 0;

unsigned long thresh_data[NUM_DATA_POINTS];
unsigned long threshold = THRESHOLD;
double avg_hit_latency = 0;
double std_dev_hit_latency = 0;
double avg_miss_latency = 0;
double std_dev_miss_latency = 0;

char train_data_set[TRAIN_SUIT_LEN][STRLEN+1];
char test_data_set[TEST_SUIT_LEN][STRLEN+1];

/* 
  Test function to check if a given set of cache blocks form an eviction set 
  
  # num_tests => number of test iterations 
  # nB => number of elements to be taken from index_arrayB

  -> ASSOC+1 elements are always taken from index_arrayA
  -> nB = 0 when receiver is finding eviction set
  -> nb > 0 when recevier is finding occupancy set
*/
int test(int nB, int num_tests){
    int i, y, miss = 0, t, *ptr, j;
    // Reset and flush all the elements in the set to be tested 
    if ((nB == 0) && (num_tests == MAX_NUM_TESTS)){
        for (i = 0; i < MAX_NUM_BLOCKS; i++){
            A[i].a[0] = 0;
            flush(&(A[i].a[0]));
        }
    }
    else{
        for (i = 0; i < ASSOC + 1; i++){
            flush(&(A[index_arrayA[i]].a[0]));
        }
    }
    for (i = 0; i < nB; i++){
        flush(&(B[index_arrayB[i]].a[0]));
    }

    // access each block multiple times and count misses per cache block, and total misses 
    for (t = 0; t < num_tests; t++){
        for (i = 0; ((i < num_nz) && (array[i] >= ISOLATED_TESTING_PROB * MAX_NUM_TESTS) && (nB == 0) && (num_tests == MAX_NUM_TESTS)) || (i < ASSOC + 1); i++){
            register uint64_t start, end;
            ptr = &(A[index_arrayA[i]].a[0]);
            RDTSCFENCE(start);
            y = *ptr;
            RDTSCFENCE(end);
            if (((end - start) > THRESHOLD) && ((end - start) <= OUTLIER)){
                miss++;
                if ((nB == 0) && (num_tests == MAX_NUM_TESTS))
                    (*ptr)++; // store miss count for a particular block in the block itself
            }
            x += y;
        }
        for (i = 0; i < nB; i++){
            register uint64_t start, end;
            ptr = &(B[index_arrayB[i]].a[0]);
            RDTSCFENCE(start);
            y = *ptr;
            RDTSCFENCE(end);
            if (((end - start) > THRESHOLD) && ((end - start) <= OUTLIER)){
                miss++;
            }
            x += y;
        }
    }
    // sufficient average misses check
    if ((miss >= num_tests * (ASSOC + 1) * SUCCESS_PROB) && (nB == 0) && (num_tests == MAX_NUM_TESTS)){
        // if nB = 0 => eviction set finding 
        num_nz = 0;

        // store indices in index_arrayA and miss count in array for all blocks having one or more misses
        for (i = 0; i < MAX_NUM_BLOCKS; i++){
            if (A[i].a[0]){
                index_arrayA[num_nz] = i;
                array[num_nz] = A[i].a[0];
                num_nz++;
            }
        }

        // Sort in descending order of miss counts
        for (i = 0; i < num_nz; i++)
            for (j = 0; j < num_nz - 1; j++){
                if (array[j] < array[j + 1]){
                    int x1 = array[j];
                    array[j] = array[j + 1];
                    array[j + 1] = x1;
                    x1 = index_arrayA[j];
                    index_arrayA[j] = index_arrayA[j + 1];
                    index_arrayA[j + 1] = x1;
                }
            }
        
        // check if atleast first ASSOC+1 elements showed high miss rate
        for (i = 0; i < ASSOC + 1; i++){
            if (array[i] < ISOLATED_TESTING_PROB * t)
                return 0;
        }
        miss = 0;
        for (i = 0; i < ASSOC + 1; i++){
            flush(&(A[index_arrayA[i]].a[0]));
        }
        // Test again to confirm with higher probability threshold
        for (t = 0; t < num_tests; t++){
            for (i = 0; i < ASSOC + 1; i++){
                register uint64_t start, end;
                ptr = &(A[index_arrayA[i]].a[0]);
                RDTSCFENCE(start);
                y = *ptr;
                RDTSCFENCE(end);
                if (((end - start) > THRESHOLD) && ((end - start) <= OUTLIER)){
                    miss++;
                }
                x += y;
            }
        }
        if (miss >= num_tests * (ASSOC + 1) * SUCCESS_PROB)
            return 1; // test passed
    }
    else if (miss >= num_tests * (ASSOC + 1 + nB) * SUCCESS_PROB)
        // if nB > 0 => occupancy set finding 
        return 1; // test passed
    
    return 0; // test failed
}

/*
  Final Test function to determine if the elements of the receiver_array and sender_array belong to the same cache set
*/
int test_final(void){
    int i, y, miss = 0, t, *ptr;
    assert((OCC_SET_SIZE + DS) > ASSOC);
    for (i = 0; i < ASSOC; i++) flush((void *)(receiver_array[i]));
    for (i = 0; i < DS; i++) flush((void *)(sender_array[i]));
    for (t = 0; t < MAX_NUM_TESTS; t++){
        for (i = 0; i < ASSOC; i++){
            register uint64_t start, end;
            ptr = receiver_array[i];
            RDTSCFENCE(start);
            y = *ptr;
            RDTSCFENCE(end);
            if (((end - start) > THRESHOLD) && ((end - start) <= OUTLIER)){
                miss++;
            }
            x += y;
        }
        for (i = 0; i < DS; i++){
            register uint64_t start, end;
            ptr = sender_array[i];
            RDTSCFENCE(start);
            y = *ptr;
            RDTSCFENCE(end);
            if (((end - start) > THRESHOLD) && ((end - start) <= OUTLIER)){
                miss++;
            }
            x += y;
        }
    }
    if (miss >= MAX_NUM_TESTS * (ASSOC + DS) * SUCCESS_PROB)
        return miss;
    
    return 0;
}

/*
  Function executed by the sender thread
*/
void *sender(){
    // set CPU affinity and scheduling priority for the sender
    // Pin sender to CPU 4
    cpu_set_t set;
    struct sched_param sp;

    x = system("echo performance | sudo tee /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor");
    CPU_ZERO(&set);
    CPU_SET(4, &set);

    if (sched_setaffinity(0, sizeof(set), &set) == -1)
        perror("sched_setaffinity");

    sp.sched_priority = 50;
    if(sched_setscheduler(0, SCHED_FIFO, &sp) < 0)
        perror("sched_setrt");
    
    int i, j, k, *ptr, y;

    /*---------------------------------Validation using Train Suite ------------------------------------*/
    validation:
    // Let receiver work
    asm volatile("movl $0, %0;"
        : "=m"(flag)
        :
        : "memory");
    for(i=0; i<TRAIN_SUIT_LEN; i++){
        for(j=0; j<STRLEN; j++){

            // Wait for receiver to perform CRFill + CRProbe (only in first iteration) and to infer the bit sent in the previous iteration
            while(flag==0);

            // for each bit, communicate using the disturbance set
            if(train_data_set[i][j]=='1'){
                // access the disturbance set elements
                for (k = 0; k < DS; k++){
                    ptr = sender_array[k];
                    asm volatile("mfence" ::: "memory");
                    y = *ptr;
                    asm volatile("mfence" ::: "memory");
                    x += y;
                }
                // flush the disturbance set elements
                for (k = 0; k < DS; k++){
                    flush(sender_array[k]);
                }
            }
            // Tell the receiver to infer the bit sent
            asm volatile("movl $0, %0;"
                : "=m"(flag)
                :
                : "memory");
        }
    }
    while(flag == 0); // Wait to receiver to infer the last bit sent and to send the validation result

    // If validation failed, go back to start
    if(validate == 1) goto validation;

    // Tell the receiver that message is read
    asm volatile("movl $0, %0;"
                : "=m"(flag)
                :
                : "memory");

    
    /*-----------------------------------Unrolling Factor Threshold----------------------------------------*/
    
    // use a string of alternating 0s and 1s to find the threshold
    for(i=0; i<NUM_DATA_POINTS; i++){
        // Synchronise with the receiver (only in the first iteration) and wait for receiver to infer the bit sent in the previous iteration 
        while(flag == 0);

        if(i&1){
            // access DS elements from the disturbance set
            for (k = 0; k < DS; k++){
                ptr = sender_array[k];
                asm volatile("mfence" ::: "memory");
                y = *ptr;
                asm volatile("mfence" ::: "memory");
                x += y;
            }

            // flush DS elements from the disturbance set
            for (k = 0; k < DS; k++){
                flush(sender_array[k]);
            }
        }
        // Tell the receiver to infer the bit 
        asm volatile("movl $0, %0;"
                    : "=m"(flag)
                    :
                    : "memory");
    }


    for(i=0; i<TEST_SUIT_LEN; i++){
        for(j=0; j<STRLEN; j++){
            // Wait for receiver 
            // - to find the threshold (only in the first iteration of the outer loop)
            // - to perform CRFill + CRProbe (only in the first iteration of the inner loop for each iteration of the outerloop)
            // - to infer the bit sent in the previous iteration
            while(flag == 0);

            // for each bit, communicate using the disturbance set
            if(test_data_set[i][j]=='1'){
                // access DS elements from the disturbance set
                for (k = 0; k < DS; k++){
                    ptr = sender_array[k];
                    asm volatile("mfence" ::: "memory");
                    y = *ptr;
                    asm volatile("mfence" ::: "memory");
                    x += y;
                }
                // flush DS elements from the disturbance set
                for (k = 0; k < DS; k++){
                    flush(sender_array[k]);
                }
            }

            // Tell the receiver to infer the bit sent
            asm volatile("movl $0, %0;"
                        : "=m"(flag)
                        :
                        : "memory");
        }
    }
}


/*
  Main function

  -> Creates a sender thread
  -> Acts as receiver thread itself
*/
int main(int argc, char *argv[]){
    pthread_t sender_thread;
    // Create a thread for the sender
    if (pthread_create(&sender_thread, NULL, sender, NULL)){
        perror("Failed to create thread");
        return 1;
    }

    // Set CPU affinity and scheduling priority for the receiver
    // Pin receiver to CPU 5
    cpu_set_t set;
    struct sched_param sp;

    x = system("echo performance | sudo tee /sys/devices/system/cpu/cpu5/cpufreq/scaling_governor");
    CPU_ZERO(&set);
    CPU_SET(5, &set);

    if (sched_setaffinity(0, sizeof(set), &set) == -1)
        perror("sched_setaffinity");

    sp.sched_priority = 50;
    if(sched_setscheduler(0, SCHED_FIFO, &sp) < 0)
        perror("sched_setrt");

    
    // Initialize variables
    register uint64_t cr_start, cr_end, comm_start0, comm_end0, comm_start1, comm_end1;
	setvbuf(stdout, NULL, _IONBF, 0);
    int count, i, j, k, miss, t, num_tests, jstart, istart;
    register uint64_t start, end;
    int *ptr, y;
    int *ptri[UNROLLING_FACTOR];
    int yi[UNROLLING_FACTOR];
    unsigned long maxi = 0;

    char recv_str[TEST_SUIT_LEN][STRLEN+1];

    // Indices for the eviction set blocks
    index_arrayA = (int *)malloc(sizeof(int) * MAX_NUM_BLOCKS);
    
    // Indices for the occupancy set blocks
    index_arrayB = (int *)malloc(sizeof(int) * (OCC_SET_SIZE));
    
    // Array to store the number of misses for each cache block
    array = (int *)malloc(sizeof(int) * MAX_NUM_BLOCKS);

    // Array to store the addresses of the cache blocks in the disturbance set
    sender_array = (int **)malloc(sizeof(int *) * DS);
    assert(sender_array);
    
    // Array to store the addresses of the cache blocks in the occupancy set
    receiver_array = (int **)malloc(sizeof(int *) * OCC_SET_SIZE);
    assert(receiver_array);

    // Allocate large number of blocks to find eviction set elements from
    assert(!posix_memalign((void *)&A, PAGE_SIZE, MAX_NUM_BLOCKS * sizeof(Block)));
    assert(A);

    // Allocate large number of blocks to extend eviction set elements and to create disturbance set
    assert(!posix_memalign((void *)&B, PAGE_SIZE, MAX_NUM_BLOCKS_B * sizeof(Block)));
    assert(B);

    // Touch then flush all blocks in B
    for (i = 0; i < MAX_NUM_BLOCKS_B; i++) B[i].a[0] = 0;
    for (i = 0; i < MAX_NUM_BLOCKS_B; i++) flush(&(B[i].a[0]));

    // Read the training and testing data sets from files
    FILE* train_fp = fopen("benchmark_train.txt", "r");
    if(train_fp == NULL){
        perror("Error opening file train file");
        exit(EXIT_FAILURE);
    }   
    x = fread(train_data_set, sizeof(char), TRAIN_SUIT_LEN * (STRLEN+1), train_fp);

    FILE* test_fp = fopen("benchmark_test.txt", "r");
    if(test_fp == NULL){
        perror("Error opening file test file");
        exit(EXIT_FAILURE);
    }   
    x = fread(test_data_set, sizeof(char), TEST_SUIT_LEN * (STRLEN+1), test_fp);

    /* ------------------------------Eviction Set Finding --------------------------------------- */
    evSet_finding:
    while (run <= MAX_RUNS){
        // printf("Run %d: Finding new eviction set...\n", run);
        trial = 0; occ= 0; 
        while (trial <= MAX_TRIALS_PER_RUN){
            count = ASSOC + 1;
            jstart = random() % ((MAX_NUM_BLOCKS * CACHE_BLOCK_SIZE) / PAGE_SIZE);
            istart = random() % (PAGE_SIZE / CACHE_BLOCK_SIZE);

            while (count <= MAX_NUM_BLOCKS){

                // Touch (reset) then flush all blocks in A
                for (i = 0; i < MAX_NUM_BLOCKS; i++)
                    A[i].a[0] = 0;
                for (i = 0; i < MAX_NUM_BLOCKS; i++)
                    flush(&(A[i].a[0]));
                i = istart;
                j = jstart;
                k = 0;

                // access count blocks in A from istart,jstart
                while (k < count){
                    x += A[j * (PAGE_SIZE / CACHE_BLOCK_SIZE) + i].a[0];
                    k++;
                    j = (j + 1) & ((MAX_NUM_BLOCKS * CACHE_BLOCK_SIZE) / PAGE_SIZE - 1);
                    if (j == jstart){
                        i = (i + 1) & (PAGE_SIZE / CACHE_BLOCK_SIZE - 1);
                    }
                }

                // access each block multiple times and count misses per cache block, and total misses 
                miss = 0;
                num_tests = MIN_NUM_TESTS;
                for (t = 0; t < num_tests; t++){
                    i = istart;
                    j = jstart;
                    k = 0;
                    while (k < count){
                        ptr = &(A[j * (PAGE_SIZE / CACHE_BLOCK_SIZE) + i].a[0]);
                        RDTSCFENCE(start);
                        y = *ptr;
                        RDTSCFENCE(end);
                        if (((end - start) > THRESHOLD) && ((end - start) <= OUTLIER)){
                            miss++;
                            (*ptr)++; // store miss count for a particular block in the block itself
                        }
                        x += y;
                        k++;
                        j = (j + 1) & ((MAX_NUM_BLOCKS * CACHE_BLOCK_SIZE) / PAGE_SIZE - 1);
                        if (j == jstart){
                            i = (i + 1) & (PAGE_SIZE / CACHE_BLOCK_SIZE - 1);
                        }
                    }
                    if (miss > TESTING_PROB * MIN_NUM_TESTS) num_tests = MAX_NUM_TESTS;
                }

                // sufficient average misses check
                if (miss >= (ASSOC + 1) * t){
                    num_nz = 0;
                    // store indices in index_arrayA and miss count in array for all blocks having one or more misses
                    for (i = 0; i < MAX_NUM_BLOCKS; i++){
                        if (A[i].a[0]){
                            index_arrayA[num_nz] = i;
                            array[num_nz] = A[i].a[0];
                            num_nz++;
                        }
                    }

                    // Sort in descending order of miss counts
                    for (i = 0; i < num_nz; i++){
                        for (j = 0; j < num_nz - 1; j++){
                            if (array[j] < array[j + 1]){
                                int x1 = array[j];
                                array[j] = array[j + 1];
                                array[j + 1] = x1;
                                x1 = index_arrayA[j];
                                index_arrayA[j] = index_arrayA[j + 1];
                                index_arrayA[j + 1] = x1;
                            }
                        }
                    }

                    // check if atleast first ASSOC+1 elements showed high miss rate
                    for (i = 0; i < ASSOC + 1; i++){
                        if (array[i] < ISOLATED_TESTING_PROB * t)
                            break;
                    }
                    if (i == (ASSOC + 1)){
                        // if found ASSOC+1 elements above, then test them twice
                        if (test(0, MIN_NUM_TESTS)){
                            if (test(0, MAX_NUM_TESTS)) break; // eviction set found
                            else{
                                 // second test failed, start a new trial
                                jstart = random() % ((MAX_NUM_BLOCKS * CACHE_BLOCK_SIZE) / PAGE_SIZE);
                                istart = random() % (PAGE_SIZE / CACHE_BLOCK_SIZE);
                                trial++;
                                CHECK_NUM_TRIAL;
                                count = 0;
                            }
                        }
                        else{
                            // first test failed, start a new trial
                            jstart = random() % ((MAX_NUM_BLOCKS * CACHE_BLOCK_SIZE) / PAGE_SIZE);
                            istart = random() % (PAGE_SIZE / CACHE_BLOCK_SIZE);
                            trial++;
                            CHECK_NUM_TRIAL;
                            count = 0;
                        }
                    }
                }
                count += EXPLORATION_BATCH_SIZE; // increase the number of blocks to find eviction set
            }


            /* ----------------------------- Debugging prints ----------------------------- */
            // printf("\nEviction set (array index: number of LLC misses out of %d accesses):\n", MAX_NUM_TESTS);
            // for (i = 0; i < 1 + ASSOC; i++){
            //     printf("%d: %d, ", index_arrayA[i], array[i]);
            // }
            // if (num_nz > (ASSOC + 1)){
            //     printf("\n\n");
            //     printf("Remaining elements (array index: number of LLC misses out of %d accesses):\n", MAX_NUM_TESTS);
            //     for (i = ASSOC + 1; i < num_nz; i++){
            //         printf("%d: %d, ", index_arrayA[i], array[i]);
            //     }
            // }
            // printf("\n\nFinding additional eviction set elements:\n");

            
            // Extend the eviction set by choosing elements from B and testing if they belong to the same cache set
            i = 0;
            j = 0;
            index_arrayB[j] = i;
            while (i < MAX_NUM_BLOCKS_B){
                if (test(j + 1, MIN_NUM_TESTS)){
                    if (test(j + 1, MAX_NUM_TESTS))
                        break;
                }
                i++;
                index_arrayB[j] = i;
            }
            if (i < MAX_NUM_BLOCKS_B){
                count = 1;
                j++;
                i += (PAGE_SIZE / CACHE_BLOCK_SIZE);
                if (i >= MAX_NUM_BLOCKS_B) i = ((1 + i) % MAX_NUM_BLOCKS_B) % (PAGE_SIZE / CACHE_BLOCK_SIZE);
                while ((j != OCC_SET_SIZE) && (count < MAX_NUM_BLOCKS_B)){
                    index_arrayB[j] = i;
                    if (test(j + 1, MIN_NUM_TESTS)){
                        if (test(j + 1, MAX_NUM_TESTS)){
                            j++;
                            fflush(stdout);
                        }
                    }
                    i += (PAGE_SIZE / CACHE_BLOCK_SIZE);
                    if (i >= MAX_NUM_BLOCKS_B) i = ((1 + i) % MAX_NUM_BLOCKS_B) % (PAGE_SIZE / CACHE_BLOCK_SIZE);
                    count++;
                }
                if (j == OCC_SET_SIZE) break; // required number of elements found
                else{ // try again
                    trial++;
                    CHECK_NUM_TRIAL;
                }
            }
            else { trial++; CHECK_NUM_TRIAL; }
        }

        CHECK_NUM_TRIAL;
        // fill in the receiver array with the addresses of the blocks in the occupancy set
        for (i = 0; i < OCC_SET_SIZE; i++){
            receiver_array[i] = &(B[index_arrayB[i]].a[0]);
        }
        // fill in the sender array with the addresses of the blocks in the disturbance set
        for (i = 0; i < DS; i++){
            sender_array[i] = &(A[index_arrayA[i]].a[0]);
        }
        for (i = 0; i < OCC_SET_SIZE; i++) flush(receiver_array[i]);
        for (i = 0; i < DS; i++) flush(sender_array[i]);
        assert(OCC_SET_SIZE >= ASSOC);

        /* ----------------------------- Debugging prints ----------------------------- */
        // printf("\nEviction set (array A index) [starts at %p]:\n", A);
        // for (i = 0; i < 1 + ASSOC; i++){
        //     printf("%d, ", index_arrayA[i]);
        // }
        // printf("\nEviction set (array B index) [starts at %p]:\n", B);
        // for (i = 0; i < OCC_SET_SIZE; i++){
        //     printf("%d, ", index_arrayB[i]);
        // }
        // printf("\n\n");
        // printf("Occupancy set addresses (from array B):\n");
        // for (i = 0; i < OCC_SET_SIZE; i++) printf("%p, ", receiver_array[i]);
        // printf("\n");
        // printf("Disturbance set addresses (from array A):\n");
        // for (i = 0; i < DS; i++) printf("%p, ", sender_array[i]);
        // printf("\n\n");

        int miss_final;
        assert(miss_final = test_final()); // Test finally to validate if eviction set and disturbance set conflict


        // printf("Conflict probability between occupancy set and disturbance set: %f\n\n", ((float)miss_final) / (MAX_NUM_TESTS * (ASSOC + DS)));


        // CRFill (1x)^n
        for (j = 0; j < CRFILL_ITERS1; j++){
            for (i = 0; i < OCC_SET_SIZE1; i++){
                ptr = receiver_array[i];
                asm volatile("mfence" ::: "memory");
                y = *ptr;
                asm volatile("mfence" ::: "memory");
                x += y;
            }
        }
        for (j = 0; j < CRFILL_ITERS2; j++){
            for (i = 0; i < OCC_SET_SIZE2; i++){
                ptr = receiver_array[i];
                asm volatile("mfence" ::: "memory");
                y = *ptr;
                asm volatile("mfence" ::: "memory");
                x += y;
            }
        }
        for (j = 0; j < CRFILL_ITERS3; j++){
            for (i = 0; i < OCC_SET_SIZE; i++){
                ptr = receiver_array[i];
                asm volatile("mfence" ::: "memory");
                y = *ptr;
                asm volatile("mfence" ::: "memory");
                x += y;
            }
        }

        // CRProbe
        for (i = OCC_SET_SIZE - 1; i >= 0; i--){
            ptr = receiver_array[i];
            RDTSCFENCE(start);
            y = *ptr;
            RDTSCFENCE(end);
            x += y;
            if ((end - start) <= THRESHOLD){
                occ++;
            }
            flush(ptr);
        }
        
        for (i = 0; i < occ; i++){
            ptr = receiver_array[i];
            asm volatile("mfence" ::: "memory");
            y = *ptr;
            asm volatile("mfence" ::: "memory");
            x += y;
        }
        
        /* ------------------------------ Validation 1 -------------------------------------- */
        // receiver itself checks for a string of all 0s.
        // expects all hits
        int err=0;
        for (i = 0; i < CHECK_STRLEN; i++){
            if(i%(CHECK_STRLEN/10) == 0) sleep(1);
            int miss = 0;
            for (k = 0; k < occ; k++){
                ptr = receiver_array[k];
                RDTSCFENCE(start);
                y = *ptr;
                RDTSCFENCE(end);
                if (end - start > THRESHOLD){
                    miss++;
                }
                x += y;
            }
            if(miss > 0) err++;
        }


        double fraction_err = ((double)err/(double)CHECK_STRLEN);
        // printf("Number of errors in validation 1 = %d\n", err);
        printf("Percentage of errors in validation 1 = %lf %%\n", fraction_err * 100);

        if (fraction_err >= VALIDATION_ERROR_THRESHOLD) {
            // printf("Validation 1 failed\n");
            run++;
            goto evSet_finding; // start a new run
        }

        // Validation 1 passed 


        /*---------------------------------Validation using Train Suite ------------------------------------*/
        int total_error = 0;
        unsigned char inferred_bit;
        int zero = 0, one = 0;
        for(i=0; i<TRAIN_SUIT_LEN; i++){
            err=0;
	        occ = 0;
            // CRFill to occupy the cache set
            for (j = 0; j < CRFILL_ITERS1; j++){
                for (k = 0; k < OCC_SET_SIZE1; k++){
                    ptr = receiver_array[k];
                    asm volatile("mfence" ::: "memory");
                    y = *ptr;
                    asm volatile("mfence" ::: "memory");
                    x += y;
                }
            }
            for (j = 0; j < CRFILL_ITERS2; j++){
                for (k = 0; k < OCC_SET_SIZE2; k++){
                    ptr = receiver_array[k];
                    asm volatile("mfence" ::: "memory");
                    y = *ptr;
                    asm volatile("mfence" ::: "memory");
                    x += y;
                }
            }
            for (j = 0; j < CRFILL_ITERS3; j++){
                for (k = 0; k < OCC_SET_SIZE; k++){
                    ptr = receiver_array[k];
                    asm volatile("mfence" ::: "memory");
                    y = *ptr;
                    asm volatile("mfence" ::: "memory");
                    x += y;
                }
            }
    
            // CRProbe
            for (j = OCC_SET_SIZE - 1; j >= 0; j--){
                ptr = receiver_array[j];
                RDTSCFENCE(start);
                y = *ptr;
                RDTSCFENCE(end);
                x += y;
                if ((end - start) <= THRESHOLD){
                    occ++;
                }
                flush(ptr);
            }
            
            for (j = 0; j < occ; j++){
                ptr = receiver_array[j];
                asm volatile("mfence" ::: "memory");
                y = *ptr;
                asm volatile("mfence" ::: "memory");
                x += y;
            }
            
            // Let the sender work
            asm volatile("movl $1, %0;"
                : "=m"(flag)
                :
                : "memory");
            for(j=0; j<STRLEN; j++){
                miss=0;
                while(flag == 1); // Wait for sender to send the bit
                for(k=0; k<occ; k++){
                    ptr = receiver_array[k];
                    RDTSCFENCE(start);
                    y = *ptr;
                    RDTSCFENCE(end);
                    if (end - start > THRESHOLD){
                        miss++;
                    }
                    x += y;
                }
                if(miss>0) inferred_bit = '1';
                else inferred_bit = '0';
                if(inferred_bit != train_data_set[i][j]){
                    err++;
                }
                if(j==STRLEN-1) break;
                // Tell the sender that bit is received and inferred
                asm volatile("movl $1, %0;"
                            : "=m"(flag)
                            :
                            : "memory");
            }
            total_error += err;
        }

        // Criteria to determine if the isolated cache set is quiet
        int fail = (total_error > VALIDATION_ERROR_THRESHOLD * TRAIN_SUIT_LEN * STRLEN) ? 1 : 0;
        fraction_err = ((double)total_error/(double)(TRAIN_SUIT_LEN * STRLEN));
        printf("Train BER = %lf %%\n", fraction_err * 100);

        if (fail) { 
            // Tell the sender that validation failed
            asm volatile("movl $1, %0;"
                        : "=m"(validate)
                        :
                        : "memory");    
        }
         else{
            // Tell the sender that validation passed
            asm volatile("movl $2, %0;"
                        : "=m"(validate)
                        :
                        : "memory");
        }
        // Let the sender check the validation result
        asm volatile("movl $1, %0;"
                    : "=m"(flag)
                    :
                    : "memory");
        while(flag == 1); // Wait for sender to check the validation result

        // Reset validate
        asm volatile("movl $0, %0;"
                    : "=m"(validate)
                    :
                    : "memory");
        if (fail) {
            // Start a new run
            run++;
            goto evSet_finding;
        }
        else break;
    }

    // if all runs failed, exit
    if(run > MAX_RUNS) {
        exit(0);
    }
    
    // Synchronise with the sender
    asm volatile("movl $1, %0;"
            : "=m"(flag)
            :
            : "memory");
    
    // expect a string of alternating 0s and 1s to find the threshold
    for (i = 0; i < NUM_DATA_POINTS; i++){
        maxi=0;
        // Wait for sender to send the bit
        while (flag == 1);

        // Time access the occupancy set and count misses in bunches of size uf
        for (k = 0; k < occ; k+=UNROLLING_FACTOR){
            for (int t=0; t<UNROLLING_FACTOR; t++){
                ptri[t] = receiver_array[k+t];
            }
            RDTSCFENCE(start);
            #if UNROLLING_FACTOR==1
                yi[0] = *ptri[0];
            #elif UNROLLING_FACTOR==2
                yi[0] = *ptri[0];
                yi[1] = *ptri[1];
            #elif UNROLLING_FACTOR==4
                yi[0] = *ptri[0];
                yi[1] = *ptri[1];
                yi[2] = *ptri[2];
                yi[3] = *ptri[3];
            #elif UNROLLING_FACTOR==8
                yi[0] = *ptri[0];
                yi[1] = *ptri[1];
                yi[2] = *ptri[2];
                yi[3] = *ptri[3];
                yi[4] = *ptri[4];
                yi[5] = *ptri[5];
                yi[6] = *ptri[6];
                yi[7] = *ptri[7];
            #endif
            RDTSCFENCE(end);
            for (int t=0; t<UNROLLING_FACTOR; t++){
                x += yi[t];
            }

            // find the maximum time taken across all accesses
            if(end-start>maxi) maxi=end-start;
        }

        // if expected a bit 1, the maximum time is miss time
        if(i&1) thresh_data[idx2--]=maxi;
        // if expected a bit 0, the maximum time is hit time
        else thresh_data[idx1++]=maxi;

        // If last bit received, do not start the sender. Receiver continues working
        if(i==NUM_DATA_POINTS-1) break;
        // If bits are remaining to be received, tell the sender to send the next bit
        asm volatile("movl $1, %0;"
                    : "=m"(flag)
                    :
                    : "memory");
    }

    /*---------------------Calibrating the threshold----------------------*/

    // find global min and max time
    unsigned long temp;
    for(int i=0; i<NUM_DATA_POINTS; i++){
        temp = thresh_data[i];
        if(temp < glo_min) glo_min = temp;
        if(temp > glo_max) glo_max = temp;
    }
    // printf("min latency = %ld, max latency = %ld\n", glo_min, glo_max);
    for(i=0; i<NUM_DATA_POINTS/2; i++){
        temp = thresh_data[i];
        avg_hit_latency += temp;
        std_dev_hit_latency += temp*temp;
    }
    avg_hit_latency /= (NUM_DATA_POINTS/2);
    std_dev_hit_latency /= (NUM_DATA_POINTS/2);
    std_dev_hit_latency -= avg_hit_latency*avg_hit_latency;
    std_dev_hit_latency = sqrt(std_dev_hit_latency);
    
    for(i=NUM_DATA_POINTS/2; i<NUM_DATA_POINTS; i++){
        temp = thresh_data[i];
        avg_miss_latency += temp;
        std_dev_miss_latency += temp*temp;
    }
    avg_miss_latency /= (NUM_DATA_POINTS/2);
    std_dev_miss_latency /= (NUM_DATA_POINTS/2);
    std_dev_miss_latency -= avg_miss_latency*avg_miss_latency;
    std_dev_miss_latency = sqrt(std_dev_miss_latency);

    printf("Hit  : Avg = %0.12lf, Std = %0.12lf\n", avg_hit_latency, std_dev_hit_latency);
    printf("Miss : Avg = %0.12lf, Std = %0.12lf\n", avg_miss_latency, std_dev_miss_latency);
    
    // sweep through the time values to find the threshold giving minimum error
    long err;
    long min_err = 2e9;
    unsigned long start_th = glo_min;
    count = 0;
    for(unsigned long th = glo_min; th < glo_max; th++){
        err = 0;
        for(i=0; i<NUM_DATA_POINTS/2; i++){
            if(thresh_data[i] > th) err++;
        }
        for(i=NUM_DATA_POINTS/2; i<NUM_DATA_POINTS; i++){
            if(thresh_data[i] <= th) err++;
        }
        if(err < min_err){
            min_err = err;
            start_th = th;
            count = 1;
        }
        else if(err == min_err){
            count++;
        }
        else if(err > min_err*10) break;
    }

    // take the median of the time values giving minimum error
    threshold = (2*start_th + count -1)/2;
    
    // printf("start_th = %lu, end_th = %lu, min_err = %ld\n", start_th, start_th + count-1, min_err);
    printf("threshold = %lu\n", threshold);
    
    /*---------------------Communication----------------------*/

    // measure total time for communication of all strings
    RDTSCFENCE(comm_start0);
    for(i=0; i<TEST_SUIT_LEN; i++){
        // Perform CRFill+CRProbe
        for (j = 0; j < CRFILL_ITERS1; j++){
            for (k = 0; k < OCC_SET_SIZE1; k++){
                ptr = receiver_array[k];
                asm volatile("mfence" ::: "memory");
                y = *ptr;
                asm volatile("mfence" ::: "memory");
                x += y;
            }
        }
        for (j = 0; j < CRFILL_ITERS2; j++){
            for (k = 0; k < OCC_SET_SIZE2; k++){
                ptr = receiver_array[k];
                asm volatile("mfence" ::: "memory");
                y = *ptr;
                asm volatile("mfence" ::: "memory");
                x += y;
            }
        }
        for (j = 0; j < CRFILL_ITERS3; j++){
            for (k = 0; k < OCC_SET_SIZE; k++){
                ptr = receiver_array[k];
                asm volatile("mfence" ::: "memory");
                y = *ptr;
                asm volatile("mfence" ::: "memory");
                x += y;
            }
        }

        occ = 0;
        for (j = OCC_SET_SIZE - 1; j >= 0; j--){
            ptr = receiver_array[j];
            RDTSCFENCE(start);
            y = *ptr;
            RDTSCFENCE(end);
            x += y;
            if ((end - start) <= THRESHOLD){
                occ++;
            }
            flush(ptr);
        }
        
        for (j = 0; j < occ; j++){
            ptr = receiver_array[j];
            asm volatile("mfence" ::: "memory");
            y = *ptr;
            asm volatile("mfence" ::: "memory");
            x += y;
        }
        
        // Tell sender that CRFill and CRProbe are done
        asm volatile("movl $1, %0;"
                    : "=m"(flag)
                    :
                    : "memory");
        
        for(j=0; j<STRLEN; j++){
            miss=0;

            // Wait for sender to send the bit 
            while(flag == 1);

            // for each bit, infer using the eviction set
            for (k = 0; k < occ; k+=UNROLLING_FACTOR){
                for (int t=0; t<UNROLLING_FACTOR; t++){
                    ptri[t] = receiver_array[k+t];
                }
                RDTSCFENCE(start);
                #if UNROLLING_FACTOR==1
                    yi[0] = *ptri[0];
                #elif UNROLLING_FACTOR==2
                    yi[0] = *ptri[0];
                    yi[1] = *ptri[1];
                #elif UNROLLING_FACTOR==4
                    yi[0] = *ptri[0];
                    yi[1] = *ptri[1];
                    yi[2] = *ptri[2];
                    yi[3] = *ptri[3];
                #elif UNROLLING_FACTOR==8
                    yi[0] = *ptri[0];
                    yi[1] = *ptri[1];
                    yi[2] = *ptri[2];
                    yi[3] = *ptri[3];
                    yi[4] = *ptri[4];
                    yi[5] = *ptri[5];
                    yi[6] = *ptri[6];
                    yi[7] = *ptri[7];
                #endif
                RDTSCFENCE(end);
                // count number of misses for each bit
                if (end - start > threshold){
                    miss++;
                }
                for (int t=0; t<UNROLLING_FACTOR; t++){
                    x += yi[t];
                }
            }

            // infer the bit sent by sender
            if(miss>0) recv_str[i][j] = '1';
            else recv_str[i][j] = '0';
            if(j==STRLEN-1) break;
            asm volatile("movl $1, %0;"
                        : "=m"(flag)
                        :
                        : "memory");
        }
    }
    RDTSCFENCE(comm_end0);

    // Communication completed
    // Wait for sender thread to finish
    pthread_join(sender_thread, NULL);

    // find number of errors
    int num_errors = 0;
    for(i=0; i<TEST_SUIT_LEN; i++){
        for (j = 0; j < STRLEN; j++){
            if (recv_str[i][j] != test_data_set[i][j]){
                num_errors++;
            }
        }
    }

    printf("x=%d\n", x);
    printf("Test BER = %lf %%\n", ((double)num_errors/(double)(TEST_SUIT_LEN*STRLEN)) * 100);

    // find the bandwidth 
    uint64_t comm_cycles0 = comm_end0 - comm_start0;
    double overall_bw_bps0 = (CLOCK_FREQ * STRLEN * TEST_SUIT_LEN) / comm_cycles0;
    printf("Test BW = %.12lf bps\n", overall_bw_bps0);

    return 0;
}
