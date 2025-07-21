arr_size_multiples_max=$1
num_iterations_max=6
num_iterations_max_1=$2
rand_seed_iterations_max=$3
inc_counter=0.1
built_binary_once=0
num_iterations=1  
num_iterations_1=1 

parallel_processes=8
process_count=1

if [ $# -ne 3 ]; then
    echo "The command-line arguments should be equal to 3. order should be arr_size_multiples_max, num_iterations_max_1, rand_seed_iterations_max"
    exit 0
else
    echo "The command-line arguments are three."
    echo "arr_size_multiples_max: ${arr_size_multiples_max} , num_iterations_max: ${num_iterations_max}, num_iterations_max_1: ${num_iterations_max_1} , rand_seed_iterations_max: ${rand_seed_iterations_max} "
    #exit 0
fi


mkdir -p clear_cache_result_files # no error if the directory already exists.
rm -f clear_cache_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt  #Force remove everything inside this directory.

mkdir -p temp_binaries/binary_path_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}

echo "arr_size_multiples,rand_seed,num_iterations_1x,num_iterations_extra_%,LLC_OCCUPANCY_after_1x,LLC_OCCUPANCY_after_%,Total_cycles,LLC_Filling_cycles,Thrashing_cycles,Probe_cycles" >> clear_cache_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt #; done    
#exit

binary_path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning/temp_binaries/binary_path_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}

#Iterate the loop num_iterations_max_1 times.
while [[ ${num_iterations_1} -lt ${num_iterations_max_1} ]]
do
		arr_size_multiples=1

    while [ $(bc <<< "$arr_size_multiples < $arr_size_multiples_max") -eq 1 ] 
    do
				# Print the values
				echo "arr_size_multiples: ${arr_size_multiples}, num_iterations: ${num_iterations_max}, num_iterations_1: ${num_iterations_1} "

				rand_seed_iteration=1

				#Iterate the loop rand_seed_iterations_max times.
				while [[ ${rand_seed_iteration} -lt ${rand_seed_iterations_max} ]]
				do
            LOG_FILE_NAME="temp_"$arr_size_multiples"_"$num_iterations_max"_"$num_iterations_1"_"$rand_seed_iterations".txt"
            #rand_seed=$((RANDOM % 100000 + 1))
						#echo "rand_seed : ${rand_seed}"

           
            # Build the binary only once.
						if [ ${built_binary_once} -lt 1 ]; then
								 # build the binary
                 ./build_1core_to_run_multiple_programs.sh 0 0 0 0
                 
                 #Copy binary to other path.
                 cp ../bin/bimodal-no-no-random-1core temp_binaries/binary_path_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}                
 
								 # increment the value
								 built_binary_once=`expr ${built_binary_once} + 1`
						fi

            if [ ${process_count} -lt ${parallel_processes} ]; then
            
                ./clear_cache_slave_script_mix_1x_and_a_percentage_of_array_run_multiple_program.sh "$arr_size_multiples" "$num_iterations_max" "$num_iterations_1" "$rand_seed_iterations" "$binary_path" > "$LOG_FILE_NAME" 2>&1 &
						    process_count=`expr ${process_count} + 1`

            else
                ./clear_cache_slave_script_mix_1x_and_a_percentage_of_array_run_multiple_program.sh "$arr_size_multiples" "$num_iterations_max" "$num_iterations_1" "$rand_seed_iterations" "$binary_path" > "$LOG_FILE_NAME" 
                process_count=1

            fi

						# increment the variable that keeps count of seeds.
						rand_seed_iteration=`expr ${rand_seed_iteration} + 1`
				done


				# increment the arr_size value
        arr_size_multiples=`bc -l<<< " ${arr_size_multiples} + ${inc_counter}"`  # expr command does not work on floating point numbers.

		done

  	# increment the value
		num_iterations_1=`expr ${num_iterations_1} + 1`

done

## Wait for completion of all experiments.
wait
#exp_count=`ps aux | grep -i "clear_cache_slave_script_mix_1x_and_a_percentage_of_array" | grep -v "grep" | wc -l`
#while [ $exp_count -gt 0 ]
#do
#    sleep 30
#    exp_count=`ps aux | grep -i "clear_cache_slave_script_mix_1x_and_a_percentage_of_array" | grep -v "grep" | wc -l`
#    echo "waiting in sleep"
#done

### Copy the result back to the original text file.

result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning/clear_cache_result_files/temp_result_files

num_iterations_1=1

#Iterate the loop num_iterations_max_1 times.
while [[ ${num_iterations_1} -lt ${num_iterations_max_1} ]]
do
    arr_size_multiples=1

    while [ $(bc <<< "${arr_size_multiples} < ${arr_size_multiples_max}") -eq 1 ]; do

				### TODO copy the result back to original text file. rand_seed_iteration loop cant be run here.

        # Get the list of files whose name starts with abc_def_
				files=$(ls "$result_dir" | grep "clear_cache_stats_mix_1x_and_a_percentage_of_array_${arr_size_multiples}_${num_iterations_max}_${num_iterations_1}_${rand_seed_iterations}")

				# Loop through the files
				for file in $files; do

					# Read the file
					line=$(tail -n 1 ${result_dir}/${file})

					# Print the line
					echo "$line" >> clear_cache_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt
				done

        # increment the arr_size value
        arr_size_multiples=`bc -l<<< " ${arr_size_multiples} + ${inc_counter}"`  # expr command does not work on floating point numbers.

    done

    # increment the value
    num_iterations_1=`expr ${num_iterations_1} + 1`

done
