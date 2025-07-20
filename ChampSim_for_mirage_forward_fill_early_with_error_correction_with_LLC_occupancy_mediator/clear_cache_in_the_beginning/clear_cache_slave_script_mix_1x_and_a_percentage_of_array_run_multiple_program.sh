arr_size_multiples_max=$1
num_iterations_max=$2
num_iterations_max_1=$3
rand_seed_iterations_max=$4
binary_path=$5

inc_counter=1

if [ $# -ne 5 ]; then
    echo " $# The commandline arguments should be equal to 5. order should be arr_size_multiples_max, num_iterations_max, num_iterations_max_1, rand_seed_iterations_max"
    exit 0
else
    echo "The commandline arguments are five."
    echo "arr_size_multiples_max: ${arr_size_multiples_max} , num_iterations_max: ${num_iterations_max}, num_iterations_max_1: ${num_iterations_max_1} , rand_seed_iterations_max: ${rand_seed_iterations_max} path: ${binary_path}"
    #exit 0
fi

mkdir -p clear_cache_result_files/temp_result_files # no error if the directory already exists.
rm -f clear_cache_result_files/temp_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt  #Force remove everything inside this directory.

echo "arr_size_multiples,rand_seed,num_iterations_1x,num_iterations_extra_%,LLC_OCCUPANCY_after_1x,LLC_OCCUPANCY_after_%,Total_cycles,LLC_Filling_cycles,Thrashing_cycles,Probe_cycles" >> clear_cache_result_files/temp_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt #; done    
#exit

path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

#Iterate the loop num_iterations_max_1 times.

# Print the values
echo "arr_size_multiples: ${arr_size_multiples_max}, num_iterations: ${num_iterations_max}, num_iterations_1: ${num_iterations_max_1} "

cd ../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples 
./clear_cache_run_multiple_program_mix_1x_and_a_percentage_of_array.sh ${arr_size_multiples_max} ${num_iterations_max} ${num_iterations_max_1} ${path}

sim=`grep -nr "Count_instr:" clear_cache_temp_code_files/clear_cache_mix_1x_and_a_percentage_of_array_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}/clear_cache.txt | awk '{ print $2 }'`
echo "sim is: ${sim}"
rm clear_cache_temp_code_files/clear_cache_mix_1x_and_a_percentage_of_array_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}/clear_cache.txt

cd ../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

llc_occ_value=()
llc_occ_value_1=()
total_cycle=()
access_cycle=()
probe_cycle=()

rand_seed=$((RANDOM % 100000 + 1))
echo "rand_seed : ${rand_seed}"

i=0
# Print the values

echo "arr_size_multiples_max: ${arr_size_multiples_max} , num_iterations_max: ${num_iterations_max}, num_iterations_max_1: ${num_iterations_max_1} , rand_seed_iterations_max: ${rand_seed_iterations_max}"

./run_1core_to_run_multiple_programs.sh ${sim} champsim.trace_clear_cache_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}.gz 0 ${rand_seed} 0 0 0 0 ${binary_path}

val=`grep -nri "Calculat" result_random_1_champsim.trace_clear_cache_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}.gz.txt | head -1 | awk '{print $8 }'`
val_1=`grep -nri "Calculat" result_random_1_champsim.trace_clear_cache_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}.gz.txt | tail -1 | awk '{print $8 }'`
tot_cycle=`grep -nri "Finished CPU 0 instructions:" result_random_1_champsim.trace_clear_cache_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}.gz.txt | awk '{print $7 }'`
#echo "total cycles: ${tot_cycle}"

# Instr_id of first fence.
a=`grep -nri "fence: 1" result_random_1_champsim.trace_clear_cache_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}.gz.txt | head -1 | awk '{ print $11}'` 

# Retirement cycle of first fence.
LLC_filling_cycle=`grep -nr "Retiring instr_id: ${a} " result_random_1_champsim.trace_clear_cache_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}.gz.txt | head -1 | awk '{print $10}'`

# Instr_id of next occurrence of fence.
thrash_fin_instr=`grep -nr "fence: 1" result_random_1_champsim.trace_clear_cache_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}.gz.txt | head -5 | tail -1 | awk '{print $11}'`

# Retirement cycle of fence after thrashing.
thrashing_finish_cycle=`grep -nr "Retiring instr_id: ${thrash_fin_instr} " result_random_1_champsim.trace_clear_cache_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}.gz.txt | head -1 | awk '{print $10}'`

#Access latency to thrash LLC.
access_cycle[i]=`expr ${thrashing_finish_cycle} - ${LLC_filling_cycle}`


llc_occ_value[i]=${val} #Copying LLC occupancy from variable val to array llc_occ_value.
llc_occ_value_1[i]=${val_1} #Copying LLC occupancy from variable val_1 to array llc_occ_value_1.
total_cycle[i]=${tot_cycle} #Copying Total cycles spent from variable tot_cycle to array total_cycle.
#i=`expr ${i} + 1` 

#Latency to probe the whole array
probe_cycle[i]=`expr ${tot_cycle} - ${thrashing_finish_cycle}`

printf "%s" "${arr_size_multiples_max}," "${rand_seed}," "${num_iterations_max}," "${num_iterations_max_1}," "${llc_occ_value[@]}," "${llc_occ_value_1[@]}," "${total_cycle[@]}," "${LLC_filling_cycle}," "${access_cycle[@]}," "${probe_cycle[@]}" >> clear_cache_result_files/temp_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt #; done 
echo "" >> clear_cache_result_files/temp_result_files/clear_cache_stats_mix_1x_and_a_percentage_of_array_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}_${rand_seed_iterations_max}.txt 

rm result_random_1_champsim.trace_clear_cache_${arr_size_multiples_max}_${num_iterations_max}_${num_iterations_max_1}.gz.txt
