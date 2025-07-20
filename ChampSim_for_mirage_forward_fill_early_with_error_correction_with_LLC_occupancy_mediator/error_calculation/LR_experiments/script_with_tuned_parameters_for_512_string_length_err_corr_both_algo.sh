#### These three are to be varied to create different set-ups. ####

#benchmark_test=$1
#arr_size=$2
#unroll_fact=$3
#do_err_corr_at=$4  
do_llc_occ_chk_at=0   #XXX To be decided. This is not needed to be transferred separately as this is triggered along with the do_err_corr_at.

benchmark_test=0
do_err_corr_at=8  #4 #8  
unroll_fact=32
unroll_fact_for_llc_occ_chk=32  #XXX should be 16 or 32 ?
string_size=512  #11   #512  
max_arr_len=0
extra=0
extra_for_llc_occ_chk=0
start_arr_addr=0
extra_arr_max_addr=0
LR_no_error_corr=1

# XXX below two are kept as same.
err_corr_iterations=1  #XXX Tobe removed   #XXX Defines multiple iterations of error correction. If we assume sender create multiple holes.

seed=24872  #7633   #24872 36578 (caused an error for 32 unrolling factor and message string of length 20)
LLC_NUM_BLOCKS=32768


### Calculating maximum increase in array after every round of error-correction.
max_inc_in_array=`expr ${string_size} / ${do_err_corr_at}`
#let max_inc_in_array="${max_inc_in_array} * ${err_corr_iterations}"

echo "max_inc_in_array: ${max_inc_in_array} "


sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../../inc/config.h
sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'/g' ../../inc/champsim.h
sed -i.bak 's/\<LR_NO_ERR_CORR 0\>/LR_NO_ERR_CORR '${LR_no_error_corr}'/g' ../../inc/champsim.h

pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code

pin_output_file_sender=all_1_sender.txt
array_elements_in_one_cache_block=8
string_num=26
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments
back_to_exp_dir=../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments
Result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments


######### 1. Generate pintrace for sender
cd ${pin_files_path} 
./commands_sender_LR_ERR_CORR_both_algo_with_benchmark_suit.sh ${string_num} ${string_size} ${do_err_corr_at} ${do_llc_occ_chk_at} ${path_to_copy_trace} ${benchmark_test}


#### run single-core experiment to estimate sender's LLC occupancy ####

