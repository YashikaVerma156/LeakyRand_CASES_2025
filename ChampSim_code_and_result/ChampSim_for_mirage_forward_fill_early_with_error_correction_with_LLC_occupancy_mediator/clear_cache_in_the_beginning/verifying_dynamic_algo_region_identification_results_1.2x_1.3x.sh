. function_header_file.sh

#count_total_mem_src_access_btwn_two_lines 2 7

arr_size_multiples_max=1.3          #$1
num_iterations_max=1              #$2
rand_seed_iterations_max=1        #$3
#counter2=(1 2 3 4 5 6 7)
#counter3=(7 6 5 4 3 2 1)

counter2=(5) #1 2 3 4)
counter3=(2) #6 5 4 3)

inc_counter=0.1
built_binary_once=0
exp=0
num_iterations_start=1
arr_size_multiples_start=1.3

if [ $# -ne 3 ]; then
    echo "The command-line arguments should be equal to 3. order should be arr_size_multiples_max, num_iterations_max, rand_seed_iterations_max"
#    exit 0
else
    echo "The command-line arguments are three."
    echo "arr_size_multiples_max: ${arr_size_multiples_max} , num_iterations_max: ${num_iterations_max} , rand_seed_iterations_max: ${rand_seed_iterations_max} "
fi


mkdir -p clear_cache_result_files # no error if the directory already exists.
rm -f clear_cache_result_files/clear_cache_stats_verify_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_${#counter2[@]}_${#counter3[@]}_new.txt  #Force remove everything inside this directory.

### Setting headers in the result file.
# -n removes newline character from the end.
echo -n "arr_size_multiples,2x,3x,rand_seed,num_iterations,Total_cycles,LLC_Filling_cycles," >> clear_cache_result_files/clear_cache_stats_verify_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_${#counter2[@]}_${#counter3[@]}_new.txt
#Iterate the loop over the number of

sum=`expr ${counter2[0]} + ${counter3[0]}`
while [[ ${exp} -lt ${sum} ]]
do
    echo -n "LLC_OCCUPANCY_round${exp},Thrashing_cycles_round${exp}," >> clear_cache_result_files/clear_cache_stats_verify_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_${#counter2[@]}_${#counter3[@]}_new.txt
    # increment the value
    exp=`expr ${exp} + 1`
done

echo "Probe_cycles" >> clear_cache_result_files/clear_cache_stats_verify_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_${#counter2[@]}_${#counter3[@]}_new.txt

### Setting headers in the result file completed.

path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

num_iterations=${num_iterations_start}  #1
exp=0

echo "${#counter2[@]}"

#Iterate the loop over the number of
while [[ ${exp} -lt ${#counter2[@]} ]]
do
    echo "exp is $exp"
    num_iterations=${num_iterations_start} 
#Iterate the loop num_iterations_max times.
while [[ ${num_iterations} -le ${num_iterations_max} ]]
do

		arr_size_multiples=${arr_size_multiples_start}

		#Iterate the loop arr_size_multiples_max times.
 		##while [[ ${arr_size_multiples} -lt ${arr_size_multiples_max} ]] # XXX bc should be used for floating point variables.

    while [ $(bc <<< "${arr_size_multiples} <= ${arr_size_multiples_max}") -eq 1 ]; do

				# Print the values
				echo "arr_size_multiples: ${arr_size_multiples} , num_iterations: ${num_iterations} "

				cd ../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples 
				./clear_cache_verifying_dynamic_algo_region_identification_results_1.2x_1.3x.sh ${arr_size_multiples} ${num_iterations} ${path} ${counter2[${exp}]} ${counter3[${exp}]}

				sim=`grep -nr "Count_instr:" clear_cache.txt | awk '{ print $2 }'`
				echo "sim is: ${sim}"

				cd ../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

				rand_seed_iteration=1
				llc_occ_value=()
				total_cycle=()
				access_cycle=()

				#Iterate the loop rand_seed_iterations_max times.
				while [[ ${rand_seed_iteration} -le ${rand_seed_iterations_max} ]]
				do
            rand_seed=$((RANDOM % 100000 + 1))
						echo "rand_seed : ${rand_seed}"

						i=0
						# Print the values

            trace=champsim.trace_clear_cache_${arr_size_multiples}_${counter2[$exp]}_${counter3[$exp]}.gz           

            # Build the binary only once.
						if [ ${built_binary_once} -lt 1 ]; then
                 ./build_1core.sh ${sim} ${trace} 0 ${rand_seed} 0 0 0 0

								 # increment the value
								 built_binary_once=`expr ${built_binary_once} + 1`
						fi

						./run_1core.sh ${sim} ${trace} 0 ${rand_seed} 0 0 0 0
						
						tot_cycle=`grep -nri "Finished CPU 0 instructions:" result_random_1_${trace}.txt | awk '{print $7 }'`
						#echo "total cycles: ${tot_cycle}"

						# Instr_id of first fence.
						a=`grep -nri "fence: 1" result_random_1_${trace}.txt | head -1 | awk '{ print $11}'` 
						# Retirement cycle of first fence.
						LLC_filling_cycle=`grep -nr "Retiring instr_id: ${a} " result_random_1_${trace}.txt | head -1 | awk '{print $10}'`

            max_num=$(( ${counter2[0]} + ${counter3[0]} ))
						val=`grep -nri "Calculat" result_random_1_${trace}.txt | head -${max_num} | awk '{print $8 }'`					
            readarray -t occ_array <<< "$val"

            thrash_cycl=`grep -n "fence: 1" result_random_1_${trace}.txt | grep -n "Retiring" | head -9 | tail -n +2 | awk '{print $10}'`
            echo "$thrash_cycl"
            readarray -t thrash_cycl_array <<< "$thrash_cycl"

            itr_count=0
            max_num=`expr ${counter2[0]} + ${counter3[0]}`
						while [[ ${itr_count} -lt ${max_num} ]]
						do
                echo "itr_count: $itr_count"
                if [ "$itr_count" -eq 0 ]; then
										#Access latency to thrash LLC.
										access_cycle[itr_count]=`expr ${thrash_cycl_array[$itr_count]} - ${LLC_filling_cycle}`        
                    echo "${thrash_cycl_array[$itr_count]}   ${LLC_filling_cycle}"
                else
                    prev_index=`expr $itr_count - 1`
                    echo "${thrash_cycl_array[$itr_count]}  ${thrash_cycl_array[${prev_index}]}"
								    access_cycle[itr_count]=`expr ${thrash_cycl_array[$itr_count]} - ${thrash_cycl_array[${prev_index}]}`        
                fi

                thrashing_finish_cycle=${thrash_cycl_array[$itr_count]}

								# increment the value
								itr_count=`expr ${itr_count} + 1`


						done
						#llc_occ_value[i]=${val} #Copying LLC occupancy from variable val to array llc_occ_value.
						total_cycle[i]=${tot_cycle} #Copying Total cycles spent from variable tot_cycle to array total_cycle.
						#i=`expr ${i} + 1` 

						#Latency to probe the whole array
						probe_cycle=`expr ${tot_cycle} - ${thrashing_finish_cycle}`

            ### Setting headers in the result file.
            # -n removes newline character from the end.
            echo -n "$arr_size_multiples,${counter2[$exp]},${counter3[$exp]},$rand_seed,$num_iterations,${total_cycle[@]},${LLC_filling_cycle}," >> clear_cache_result_files/clear_cache_stats_verify_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_${#counter2[@]}_${#counter3[@]}_new.txt

            itr_count=0
            #Iterate the loop over the number of
            max_num=`expr ${counter2[0]} + ${counter3[0]}`
            while [[ ${itr_count} -lt ${max_num} ]]
            do
                echo -n "${occ_array[${itr_count}]},${access_cycle[${itr_count}]}," >> clear_cache_result_files/clear_cache_stats_verify_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_${#counter2[@]}_${#counter3[@]}_new.txt
                # increment the value
                itr_count=`expr ${itr_count} + 1`
            done
            echo "${probe_cycle}"
            echo "${probe_cycle}" >> clear_cache_result_files/clear_cache_stats_verify_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_${#counter2[@]}_${#counter3[@]}_new.txt

            ### Setting headers in the result file.

						# increment the seed value
						rand_seed_iteration=`expr ${rand_seed_iteration} + 1`
				done


				# increment the value
        arr_size_multiples=`bc -l<<< " ${arr_size_multiples} + ${inc_counter}"`  # expr command does not work on floating point numbers.

		done

  	# increment the value
		num_iterations=`expr ${num_iterations} + 1`

done
# increment the value
exp=`expr ${exp} + 1`

done
