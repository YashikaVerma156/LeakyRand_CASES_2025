. function_header_file.sh

file_name=$1

#sed -n '5,10p' example.txt | grep 'pattern'
#count_total_mem_src_access_btwn_two_lines 492628 1123279 src_mem ${file_name} 

exp=0
counter2=(1 2 3 4)
counter3=(5 6 7 8)

undob=hi_Iam_Yashika
echo $undob

product=$(( ${#counter2[@]} + ${#counter3[@]} ))

echo $product

  # -n removes newline character from the end.
echo -n "arr_size_multiples,rand_seed,num_iterations,LLC_OCCUPANCY,Total_cycles,LLC_Filling_cycles," >> clear_cache_result_files/clear_cache_stats_verify_new_abcd.txt
#Iterate the loop over the number of
while [[ ${exp} -le ${#counter2[@]} ]]
do

    echo -n "LLC_OCCUPANCY_round${exp},Thrashing_cycles_round${exp}," >> clear_cache_result_files/clear_cache_stats_verify_new_abcd.txt
		# increment the value
		exp=`expr ${exp} + 1`
done

echo "Probe_cycles" >> clear_cache_result_files/clear_cache_stats_verify_new_abcd.txt

exp=0
#Iterate the loop over the number of
while [[ ${exp} -le ${#counter2[@]} ]]
do
    echo ${counter2[${exp}]} ${counter3[${exp}]}

# increment the value
exp=`expr ${exp} + 1`
done
