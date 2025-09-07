#### These three are to be varied to create different set-ups. ####

benchmark_test=$1
receiver_arr_size=$2
unroll_fact=$3
do_err_corr_at=$4  
other_blocks=$5
string_num=$6
hit_miss_threshold=$7
err_corr_iterations=$8  #XXX Defines multiple iterations of error correction. If we assume sender create multiple holes.
other_blocks_previous=$5

do_llc_occ_chk_at=0   #XXX This flag is not needed to be transferred separately as this is triggered along with the do_err_corr_at.

unroll_fact_for_llc_occ_chk=16 
string_size=512  #11   #512  
max_arr_len=0
extra=0
extra_for_llc_occ_chk=0
start_arr_addr=0
extra_arr_max_addr=0
LR_error_corr=1

# Check if exactly ten argument is passed
if [ "$#" -ne 8 ]; then
    echo "benchmark_test: ${benchmark_test}, receiver_arr_size: ${receiver_arr_size}, unroll_fact: ${unroll_fact}, do_err_corr_at: ${do_err_corr_at}, other_blocks: ${other_blocks}, string_num: ${string_num}, hit_miss_threshold: ${hit_miss_threshold}, err_corr_iterations: ${err_corr_iterations}"
    echo "Appropriate number of arguments not passed, exiting. $#"
    echo "Inside script_update_arr_high_UF_size_LLC_size_sensitivity.sh"
    exit 1
fi

LLC_NUM_BLOCKS=32768

line_number=`expr ${string_num} + 1`

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

mkdir -p results

### Calculating maximum increase in array after every round of error-correction.

number=`expr ${do_err_corr_at} - 1` # This is needed as we trigger error correction algo at that bit-position itself.
echo "number: $number string_size: ${string_size}"
max_inc_in_array=`expr ${string_size} / ${number}`
let max_inc_in_array="${max_inc_in_array} * ${err_corr_iterations}"  # max_inc_in_array calculation should be different for the champsim directory and for the pin directory i.e. for champsim directory the max_inc_in_array should be multiplied with err_corr_iterations but not in pin directory.
#In champsim directory max_inc_in_array decide how many maximum addresses are needed to be flushed. While in pin directory, max_inc_in_array decide how many total message bits to be transferred.

echo "max_inc_in_array: ${max_inc_in_array} "

#exit

sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../inc/config.h
sed -i.bak 's/\<NUM_SENDER_DISTURBANCES 0\>/NUM_SENDER_DISTURBANCES '${err_corr_iterations}'/g' ../inc/config.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'/g' ../inc/champsim.h
sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO 0\>/LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'/g' ../inc/champsim.h
sed -i.bak 's/\<DO_ERR_CORR_AT 0\>/DO_ERR_CORR_AT '${do_err_corr_at}'/g' ../inc/config.h

pin_files_path=/home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code

pin_output_file_sender=all_1_sender.txt
array_elements_in_one_cache_block=8
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_fa_lru_multithreaded_receiver_with_coherence_details/fa_lru_covert_channel/pin_traces
back_to_exp_dir=../../../../../ChampSim_for_mirage_fa_lru_multithreaded_receiver_with_coherence_details/fa_lru_covert_channel
Result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_fa_lru_multithreaded_receiver_with_coherence_details/fa_lru_covert_channel/results


######### 1. Generate pintrace for sender
cd ${pin_files_path} 
./commands_sender_fa_lru.sh ${string_num} ${string_size} ${do_err_corr_at} ${do_llc_occ_chk_at} ${path_to_copy_trace} ${benchmark_test} ${err_corr_iterations}


#### run single-core experiment to estimate sender's LLC occupancy ####

########### 1.a Extract the number of instructions in the trace file.
sim_count_sender=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_sender} | awk '{print $2}'`

echo "Number of instructions traced for sender: ${sim_count_sender}"

if [ ${benchmark_test} -eq 1 ];
then
    trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${do_err_corr_at}_with_both_algo_${err_corr_iterations}_access_test.gz
else
    trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${do_err_corr_at}_with_both_algo_${err_corr_iterations}_access_train.gz
fi

cd ${back_to_exp_dir}

echo "${max_arr_len} ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr}"


########### 2. Run script for sender
./run_build_1core_err_corr.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr} ${path_to_copy_trace} ${Result_dir}


########### 2.a) Extract LLC occupancy of sender
#result_file=`ls -lthr | tail -1 | awk '{ print $9 }'`
result_file=result_lru_1_${trace_sender}.txt
result_file_path=results/${result_file}
start_line=`grep -n "cpu 0 is on wait" ${result_file_path} | head -n 3 | tail -n 1 | awk -F ":" '{print $1}'`
end_line=`grep -n "cpu 0 is on wait" ${result_file_path} | head -n 4 | tail -n 1 | awk -F ":" '{print $1}'`
echo "start_line: $start_line end_line: $end_line"
LLC_occupancy_of_sender=`sed -n "${start_line},${end_line}p" ${result_file_path} | grep "Tag LLC Valid blocks" | tail -n 1 | awk '{print $5}'` #surround variables name with double quotes.
echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}"  # Sender's occupancy will not change for a specific trace-file.

#sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../inc/config.h
#sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../inc/config.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../inc/champsim.h
#sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../inc/champsim.h
#sed -i.bak 's/\<DO_ERR_CORR_AT '${do_err_corr_at}'\>/DO_ERR_CORR_AT 0/g' ../inc/config.h

#exit

#sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../inc/config.h
#sed -i.bak 's/\<NUM_SENDER_DISTURBANCES 0\>/NUM_SENDER_DISTURBANCES '${err_corr_iterations}'/g' ../inc/config.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'/g' ../inc/champsim.h
#sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO 0\>/LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'/g' ../inc/champsim.h
#sed -i.bak 's/\<DO_ERR_CORR_AT 0\>/DO_ERR_CORR_AT ${do_err_corr_at}/g' ../inc/config.h

######### 3. Run script for receiver

receiver_num_access_in_rdtsc=${unroll_fact}
  echo "======================= starting for unrolling factor ${receiver_num_access_in_rdtsc} ======================="

pin_output_file_receiver_prime_probe=1_multiple_access_${receiver_num_access_in_rdtsc}_for_all_1_prime_probe.txt
trace_receiver_prime_probe=champsim.trace_receiver_fa_lru_prime_probe_final.gz
trace_receiver_helper=champsim.trace_receiver_fa_lru_helper_final.gz

LLC_occupancy_of_receiver=0
Total=`expr $LLC_occupancy_of_receiver + $LLC_occupancy_of_sender`
echo "line 51, Total: $Total"

cd ${pin_files_path} 
./commands_receiver_fa_lru_multithreaded_with_helper_final.sh ${receiver_num_access_in_rdtsc} ${receiver_arr_size} ${string_size} ${path_to_copy_trace}
sim_count_receiver=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver_prime_probe} | awk '{print $2}'`
echo "Number of instructions traced for receiver: ${sim_count_receiver}"
cd ${back_to_exp_dir}
./run_build_1core_fa_lru.sh ${sim_count_receiver} ${trace_receiver_prime_probe} ${string_size} ${seed} ${path_to_copy_trace} ${Result_dir} &
echo "Completed till here."  
# kill the process after a minute.
sleep 30
process_name="bimodal-no-no-lru-1core"
pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
echo "pid is: $pid"
killing_status=`kill -9 ${pid}`
echo "killing_status: ${killing_status}"

