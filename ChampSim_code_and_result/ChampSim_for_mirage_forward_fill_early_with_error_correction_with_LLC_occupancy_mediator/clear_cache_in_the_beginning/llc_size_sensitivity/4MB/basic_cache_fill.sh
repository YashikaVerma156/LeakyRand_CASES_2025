arr_size_times=$1
LLC_NUM_BLOCKS=65536
repetition=1
llc_sets=`expr $LLC_NUM_BLOCKS / 16` # for Mirage setup, the base LLC associativity is 16.

champsim_path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning/llc_size_sensitivity/4MB
pin_path=/home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/clear_cache_code

sed -i 's/#define LLC_SET 1\*2048/#define LLC_SET 1\*'${llc_sets}'/' ../../../inc/cache.h
sed -i 's/#define data_LLC_WAY 16\*2048\*1/#define data_LLC_WAY 16\*'${llc_sets}'\*1/' ../../../inc/cache.h

#exit

cd ${pin_path}
./commands_receiver_LR_clear_cache_one_array_diff_llc_size.sh ${arr_size_times} ${repetition} ${LLC_NUM_BLOCKS} ${champsim_path}
sim=`grep -nr "Count_instr:" clear_cache.txt | awk '{ print $4 }'`
cd ${champsim_path}

rand_seed=10001
built_binary_once=0
if [ ${built_binary_once} -lt 1 ]; then
			sed -i.bak 's/\<CLEAR_CACHE 0\>/CLEAR_CACHE 1/g' ../../../inc/champsim.h
			#sed -i.bak 's/\<DYNAMIC_ALGO_ITR 0\>/DYNAMIC_ALGO_ITR '$dynamic_algo_itr'/g' ../../../inc/champsim.h
			# build the binary
			./build_1core.sh ${sim} champsim.trace_clear_cache_${LLC_NUM_BLOCKS}_one_array_diff_llc_size.gz 0 ${rand_seed} 0 0 0 0
			sed -i.bak 's/\<CLEAR_CACHE 1\>/CLEAR_CACHE 0/g' ../../../inc/champsim.h
			#sed -i.bak 's/\<DYNAMIC_ALGO_ITR '${dynamic_algo_itr}'\>/DYNAMIC_ALGO_ITR 0/g' ../../../inc/champsim.h
			# increment the value
			built_binary_once=`expr ${built_binary_once} + 1`
fi

./run_1core.sh ${sim} champsim.trace_clear_cache_${arr_size_times}_one_array_diff_llc_size.gz 0 ${rand_seed} 0 0 0 0

#Reset cache size.
sed -i 's/#define LLC_SET 1\*'${llc_sets}'/#define LLC_SET 1\*2048/' ../../../inc/cache.h
sed -i 's/#define data_LLC_WAY 16\*'${llc_sets}'\*1/#define data_LLC_WAY 16\*2048\*1/' ../../../inc/cache.h


val=`grep -nri "Calculat" result_random_1_champsim.trace_clear_cache_${arr_size_times}_one_array_diff_llc_size.gz.txt | head -1 | awk '{print $8 }'`
tot_cycle=`grep -nri "Finished CPU 0 instructions:" result_random_1_champsim.trace_clear_cache_${arr_size_times}_one_array_diff_llc_size.gz.txt | awk '{print $7 }'`
#echo "total cycles: ${tot_cycle}"
# Instr_id of first fence.
a=`grep -nri "fence: 1" result_random_1_champsim.trace_clear_cache_${arr_size_times}_one_array_diff_llc_size.gz.txt | head -1 | awk '{ print $11}'`

# Retirement cycle of first fence.
LLC_filling_cycle=`grep -nr "Retiring instr_id: ${a} " result_random_1_champsim.trace_clear_cache_${arr_size_times}_one_array_diff_llc_size.gz.txt | head -1 | awk '{print $10}'`

# Instr_id of next occurrence of fence.
thrash_fin_instr=`grep -nr "fence: 1" result_random_1_champsim.trace_clear_cache_${arr_size_times}_one_array_diff_llc_size.gz.txt | head -3 | tail -1 | awk '{print $11}'`

