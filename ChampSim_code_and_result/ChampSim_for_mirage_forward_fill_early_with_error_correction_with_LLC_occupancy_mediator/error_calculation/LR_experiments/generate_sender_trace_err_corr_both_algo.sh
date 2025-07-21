##################### This script is for error calculation plot ###########################
string_num=$1
benchmark_test=$2
err_corr_interval=$3
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
Result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments/single_core_results
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments/sender_err_corr_both_algo

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

######### 1.b Generate pintrace for sender.  #############
cd ${pin_files_path}
./commands_sender_LR_ERR_CORR_both_algo_with_benchmark_suit.sh ${string_num} ${string_size} ${err_corr_interval} ${llc_occ_chk_at} ${path_to_copy_trace} ${benchmark_test}

########### 1.c Extract the number of instructions in the trace file.  ##############
sim_count_sender=`grep -oE "instrCount: [0-9]+" all_1_sender.txt | awk '{print $2}'`
cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments
echo "sim_count: ${sim_count_sender}, string size: ${string_size}, seed: ${seed}"

########### 2.a Run script for sender. ############
if [ ${benchmark_test} -eq 1 ];
then
    trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${err_corr_interval}_with_both_algo_test.gz
else
   trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${err_corr_interval}_with_both_algo_train.gz
fi

sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO 0\>/LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'/g' ../../inc/champsim.h
./run_build_1core_err_corr.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr} ${path_to_copy_trace} ${Result_dir}

########### 2.b Extract LLC occupancy of sender
result_file=result_random_1_${trace_sender}.txt
lines_to_skip=`grep -n "Cache state before clflush" ${Result_dir}/${result_file} | head -n 1 | awk -F ':' '{print $1}'`  #TODO this will not work, if all are zero in a string.
echo "lines_to_skip : ${lines_to_skip}"
buffer_lines=30
lines_to_skip=`expr $lines_to_skip + ${buffer_lines}`
echo "lines_to_skip : ${lines_to_skip}"
LLC_occupancy_of_sender=`cat ${Result_dir}/${result_file} | head -n ${lines_to_skip} | tail -n ${buffer_lines} | grep "Tag LLC Valid blocks" | awk '{print $5}' | head -2 | tail -1`
echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}"  # Sender's occupancy will not change for a specific trace-file.

echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}" >> occupancy_tracker_${err_corr_interval}_${benchmark_test}.txt

sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../../inc/champsim.h
