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
pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_code_sender_receiver_sender_arr_size_${sender_arr_size}
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4
pin_output_file_sender=all_1_sender_seed.txt
pin_output_file_receiver=receiver_seed.txt
trace_sender=champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed.gz
trace_receiver=champsim.trace_receiver_${string_size}_seed.gz
trace1=champsim.trace_receiver_${string_size}_seed.gz
trace2=champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed.gz
result_file_2core=result_random_2_champsim.trace_receiver_${string_size}_seed.gz_champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed_${seed}.gz.txt

#============= 1.a Generate sender traces ==============#
cd ${pin_files_path}
# XXX Don't generate the sender trace again. Just extract the seed value. Hence commented the below command.
#./commands_sender_all_1_seed.sh ${string_num} ${string_size} ${sender_arr_size} ${repetition_coding} ${differential_signalling} ${path_to_copy_trace}  

#============= 1.b Extract the number of instructions in the trace file ==============#
sim_count_sender=`grep "instrCount" ${pin_output_file_sender} | awk '{print $4}'`
echo "Number of instructions traced for sender: ${sim_count_sender}"
cd ${path_to_copy_trace}

#============= 2.a Generate receiver traces ==============#
#cd ${pin_files_path} 
#./commands_receiver_all_1_seed.sh ${receiver_arr_size} ${string_size} ${repetition_coding} ${differential_signalling} ${path_to_copy_trace}

#============= 2.b Extract the number of instructions in the trace file ==============#
#sim_count_receiver=`grep "instrCount" ${pin_output_file_receiver} | awk '{print $4}'`
#echo "Number of instructions traced for receiver: ${sim_count_receiver}"
#cd ${path_to_copy_trace} 

#============ 3. Run a 2-core simulation with wait_implementation off ============#
./run_2core_all1s.sh ${sim_count_sender} ${trace1} ${trace2} ${seed} ${string_size} 0 0 ${repetition_coding} ${differential_signalling} ${sender_arr_size} ${path_to_copy_trace}
#python3 collect_seed_results.py sender_arr_size_${sender_arr_size}/${result_file_2core} ${sender_arr_size} ${string_num} ${seed}

