#!/bin/bash
#benchmark_test=$1

#if [ "$1" > 1 ] || [ "$1" < 0 ]; then
#    echo " ERROR: The value of benchmark_test should be either 0 or 1."
#    exit 1
#fi

other_blocks=$1


# Check if exactly one argument is passed
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <other_blocks>"
    echo "other_blocks should be either 31 or 326"
    exit 1
fi

# Check if the argument is either 326 or 31
if [ "$1" -ne 326 ] && [ "$1" -ne 31 ]; then
    echo "The value of other_blocks must be either 326 or 31"
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

#exit 1

for benchmark_test in 0   #1         #0  1
do
    for unroll_fact in 16   #16 32
    do
        for algo_trigger_point in 8  # 8 16 32 64 128
        do
            echo "====================RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test} other_blocks: ${other_blocks} ======================="
            ./calibration_and_variables_with_err_corr_both_algo.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${other_blocks}
        done
    done
done
