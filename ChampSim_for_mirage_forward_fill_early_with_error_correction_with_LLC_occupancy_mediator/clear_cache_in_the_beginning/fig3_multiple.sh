back_path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning
num_iterations=1
rand_seed=20911

rm latest_clear_cache_stats_${num_iterations}_new_5_itr.txt

for arr_sizes in 1 2 3 4 5 
do
   built_binary_once=0
   for itr in 1 2 3 4 5 
   do 
        arr_size_multiples=${arr_sizes} 
        rand_seed_iterations_max=1

        cd ../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/clear_cache_code 
        ./commands_receiver_LR_clear_cache_with_benchmark_suit.sh ${arr_size_multiples} ${num_iterations} ${back_path}
        sim=`grep -nr "Count_instr:" clear_cache.txt | awk '{ print $4 }'`

        cat clear_cache.txt

        echo "sim is: ${sim}"
        cd ${back_path}

        rand_seed_iteration=1
        llc_occ_value=()
        total_cycle=()
        access_cycle=()
        probe_cycle=()
        rand_seed=$((rand_seed + 1))

        echo "rand_seed : ${rand_seed}"

        i=0
        # Print the values
        # Build the binary only once.
        if [ ${built_binary_once} -lt 1 ]; then
              sed -i.bak 's/\<CLEAR_CACHE 0\>/CLEAR_CACHE 1/g' ../inc/champsim.h 
              # build the binary
              ./build_1core.sh ${sim} champsim.trace_clear_cache_${arr_size_multiples}.gz 0 ${rand_seed} 0 0 0 0
              sed -i.bak 's/\<CLEAR_CACHE 1\>/CLEAR_CACHE 0/g' ../inc/champsim.h 
              # increment the value
              built_binary_once=`expr ${built_binary_once} + 1`
        fi

        ./run_1core.sh ${sim} champsim.trace_clear_cache_${arr_size_multiples}.gz 0 ${rand_seed} 0 0 0 0
        val=`grep -nri "Calculat" result_random_1_champsim.trace_clear_cache_${arr_size_multiples}.gz.txt | head -1 | awk '{print $8 }'`
        tot_cycle=`grep -nri "Finished CPU 0 instructions:" result_random_1_champsim.trace_clear_cache_${arr_size_multiples}.gz.txt | awk '{print $7 }'`
        #echo "total cycles: ${tot_cycle}"
        # Instr_id of first fence.
        a=`grep -nri "fence: 1" result_random_1_champsim.trace_clear_cache_${arr_size_multiples}.gz.txt | head -1 | awk '{ print $11}'` 
          
        # Retirement cycle of first fence.
        LLC_filling_cycle=`grep -nr "Retiring instr_id: ${a} " result_random_1_champsim.trace_clear_cache_${arr_size_multiples}.gz.txt | head -1 | awk '{print $10}'`

        # Instr_id of next occurrence of fence.
        thrash_fin_instr=`grep -nr "fence: 1" result_random_1_champsim.trace_clear_cache_${arr_size_multiples}.gz.txt | head -3 | tail -1 | awk '{print $11}'`

        # Retirement cycle of fence after thrashing.
        thrashing_finish_cycle=`grep -nr "Retiring instr_id: ${thrash_fin_instr} " result_random_1_champsim.trace_clear_cache_${arr_size_multiples}.gz.txt | head -1 | awk '{print $10}'`

         #Access latency to thrash LLC.
         access_cycle[i]=`expr ${thrashing_finish_cycle} - ${LLC_filling_cycle}`


         llc_occ_value[i]=${val} #Copying LLC occupancy from variable val to array llc_occ_value.
         total_cycle[i]=${tot_cycle} #Copying Total cycles spent from variable tot_cycle to array total_cycle.
         #Latency to probe the whole array
         probe_cycle[i]=`expr ${tot_cycle} - ${thrashing_finish_cycle}`

         printf "%s" "${arr_size_multiples}," "${rand_seed}," "${num_iterations}," "${llc_occ_value[@]}," "${total_cycle[@]}," "${LLC_filling_cycle}," "${access_cycle[@]}," "${probe_cycle[@]}" >> latest_clear_cache_stats_${rand_seed_iterations_max}_new_5_itr.txt #; done 
         echo "" >> latest_clear_cache_stats_${rand_seed_iterations_max}_new_5_itr.txt 
    done
done
