#!/bin/bash
#benchmark_test=$1

#if [ "$1" > 1 ] || [ "$1" < 0 ]; then
#    echo " ERROR: The value of benchmark_test should be either 0 or 1."
#    exit 1
#fi

other_blocks=$1
str_num=$2
num_prl=$3

# Check if exactly three arguments are passed
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <other_blocks> <str_num> <num_prl>"
    echo "other_blocks should be either 31 or 326"
    exit 1
fi

# Check if the argument is 326
if [ "$1" -ne 326 ]; then
    echo "The value of other_blocks must be 326"
    exit 1
fi

# The commandline argument should be an integer.
integer_regex='^[0-9]+$'

# Check if the argument matches the integer pattern
if ! [[ $1 =~ $integer_regex ]]; then
    echo "The value of other_blocks must be an integer"
    exit 1
fi

echo "other blocks: $other_blocks"

for benchmark_test in 1 
do
    for unroll_fact in 16   
    do
        for algo_trigger_point in 514 
        do
            for noise_trigger_point in 1 2 4 8 16 32 
            do
	        for err_corr_iterations in 100 800
		do
                    echo "====================RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test} other_blocks: ${other_blocks} ======================= err_corr_iterations: ${err_corr_iterations}, noise_trigger_point: ${noise_trigger_point}"   
                    #exit 1 
                    ./calibration_and_variables_with_err_corr_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${other_blocks} ${err_corr_iterations} ${noise_trigger_point} ${str_num} ${num_prl}
		done
            done
        done
    done
done
