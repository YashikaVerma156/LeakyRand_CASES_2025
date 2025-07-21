##################### This script is for SPP error calculation ###########################
sender_arr_size=$1
seed=$2
string_num=$3
bench_test=$4
build_2_core_binary=$5 
generate_receiver_trace=$6
LLC_size=$7
llc_sets=$8



#====== Constants ========#
blocks_in_1_MB=16384 # 1024 sets *16 ways = 1MB
string_size=512  #512
repetition_coding=1
differential_signalling=2
array_elements_in_one_cache_block=8
receiver_arr_size=$((blocks_in_1_MB * LLC_size))
echo "Total LLC blocks : $receiver_arr_size"
receiver_arr_size=$((receiver_arr_size * array_elements_in_one_cache_block))
echo "receiver_arr_size : $receiver_arr_size"
#exit 1

#====== Paths and file names ========#
pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_LLC_size_sensitivity
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4/seed_pin_traces
local_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4/
traces_directory="seed_pin_traces"
pin_output_file_receiver=receiver_seed.txt
#trace_sender=champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed.gz
trace1=champsim.trace_receiver_${string_size}_${LLC_size}_seed.gz
#result_file_2core=result_random_2_champsim.trace_receiver_${string_size}_seed.gz_champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed.gz.txt

#============= 1.a Generate sender traces ==============#
#cd ${pin_files_path}
#./commands_sender_all_1_seed_with_benchmark_suit.sh ${string_num} ${string_size} ${sender_arr_size} ${repetition_coding} ${differential_signalling} ${path_to_copy_trace}

#============= 1.b Extract the number of instructions in the trace file ==============#
#sim_count_sender=`grep "instrCount" ${pin_output_file_sender} | awk '{print $4}'`
#echo "Number of instructions traced for sender: ${sim_count_sender}"
#cd ${path_to_copy_trace}

search_string1="champsim.trace_sender_${string_size}_${string_num}_"

if [ $bench_test == 0 ]
then
		search_string2="_arr_size_${sender_arr_size}_seed_train"
elif [ $bench_test == 1 ]
then
		search_string2="_arr_size_${sender_arr_size}_seed_test"
fi
sim_count=0

# Use a loop to iterate through each file in the directory
for filename in $(ls "$traces_directory"); do

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

if [ $bench_test == 0 ]
then
    sender_trace=champsim.trace_sender_${string_size}_${string_num}_${sim_count}_arr_size_${sender_arr_size}_seed_train.gz
elif [ $bench_test == 1 ]
then
    sender_trace=champsim.trace_sender_${string_size}_${string_num}_${sim_count}_arr_size_${sender_arr_size}_seed_test.gz
fi


if [ $generate_receiver_trace -eq 1 ];
then
    #============= 2.a Generate receiver traces ==============#
    cd ${pin_files_path} 
    ./commands_receiver_all_1_seed.sh ${receiver_arr_size} ${string_size} ${repetition_coding} ${differential_signalling} ${path_to_copy_trace} ${LLC_size}

    #============= 2.b Extract the number of instructions in the trace file ==============#
    sim_count_receiver=`grep "instrCount" ${pin_output_file_receiver} | awk '{print $4}'`
    echo "Number of instructions traced for receiver: ${sim_count_receiver}"
    cd ${local_dir}
    ./run_build_1core_LLC_size_sensitivity.sh ${sim_count_receiver} ${trace1} ${seed} ${string_size} ${repetition_coding} ${differential_signalling} ${llc_sets} ${path_to_copy_trace} &

    #Sleep is needed so that the build can be finished.
    sleep 90   #60  #900
    process_name="./bin/bimodal-no-no-random-1core"
    pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
    #pid=$(ps -eo pid,lstart,cmd | grep "$process_name" | sort -rk 3,3 | head -1 | awk '{print $1}')
    echo "kill pid is: $pid"
    killing_status=`kill -9 ${pid}`
    echo "killing_status: ${killing_status}"
    sleep 10
 
fi

#============ 3. Run a 2-core simulation with wait_implementation off ============#
if [ $build_2_core_binary -eq 1 ];
then
    ./run_build_2core_LLC_sensitivity.sh ${sim_count} ${trace1} ${sender_trace} ${seed} ${string_size} 0 0 ${repetition_coding} ${differential_signalling} ${LLC_size} ${llc_sets} ${path_to_copy_trace} &

    #Sleep is needed so that the build can be finished.
    sleep 90   #60  #900
    process_name="./bin/bimodal-no-no-random-2core"
    pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
    #pid=$(ps -eo pid,lstart,cmd | grep "$process_name" | sort -rk 3,3 | head -1 | awk '{print $1}')
    echo "kill pid is: $pid"
    killing_status=`kill -9 ${pid}`
    echo "killing_status: ${killing_status}"
    sleep 10
 
    #Copy this binary to /binary folder
    cp ../../bin/bimodal-no-no-random-2core binary/ 
else
    ./run_2core.sh ${sim_count} ${trace1} ${sender_trace} ${seed} ${string_size} 0 0 ${repetition_coding} ${differential_signalling} ${sender_arr_size} ${path_to_copy_trace}
fi



#python3 collect_seed_results.py sender_arr_size_${sender_arr_size}/${result_file_2core} ${sender_arr_size} ${string_num} ${seed}


