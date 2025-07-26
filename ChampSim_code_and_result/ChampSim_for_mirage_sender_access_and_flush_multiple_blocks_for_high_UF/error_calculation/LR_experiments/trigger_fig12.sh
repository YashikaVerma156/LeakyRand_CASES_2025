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
err_corr_iterations=${11}
hit_miss_threshold_for_err_corr=${12}
LLC_NUM_BLOCKS=${13}

mkdir -p binary
mkdir -p binary/${unroll_fact}_${algo_trigger_point}_${benchmark_test}_${err_corr_iterations}

########### XXX Below lines generate receiver trace and binary as per the unroll_fact and algo_trigger_point. XXX  ############
#1a. Remove older binaries from bin/
rm ../../bin/bimodal-no-no-random*

#1b. Build 2-core LR binary, keeping a check on calibration constraints.
./script_high_UF_with_LLC_sensitivity.sh ${benchmark_test} ${arr_size} ${unroll_fact} ${algo_trigger_point} ${other_blocks} ${END_POINT} ${hit_miss_threshold} ${err_corr_iterations} ${hit_miss_threshold_for_err_corr} ${LLC_NUM_BLOCKS}

#1c. Copy to binary/ 
cp ../../bin/bimodal-no-no-random-2core binary/${unroll_fact}_${algo_trigger_point}_${benchmark_test}_${err_corr_iterations}

########## XXX Above lines generate receiver trace and binary as per the unroll_fact and algo_trigger_point. XXX  ############

#exit 1

binary_path=binary/${unroll_fact}_${algo_trigger_point}_${benchmark_test}_${err_corr_iterations}
#exit 1

sender_traces_directory="sender_err_corr_both_algo_sender_access_and_flush_multiple_blocks"
#result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks_for_high_UF/error_calculation/LR_experiments/LR_ERR_CORR_2core_results_both_algo_${err_corr_iterations}_access
result_dir=/home/yashikav/Desktop/Mirage_project/fig12/champsim_results
mkdir -p /home/yashikav/Desktop/Mirage_project/fig12/champsim_results
 
STR_NUM=${START_POINT}
echo "STR_NUM: $STR_NUM"

# Total Number of Experiments to run.
EXP_NUM=`expr \( ${END_POINT} - ${START_POINT} + 1 \) `

while [ ${EXP_NUM} -gt 0 ]
do
    #3. Run 'num_prl' experiments parallely.
    i=${num_prl}
    echo "Running parallel experiments"
    while [ ${i} -gt 0 ]
    do
            echo "i: $i STR_NUM: $STR_NUM EXP_NUM: $EXP_NUM"
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
            
            search_string1="champsim.trace_sender_${string_size}_${STR_NUM}_"
            
            if [ $benchmark_test == 0 ] 
            then
                search_string2="_${algo_trigger_point}_with_both_algo_${err_corr_iterations}_access_train"
            elif [ $benchmark_test == 1 ]
            then
                search_string2="_${algo_trigger_point}_with_both_algo_${err_corr_iterations}_access_test"
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

            receiver_trace=champsim.trace_receiver_multiple_access_${unroll_fact}_other_blocks_${other_blocks}_${string_size}_${algo_trigger_point}_with_both_algo_${err_corr_iterations}_algo_itr.gz
            receiver_trace_path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks_for_high_UF/error_calculation/LR_experiments/pin_traces/${receiver_trace}

            if [ $benchmark_test == 0 ] 
            then
                sender_trace=champsim.trace_sender_${string_size}_${STR_NUM}_${sim_count}_${algo_trigger_point}_with_both_algo_${err_corr_iterations}_access_train.gz 
            elif [ $benchmark_test == 1 ]
            then
                sender_trace=champsim.trace_sender_${string_size}_${STR_NUM}_${sim_count}_${algo_trigger_point}_with_both_algo_${err_corr_iterations}_access_test.gz 
            fi
            sender_trace_path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks_for_high_UF/error_calculation/LR_experiments/sender_err_corr_both_algo_sender_access_and_flush_multiple_blocks/${sender_trace}

            ./run_2core_LR_ERR_CORR_both_algo.sh $sim_count $receiver_trace_path $sender_trace_path $seed $string_size $result_dir $binary_path &

            # kill the process after 2 min.
            #sleep 60
            #process_name="bimodal-no-no-random-2core"
            #pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
            #echo "pid is: $pid"
            #kill -9 ${pid}
          
    #        while true; do
                # Define the process name
		#				    process_name="bimodal-no-no-"

								# Get the number of CPU cores
		#						num_cores=$(nproc)

								# Get the number of currently running processes with the specified name
		#						num_processes=$(pgrep -c "$process_name")
     #           echo "Currently running processes are: $num_processes"

								# Compare the number of running processes with the number of CPU cores
			#					if [ "$num_processes" -lt "$((num_cores - 1))" ]; then
			#							# Schedule new process if the condition is met
			#							echo ".......... Scheduling new process .........."
			#							break
			#					else
			#							# Sleep for 30 seconds if the condition is not met
			#							echo "........Waiting for available resources..."
			#							sleep 60
			#					fi
      #      done 
            STR_NUM=`expr ${STR_NUM} + 1`
            i=`expr ${i} - 1`
            EXP_NUM=`expr $EXP_NUM - 1`
            if [ $EXP_NUM == 0 ]
            then
                break
            fi
    done 
    wait
#    isr=`ps -el | grep run_2core_LR_ERR_CORR_both_algo.sh`
#    while [ "$isr" != "" ];
#    do
#         sleep 5
#         isr=`ps -el | grep run_2core_LR_ERR_CORR_both_algo.sh`
#    done
done
