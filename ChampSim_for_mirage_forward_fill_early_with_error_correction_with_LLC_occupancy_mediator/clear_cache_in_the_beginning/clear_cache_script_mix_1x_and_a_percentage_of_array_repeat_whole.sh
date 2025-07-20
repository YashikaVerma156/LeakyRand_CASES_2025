arr_size_multiples_max=$1
num_iterations_max=6
num_iterations_max_1=$2
rand_seed_iterations_max=$3

inc_counter=1
built_binary_once=0


if [ $# -ne 3 ]; then
    echo "The command-line arguments should be equal to 3. order should be arr_size_multiples_max, num_iterations_max_1, rand_seed_iterations_max"
    exit 0
else
    echo "The command-line arguments are three."
    echo "arr_size_multiples_max: ${arr_size_multiples_max} , num_iterations_max: ${num_iterations_max}, num_iterations_max_1: ${num_iterations_max_1} , rand_seed_iterations_max: ${rand_seed_iterations_max} "
    #exit 0
fi

arr_size_multiples=1 #1
num_iterations=1  #1
num_iterations_1=1  #1

mkdir -p clear_cache_result_files # no error if the directory already exists.
rm -f clear_cache_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_repeat_whole_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt  #Force remove everything inside this directory.

echo "arr_size_multiples,rand_seed,num_iterations_1x,num_iterations_extra_%,LLC_OCCUPANCY_after_1x,LLC_OCCUPANCY_after_%,Total_cycles,LLC_Filling_cycles,Thrashing_cycles,Probe_cycles" >> clear_cache_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_repeat_whole_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt #; done    
#exit

path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

#Iterate the loop num_iterations_max_1 times.
while [[ ${num_iterations_1} -lt ${num_iterations_max_1} ]]
do
		arr_size_multiples=1

		#Iterate the loop arr_size_multiples_max times.
 		##while [[ ${arr_size_multiples} -lt ${arr_size_multiples_max} ]] # XXX bc should be used for floating point variables.

    while [ $(bc <<< "${arr_size_multiples} < ${arr_size_multiples_max}") -eq 1 ]; do

				# Print the values
				echo "arr_size_multiples: ${arr_size_multiples}, num_iterations: ${num_iterations_max}, num_iterations_1: ${num_iterations_1} "

				cd ../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples 
				./clear_cache_mix_1x_and_a_percentage_of_array_repeat_whole.sh ${arr_size_multiples} ${num_iterations_max} ${num_iterations_1} ${path}

				sim=`grep -nr "Count_instr:" clear_cache_temp_code_files/clear_cache_mix_1x_and_a_percentage_of_array_repeat_whole/clear_cache_repeat_whole.txt | awk '{ print $2 }'`
				echo "sim is: ${sim}"

				cd ../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

				rand_seed_iteration=1
				llc_occ_value=()
				llc_occ_value_1=()
				total_cycle=()
				access_cycle=()
				probe_cycle=()

				#Iterate the loop rand_seed_iterations_max times.
				while [[ ${rand_seed_iteration} -lt ${rand_seed_iterations_max} ]]
				do
            rand_seed=$((RANDOM % 100000 + 1))
						echo "rand_seed : ${rand_seed}"

						i=0
						# Print the values
           
            # Build the binary only once.
						if [ ${built_binary_once} -lt 1 ]; then
								 # build the binary
                 ./build_1core.sh ${sim} champsim.trace_clear_cache_repeat_whole.gz 0 ${rand_seed} 0 0 0 0

								 # increment the value
								 built_binary_once=`expr ${built_binary_once} + 1`
						fi

						./run_1core.sh ${sim} champsim.trace_clear_cache_repeat_whole.gz 0 ${rand_seed} 0 0 0 0
						
						val=`grep -nri "Calculat" result_random_1_champsim.trace_clear_cache_repeat_whole.gz.txt | head -1 | awk '{print $8 }'`
						val_1=`grep -nri "Calculat" result_random_1_champsim.trace_clear_cache_repeat_whole.gz.txt | tail -1 | awk '{print $8 }'`
						tot_cycle=`grep -nri "Finished CPU 0 instructions:" result_random_1_champsim.trace_clear_cache_repeat_whole.gz.txt | awk '{print $7 }'`
						#echo "total cycles: ${tot_cycle}"

						# Instr_id of first fence.
						a=`grep -nri "fence: 1" result_random_1_champsim.trace_clear_cache_repeat_whole.gz.txt | head -1 | awk '{ print $11}'` 
						
						# Retirement cycle of first fence.
						LLC_filling_cycle=`grep -nr "Retiring instr_id: ${a} " result_random_1_champsim.trace_clear_cache_repeat_whole.gz.txt | head -1 | awk '{print $10}'`

						# Instr_id of next occurrence of fence.
						thrash_fin_instr=`grep -nr "fence: 1" result_random_1_champsim.trace_clear_cache_repeat_whole.gz.txt | head -5 | tail -1 | awk '{print $11}'`

						# Retirement cycle of fence after thrashing.
						thrashing_finish_cycle=`grep -nr "Retiring instr_id: ${thrash_fin_instr} " result_random_1_champsim.trace_clear_cache_repeat_whole.gz.txt | head -1 | awk '{print $10}'`

						#Access latency to thrash LLC.
						access_cycle[i]=`expr ${thrashing_finish_cycle} - ${LLC_filling_cycle}`


						llc_occ_value[i]=${val} #Copying LLC occupancy from variable val to array llc_occ_value.
						llc_occ_value_1[i]=${val_1} #Copying LLC occupancy from variable val_1 to array llc_occ_value_1.
						total_cycle[i]=${tot_cycle} #Copying Total cycles spent from variable tot_cycle to array total_cycle.
						#i=`expr ${i} + 1` 

						#Latency to probe the whole array
						probe_cycle[i]=`expr ${tot_cycle} - ${thrashing_finish_cycle}`

						#for i in ${value[@]}; do 
						printf "%s" "${arr_size_multiples}," "${rand_seed}," "${num_iterations_max}," "${num_iterations_1}," "${llc_occ_value[@]}," "${llc_occ_value_1[@]}," "${total_cycle[@]}," "${LLC_filling_cycle}," "${access_cycle[@]}," "${probe_cycle[@]}" >> clear_cache_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_repeat_whole_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt #; done 
            echo "" >> clear_cache_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_repeat_whole_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt 

						# increment the seed value
						rand_seed_iteration=`expr ${rand_seed_iteration} + 1`
				done


				# increment the value
        arr_size_multiples=`bc -l<<< " ${arr_size_multiples} + ${inc_counter}"`  # expr command does not work on floating point numbers.

		done

  	# increment the value
		num_iterations_1=`expr ${num_iterations_1} + 1`

done
