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
#define TEST_SUITE_LEN 500
#define TRAIN_SUITE_LEN 250
#define STRLEN 512 
#define DS 4 // disturbance set size
#define UNROLLING_FACTOR 16
#define OCC_SET_SIZE1 ASSOC
#define OCC_SET_SIZE2 (int)(ceil(ASSOC * 1.1))
#define OCC_SET_SIZE (int)(ceil(ASSOC * 1.2))
#define CRFILL_ITERS1 1
#define CRFILL_ITERS2 2
#define CRFILL_ITERS3 6

// Other parameters
#define MAX_NUM_BLOCKS ((CACHE_CAPACITY_MB_LEAST_POWER_OF_TWO << 21) / CACHE_BLOCK_SIZE)
#define MAX_NUM_BLOCKS_B MAX_NUM_BLOCKS
#define MAX_NUM_BLOCKS_S (4*MAX_NUM_BLOCKS)
#define TESTING_PROB 0.8
#define ISOLATED_TESTING_PROB 0.9
#define SUCCESS_PROB 0.99
#define SELF_VALIDATION_ERROR_THRESHOLD 6e-4 
#define VALIDATION_ERROR_THRESHOLD 4e-3
#define MAX_TRIALS_PER_RUN 500
#define EV_SET_FINDING_RUNS 20
#define MIN_NUM_TESTS 50
#define MAX_NUM_TESTS 1000
#define EXPLORATION_BATCH_SIZE 256
#define CHECK_STRLEN 1000000 
#define BOOST_AGING_FACTOR 4
#define DS_FINDING_TRIALS 1000
#define EXPLORATION_BATCH_SIZE_SNDR 256
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
        run++; \
        goto eviction_set_finding; \
        exit(0); \
    } \
} while (0)

// Macros for reading and writing to pipes
#define READ(flag)                  \
do {                                             \
    if (read((read_fd), ((void*)(&flag)), (sizeof(flag))) <= 0) {     \
        printf("Read failed. fd = %d, pid = %d\n", read_fd, pid);  \
        exit(EXIT_FAILURE);                      \
    }                                            \
} while (0)

#define WRITE(flag)                  \
do {                                             \
    if (write((write_fd), ((void*)(&flag)), (sizeof(flag))) <= 0) {     \
        printf("Write failed. fd = %d, pid = %d\n", write_fd, pid);  \
        exit(EXIT_FAILURE);                      \
    }                                            \
} while (0)

// Global variables
pid_t pid;
int x = 0, trial = 1, run = 1;
Block *A, *B;
int *index_arrayA, *index_arrayB, *array;
int **receiver_array;
volatile long long int flag = 0; 
int occ = 0, num_nz, idx;

unsigned int idx1=0, idx2=NUM_DATA_POINTS-1;
unsigned long glo_min=LONG_MAX, glo_max = 0;

unsigned long thresh_data[NUM_DATA_POINTS];
unsigned long threshold = THRESHOLD;
double avg_hit_latency = 0;
double std_dev_hit_latency = 0;
double avg_miss_latency = 0;
double std_dev_miss_latency = 0;

char train_data_set[TRAIN_SUITE_LEN][STRLEN+1];
char test_data_set[TEST_SUITE_LEN][STRLEN+1];

// All ds and uf configurations
int ds_vals[3] = {1,2,4};
int uf_vals[5] = {1,2,4,8,16};


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
  Function executed by the sender process

  # read_fd => file descriptor from which sender reads
  # write_fd => file descriptor to which sender writes