########### 1.a Extract the number of instructions in the trace file.
sim_count_sender=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_sender} | awk '{print $2}'`

echo "Number of instructions traced for sender: ${sim_count_sender}"

if [ ${benchmark_test} -eq 1 ];
then
    trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${do_err_corr_at}_with_both_algo_test.gz
else
    trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${do_err_corr_at}_with_both_algo_train.gz
fi

cd ${back_to_exp_dir}

echo "${max_arr_len} ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr}"


########### 2. Run script for sender
./run_build_1core_err_corr.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr} ${path_to_copy_trace} ${Result_dir}


########### 2.a) Extract LLC occupancy of sender
#result_file=`ls -lthr | tail -1 | awk '{ print $9 }'`
result_file=result_random_1_${trace_sender}.txt
lines_to_skip=`grep -n "Cache state before clflush" ${result_file} | head -n 1 | awk -F ':' '{print $1}'`  #TODO this will not work, if all are zero in a string.
echo "lines_to_skip : ${lines_to_skip}"
buffer_lines=30
lines_to_skip=`expr $lines_to_skip + ${buffer_lines}`
echo "lines_to_skip : ${lines_to_skip}"
LLC_occupancy_of_sender=`cat ${result_file} | head -n ${lines_to_skip} | tail -n ${buffer_lines} | grep "Tag LLC Valid blocks" | awk '{print $5}' | head -2 | tail -1`
echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}"  # Sender's occupancy will not change for a specific trace-file.


#sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
#sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<LR_NO_ERR_CORR '${LR_no_error_corr}'\>/LR_NO_ERR_CORR 0/g' ../../inc/champsim.h


#exit

#sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../../inc/config.h
#sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'/g' ../../inc/champsim.h
#sed -i.bak 's/\<LR_NO_ERR_CORR 0\>/LR_NO_ERR_CORR '${LR_no_error_corr}'/g' ../../inc/champsim.h


######### 3. Run script for receiver

other_blocks=19661 #326 #325 #19660  #326 # 19660  #326  #19659 #326 #19660 #327  #19659 #19660 #328 #19660 #327     #1638 #XXX edited it for testing purpose.
for ((ki=0; ki<${#unroll_fact[@]}; ki++))
do
  receiver_num_access_in_rdtsc=${unroll_fact[$ki]}
  echo "======================= starting for unrolling factor ${receiver_num_access_in_rdtsc} ======================="
#done
#receiver_num_access_in_rdtsc=4

pin_output_file_receiver=1_multiple_access_${receiver_num_access_in_rdtsc}_for_all_1.txt
trace_receiver=champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}_${do_err_corr_at}_with_both_algo.gz

LLC_occupancy_of_receiver=0
Total=`expr $LLC_occupancy_of_receiver + $LLC_occupancy_of_sender`
echo "line 51, Total: $Total"

receiver_arr_size=104304    #104296  # for 11: 104544  # for 512: 104376 #104552  #259224  # 259232  #104568 #259232  #104552 #259248 
let a="($receiver_arr_size / $array_elements_in_one_cache_block) % $receiver_num_access_in_rdtsc" 
extra=${a} #`expr $receiver_arr_size % $receiver_num_access_in_rdtsc`

let a1="($receiver_arr_size / $array_elements_in_one_cache_block) % $unroll_fact_for_llc_occ_chk"
extra_for_llc_occ_chk=${a1} #`expr $receiver_arr_size % $receiver_num_access_in_rdtsc`

echo "line 54, receiver array size is: ${receiver_arr_size} extra cache blocks are : ${extra} do_err_corr_at: ${do_err_corr_at} extra_for_llc_occ_chk: ${extra_for_llc_occ_chk}"

tim=0

while [ ${Total} -ne ${LLC_NUM_BLOCKS} ]
do
   cd ${pin_files_path} 
   ./commands_receiver_LR_ERR_CORR_both_algo_with_benchmark_suit.sh ${receiver_num_access_in_rdtsc} ${receiver_arr_size} ${extra} ${other_blocks} ${string_size} ${do_err_corr_at} ${do_llc_occ_chk_at} ${err_corr_iterations} ${path_to_copy_trace}
   sim_count_receiver=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver} | awk '{print $2}'`


   echo "Number of instructions traced for receiver: ${sim_count_receiver}"
   cd ${back_to_exp_dir}

   # Update array start point, needed while llc_occ_chk.   
	 typeset -i variable1=$(cat array_end_address.txt)
   start_arr_addr=${variable1}

   # Update array maximum size, needed while llc_occ_chk.   
	 typeset -i variable=$(cat array_start_address.txt)
   max_arr_len=${variable}

   # Update extra array maximum size used during error check.   
	 typeset -i variable=$(cat test2.txt | cut -d ' ' -f 1)
   extra_arr_max_addr=${variable}

   ./run_build_1core_err_corr.sh ${sim_count_receiver} ${trace_receiver} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr} ${path_to_copy_trace} ${Result_dir}

   echo "Completed till here."  
 
  #3.a) Extract marker for region of interest
  result_file=result_random_1_${trace_receiver}.txt
  echo "Completed till here. line 132"  
  lines_to_skip=`grep -n "wait" ${result_file} | head -n 1 | awk -F ':' '{print $1}'`
  echo "Completed till here. line 134"  
  buffer_lines=50
  echo "Completed till here. line 136 ${lines_to_skip}"  
  lines_to_skip=`expr $lines_to_skip + ${buffer_lines}`
  echo "Completed till here. line 138"  
  LLC_occupancy_of_receiver=`cat ${result_file} | head -n ${lines_to_skip} | tail -n ${buffer_lines} | grep "Tag LLC Valid blocks" | awk '{print $5}' | head -n 1 | tail -n 1`
  Total=`expr $LLC_occupancy_of_receiver + $LLC_occupancy_of_sender`
  echo "line 81, Total $Total"

  if [ ${Total} -gt ${LLC_NUM_BLOCKS} ];
  then
      let result="$receiver_arr_size - ( ($Total - $LLC_NUM_BLOCKS) * 8)"
      receiver_arr_size=${result}
      echo "I am in if"
  elif [ ${Total} -lt ${LLC_NUM_BLOCKS} ];
  then
      let result="$receiver_arr_size + ( ( $LLC_NUM_BLOCKS - $Total) * 8)"
      receiver_arr_size=${result}
      echo "I am in elif" 
  fi

  let a="($receiver_arr_size / $array_elements_in_one_cache_block) % $receiver_num_access_in_rdtsc"
  extra=$a
  let a1="($receiver_arr_size / $array_elements_in_one_cache_block) % $unroll_fact_for_llc_occ_chk"
  extra_for_llc_occ_chk=${a1}
  
  echo "new receiver_arr_size: ${receiver_arr_size}, extra: ${extra}"

  if [[ ${tim} -eq 3 ]] #3  ] #### Terminate the loop if it is running for more than 5 times.
  then
      exit
  fi
 
  tim=`expr ${tim} + 1`  
  echo $tim

