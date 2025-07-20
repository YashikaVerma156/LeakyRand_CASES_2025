# This script is used specifically for verifying the dynamic occupancy sequences that are a combination of 3 array sizes.
# XXX Manually set the following variables: mid_number1, mid_number2, arr_num_itr_mid_1, arr_num_itr_mid_2, arr_num_itr and arr_sizes  

mid_number1=$1
arr_num_itr_mid_1=$2
mid_number2=$3
arr_num_itr_mid_2=$4
mid_number3=$5
arr_num_itr_mid_3=$6
arr_sizes=$7                  
arr_num_itr=$8
with_skt_lat=$9
with_skt_lat_sync=${10}

# Check if exactly one argument is passed
if [ "$#" -ne 10 ]; then
    echo "Usage: $0 <mid_number1> <arr_num_itr_mid_1> <mid_number2> <arr_num_itr_mid_2> <mid_number3> <arr_num_itr_mid_3> <arr_sizes> <arr_num_itr> <with_socket_latency> <with_socket_latency_sync>"
    echo "The inputs should match the dynamic sequence generated from dynamic algo."
    echo "The value of arguments 7 and 8 can be 0 0 or 0 1 or 1 0. But these values should not be 1 1."
    echo "for example: <  ./clear_cache_basic_script_dynamic_strategy_combination_of_four_array_sizes_socket_latency.sh 1 1 1.1 1 1.2 1 1.3 8 0 1>  : 1.1 1 means 1.1x of LLC and 1 iterations of 1.1x"
    exit 1
fi



				#echo "arr_sizes: ${arr_sizes}"
				arr_size_multiples=${arr_sizes}  #$1
				built_binary_once=0
        num_iterations=${arr_num_itr}
        num_iterations_mid1=${arr_num_itr_mid_1}
        num_iterations_mid2=${arr_num_itr_mid_2}
        num_iterations_mid3=${arr_num_itr_mid_3}
        rand_seed_iterations_max=1
				dynamic_algo_itr=$(( ${arr_num_itr} + ${arr_num_itr_mid_1} + ${arr_num_itr_mid_2} + ${arr_num_itr_mid_3} ))
				echo "${dynamic_algo_itr}"
        path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

        cd ../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/clear_cache_code 
        ./commands_receiver_LR_clear_cache_dynamic_strategy_four_array_sizes.sh ${arr_size_multiples} ${num_iterations_mid1} ${num_iterations_mid2} ${num_iterations_mid3} ${num_iterations} ${path} ${mid_number1} ${mid_number2} ${mid_number3}
        sim=`grep -nr "Count_instr:" clear_cache.txt | awk '{ print $3 }'`

        cat clear_cache.txt

        echo "sim is: ${sim}"
        cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

        trace="champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${mid_number1}_${num_iterations_mid1}_${mid_number2}_${num_iterations_mid2}_${mid_number3}_${num_iterations_mid3}_${arr_size_multiples}_${num_iterations}.gz"
        result_file="result_random_1_champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${mid_number1}_${num_iterations_mid1}_${mid_number2}_${num_iterations_mid2}_${mid_number3}_${num_iterations_mid3}_${arr_size_multiples}_${num_iterations}.gz.txt"

        latency=0
        if [ "$9" -eq 1 ]; then
              latency=1100
							result_file="result_random_1_champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${mid_number1}_${num_iterations_mid1}_${mid_number2}_${num_iterations_mid2}_${mid_number3}_${num_iterations_mid3}_${arr_size_multiples}_${num_iterations}.gz_${latency}_${with_skt_lat}_${with_skt_lat_sync}.txt"
        fi
        if [ "${10}" -eq 1 ]; then
              latency=5600
							result_file="result_random_1_champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy_${mid_number1}_${num_iterations_mid1}_${mid_number2}_${num_iterations_mid2}_${mid_number3}_${num_iterations_mid3}_${arr_size_multiples}_${num_iterations}.gz_${latency}_${with_skt_lat}_${with_skt_lat_sync}.txt"
        fi

        echo "$trace    $result_file"
        #exit

        rand_seed_iteration=1
        llc_occ_value=()
        total_cycle=()
        access_cycle=()
        probe_cycle=()
        #rand_seed=$((RANDOM % 100000 + 1))
        rand_seed=1830
        echo "rand_seed : ${rand_seed}"

        i=0
        # Print the values
        # Build the binary only once.
        if [ ${built_binary_once} -lt 1 ]; then
              sed -i.bak 's/\<CLEAR_CACHE 0\>/CLEAR_CACHE 1/g' ../inc/champsim.h
              sed -i.bak 's/\<CLEAR_CACHE_WITH_SKT_LAT 0\>/CLEAR_CACHE_WITH_SKT_LAT '$with_skt_lat'/g' ../inc/champsim.h
              sed -i.bak 's/\<CLEAR_CACHE_WITH_SKT_LAT_SYN 0\>/CLEAR_CACHE_WITH_SKT_LAT_SYN '$with_skt_lat_sync'/g' ../inc/champsim.h 
              sed -i.bak 's/\<DYNAMIC_ALGO_ITR 0\>/DYNAMIC_ALGO_ITR '$dynamic_algo_itr'/g' ../inc/champsim.h 
              # build the binary
              ./build_1core.sh ${sim} champsim.trace_clear_cache_${arr_size_multiples}_dynamic_strategy.gz 0 ${rand_seed} 0 0 0 0
              sed -i.bak 's/\<CLEAR_CACHE_WITH_SKT_LAT '$with_skt_lat'\>/CLEAR_CACHE_WITH_SKT_LAT 0/g' ../inc/champsim.h
              sed -i.bak 's/\<CLEAR_CACHE_WITH_SKT_LAT_SYN '$with_skt_lat_sync'\>/CLEAR_CACHE_WITH_SKT_LAT_SYN 0/g' ../inc/champsim.h 
              sed -i.bak 's/\<CLEAR_CACHE 1\>/CLEAR_CACHE 0/g' ../inc/champsim.h
              sed -i.bak 's/\<DYNAMIC_ALGO_ITR '${dynamic_algo_itr}'\>/DYNAMIC_ALGO_ITR 0/g' ../inc/champsim.h 
              # increment the value
              built_binary_once=`expr ${built_binary_once} + 1`
        fi

        ./run_1core_skt_lat.sh ${sim} ${trace} 0 ${rand_seed} 0 0 0 0 ${latency} ${with_skt_lat} ${with_skt_lat_sync}
        grep -nri "dyn_algo" ${result_file}

       # XXX The following lines of code has not been put into use. Calculate the total cycles = fill_cycles + probe_cycles manually from the result file after subtracting the cycles spent in filling the LLC in the beginning. 
        val=`grep -nri "Calculat" ${result_file} | head -1 | awk '{print $8 }'`
        tot_cycle=`grep -nri "Finished CPU 0 instructions:" ${result_file} | awk '{print $7 }'`
        #echo "total cycles: ${tot_cycle}"
        # Instr_id of first fence.
        a=`grep -nri "fence: 1" ${result_file} | head -1 | awk '{ print $11}'` 
          
        # Retirement cycle of first fence.
        LLC_filling_cycle=`grep -nr "Retiring instr_id: ${a} " ${result_file} | head -1 | awk '{print $10}'`

        #Access latency to thrash LLC.
        total_cycle=$(( ${tot_cycle} - ${LLC_filling_cycle} ))
        echo "Total cycles spent in simulation: $tot_cycle"
        echo "Cycles spent in initial filling of empty cache: $LLC_filling_cycle"
        echo "Cycles spent including Fill and Probe step, excluding initial cache fill in the beginning: $total_cycle"


