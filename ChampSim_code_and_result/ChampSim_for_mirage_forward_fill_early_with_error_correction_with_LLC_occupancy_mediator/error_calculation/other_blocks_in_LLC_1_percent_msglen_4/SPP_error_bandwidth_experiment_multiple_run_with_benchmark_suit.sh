#!/bin/bash
##################### This script is for SPP error calculation ###########################
sender_arr_size=$1 
seed=$2
string_num=$3

#====== Constants ========#
receiver_arr_size=262144 
string_size=512  #512
repetition_coding=1
differential_signalling=2
array_elements_in_one_cache_block=8

#====== Paths and file names ========#
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4/sender_${sender_arr_size}_pin_trace
trace1=champsim.trace_receiver_${string_size}_seed.gz
result_dir=/home/yashikav/Desktop/Mirage_project/fig4b/with_diff_sig/sender_arr_size_${sender_arr_size}
mkdir -p /home/yashikav/Desktop/Mirage_project/fig4b/with_diff_sig/sender_arr_size_${sender_arr_size}

#============= 1. Read sender traces ==============#
search_string1="champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed"

            sim_count=0

            # Use a loop to iterate through each file in the directory
            for filename in $(ls "$path_to_copy_trace"); do
                echo "$filename"

                # Check if the filename contains the search string
                if [[ $filename == *"$search_string1"* ]]; then
                    echo "Found match: $filename"

                    # Split the filename using both '_' and '.' as delimiters
                    IFS='_.' read -ra array <<< "$filename"

                    # Extract the desired part (index 8 in this case)
                    sim_count_sender="${array[9]}"
                    break
                fi
            done
echo "$sim_count_sender"
trace2=champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed_${sim_count_sender}.gz

#============ 2. Run a 2-core simulation with wait_implementation off ============#
./run_2core.sh ${sim_count_sender} ${trace1} ${trace2} ${seed} ${string_size} 0 0 ${repetition_coding} ${differential_signalling} ${sender_arr_size} ${path_to_copy_trace} ${result_dir}