#ex
done

echo "Receiver LLC occupancy: ${LLC_occupancy_of_receiver}, Sender LLC occupancy: ${LLC_occupancy_of_sender} extra_for_llc_occ_chk: ${extra_for_llc_occ_chk}" # Adjust Receiver occupancy, don't change Sender occupancy.

#fi

result_file=result_random_1_${trace_receiver}.txt
receiver_setup_wait=`grep -nri "wait" ${result_file} | head -n 1 | awk '{ print $12 }'`

echo "receiver_setup_wait ${receiver_setup_wait}"
#exit 

######### Run a 2-core simulation with wait_implementation off.

# Update array start point, needed while llc_occ_chk.   
typeset -i start_arr_addr=$(cat array_end_address.txt)

# Update array maximum size, needed while llc_occ_chk.   
typeset -i max_arr_len=$(cat array_start_address.txt)

# Update extra array maximum size used during error check.   
typeset -i variable=$(cat test2.txt | cut -d ' ' -f 1)
extra_arr_max_addr=${variable}

./run_build_2core_err_corr.sh ${sim_count_sender} ${trace_receiver} ${trace_sender} ${seed} ${string_size} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr}
result_file=result_random_2_${trace_receiver}_${trace_sender}.txt

sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h
sed -i.bak 's/\<LR_NO_ERR_CORR '${LR_no_error_corr}'\>/LR_NO_ERR_CORR 0/g' ../../inc/champsim.h

exit 

######### Extract rdtsc latencies for misses
a=`grep -nri "found a miss for CPU: 0" ${result_file} | awk -F ':' '{print $1}'`
as=($(echo "$a" | tr ' ' '\n')) 

miss_arr=()
min=700000 #Any random value big enough than rdtsc values
for ((kk=0; kk<${#as[@]}; kk++))
do
  a=${as[$kk]}
  a=`expr ${a} + 40`
  miss=`cat ${result_file} | head -n ${a} | tail -n 40 | grep -n "rdtsc_subtraction" | head -n 1 | awk '{print $2}'`
  #echo "$miss "
  if [ ${min} -gt ${miss} ]
  then
      min=${miss}
      #echo "$min"
  fi
  miss_arr[$kk]=${miss}
done
echo "Minimum miss latency is: $min"
echo "Miss array elements are: "
for n in "${miss_arr[@]}"
do
  echo "$n "
done 

######### Extract rdtsc subtraction values. Set the hit/miss threshold.
grep -nri "rdtsc_subtraction" result_random_2_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}.gz_champsim.trace_sender_old.gz.txt | awk '{ print $2 }' > access_${receiver_num_access_in_rdtsc}_plot.txt

python3 cdf.py ${min} access_${receiver_num_access_in_rdtsc}_plot.txt ${receiver_num_access_in_rdtsc}

done

######### Set wait_implementation on.
