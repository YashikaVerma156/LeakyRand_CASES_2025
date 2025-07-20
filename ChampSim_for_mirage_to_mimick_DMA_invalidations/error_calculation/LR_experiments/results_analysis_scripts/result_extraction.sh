result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_to_mimick_DMA_invalidations/error_calculation/LR_experiments/invalidation_results
invalid_block_priority=1

rm extracted_result.txt
printf "%s" "Initially_allocated_array_size(x_times_of_LLC)," "num_IO_calls," "number_of_times_IO_buffer_accessed_in_a_IO_call," "portion_of_array_invalidated_&_accessed_in_a_IO_call," "ipc_dont_prioritize_invalid_LLC_ways," "ipc_prioritize_invalid_LLC_ways," "normalized_ipc," "LLC_occupancy_dont_prioritize_invalid_LLC_ways," "LLC_occupancy_prioritize_invalid_LLC_ways," "IO_instructions_count_dont_prioritize_invalid_LLC_ways," "IO_instructions_count_prioritize_invalid_LLC_ways," "IO_cpu_cyles_dont_prioritize_invalid_LLC_ways," "IO_cpu_cycles_prioritize_invalid_LLC_ways" >> extracted_result.txt
echo "" >> extracted_result.txt

not_present=0
for num_io_calls in 20    
do
		for num_arr_accesses in 2 4 6 8
		do
        for array_to_llc_size_ratio in 1 2
        do
				    for num_mid_points in 0.125 0.25 0.5 0.75 1  #2 #TODO a special case, where array size is needed to be increased.
				    do
						    for invalid_block_priority in 0 1   
						    do
                    result_file=result_random_1_champsim.trace_invalidation_${num_mid_points}_io_calls_${num_io_calls}_arr_access_${num_arr_accesses}_array_size_${array_to_llc_size_ratio}.gz_priority_${invalid_block_priority}.txt
                    file_path=${result_dir}/${result_file}
                    #echo "${file_path}"
                    if [ -f "$file_path" ]; then
                        if [ ${invalid_block_priority} -eq 1 ]; then
                            ipc1=`grep -nri " Calculated IPC: " ${result_dir}/${result_file} | awk '{ print $7 }'`
                            cycles1=`grep -nri " Calculated IPC: " ${result_dir}/${result_file} | awk '{ print $4 }'`
                            instr1=`grep -nri " Calculated IPC: " ${result_dir}/${result_file} | awk '{ print $2 }'`
                            LLC_occ1=`grep -nri "Data LLC Valid blocks:" ${result_dir}/${result_file} | awk '{ print $5 }'`
                        else
                            ipc0=`grep -nri " Calculated IPC: " ${result_dir}/${result_file} | awk '{ print $7 }'`
                            cycles0=`grep -nri " Calculated IPC: " ${result_dir}/${result_file} | awk '{ print $4 }'`
                            instr0=`grep -nri " Calculated IPC: " ${result_dir}/${result_file} | awk '{ print $2 }'`
                            LLC_occ0=`grep -nri "Data LLC Valid blocks:" ${result_dir}/${result_file} | awk '{ print $5 }'`
                        fi
                    else
                        #echo "File not found"
                        not_present=1
                    fi
                done
                normalized_ipc=$(echo "scale=2; $ipc0 / $ipc1" | bc)
                if [ $not_present -eq 0 ]; then
                    printf "%s" "${array_to_llc_size_ratio}," "${num_io_calls}," "${num_arr_accesses}," "${num_mid_points}," "${ipc0}", "${ipc1}", "${normalized_ipc}", "${LLC_occ0}", "${LLC_occ1}", "${instr0}", "${instr1}", "${cycles0}", "${cycles1}" >> extracted_result.txt
                    echo "" >> extracted_result.txt
                fi
                not_present=0
            done
        done
    done
done

rm extracted_result_llc_misses.txt
printf "%s" "invalid_block_priority," "Initially_allocated_array_size(x_times_of_LLC)," "num_IO_calls," "number_of_times_IO_buffer_accessed_in_a_IO_call," "portion_of_array_invalidated_&_accessed_in_a_IO_call" >> extracted_result_llc_misses.txt
for number in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
 do
		printf "%s" ",miss_for_IO_call_"  >> extracted_result_llc_misses.txt
		printf "%s" "$number"  >> extracted_result_llc_misses.txt
done
echo "" >> extracted_result_llc_misses.txt

not_present=0
for num_io_calls in 20
do
    for num_arr_accesses in 2 4 6 8
    do
        for array_to_llc_size_ratio in 1 2
        do
            for num_mid_points in 0.125 0.25 0.5 0.75 1  #2 #TODO a special case, where array size is needed to be increased.
            do
                for invalid_block_priority in 0 1
                do
                    result_file=result_random_1_champsim.trace_invalidation_${num_mid_points}_io_calls_${num_io_calls}_arr_access_${num_arr_accesses}_array_size_${array_to_llc_size_ratio}.gz_priority_${invalid_block_priority}.txt
                    file_path=${result_dir}/${result_file}
                    #echo "${file_path}"
                    if [ -f "$file_path" ]; then
                        mapfile -t result_array < <(grep "Total_IO_miss" "$file_path" | awk '{ print $4 }' )
												printf "%s" "${invalid_block_priority}," "${array_to_llc_size_ratio}," "${num_io_calls}," "${num_arr_accesses}," "${num_mid_points}" >> extracted_result_llc_misses.txt
                        for number in "${result_array[@]}"; do
                            printf "%s" ","  >> extracted_result_llc_misses.txt
                            printf "%s" "$number"  >> extracted_result_llc_misses.txt
                        done
												echo "" >> extracted_result_llc_misses.txt
                    fi
                done
                not_present=0
            done
        done
    done
done