*/
void sender(int read_fd, int write_fd){

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
    printf("sender on cpu : %d\n", sched_getcpu());
    setvbuf(stdout, NULL, _IONBF, 0);

    // Initialize variables
    int i, j, k, *ptr, y, miss = 0, jstart, istart, runS=0;
    int t, num_tests;
    register uint64_t start, end, comm_start, comm_end;
    long long int max_timeS = 0;

    // Read the training and testing data sets from files
    FILE* train_fp = fopen("benchmark_train.txt", "r");
    if(train_fp == NULL){
        perror("Error opening file train file");
        exit(EXIT_FAILURE);
    }   
    x = fread(train_data_set, sizeof(char), TRAIN_SUITE_LEN * (STRLEN+1), train_fp);

    FILE* test_fp = fopen("benchmark_test.txt", "r");
    if(test_fp == NULL){
        perror("Error opening file test file");
        exit(EXIT_FAILURE);
    }   
    x = fread(test_data_set, sizeof(char), TEST_SUITE_LEN * (STRLEN+1), test_fp);

    fclose(train_fp);
    fclose(test_fp);    

    // Indices for the disturbance set blocks
    index_arrayA = (int *)malloc(sizeof(int) * MAX_NUM_BLOCKS_S);

    // Array to store the number of misses for each block in the disturbance set
    array = (int *)malloc(sizeof(int) * MAX_NUM_BLOCKS_S);

    // Allocate large number of blocks to find disturbance set elements from
    assert(!posix_memalign((void *)&A, PAGE_SIZE, MAX_NUM_BLOCKS_S * sizeof(Block)));
    assert(A);

    int runs_left = EV_SET_FINDING_RUNS;
    int ds=1, uf=1;

    /* ------------------------------Eviction Set Finding --------------------------------------- */
    eviction_set_finding: // receiver finding eviction set
    if(!runs_left)  goto quit;
    runs_left--;

    // Wait for receiver to find eviction set
    READ(flag); // 1
    
    int trials_left = DS_FINDING_TRIALS;

    // Touch then flush all blocks in A
    for(int i=0; i<MAX_NUM_BLOCKS_S; i++) A[i].a[0] = 0;
    for(int i=0; i<MAX_NUM_BLOCKS_S; i++) flush(&(A[i].a[0]));
    
    /* ----------------------------- Disturbance Set Finding --------------------------- */
    ds_finding: // start a new trial
    if(!trials_left) goto quit;
    trials_left--;

    idx = 0, j=0, i=0;
    memset(array, 0, sizeof(int) * MAX_NUM_BLOCKS_S);
    jstart = random() % ((MAX_NUM_BLOCKS_S * CACHE_BLOCK_SIZE) / PAGE_SIZE);
    istart = random() % (PAGE_SIZE / CACHE_BLOCK_SIZE);
    printf("S: Trials left %d. Starting new exploration at page no. %d, page offset %d\n", trials_left, jstart, istart);
    int count = ASSOC + 1;

    sender_prime:
    // prime the LLC with count blocks
    for(int iter=0; iter<BOOST_AGING_FACTOR; iter++){
        k = 0;
        i=istart;
        j=jstart;
        while(k < count){            
            ptr = &(A[j * (PAGE_SIZE / CACHE_BLOCK_SIZE) + i].a[0]);
            asm volatile("mfence" ::: "memory");
            y = *ptr;
            asm volatile("mfence" ::: "memory");
            x += y;
            k++;
            j = (j + 1) & ((MAX_NUM_BLOCKS_S * CACHE_BLOCK_SIZE) / PAGE_SIZE - 1);
            if (j == jstart) {
                i = (i + 1) & (PAGE_SIZE / CACHE_BLOCK_SIZE - 1);
            }
        }
    }
    
    // Tell the receiver to perform CRFill
    WRITE(flag); // 2

    // Wait for the receiver to finish CRFill
    READ(flag); // 3

    // Probe the LLC to find number of misses
    idx = 0;
    miss = 0;
    num_tests = 1;
    for(t=0; t<num_tests; t++){
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
            j = (j + 1) & ((MAX_NUM_BLOCKS_S * CACHE_BLOCK_SIZE) / PAGE_SIZE - 1);
            if (j == jstart){
                i = (i + 1) & (PAGE_SIZE / CACHE_BLOCK_SIZE - 1);
            }
        }
    }
    // sufficient average misses check
    if (miss >= 2*DS * t){
        num_nz = 0;

        // store indices in index_arrayA and miss count in array for all blocks having one or more misses
        for (i = 0; i < MAX_NUM_BLOCKS_S; i++){
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

        // check if atleast first 2*DS elements showed high miss rate
        for (i = 0; i < 2*DS; i++){
            if (array[i] < ISOLATED_TESTING_PROB * t)
                break;
        }
        if (i >= 2*DS){
            // Potential DS elements found
            flag = 0;

            // Tell receiver that potential DS elements are found
            WRITE(flag); // 4a

            // Wait for receiver to read the flag
            READ(flag); // 5a

            goto check_ds_elements;
        }
    }

    count += EXPLORATION_BATCH_SIZE_SNDR; // increase the number of blocks to find disturbance set

    if (count > MAX_NUM_BLOCKS_S){
        flag = 1;

        // Tell receiver to start a new trial
        WRITE(flag); // 4b

        // Wait for receiver to read the flag
        READ(flag); // 5b
        goto ds_finding;
    }

    flag = 2;

    // finding DS elements with increased count size
    // Tell receiver to goto CRFill
    WRITE(flag); // 4c

    // Wait for receiver to read the flag
    READ(flag); // 5c
    goto sender_prime;

    // Check if potential DS elements conflict with receiver's eviction set
    check_ds_elements:
    num_nz = i;
    miss = 0, idx = 0;

    // Reset and flush potential DS elements
    asm volatile("mfence" ::: "memory");
    for(i=0; i<num_nz; i++){
        ptr = &(A[index_arrayA[i]].a[0]);
        (*ptr) = 0;
        flush(ptr);
    }
    asm volatile("mfence" ::: "memory");

    for(t=0; t<MAX_NUM_TESTS; t++){

        // Prime the LLC with potential DS elements
        for(int iter=0; iter<BOOST_AGING_FACTOR; iter++){
            k = 0;
            while(k < num_nz){            
                ptr = &(A[index_arrayA[k]].a[0]);
                asm volatile("mfence" ::: "memory");
                y = *ptr;
                asm volatile("mfence" ::: "memory");
                x += y;
                k++;
            }
        }

        // Tell the receiver to perform CRFill
        WRITE(flag); // 6

        // Wait for the receiver to finish CRFill
        READ(flag); // 7

        // Probe the LLC to find number of misses
        for (i = 0; i < num_nz; i++){
            ptr = &(A[index_arrayA[i]].a[0]);
            RDTSCFENCE(start);
            y = *ptr;
            RDTSCFENCE(end);
            if (((end - start) > THRESHOLD) && ((end - start) <= OUTLIER)){
                (*ptr)++; // store miss count for a particular block in the block itself
            }
            x += y;
        }
    }

    idx = 0;
    
    // store indices in index_arrayA and miss count in array for all blocks having high miss rate
    for(k=0; k<num_nz; k++){
        if(A[index_arrayA[k]].a[0] > ISOLATED_TESTING_PROB * MAX_NUM_TESTS){
            index_arrayA[idx] = index_arrayA[k];
            array[idx] = A[index_arrayA[k]].a[0];
            idx++;
        }
    }
    if(idx < DS){
        // if less than DS elements found, start a new trial
        flag = 1;

        // Tell receiver to goto a new trial
        WRITE(flag); // 8

        // Wait for receiver to read the flag
        READ(flag); // 9

        goto ds_finding;
    }

    num_nz = idx;
    printf("S: Potential DS elements: ");
    for(i=0; i<num_nz; i++) printf("%d ", index_arrayA[i]);
    printf("\n");
    
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

    /*---------------------------------Validation using Train Suite ------------------------------------*/
    validation:
    flag = 0;

    // Tell receiver that DS elements are found
    WRITE(flag); // 8
    
    for(i=0; i<TRAIN_SUITE_LEN; i++){

        // Wait for receiver to perform CRFill + CRProbe
        READ(flag); // 9

        for(j=0; j<STRLEN; j++){
            
            // for each bit, communicate using the disturbance set
            if(train_data_set[i][j] == '1') {
                // access the disturbance set elements
                for (k = 0; k < DS; k++){
                    ptr = &A[index_arrayA[k]].a[0];
                    asm volatile("mfence" ::: "memory");
                    y = *ptr;
                    asm volatile("mfence" ::: "memory");
                    x += y;
                }
                // flush the disturbance set elements
                for (k = 0; k < DS; k++){
                    flush(&A[index_arrayA[k]].a[0]);
                }
            }
            
            // Tell the receiver to infer the bit sent
            WRITE(flag); // 10

            // Wait for the receiver to infer it
            READ(flag); // 11
        }

        // Synchronize with the receiver
        WRITE(flag); // 12
    }    

    // Wait for receiver to check number of errors 
    READ(flag); // 13


    if(flag == 0){
        // validation failed, need to find a quieter EV and DS set
        // Tell receiver that message is read
        WRITE(flag); // 14a
        goto eviction_set_finding;
    }
 
    // validation passed

    /*---------------------------------------Communicate Test Suite----------------------------------------------*/
    //Communicate using all possible {uf,ds} pairs
    for(int uf_iter = 0; uf_iter < 5; uf_iter++){
        uf = uf_vals[uf_iter];
        for(int ds_iter = 0; ds_iter < 3; ds_iter++){
            ds = ds_vals[ds_iter];

            // Tell receiver to perform CRFill + CRProbe
            WRITE(flag); // 14b 

            // Wait for receiver to perform CRFill + CRProbe
            READ(flag); // 15

            /*-----------------------------------Unrolling Factor Threshold----------------------------------------*/
            // use a string of alternating 0s and 1s to find the threshold
            for (i = 0; i < NUM_DATA_POINTS; i++){
                if(i&1){
                    // access ds elements from the disturbance set
                    for (k = 0; k < ds; k++){
                        ptr = &A[index_arrayA[k]].a[0];
                        asm volatile("mfence" ::: "memory");
                        y = *ptr;
                        asm volatile("mfence" ::: "memory");
                        x += y;
                    }

                    // flush ds elements from the disturbance set
                    for (k = 0; k < ds; k++){
                        flush(&A[index_arrayA[k]].a[0]);
                    }
                }

                // Tell the receiver to infer the bit
                WRITE(flag); // 16

                // Wait for the receiver to infer it
                READ(flag); // 17
            }

            // Synchronize with the receiver
            WRITE(flag); // 18

            /*---------------------Communication----------------------*/
            for(i=0; i<TEST_SUITE_LEN; i++){
                max_timeS = 0;

                // Wait for receiver to find the threshold (only in first iteration) and also do CRFill+CRProbe
                READ(flag); // 19
                
                for(j=0; j<STRLEN; j++){
                    // for each bit, communicate using the disturbance set
                    // and also measure time taken to communicate
                    RDTSCFENCE(comm_start);
                    if(test_data_set[i][j] == '1') {
                        // access ds elements from the disturbance set 
                        for (k = 0; k < ds; k++){
                            ptr = &A[index_arrayA[k]].a[0];
                            asm volatile("mfence" ::: "memory");
                            y = *ptr;
                            asm volatile("mfence" ::: "memory");
                            x += y;
                        }
                        // flush ds elements from the disturbance set
                        for (k = 0; k < ds; k++){
                            flush(&A[index_arrayA[k]].a[0]);
                        }
                    }
                    RDTSCFENCE(comm_end);
                    
                    // record maximum send time across all the bits in a string
                    max_timeS = ((comm_end-comm_start) > max_timeS) ? (comm_end - comm_start) : max_timeS;
                    
                    // Tell the receiver to infer the bit sent
                    WRITE(flag); // 20

                    // Wait for the receiver to infer it
                    READ(flag); // 21
                }

                // Tell the maximum send time for that string to the receiver
                flag = max_timeS;
                WRITE(flag); // 22
            }    

            // Wait for receiver to find bandwidth and BER for that {uf,ds} pair
            READ(flag); // 23
        }

    }

    quit:
    printf("S: x = %d\n", x);
    exit(0);
    
   
}

