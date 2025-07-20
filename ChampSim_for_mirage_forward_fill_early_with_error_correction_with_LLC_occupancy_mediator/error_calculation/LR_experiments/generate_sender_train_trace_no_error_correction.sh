##################### This script is for error calculation plot ###########################
string_num=$1
benchmark_test=$2
string_size=512
LR_no_error_corr=1


pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code
pin_output_file_sender=all_1_sender.txt
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments/sender_train_benchmark_traces

######### 1.a Extract seed number.  ############
line_num=`expr $string_num + 1` # Needed as there is a header in the beginning.

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
./commands_sender_LR_NO_ERR_CORR_with_train_benchmark_suit.sh ${string_num} ${string_size} ${benchmark_test} ${path_to_copy_trace}

########### 1.c Extract the number of instructions in the trace file.  ##############
sim_count_sender=`grep -oE "instrCount: [0-9]+" all_1_sender.txt | awk '{print $2}'`
cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments
echo "sim_count: ${sim_count_sender}, string size: ${string_size}, seed: ${seed}"

########### 2.a Run script for sender. ############
trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_train.gz
sed -i.bak 's/\<LR_NO_ERR_CORR 0\>/LR_NO_ERR_CORR '${LR_no_error_corr}'/g' ../../inc/champsim.h
./run_build_1core_to_generate_sender_train_traces.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed}

########### 2.b Extract LLC occupancy of sender
result_file=result_random_1_${trace_sender}.txt
lines_to_skip=`grep -n "Cache state before clflush" single_core_results/${result_file} | head -n 1 | awk -F ':' '{print $1}'`  #TODO this will not work, if all are zero in a string.
echo "lines_to_skip : ${lines_to_skip}"
buffer_lines=30
lines_to_skip=`expr $lines_to_skip + ${buffer_lines}`
echo "lines_to_skip : ${lines_to_skip}"
LLC_occupancy_of_sender=`cat single_core_results/${result_file} | head -n ${lines_to_skip} | tail -n ${buffer_lines} | grep "Tag LLC Valid blocks" | awk '{print $5}' | head -2 | tail -1`
echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}"  # Sender's occupancy will not change for a specific trace-file.

echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}" >> occupancy_tracker.txt

sed -i.bak 's/\<LR_NO_ERR_CORR '${LR_no_error_corr}'\>/LR_NO_ERR_CORR 0/g' ../../inc/champsim.h
