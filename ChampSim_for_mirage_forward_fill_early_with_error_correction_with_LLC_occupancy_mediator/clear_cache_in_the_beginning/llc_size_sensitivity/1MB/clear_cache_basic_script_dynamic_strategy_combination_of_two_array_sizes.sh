#
# This script is used specifically for verifying the dynamic approach.
# XXX Count the number of array iterations required and use that as dynamic_algo_itr.
dynamic_algo_itr=9
MID_NUMBER=1

for arr_sizes in  1.2      #1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9 2
do
    #echo "arr_sizes: ${arr_sizes}"
    arr_size_multiples=${arr_sizes}  #$1
    built_binary_once=0
    arr_num_itr_mid=2
    arr_num_itr=7
    #while [ $arr_num_itr -ge 1 ];
    #do
        num_iterations=${arr_num_itr}
        num_iterations_mid=${arr_num_itr_mid}
        rand_seed_iterations_max=1
        path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

        cd ../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/clear_cache_code 
        ./commands_receiver_LR_clear_cache_dynamic_strategy.sh ${arr_size_multiples} ${num_iterations_mid} ${num_iterations} ${path} ${MID_NUMBER}
        sim=`grep -nr "Count_instr:" clear_cache.txt | awk '{ print $3 }'`

        cat clear_cache.txt

        echo "sim is: ${sim}"
        cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

        #exit

        rand_seed_iteration=1
        llc_occ_value=()
        total_cycle=()
        access_cycle=()
        probe_cycle=()
        rand_seed=$((RANDOM % 100000 + 1))
        #rand_seed=20911
        echo "rand_seed : ${rand_seed}"

        i=0
        # Print the values
        # Build the binary only once.
        if [ ${built_binary_once} -lt 1 ]; then
              sed -i.bak 's/\<CLEAR_CACHE 0\>/CLEAR_CACHE 1/g' ../inc/champsim.h 
              sed -i.bak 's/\<DYNAMIC_ALGO_ITR 0\>/DYNAMIC_ALGO_ITR '$dynamic_algo_itr'/g' ../inc/champsim.h 
              # build the binary
              ./build_1core.sh ${sim} champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy.gz 0 ${rand_seed} 0 0 0 0
              sed -i.bak 's/\<CLEAR_CACHE 1\>/CLEAR_CACHE 0/g' ../inc/champsim.h 
              sed -i.bak 's/\<DYNAMIC_ALGO_ITR '${dynamic_algo_itr}'\>/DYNAMIC_ALGO_ITR 0/g' ../inc/champsim.h 
              # increment the value
              built_binary_once=`expr ${built_binary_once} + 1`
        fi

        ./run_1core.sh ${sim} champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${MID_NUMBER}_${num_iterations_mid}_${arr_size_multiples}_${num_iterations}.gz 0 ${rand_seed} 0 0 0 0
        val=`grep -nri "Calculat" result_random_1_champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${MID_NUMBER}_${num_iterations_mid}_${arr_size_multiples}_${num_iterations}.gz.txt | head -1 | awk '{print $8 }'`
        tot_cycle=`grep -nri "Finished CPU 0 instructions:" result_random_1_champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${MID_NUMBER}_${num_iterations_mid}_${arr_size_multiples}_${num_iterations}.gz.txt | awk '{print $7 }'`
        #echo "total cycles: ${tot_cycle}"
        # Instr_id of first fence.
        a=`grep -nri "fence: 1" result_random_1_champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${MID_NUMBER}_${num_iterations_mid}_${arr_size_multiples}_${num_iterations}.gz.txt | head -1 | awk '{ print $11}'` 
          
        # Retirement cycle of first fence.
        LLC_filling_cycle=`grep -nr "Retiring instr_id: ${a} " result_random_1_champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${MID_NUMBER}_${num_iterations_mid}_${arr_size_multiples}_${num_iterations}.gz.txt | head -1 | awk '{print $10}'`

        # Instr_id of next occurrence of fence.
        thrash_fin_instr=`grep -nr "fence: 1" result_random_1_champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${MID_NUMBER}_${num_iterations_mid}_${arr_size_multiples}_${num_iterations}.gz.txt | head -3 | tail -1 | awk '{print $11}'`

        # Retirement cycle of fence after thrashing.
        thrashing_finish_cycle=`grep -nr "Retiring instr_id: ${thrash_fin_instr} " result_random_1_champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${MID_NUMBER}_${num_iterations_mid}_${arr_size_multiples}_${num_iterations}.gz.txt | head -1 | awk '{print $10}'`

         #Access latency to thrash LLC.
         access_cycle[i]=`expr ${thrashing_finish_cycle} - ${LLC_filling_cycle}`


         llc_occ_value[i]=${val} #Copying LLC occupancy from variable val to array llc_occ_value.
         total_cycle[i]=${tot_cycle} #Copying Total cycles spent from variable tot_cycle to array total_cycle.
         #i=`expr ${i} + 1` 
         #Latency to probe the whole array
         probe_cycle[i]=`expr ${tot_cycle} - ${thrashing_finish_cycle}`

         #for i in ${value[@]}; do 
         #printf "%s" "${arr_size_multiples}," "${rand_seed}," "${num_iterations}," "${llc_occ_value[@]}," "${total_cycle[@]}," "${LLC_filling_cycle}," "${access_cycle[@]}," "${probe_cycle[@]}" >> clear_cache_result_files_updated/clear_cache_stats_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_new.txt #; done 
         #echo "" >> clear_cache_result_files_updated/clear_cache_stats_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_new.txt 
         printf "%s" "${arr_size_multiples}," "${rand_seed}," "${num_iterations}," "${llc_occ_value[@]}," "${total_cycle[@]}," "${LLC_filling_cycle}," "${access_cycle[@]}," "${probe_cycle[@]}" >> dynamic_strategy_latest_clear_cache_stats_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_new.txt #; done 
         echo "" >> dynamic_strategy_latest_clear_cache_stats_${arr_size_multiples_max}_${num_iterations_max}_${rand_seed_iterations_max}_new.txt 

         arr_num_itr=`expr ${arr_num_itr} - 1`
    done
#done
