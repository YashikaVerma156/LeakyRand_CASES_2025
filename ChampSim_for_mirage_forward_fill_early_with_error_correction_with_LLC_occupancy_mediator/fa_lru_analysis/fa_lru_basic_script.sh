				built_binary_once=0
        num_iterations=8   # How many times you are collecting this data.
        rand_seed_iterations_max=1
        path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/fa_lru_analysis

        cd ../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/fa_lru_code 
        ./commands_receiver_fa_lru_cache.sh ${num_iterations} ${path}
        sim=`grep -nr "Count_instr:" fa_lru_cache.txt | awk '{ print $2 }'`

        cat fa_lru_cache.txt

        echo "sim is: ${sim}"
        cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/fa_lru_analysis

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
        #if [ ${built_binary_once} -lt 1 ]; then
              sed -i.bak 's/\<FA_LRU_CACHE 0\>/FA_LRU_CACHE 1/g' ../inc/champsim.h 
              # build the binary
              ./build_1core.sh ${sim} champsim.trace_fa_lru_cache.gz 0 ${rand_seed} 0 0 0 0
              sed -i.bak 's/\<FA_LRU_CACHE 1\>/FA_LRU_CACHE 0/g' ../inc/champsim.h 
              # increment the value
              #built_binary_once=`expr ${built_binary_once} + 1`
        #fi

        ./run_1core.sh ${sim} champsim.trace_fa_lru_cache.gz 0 ${rand_seed} 0 0 0 0

        # Use grep to find the matches and store them in an array
        mapfile -t array < <(grep "Retiring fence" result_random_1_champsim.trace_fa_lru_cache.gz.txt | awk '{print $9}')
        result=0
        # Loop through the array elements
        for ((i=0; i<${#array[@]}-1; i++)); do
            diff=$((array[i+1] - array[i]))
            #echo "${array[$i]} , $diff"
            result=`expr $diff + $result`
        done
        avg_denom=`expr $num_iterations - 1` #Calculate the denominator.
        result=`expr $result / $avg_denom`
        echo "cycles_per_bit: ${result}" 
        result=$(echo "scale=2; 4000000000 / $result" | bc)
        result=$(echo "scale=2; $result / 1024" | bc)
        echo "bandwidth: ${result} Kbps" 
