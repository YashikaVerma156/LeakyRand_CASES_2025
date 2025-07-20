#!/bin/bash
########################################################################################
# 1. This script is to launch LR experiments, without error correction.                #
#    Total 'itr' iterations, each runs 'num_prl' LR experiments in parallel.           #
#    User should decide 'num_prl' by the number of cores in the system.                #
# 2. This script is called from calibration_and_variables.sh.                          #
########################################################################################

unroll_fact=$1    ## Specify unrolling factor
START_POINT=$2   ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
END_POINT=$3
num_prl=$4        ## Number of parallel instances to be run, depends on the number of cores.
benchmark_test=$5  ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
string_size=$6   # Message Length.
other_blocks=$7
arr_size=$8
algo_trigger_point=$9
hit_miss_threshold=${10}
mkdir -p binary
mkdir -p binary/${unroll_fact}_${algo_trigger_point}_${benchmark_test}

########### XXX Below lines generate receiver trace and binary as per the unroll_fact and algo_trigger_point. XXX  ############
#1a. Remove older binaries from bin/
rm ../../bin/bimodal-no-no-random*

#1b. Build 2-core LR binary, keeping a check on calibration constraints.
./script_for_err_corr_both_algo_2core.sh ${benchmark_test} ${arr_size} ${unroll_fact} ${algo_trigger_point} ${other_blocks} ${END_POINT} ${hit_miss_threshold}

#1c. Copy to binary/ 
cp ../../bin/bimodal-no-no-random-2core binary/${unroll_fact}_${algo_trigger_point}_${benchmark_test}
########## XXX Above lines generate receiver trace and binary as per the unroll_fact and algo_trigger_point. XXX  ############

binary_path=binary/${unroll_fact}_${algo_trigger_point}_${benchmark_test}

sender_traces_directory="sender_err_corr_both_algo"
result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments/LR_NO_ERR_CORR_2core_results/LR_ERR_CORR_2core_results_both_algo_specific_exp
 

# These are for train_benchmark_suite.
#a8=(106 114 120 128 195 198 208 2 236 246 3 4 5 89)
#a16=(102 117 129 149 16 169 202 224 6)
#a32=(110 129 208 42 84)
#a64=(103 88)
#a128=(155)

a8=(109 152 154 175 177 201 255 271 275 346 347 356 381 391 396 402 436 448 50 51 90)
a16=(118 190 224 251 310 395 405 442 90)
a32=(110 167 260 282 322 333 416 477)
a64=(119 219 326 359 389 406 410 5)
a128=(221 318)

if [ $algo_trigger_point -eq 8 ]; then
    redo_array=("${a8[@]}")  
elif [ $algo_trigger_point -eq 16 ]; then
    redo_array=("${a16[@]}")  
elif [ $algo_trigger_point -eq 32 ]; then
    redo_array=("${a32[@]}")  
elif [ $algo_trigger_point -eq 64 ]; then
    redo_array=("${a64[@]}")  
elif [ $algo_trigger_point -eq 128 ]; then
    redo_array=("${a128[@]}") 
fi


    echo "Running parallel experiments"
    for i in ${redo_array[@]}; 
    do
	    STR_NUM=$i
            echo "i: $i STR_NUM: $STR_NUM " #EXP_NUM: $EXP_NUM"
            line_number=`expr $STR_NUM + 1`
            if [ $benchmark_test == 0 ] 
            then
                specific_line=$(sed -n "${line_number}p" seeds_train.txt)
                IFS=',' read -ra columns <<< "$specific_line"
                seed=${columns[32]} # Hard coded inside seeds_train.txt.
                echo "Seed: $seed"
            else
                specific_line=$(sed -n "${line_number}p" seeds_test.txt)
                IFS=',' read -ra columns <<< "$specific_line"
                seed=${columns[32]}
                echo "Seed: $seed"
            fi
            
            search_string1="champsim.trace_sender_512_${STR_NUM}_"
            
            if [ $benchmark_test == 0 ] 
            then
                search_string2="_${algo_trigger_point}_with_both_algo_train"
            elif [ $benchmark_test == 1 ]
            then
                search_string2="_${algo_trigger_point}_with_both_algo_test"
            fi
            sim_count=0

            # Use a loop to iterate through each file in the directory
            for filename in $(ls "$sender_traces_directory"); do

                # Check if the filename contains the search string
                if [[ $filename == *"$search_string1"* && $filename == *"$search_string2"* ]]; then
                    echo "Found match: $filename"

                    # Split the filename using both '_' and '.' as delimiters
                    IFS='_.' read -ra array <<< "$filename"

                    # Extract the desired part (index 5 in this case)
                    sim_count="${array[5]}"
                    break
                fi
            done

            receiver_trace=champsim.trace_receiver_multiple_access_${unroll_fact}_other_blocks_${other_blocks}_${string_size}_${algo_trigger_point}_with_both_algo.gz
            receiver_trace_path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments/${receiver_trace}

            if [ $benchmark_test == 0 ] 
            then
                sender_trace=champsim.trace_sender_${string_size}_${STR_NUM}_${sim_count}_${algo_trigger_point}_with_both_algo_train.gz 
            elif [ $benchmark_test == 1 ]
            then
                sender_trace=champsim.trace_sender_${string_size}_${STR_NUM}_${sim_count}_${algo_trigger_point}_with_both_algo_test.gz 
            fi
            sender_trace_path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments/sender_err_corr_both_algo/${sender_trace}

            ./run_2core_LR_ERR_CORR_both_algo.sh $sim_count $receiver_trace_path $sender_trace_path $seed $string_size $result_dir $binary_path &
    done 
    wait
