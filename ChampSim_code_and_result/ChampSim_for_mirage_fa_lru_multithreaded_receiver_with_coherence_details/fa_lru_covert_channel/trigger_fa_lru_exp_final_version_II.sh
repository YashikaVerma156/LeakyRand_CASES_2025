unroll_fact=$1    ## Specify unrolling factor
START_POINT=$2   ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
END_POINT=$3
benchmark_test=$4  ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
string_size=$5   # Message Length.
arr_size=$6
algo_trigger_point=$7
hit_miss_threshold=$8
err_corr_iterations=$9


pin_files_path=/home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code
pin_output_file_sender=all_1_sender.txt
array_elements_in_one_cache_block=8
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_fa_lru_multithreaded_receiver_with_coherence_details/fa_lru_covert_channel/pin_traces
back_to_exp_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_fa_lru_multithreaded_receiver_with_coherence_details/fa_lru_covert_channel
Result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_fa_lru_multithreaded_receiver_with_coherence_details/fa_lru_covert_channel/results_with_helper_core

# Generate sender trace.
string_num=${START_POINT}

sed -i.bak 's/\<NUM_SENDER_DISTURBANCES 0\>/NUM_SENDER_DISTURBANCES '${err_corr_iterations}'/g' ../inc/config.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../inc/champsim.h

while [ ${string_num} -le ${END_POINT} ]
do
		######### 1. Generate pintrace for sender
		cd ${pin_files_path} 
		./commands_sender_fa_lru.sh ${string_num} ${string_size} ${algo_trigger_point} 0 ${path_to_copy_trace} ${benchmark_test} ${err_corr_iterations}
		########### 1.a Extract the number of instructions in the trace file.
		sim_count_sender=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_sender} | awk '{print $2}'`
		echo "Number of instructions traced for sender: ${sim_count_sender}"
		cd ${back_to_exp_dir}

    string_num=`expr ${string_num} + 1` 
done 

sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../inc/config.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../inc/champsim.h

# Run the 2-core experiment.
string_num=${START_POINT}
while [ ${string_num} -le ${END_POINT} ]
do
    trace_receiver_prime_probe=champsim.trace_receiver_fa_lru_prime_probe_final_version_II.gz
    trace_receiver_helper=champsim.trace_receiver_fa_lru_helper_final_version_II.gz
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

		search_string1="champsim.trace_sender_${string_size}_${string_num}_"
		if [ $benchmark_test == 0 ]
		then
				search_string2="_${algo_trigger_point}_with_both_algo_${err_corr_iterations}_access_train"
		elif [ $benchmark_test == 1 ]
		then
				search_string2="_${algo_trigger_point}_with_both_algo_${err_corr_iterations}_access_test"
		fi

		# Use a loop to iterate through each file in the directory
		for filename in $(ls "$path_to_copy_trace"); do

				# Check if the filename contains the search string
				if [[ $filename == *"$search_string1"* && $filename == *"$search_string2"* ]]; then
						echo "Found match: $filename"

						# Split the filename using both '_' and '.' as delimiters
						IFS='_.' read -ra array <<< "$filename"

						# Extract the desired part (index 5 in this case)
						sim_count_sender="${array[5]}"
						break
				fi
		done

		if [ ${benchmark_test} -eq 1 ];
		then
				trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${algo_trigger_point}_with_both_algo_${err_corr_iterations}_access_test.gz
		else
				trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${algo_trigger_point}_with_both_algo_${err_corr_iterations}_access_train.gz
		fi

    echo "trace_sender: $trace_sender"

		######### Run a 3-core simulation with wait_implementation off.
		#./run_3core.sh ${sim_count_sender} ${trace_receiver} ${trace_sender} ${seed} ${string_size} ${path_to_copy_trace} ${Result_dir} & 

    ./run_3core.sh ${sim_count_sender} ${trace_receiver_prime_probe} ${trace_sender} ${trace_receiver_helper} ${seed} ${string_size} ${path_to_copy_trace} ${Result_dir} &

wait    
    string_num=`expr ${string_num} + 1` 
done 

#sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../inc/config.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../inc/champsim.h
