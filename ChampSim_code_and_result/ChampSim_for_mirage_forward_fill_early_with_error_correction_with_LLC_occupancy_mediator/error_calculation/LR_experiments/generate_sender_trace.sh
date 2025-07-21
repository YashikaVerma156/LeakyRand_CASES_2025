##################### This script is for error calculation plot ###########################
string_num=$1
benchmark_test=$2
check=1

#### These three are to be varied to create different set-ups. ####
unroll_fact=(32) #16 8 4 2 1
unroll_fact_for_llc_occ_chk=0
string_size=512  #18   #34  #18 #100 125 150 175 200
do_llc_occ_chk_at=542   #17
max_arr_len=0
extra=0
extra_for_llc_occ_chk=0
trigger_extra_access_at=569
start_arr_addr=0
extra_arr_max_addr=0
LR_no_error_corr=1

# XXX below two are kept as same.
err_corr_iterations=1   #XXX Defines multiple iterations of error correction. If we assume sender create multiple holes.
max_holes=1

# XXX Holes should be triggered earlier or during the transmission of same bit as that of error correction( i.e. holes filling).
trigger_holes_at=569
do_err_corr_at=547  #16  #18 # 21 #20  #3 #4 #20  #XXX This can vary, but should be such that there is atleast a single increase in array length.

seed=7633   #24872 36578 (caused an error for 32 unrolling factor and message string of length 20)
LLC_NUM_BLOCKS=32768


### Calculating maximum increase in array after every round of error-correction.
max_inc_in_array=`expr ${string_size} / ${do_err_corr_at}`
let max_inc_in_array="${max_inc_in_array} * ${err_corr_iterations}"

echo "max_inc_in_array: ${max_inc_in_array} "


sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../../inc/config.h
sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'/g' ../../inc/champsim.h
sed -i.bak 's/\<LR_NO_ERR_CORR 0\>/LR_NO_ERR_CORR '${LR_no_error_corr}'/g' ../../inc/champsim.h


pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/error_correction_and_occupancy_check_string_length_23

pin_output_file_sender=all_1_sender.txt

array_elements_in_one_cache_block=8
is_it_sender=1

trace_sender=champsim.trace_sender_${string_size}_${string_num}.gz
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23

######### 1. Generate pintrace for sender
cd ${pin_files_path}

if [ $is_it_sender == 1 ]
then
    echo "It is sender's turn to run. Is_it_sender: ${is_it_sender} do_err_corr_at: ${do_err_corr_at}"
fi

./commands_sender_all_1.sh ${string_num} ${string_size} ${do_err_corr_at} ${trigger_holes_at} ${max_holes} ${trigger_extra_access_at} ${do_llc_occ_chk_at} ${path_to_copy_trace}


#### run single-core experiment to estimate sender's LLC occupancy ####

########### 1.a Extract the number of instructions in the trace file.
sim_count_sender=`grep "instrCount" ${pin_output_file_sender} | awk '{print $6}'`

echo "Number of instructions traced for sender: ${sim_count_sender}"

cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_23

echo "${max_arr_len} ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr}"


########### 2. Run script for sender
./run_build_1core.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr}

########### 2.a) Extract LLC occupancy of sender
#result_file=`ls -lthr | tail -1 | awk '{ print $9 }'`
result_file=result_random_1_champsim.trace_sender_${string_size}_${string_num}.gz.txt
lines_to_skip=`grep -n "Cache state before clflush" ${result_file} | head -n 1 | awk -F ':' '{print $1}'`  #TODO this will not work, if all are zero in a string.
echo "lines_to_skip : ${lines_to_skip}"
buffer_lines=30
lines_to_skip=`expr $lines_to_skip + ${buffer_lines}`
echo "lines_to_skip : ${lines_to_skip}"
LLC_occupancy_of_sender=`cat ${result_file} | head -n ${lines_to_skip} | tail -n ${buffer_lines} | grep "Tag LLC Valid blocks" | awk '{print $5}' | head -2 | tail -1`
echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}"  # Sender's occupancy will not change for a specific trace-file.

if [ $is_it_sender == 1 ]
then
    echo "Sender's turn is over."
fi


sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h
sed -i.bak 's/\<LR_NO_ERR_CORR '${LR_no_error_corr}'\>/LR_NO_ERR_CORR 0/g' ../../inc/champsim.h

