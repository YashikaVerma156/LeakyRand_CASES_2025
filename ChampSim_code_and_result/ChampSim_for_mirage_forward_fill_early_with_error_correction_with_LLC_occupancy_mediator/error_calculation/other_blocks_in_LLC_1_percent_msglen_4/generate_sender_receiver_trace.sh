##################### This script is for SPP error calculation ###########################
sender_arr_size=$1 
string_num_start=$2
string_num_end=$3

#====== Constants ========#
receiver_arr_size=262144 
string_size=512  #512
repetition_coding=1
differential_signalling=2
array_elements_in_one_cache_block=8

#====== Paths and file names ========#
pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_code_sender_receiver_sender_arr_size_${sender_arr_size}
back_path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4/
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4/sender_${sender_arr_size}_pin_trace
pin_output_file_sender=all_1_sender_seed.txt
pin_output_file_receiver=receiver_seed.txt

mkdir -p sender_${sender_arr_size}_pin_trace

i=${string_num_end}

while [ $i -ge ${string_num_start} ]
do
    string_num=$i
    #============= 1.a Generate sender traces ==============#
    cd ${pin_files_path} 
    ./commands_sender_all_1_seed_with_benchmark_suit.sh ${string_num} ${string_size} ${sender_arr_size} ${repetition_coding} ${differential_signalling} ${path_to_copy_trace}  

    #============= 1.b Extract the number of instructions in the trace file ==============#
    sim_count_sender=`grep "instrCount" ${pin_output_file_sender} | awk '{print $4}'`
    echo "Number of instructions traced for sender: ${sim_count_sender}"
    cd ${back_path} 

    i=`expr $i - 1`
done
#============= 2.a Generate receiver traces ==============#
cd ${pin_files_path} 
./commands_receiver_all_1_seed.sh ${receiver_arr_size} ${string_size} ${repetition_coding} ${differential_signalling} ${path_to_copy_trace}

#============= 2.b Extract the number of instructions in the trace file ==============#
sim_count_receiver=`grep "instrCount" ${pin_output_file_receiver} | awk '{print $4}'`
echo "Number of instructions traced for receiver: ${sim_count_receiver}"

cd ${back_path} 

./build_2core.sh ${repetition_coding} ${differential_signalling} 
mkdir -p binary_with
cp ../../bin/bimodal-no-no-random-2core binary_with/ 
