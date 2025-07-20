##################### This script is for error calculation plot ###########################
string_num=$1
benchmark_test=$2
err_corr_interval=$3
iteration_num=$4
err_corr_iterations=$5   #XXX Defines multiple iterations of error correction. If we assume sender create multiple holes.

unroll_fact_for_llc_occ_chk=16
unroll_fact=16
string_size=512
llc_occ_chk_at=0
extra=0
extra_for_llc_occ_chk=0
start_arr_addr=0 
max_arr_len=0 
extra_arr_max_addr=0 
LR_error_corr=1


pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code
pin_output_file_sender=all_1_sender.txt
Result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks_for_high_UF/error_calculation/LR_experiments/single_core_results
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks_for_high_UF/error_calculation/LR_experiments/sender_err_corr_both_algo_sender_access_and_flush_multiple_blocks

######### 1.a Extract seed number.  ############
line_num=`expr $string_num + 1` # Needed as there is a header in the beginning in the text file.

if [ ${benchmark_test} == 1 ]; then
    l=`sed -n "${line_num}p" seeds_test.txt`
    IFS=',' read -ra columns <<< "$l"
    seed=${columns[32]}

elif [ ${benchmark_test} == 0 ]; then
    l=`sed -n "${line_num}p" seeds_train.txt`
    IFS=',' read -ra columns <<< "$l"
    seed=${columns[32]}
fi


### Calculating maximum increase in array after every round of error-correction.

number=`expr ${err_corr_interval} - 1` # This is needed as we trigger error correction algo at that bit-position itself.
echo "number: $number string_size: ${string_size}"
max_inc_in_array=`expr ${string_size} / ${number}`
let max_inc_in_array="${max_inc_in_array} * ${err_corr_iterations}"  

echo "max_inc_in_array: ${max_inc_in_array} "



######### 1.b Generate pintrace for sender.  #############
cd ${pin_files_path}
./commands_sender_LR_ERR_CORR_both_algo_with_benchmark_suit_multiple_access_optimized.sh ${string_num} ${string_size} ${err_corr_interval} ${llc_occ_chk_at} ${path_to_copy_trace} ${benchmark_test} ${err_corr_iterations}

########### 1.c Extract the number of instructions in the trace file.  ##############
sim_count_sender=`grep -oE "instrCount: [0-9]+" all_1_sender.txt | awk '{print $2}'`
cd ../../../../../ChampSim_for_mirage_sender_access_and_flush_multiple_blocks_for_high_UF/error_calculation/LR_experiments
echo "sim_count: ${sim_count_sender}, string size: ${string_size}, seed: ${seed}"

########### 2.a Run script for sender. ############
if [ ${benchmark_test} -eq 1 ];
then
    trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${err_corr_interval}_with_both_algo_${err_corr_iterations}_access_test.gz
else
   trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${err_corr_interval}_with_both_algo_${err_corr_iterations}_access_train.gz
fi


sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../../inc/config.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../../inc/champsim.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK 0\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'/g' ../../inc/champsim.h
sed -i.bak 's/\<NUM_SENDER_DISTURBANCES 0\>/NUM_SENDER_DISTURBANCES '${err_corr_iterations}'/g' ../../inc/config.h
sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'/g' ../../inc/champsim.h
sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO 0\>/LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'/g' ../../inc/champsim.h


if [ ${iteration_num} -eq 0 ];
then
		./run_build_1core_err_corr.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr} ${path_to_copy_trace} ${Result_dir}
else
		./run_1core_err_corr.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr} ${path_to_copy_trace} ${Result_dir}
fi

########### 2.b Extract LLC occupancy of sender
result_file=result_random_1_${trace_sender}.txt

start_line=`grep -n "cpu 0 is on wait" ${Result_dir}/${result_file} | head -n 3 | tail -n 1 | awk -F ":" '{print $1}'`
end_line=`grep -n "cpu 0 is on wait" ${Result_dir}/${result_file} | head -n 4 | tail -n 1 | awk -F ":" '{print $1}'`
echo "start_line: $start_line end_line: $end_line"
LLC_occupancy_of_sender=`sed -n "${start_line},${end_line}p" ${Result_dir}/${result_file} | grep "Tag LLC Valid blocks" | tail -n 1 | awk '{print $5}'` #surround variables name with double quotes.
echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}"  # Sender's occupancy will not change for a specific trace-file.

echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}" >> occupancy_tracker_${err_corr_interval}_${err_corr_iterations}_${benchmark_test}.txt

sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../../inc/champsim.h
sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h
sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../../inc/config.h
sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../../inc/champsim.h