/*
  Main process

  -> Creates a sender process
  -> Acts as receiver process itself
*/
int main(int argc, char *argv[]){
    // Create a child process for the sender
    int pipe1[2]; // Pipe for communication: receiver -> sender
    int pipe2[2]; // Pipe for communication: sender -> receiver
    int read_fd, write_fd;
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Child process: Sender
        close(pipe1[1]); // Close write end of pipe1 (receiver -> sender)
        close(pipe2[0]); // Close read end of pipe2 (sender -> receiver)
        sender(pipe1[0], pipe2[1]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process: Receiver
        close(pipe1[0]); // Close read end of pipe1 (receiver -> sender)
        close(pipe2[1]); // Close write end of pipe2 (sender -> receiver)
        write_fd = pipe1[1];
        read_fd = pipe2[0];
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
    printf("receiver on cpu : %d\n", sched_getcpu());
	setvbuf(stdout, NULL, _IONBF, 0);

    // Initialize variables
    register uint64_t start, end;
    register uint64_t comm_start, comm_end;
    register uint64_t cr_start, cr_end;
    int count, i, j, k, miss, t, num_tests, jstart, istart, val_errors=0;
    int *ptr, y;
    int *ptri[UNROLLING_FACTOR];
    int yi[UNROLLING_FACTOR];
    unsigned long maxi = 0;
    long long int max_timeR = 0, total_time = 0;
    int ds = 1, uf = 1;

    // Indices for the eviction set blocks
    index_arrayA = (int *)malloc(sizeof(int) * MAX_NUM_BLOCKS);

    // Indices for the occupancy set blocks
    index_arrayB = (int *)malloc(sizeof(int) * (OCC_SET_SIZE));

    // Array to store the number of misses for each cache block
    array = (int *)malloc(sizeof(int) * MAX_NUM_BLOCKS);

    // Array to store the addresses of the cache blocks in the occupancy set
    receiver_array = (int **)malloc(sizeof(int *) * OCC_SET_SIZE);
    assert(receiver_array);

    // Allocate large number of blocks to find eviction set elements from
    assert(!posix_memalign((void *)&A, PAGE_SIZE, MAX_NUM_BLOCKS * sizeof(Block)));
    assert(A);

    // Allocate large number of blocks to extend eviction set elements 
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
    x = fread(train_data_set, sizeof(char), TRAIN_SUITE_LEN * (STRLEN+1), train_fp);

    FILE* test_fp = fopen("benchmark_test.txt", "r");
    if(test_fp == NULL){
        perror("Error opening file test file");
        exit(EXIT_FAILURE);
    }   
    x = fread(test_data_set, sizeof(char), TEST_SUITE_LEN * (STRLEN+1), test_fp);

    fclose(train_fp);
    fclose(test_fp);

    // Structure to store bits received from the sender
    char recv_str[TEST_SUITE_LEN][STRLEN+1];

    /* ------------------------------Eviction Set Finding --------------------------------------- */
    eviction_set_finding: // start a new run 
    while (run <= EV_SET_FINDING_RUNS){
        printf("R: Run %d, finding new eviction set...\n", run);
        trial = 0; occ= 0; 
        while (trial <= MAX_TRIALS_PER_RUN){ // start a new trial 
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
                    if (j == jstart) {
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
        for (i = 0; i < OCC_SET_SIZE; i++) flush(receiver_array[i]);
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
        
        
        // CRFill 
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
        occ = 0;
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
        printf("Percentage of errors in validation 1 = %lf %%\n", fraction_err * 100);

        if (fraction_err >= SELF_VALIDATION_ERROR_THRESHOLD) {
            printf("Validation 1 failed\n");
            run++;
            continue; // start a new run
        }
        else // Validation 1 passed 
            break;

    }
    
    // if all runs failed, exit
    if(run > EV_SET_FINDING_RUNS) goto quit;
    
    // Tell the sender that eviction set is found
    WRITE(flag); // 1

    int trials_left = DS_FINDING_TRIALS;
    
    /* ----------------------------- Disturbance Set Finding --------------------------- */
    ds_finding:
    if(!trials_left) goto quit;
    trials_left--;

    // Receiver performs CRFill
    receiver_access: 

    // Wait for sender to prime the LLC
    READ(flag); // 2
    
    // CRFill to occupy the cache set
    asm volatile("mfence" ::: "memory");
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
    asm volatile("mfence" ::: "memory");

    // Tell the sender that CRFill is done
    WRITE(flag); // 3

    // Wait for the sender to probe the LLC 
    READ(flag); // 4
    
    if(flag == 0){
        // Potential DS elements found
        // Tell sender that message is read
        WRITE(flag); // 5a
        goto check_ds_elements;
    }
    else if(flag == 1){
        // need to start a new DS finding trial
        // Tell sender that message is read
        WRITE(flag); // 5b
        goto ds_finding;
    }
    else if(flag == 2){
        // finding DS elements with increased count size
        // Tell sender that message is read
        WRITE(flag); // 5c
        goto receiver_access;
    }

    // Check if sender's potential DS elements conflict with eviction set
    check_ds_elements:
    for(t=0; t<MAX_NUM_TESTS; t++){

        // Wait for sender to prime the LLC
        READ(flag); // 6

        // CRFill to occupy the cache set
        asm volatile("mfence" ::: "memory");
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

        // Tell the sender that CRFill is done
        WRITE(flag); // 7
    }

    // Wait for the sender to probe the LLC and infer
    READ(flag); // 8

    if(flag == 1){
        // sender could not find DS elements 
        // Tell sender that message is read
        WRITE(flag); // 9
        goto ds_finding;
    }

    val_errors = 0;

    /*---------------------------------Validation using Train Suite ------------------------------------*/
    validation:
    for(t=0; t<TRAIN_SUITE_LEN; t++){

        // CRFill to occupy the cache set   
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
        occ = 0;
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
    
        // Tell sender that CRFill and CRProbe are done
        WRITE(flag); // 9
    
        for(j=0; j<STRLEN; j++){

            // Wait for sender to send the bit 
            READ(flag); // 10

            // Time access the occupancy set and count misses
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

            // infer the bit sent by sender and count errors
            if(miss>0 && train_data_set[t][j] == '0') val_errors++;
            if(miss==0 && train_data_set[t][j] == '1') val_errors++;

            // Tell sender that bit is inferred
            WRITE(flag); // 11
        }

        // Synchronize with the sender
        READ(flag); // 12
    }

    printf("R: train errors = %d\n", val_errors);
    if(val_errors > TRAIN_SUITE_LEN*STRLEN*VALIDATION_ERROR_THRESHOLD){
        // validation failed, need to find a quieter EV and DS set
        flag = 0;
        run++;

         // Tell sender to start a new eviction set finding run
        WRITE(flag); // 13a

        // Wait for sender to read the flag
        READ(flag); // 14a

        goto eviction_set_finding;
    }

    flag = 1;

    // Tell sender that validation passed
    WRITE(flag); // 13b

    // ds = 1, uf = 1;
    for(int uf_iter = 0; uf_iter < 5; uf_iter++){
        uf = uf_vals[uf_iter];
        for(int ds_iter = 0; ds_iter < 3; ds_iter++){
            ds = ds_vals[ds_iter];

            // Wait for sender to read the flag
            READ(flag); // 14b

            /* CRFill + CRProbe*/
            asm volatile("mfence" ::: "memory");
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

            /*-----------------------------------Unrolling Factor Threshold----------------------------------------*/
            threshold = THRESHOLD;
            idx1 = 0;
            idx2 = NUM_DATA_POINTS-1;

            // Tell sender that CRFill and CRProbe are done
            WRITE(flag); // 15

            // expect a string of alternating 0s and 1s to find the threshold
            for (i = 0; i < NUM_DATA_POINTS; i++){
                maxi=0;

                // Wait for sender to send the bit
                READ(flag); // 16

                // Time access the occupancy set and count misses in bunches of size uf
                for (k = 0; k < occ; k+=uf){
                    for (int it=0; it<uf; it++){
                        if(k+it < occ) ptri[it] = receiver_array[k+it];
                    }
                    if(k + uf <= occ){
                        if(uf==1){
                            RDTSCFENCE(start);
                            yi[0] = *ptri[0];
                            RDTSCFENCE(end);
                        }
                        else if(uf==2){
                            RDTSCFENCE(start);
                            yi[0] = *ptri[0];
                            yi[1] = *ptri[1];
                            RDTSCFENCE(end);
                        }
                        else if(uf==4){
                            RDTSCFENCE(start);
                            yi[0] = *ptri[0];
                            yi[1] = *ptri[1];
                            yi[2] = *ptri[2];
                            yi[3] = *ptri[3];
                            RDTSCFENCE(end);
                        }
                        else if(uf==8){
                            RDTSCFENCE(start);
                            yi[0] = *ptri[0];
                            yi[1] = *ptri[1];
                            yi[2] = *ptri[2];
                            yi[3] = *ptri[3];
                            yi[4] = *ptri[4];
                            yi[5] = *ptri[5];
                            yi[6] = *ptri[6];
                            yi[7] = *ptri[7];
                            RDTSCFENCE(end);
                        }
                        else if(uf==16){
                            RDTSCFENCE(start);
                            yi[0] = *ptri[0];
                            yi[1] = *ptri[1];
                            yi[2] = *ptri[2];
                            yi[3] = *ptri[3];
                            yi[4] = *ptri[4];
                            yi[5] = *ptri[5];
                            yi[6] = *ptri[6];
                            yi[7] = *ptri[7];
                            yi[8] = *ptri[8];
                            yi[9] = *ptri[9];
                            yi[10] = *ptri[10];
                            yi[11] = *ptri[11];
                            yi[12] = *ptri[12];
                            yi[13] = *ptri[13];
                            yi[14] = *ptri[14];
                            yi[15] = *ptri[15];
                            RDTSCFENCE(end);
                        }
                    }

                    // if occ not divisible by uf, access the remaining elements
                    else{
                        RDTSCFENCE(start);
                        for(int it=0; it<occ-k; it++){
                            yi[it] = *ptri[it];
                        }
                        RDTSCFENCE(end);
                    }
                    for (int it=0; it<uf; it++){
                        x += yi[it];
                    }

                    // find the maximum time taken across all accesses
                    if(end-start>maxi) maxi=end-start;
                }

                // if expected a bit 1, the maximum time is miss time
                if(i&1) thresh_data[idx2--]=maxi;
                // if expected a bit 0, the maximum time is hit time
                else thresh_data[idx1++]=maxi;

                // Tell sender that bit is inferred and timing recorded
                WRITE(flag); // 17
            }

            // Synchronize with the sender
            READ(flag); // 18

            /*---------------------Calibrating the threshold----------------------*/

            // find global min and max time
            unsigned long temp;
            glo_min = LLONG_MAX;
            glo_max = 0;
            for(int i=0; i<NUM_DATA_POINTS; i++){
                temp = thresh_data[i];
                if(temp < glo_min) glo_min = temp;
                if(temp > glo_max) glo_max = temp;
            }
            
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
            printf("uf %d THRESHOLD = %lu\n", uf, threshold);

            /*---------------------Communication----------------------*/
        
            total_time = 0; // record time taken to receive all the strings
            for(t=0; t<TEST_SUITE_LEN; t++){

                // Perform CRFill+CRProbe and also record its time
                RDTSCFENCE(cr_start);
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
                occ = 0;
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
                RDTSCFENCE(cr_end);
                
                // Tell sender that CRFill and CRProbe are done
                WRITE(flag); // 19
            
                for(j=0; j<STRLEN; j++){
                    int miss = 0;
                    max_timeR = 0;
                    
                    // Wait for sender to send the bit 
                    READ(flag); // 20
                    
                    // for each bit, infer using the eviction set
                    // and also measure time taken to communicate
                    RDTSCFENCE(comm_start);
                    for (k = 0; k < occ; k+=uf){
                        for (int it=0; it<uf; it++){
                            if(k+it < occ) ptri[it] = receiver_array[k+it];
                        }
                        if(k + uf <= occ){
                            if(uf==1){
                                RDTSCFENCE(start);
                                yi[0] = *ptri[0];
                                RDTSCFENCE(end);
                            }
                            else if(uf==2){
                                RDTSCFENCE(start);
                                yi[0] = *ptri[0];
                                yi[1] = *ptri[1];
                                RDTSCFENCE(end);
                            }
                            else if(uf==4){
                                RDTSCFENCE(start);
                                yi[0] = *ptri[0];
                                yi[1] = *ptri[1];
                                yi[2] = *ptri[2];
                                yi[3] = *ptri[3];
                                RDTSCFENCE(end);
                            }
                            else if(uf==8){
                                RDTSCFENCE(start);
                                yi[0] = *ptri[0];
                                yi[1] = *ptri[1];
                                yi[2] = *ptri[2];
                                yi[3] = *ptri[3];
                                yi[4] = *ptri[4];
                                yi[5] = *ptri[5];
                                yi[6] = *ptri[6];
                                yi[7] = *ptri[7];
                                RDTSCFENCE(end);
                            }
                            else if(uf==16){
                                RDTSCFENCE(start);
                                yi[0] = *ptri[0];
                                yi[1] = *ptri[1];
                                yi[2] = *ptri[2];
                                yi[3] = *ptri[3];
                                yi[4] = *ptri[4];
                                yi[5] = *ptri[5];
                                yi[6] = *ptri[6];
                                yi[7] = *ptri[7];
                                yi[8] = *ptri[8];
                                yi[9] = *ptri[9];
                                yi[10] = *ptri[10];
                                yi[11] = *ptri[11];
                                yi[12] = *ptri[12];
                                yi[13] = *ptri[13];
                                yi[14] = *ptri[14];
                                yi[15] = *ptri[15];
                                RDTSCFENCE(end);
                            }
                        }
                        else{
                            RDTSCFENCE(start);
                            for(int it=0; it<occ-k; it++){
                                yi[it] = *ptri[it];
                            }
                            RDTSCFENCE(end);
                        }
                        for (int it=0; it<uf; it++){
                            x += yi[it];
                        }
                        // count number of misses for each bit
                        if(end-start>threshold) miss++;
                    }
        
                    // infer the bit sent by sender
                    if(miss>0) recv_str[t][j] = '1';
                    else recv_str[t][j] = '0';
                    RDTSCFENCE(comm_end);

                    // find the maximum receive time across all bits in the string
                    max_timeR = ((comm_end-comm_start) > max_timeR) ? (comm_end - comm_start) : max_timeR;
                    
                    // Tell sender that bit is inferred and timing recorded
                    WRITE(flag); // 21
                }

                // Read the maximum send time recorded by the sender
                READ(flag); // 22
                
                // The time for this string is the time taken to do CRFill+CRProbe + the time taken to communicate the string
                // add this to the total time 
                total_time += (cr_end-cr_start)+STRLEN*(max_timeR+flag);
            }
        
            // find number of errors
            val_errors = 0;
            for(int i=0; i<TEST_SUITE_LEN; i++){
                for(int j=0; j<STRLEN; j++){
                    if(recv_str[i][j] != test_data_set[i][j]) val_errors++;
                }
            }
            printf("R: uf %d ds %d test errors = %d\n", uf, ds, val_errors);

            // find the bandwidth 
            double overall_bw_bps = (CLOCK_FREQ * STRLEN * TEST_SUITE_LEN) / total_time;
            
            printf("R: uf %d ds %d BW (bps) = %.12lf\n", uf, ds, overall_bw_bps);
        
            // Tell sender that bandwidth and BER are calculated
            WRITE(flag); // 23
        }
    }

    quit:
    printf("R: x=%d\n", x);

    return 0;
}
