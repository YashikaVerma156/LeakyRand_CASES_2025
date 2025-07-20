##################### This script is for error calculation plot ###########################
check=1
#LLC_NUM_BLOCKS=32768 #2MB

#### These three are to be varied to create different set-ups. ####
unroll_fact=(32)
string_size=23
do_err_corr_at=20  #XXX This can vary.

seed=36578  #24872  #36578 #24872  #7633
LLC_NUM_BLOCKS=32768

max_inc_in_array=`expr ${string_size} / ${do_err_corr_at}`
mod=`expr ${string_size} % ${do_err_corr_at}`
if [[ ${mod} -gt 0 ]]; then
    max_inc_in_array=`expr ${max_inc_in_array} + 1`
fi

sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../../inc/config.h
sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h


pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/error_correction_string_length_${string_size}

pin_output_file_sender=all_1_sender.txt

array_elements_in_one_cache_block=8
is_it_sender=1

string_num=24479  #24618
trace_sender=champsim.trace_sender_${string_size}_${string_num}.gz
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}

######### 1. Generate pintrace for sender
cd ${pin_files_path} 

if [ $is_it_sender == 1 ]
then
    echo "It is sender's turn to run. Is_it_sender: ${is_it_sender}"
fi

./commands_sender_all_1.sh ${string_num} ${string_size} ${path_to_copy_trace} 


#### run single-core experiment to estimate sender's LLC occupancy ####

########### 1.a Extract the number of instructions in the trace file.
sim_count_sender=`grep "instrCount" ${pin_output_file_sender} | awk '{print $4}'`

echo "Number of instructions traced for sender: ${sim_count_sender}"

cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}


########### 2. Run script for sender
./run_build_1core.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed}


########### 2.a) Extract LLC occupancy of sender
#result_file=`ls -lthr | tail -1 | awk '{ print $9 }'`
result_file=result_random_1_champsim.trace_sender_${string_size}_${string_num}.gz.txt
lines_to_skip=`grep -n "Cache state before clflush" ${result_file} | head -n 1 | awk -F ':' '{print $1}'`  #TODO this will not work, if all are zero in a string.
buffer_lines=30
lines_to_skip=`expr $lines_to_skip + ${buffer_lines}`
echo "lines_to_skip : ${lines_to_skip}"
LLC_occupancy_of_sender=`cat ${result_file} | head -n ${lines_to_skip} | tail -n ${buffer_lines} | grep "Tag LLC Valid blocks" | awk '{print $5}' | head -2 | tail -1`
echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}"  # Sender's occupancy will not change for a specific trace-file.

if [ $is_it_sender == 1 ]
then
    echo "Sender's turn is over."
fi

#LLC_occupancy_of_sender=13 XXX why is this so ?

#exit

######### 3. Run script for receiver
is_it_sender=0
if [ $check -eq 1 ]
then
if [ $is_it_sender == 1 ]
then
    echo "It is receiver's turn to run. Is_it_sender: ${is_it_sender}"
fi

other_blocks=327     #1638
for ((ki=0; ki<${#unroll_fact[@]}; ki++))
do
  receiver_num_access_in_rdtsc=${unroll_fact[$ki]}
  echo "======================= starting for unrolling factor ${receiver_num_access_in_rdtsc} ======================="
#done
#receiver_num_access_in_rdtsc=4

pin_output_file_receiver=1_multiple_access_${receiver_num_access_in_rdtsc}_for_all_1.txt
trace_receiver=champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz

LLC_occupancy_of_receiver=0
Total=`expr $LLC_occupancy_of_receiver + $LLC_occupancy_of_sender`
echo "line 51, Total: $Total"

receiver_arr_size=259232 #259304 #259336  #248848 #261048  #some number to start with.
let a="($receiver_arr_size / $array_elements_in_one_cache_block) % $receiver_num_access_in_rdtsc" 
extra=${a} #`expr $receiver_arr_size % $receiver_num_access_in_rdtsc`
echo "line 54, receiver array size is: ${receiver_arr_size} extra cache blocks are : ${extra}"

tim=0

max_inc_in_array=`expr ${string_size} / ${do_err_corr_at}`
mod=`expr ${string_size} % ${do_err_corr_at}`
if [[ ${mod} -gt 0 ]]; then
#  if [ ${Total} -gt ${LLC_NUM_BLOCKS} ];
    max_inc_in_array=`expr ${max_inc_in_array} + 1`
fi

while [ ${Total} -ne ${LLC_NUM_BLOCKS} ]
do
   cd ${pin_files_path} 
   ./commands_receiver_all_1.sh ${receiver_num_access_in_rdtsc} ${receiver_arr_size} ${extra} ${other_blocks} ${string_size} ${do_err_corr_at} ${path_to_copy_trace}
   sim_count_receiver=`grep "instrCount" ${pin_output_file_receiver} | awk '{print $4}'`
   
   echo "Number of instructions traced for receiver: ${sim_count_receiver}"
   cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}
  ./run_build_1core.sh ${sim_count_receiver} ${trace_receiver} ${string_size} ${seed}

  
 
  #3.a) Extract marker for region of interest
  result_file=result_random_1_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz.txt
  lines_to_skip=`grep -n "wait" ${result_file} | head -n 1 | awk -F ':' '{print $1}'`
  buffer_lines=50
  lines_to_skip=`expr $lines_to_skip + ${buffer_lines}`
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
  
  echo "new receiver_arr_size: ${receiver_arr_size}, extra: ${extra}"

  if [[ ${tim} -eq 3 ]] #3  ] #### Terminate the loop if it is running for more than 5 times.
  then
      exit
  fi
 
  tim=`expr ${tim} + 1`  
  echo $tim

#exit 0
done

echo "Receiver LLC occupancy: ${LLC_occupancy_of_receiver}, Sender LLC occupancy: ${LLC_occupancy_of_sender}" # Adjust Receiver occupancy, don't change Sender occupancy.

#fi

result_file=result_random_1_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz.txt
receiver_setup_wait=`grep -nri "wait" ${result_file} | head -n 1 | awk '{ print $12 }'`

echo "receiver_setup_wait ${receiver_setup_wait}"
#exit 

######### Run a 2-core simulation with wait_implementation off.
trace1=champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz
trace2=champsim.trace_sender_${string_size}_${string_num}.gz


./run_build_2core.sh ${sim_count_sender} ${trace1} ${trace2} ${seed} ${string_size}
result_file=result_random_2_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz_champsim.trace_sender_${string_size}.gz.txt

sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h

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
fi
