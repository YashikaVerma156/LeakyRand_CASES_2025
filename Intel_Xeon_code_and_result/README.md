# CovertTux
## Introduction
The implementation of LeakyRand on an Intel Xeon Processor is named as CovertTux.
CovertTux is a cache replacement policy oblivious timing based covert channel on a Linux System. Our covert channel implemented on a set of the 16 MB 16-way set associative LLC of an Intel E-2278G Xeon processor clocked nominally at 3.4 GHz running ubuntu 18.04 achieves a communication bandwidth varying between 30 Kbps and 570 Kbps depending on different parameter settings, while maintaining a bit error rate ranging from 8.7 × 10<sup>−4</sup> to 4.5 × 10<sup>−2</sup>, averaging around 3.3 ×10<sup>−3</sup>.

## Description of files
- **`data_collect.c`** : Collects data for finding LLC hit/miss threshold.
- **`find_thresh.cpp`** : Finds threshold using the collected data.
- **`check_thresh.cpp`** : Verifies the chosen threshold.
- **`covertTux_multiThreaded.c`** : Mutli-threaded version of the channel.
- **`covertTux_multiProcess.c`** : Mutli-process version of the channel.
- **`cpuid_leaf2.c`** & **`decode_leaf2.c`**: Prints information about all the caches and TLBs in the system.
- **`cpuid_leaf4.c`** : Prints information about all the cache levels in the cache hierarchy.


__Note:__ The multi-threaded verison reports bancdwidth and BER for a particular pair of disturbance set size and unrolling factor which can be set using macros in `covertTux_multiThreaded.c`. The multi-process version reports the bandwidth and BER for multiple pairs of disturbance set size and unrolling factor defined using the global arrays `ds_vals` and `uf_vals` respectively in `covertTux_multiProcess.c`.

## System Requirements
- The Last Level Cache (LLC) must be inclusive.
- Atleast two cores must be present.
- LLC must be shared between the two cores.

To find the number of cores:
```bash
lscpu
```

To find information about cache hierarchy and TLBs:
```bash
gcc -O3 cpuid_leaf2.c decode_leaf2.c -o cpuid_leaf2
./cpuid_leaf2
```
```bash
gcc -O3 cpuid_leaf4.c -o cpuid_leaf4
./cpuid_leaf4
```
## Important Parameters
The following parameters are defined as macros in the respective files. These need to be set accordingly:

#### `data_collect.c`
- **`CACHE_BLOCK_SIZE`**: Size of a cache block in bytes. Default is `64`.
- **`PAGE_SIZE`**: Size of a memory page in bytes. Default is `4096`.
- **`MAX_NUM_BLOCKS`**: Maximum number of blocks to allocate for cache operations. Default is `(1 << 14)`.
- **`NUM_TOTAL_ACCESS`**: Total number of memory accesses for data collection. Default is `(1 << 26)`.

#### `check_thresh.cpp`
- **`THRESHOLD`**: Threshold value (in CPU cycles) for classifying cache hits and misses. Default is `200`.
- **`OUTLIER`**: Boundary value (in CPU cycles) to ignore outliers in timing measurements. Default is `10000`.

#### `covertTux_multiThreaded.c` & `covertTux_multiProcess.c`
- **`CACHE_BLOCK_SIZE`**: Size of a cache block in bytes. Default is `64`.
- **`PAGE_SIZE`**: Size of a memory page in bytes. Default is `4096`.
- **`THRESHOLD`**: Threshold value (in CPU cycles) for classifying cache hits and misses. Default is `200`.
- **`OUTLIER`**: Boundary value (in CPU cycles) to ignore outliers in timing measurements. Default is `10000`.
- **`ASSOC`**: Associativity of LLC. Default is `16`.
- **`CLOCK_FREQ`**: CPU clock frequency in Hz. Default is `3.4e9`.
- **`DS`**: Disturbance set size. Represents max disturbance set size for multi-process verison. Default is `4`.
- **`UNROLLING_FACTOR`**: Unrolling Factor. Represents max unrolling factor for multi-process verison. Default is `1`.
- **`TEST_SUITE_LEN`**: Number of strings in the test suite. Default is `500`.
- **`TRAIN_SUITE_LEN`**: Number of strings in the train suite. Default is `250`.
- **`STRLEN`**: Maximum length of each string in the train and test suites. Default is `512`.

Following macros are used to specify the occupancy sequence:

- **`OCC_SET_SIZE1`**: Number of blocks as per the first term of the occupancy sequence. Default is `ASSOC` (i.e., `16`).
- **`OCC_SET_SIZE2`**: Number of blocks as per the second term of the occupancy sequence. Default is `ceil(ASSOC * 1.1)` (i.e., `18`).
- **`OCC_SET_SIZE`**: Number of blocks as per the second term of the occupancy sequence. Default is `ceil(ASSOC * 1.2)` (i.e., `19`).
- **`CRFILL_ITERS1`**: Exponent of the first term. Default is `1`.
- **`CRFILL_ITERS2`**: Exponent of the second term. Default is `2`.
- **`CRFILL_ITERS3`**: Exponent of the third term. Default is `6`.

For example, the default values correspond to the occupancy sequence **(1x)<sup>1</sup>(1.1x)<sup>2</sup>(1.2x)<sup>6</sup>**

## How to run
Run all the commands below with root privileges:
```bash
sudo su
```

To collect data for finding threshold run the following commands: 
```bash
gcc -O3 -static data_collect.c -o data_collect
./data_collect
```
To calculate the threshold from the collected data run the following commands:
```bash
g++ -O3 -static find_thresh.cpp -o find_thresh
./find_thresh
```
To verify the threshold, change the `THRESHOLD` macro in `check_thresh.cpp`. Then run the following commands:
```bash
g++ -O3 -static check_thresh.cpp -o check_thresh
./check_thresh
```

To launch the Multi-threaded version of the channel, set all the macros in `covertTux_multiThreaded.c`. Then run the follwing commands:
```bash
gcc -O3 -static -pthread covertTux_multiThreaded.c -o coverTux_multiThreaded -lm
./coverTux_multiThreaded
```

To launch the Multi-process version of the channel, set all the macros in `covertTux_multiProcess.c`. Then run the following commmands:
```bash
gcc -O3 -static covertTux_multiProcess.c -o coverTux_multiProcess -lm
./covertTux_multiProcess
```
