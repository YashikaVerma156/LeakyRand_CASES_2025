##################### This script is to generate SPP sender traces ###########################
sender_arr_size=$1
start_point=$2
stop_point=$3
bench_test=$4
llc_sets=$5
llc_size=$6
seed=$7

#====== Constants ========#
string_size=512  #512
repetition_coding=1
differential_signalling=1

#====== Paths and file names ========#
pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_without_differential_signalling
local_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4/
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4/seed_pin_traces
pin_output_file_sender=all_1_sender_seed.txt

i=$stop_point
test_the_trace=1  # Enable it if you want to build the champsim binary and run the trace.

while [ ${i} -ge $start_point ]
do
    echo "string_num: $i"
		cd ${pin_files_path}

		./commands_sender_all_1_seed_with_benchmark_suit.sh $i ${string_size} ${sender_arr_size} ${repetition_coding} ${differential_signalling} ${path_to_copy_trace} ${bench_test}

		sim_count=`grep "instrCount" ${pin_output_file_sender} | awk '{print $4}'`
		echo "Number of instructions traced for sender: ${sim_count}"
    
		cd ${local_dir}

    if [ $bench_test == 0 ]
    then
        trace_sender=champsim.trace_sender_${string_size}_${i}_${sim_count}_arr_size_${sender_arr_size}_no_diff_sig_seed_train.gz
    elif [ $bench_test == 1 ]
    then
        trace_sender=champsim.trace_sender_${string_size}_${i}_${sim_count}_arr_size_${sender_arr_size}_no_diff_sig_seed_test.gz
    fi

    if [ $test_the_trace -eq 1 ];
    then
        ./run_build_1core_without_diff_sig.sh ${sim_count} ${trace_sender} ${seed} ${string_size} ${repetition_coding} ${differential_signalling} ${llc_sets} ${path_to_copy_trace} 
    fi    

		i=`expr ${i} - 1`
done
