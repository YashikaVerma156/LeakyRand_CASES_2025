#!/bin/bash
array_size=$1
unrolling_factor=$2

# Check if exactly two arguments are passed.
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <array_size> <unrolling_factor>"
    echo "array_size should be in number of array elements. unrolling_factor should be 16, 32, or 64."
    exit 1
fi

# Check if the argument is either 326 or 31
if [ "$2" -ne 16 ] && [ "$2" -ne 64 ] && [ "$2" -ne 32 ]; then
    echo "The value of unrolling_factor must be either 16, 32, or 64"
    exit 1
fi

# The commandline argument should be an integer.
integer_regex='^[0-9]+$'

# Check if the argument matches the integer pattern
if ! [[ $1 =~ $integer_regex ]]; then
    echo "The array_size must be an integer."
    exit 1
fi
if ! [[ $2 =~ $integer_regex ]]; then
    echo "The unrolling_factor must be an integer."
    exit 1
fi

for atp in 8 16 32 64 128 256 512
do
    algo_trigger_point=$atp
    echo "=================== Calculating for algo_trigger_point: $algo_trigger_point  ====================="
    bit_string_length=$((algo_trigger_point - 1))
    string_length=512
    num_strings_in_test_benchmark=500
    cache_block_count=32768
    occupied_blocks=$((array_size / 8))
    identifiable_blocks=$((occupied_blocks / unrolling_factor))
    occupied_count=$((identifiable_blocks * unrolling_factor))
    num_trials=$((string_length / bit_string_length))
    num_trials=$((num_trials * num_strings_in_test_benchmark))
    num_sender_blocks=1
    echo "cache_block_count: $cache_block_count bit_string_length: $bit_string_length occupied_count: $occupied_count num_trials: $num_trials"
    #exit

    gcc error-sim.c

    while [ 1 ];
    do
        echo "num_sender_blocks: $num_sender_blocks"
        # This script runs the C program "error-sim.c" with the arguments "$1" and "$2"
        ./a.out $cache_block_count $occupied_count $num_sender_blocks $bit_string_length $num_trials > temp.txt
        cat temp.txt
        error_count=`grep "Error count:" temp.txt | awk '{print $3}' | awk '{gsub(/,/,"")}1'`
        num_sender_blocks=$(( num_sender_blocks + 1 ))
        if [ $error_count == 0 ]
        then
            break
        fi
    done
done