cd ${pin_files_path} 
sim_count_receiver=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver_prime_probe} | awk '{print $2}'`
echo "Number of instructions traced for receiver: ${sim_count_receiver}"
cd ${back_to_exp_dir}
./run_build_1core_fa_lru.sh ${sim_count_receiver} ${trace_receiver_helper} ${string_size} ${seed} ${path_to_copy_trace} ${Result_dir} &
echo "Completed till here."  
# kill the process after a minute.
sleep 30
process_name="bimodal-no-no-lru-1core"
pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
echo "pid is: $pid"
killing_status=`kill -9 ${pid}`
echo "killing_status: ${killing_status}"

#### Update arr_size in the text file.
./update_arr_size.sh ${unroll_fact} ${do_err_corr_at} ${err_corr_iterations} ${other_blocks_previous} ${receiver_arr_size}

echo "Receiver LLC occupancy: ${LLC_occupancy_of_receiver}, Sender LLC occupancy: ${LLC_occupancy_of_sender} extra_for_llc_occ_chk: ${extra_for_llc_occ_chk}" # Adjust Receiver occupancy, don't change Sender occupancy.

result_file=result_lru_1_${trace_receiver}.txt
receiver_setup_wait=`grep -nri "wait" ${result_file_path} | head -n 1 | awk '{ print $12 }'`

echo "receiver_setup_wait ${receiver_setup_wait}"


#Comment below lines when you remove exit below.
#sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../inc/config.h
#sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../inc/config.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../inc/champsim.h
#sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../inc/champsim.h
#sed -i.bak 's/\<DO_ERR_CORR_AT '${do_err_corr_at}'\>/DO_ERR_CORR_AT 0/g' ../inc/config.h

#exit

#### Run build 3-core simulation system.
#./run_build_3core_err_corr.sh ${sim_count_sender} ${trace_receiver_prime_probe} ${trace_sender} ${trace_receiver_helper} ${seed} ${string_size} ${path_to_copy_trace} ${Result_dir} &
./run_build_3core_err_corr.sh ${sim_count_sender} ${trace_receiver_prime_probe} ${trace_sender} ${trace_receiver_helper} ${seed} ${string_size} ${path_to_copy_trace} ${Result_dir} &

sleep 15



#Comment below lines when you remove exit below.
sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../inc/config.h
sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../inc/config.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../inc/champsim.h
sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../inc/champsim.h
sed -i.bak 's/\<DO_ERR_CORR_AT '${do_err_corr_at}'\>/DO_ERR_CORR_AT 0/g' ../inc/config.h

exit
######### Run a 2-core simulation with wait_implementation off.

./run_build_2core_err_corr.sh ${sim_count_sender} ${trace_receiver} ${trace_sender} ${seed} ${string_size} ${path_to_copy_trace} ${Result_dir} &
result_file=result_lru_2_${trace_receiver}_${trace_sender}.txt
# kill the process after 15 min.
#Sleep is needed so that the build can be finished.
sleep 60   #60  #900
process_name="./bin/bimodal-no-no-lru-2core"
pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
#pid=$(ps -eo pid,lstart,cmd | grep "$process_name" | sort -rk 3,3 | head -1 | awk '{print $1}')
echo "Kill pid is: $pid"
#killing_status=`kill -9 ${pid}`   # Caution: Enabling this kill command will kill all the 2-core experiments.
echo "killing_status: ${killing_status}"

sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../inc/config.h
sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../inc/config.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../inc/champsim.h
sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../inc/champsim.h
sed -i.bak 's/\<DO_ERR_CORR_AT '${do_err_corr_at}'\>/DO_ERR_CORR_AT 0/g' ../inc/config.h

