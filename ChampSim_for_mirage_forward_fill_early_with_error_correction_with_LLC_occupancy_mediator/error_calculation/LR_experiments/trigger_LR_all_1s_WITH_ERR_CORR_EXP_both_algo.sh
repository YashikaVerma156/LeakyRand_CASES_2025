#############################################################################################
# 1. This script is to launch LR experiments, without error correction.                     #
#    Total 'itr' iterations, each runs 'num_prl' LR experiments in parallel.                #
#    User should decide 'num_prl' by the number of cores in the system.                     #
# 2. This script is called from calibration_and_variables_all_1s_without_with_err_corr.sh   #
#############################################################################################

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

echo "END_POINT: $END_POINT benchmark_test: ${benchmark_test} arr_size: ${arr_size} unroll_fact: ${unroll_fact} algo_trigger_point: ${algo_trigger_point} other_blocks: ${other_blocks} threshold: ${hit_miss_threshold}"

########### XXX Below lines generate receiver trace and binary as per the unroll_fact and algo_trigger_point. XXX  ############
#1a. Remove older binaries from bin/
rm ../../bin/bimodal-no-no-random*

#1b. Build 2-core LR binary, keeping a check on calibration constraints.
./script_for_all_1s_err_corr_both_algo_2core.sh ${benchmark_test} ${arr_size} ${unroll_fact} ${algo_trigger_point} ${other_blocks} ${END_POINT} ${hit_miss_threshold}

#1c. Copy to binary/ 
cp ../../bin/bimodal-no-no-random-2core binary/${unroll_fact}_${algo_trigger_point}_${benchmark_test}
########## XXX Above lines generate receiver trace and binary as per the unroll_fact and algo_trigger_point. XXX  ############

exit

binary_path=binary/${unroll_fact}_${algo_trigger_point}_${benchmark_test}

sender_traces_directory="sender_all_1s_err_corr_both_algo"
result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments/LR_NO_ERR_CORR_2core_results/LR_all_1s_WITHOUT_WITH_ERR_CORR_2core_results
 
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
            
            search_string1="champsim.trace_sender_512_all1s_"
            search_string2="_${algo_trigger_point}_with_both_algo"
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
						sender_trace=champsim.trace_sender_${string_size}_all1s_${sim_count}_${algo_trigger_point}_with_both_algo.gz 
            sender_trace_path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments/${sender_trace}

            ./run_2core_LR_ERR_CORR_both_algo_all1s.sh $sim_count $receiver_trace_path $sender_trace_path $seed $string_size $result_dir $binary_path $STR_NUM $benchmark_test &

            # kill the process after 2 min.
            #sleep 60
            #process_name="bimodal-no-no-random-2core"
            #pid=`ps aux | grep $process_name | head -1 | awk '{print $2 }'`
            #echo "pid is: $pid"
            #kill -9 ${pid}

            STR_NUM=`expr ${STR_NUM} + 1`
            i=`expr ${i} - 1`
            EXP_NUM=`expr $EXP_NUM - 1`
            if [ $EXP_NUM == 0 ]
            then
                break
            fi
    done 
    wait
done
