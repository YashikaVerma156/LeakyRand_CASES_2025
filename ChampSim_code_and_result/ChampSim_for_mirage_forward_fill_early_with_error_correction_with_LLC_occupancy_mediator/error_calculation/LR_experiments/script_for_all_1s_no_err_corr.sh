################### This script is for error calculation plot ###########################

benchmark_test=$1
arr_size=$2 
unroll_fact=$3


if [ "$#" != 3 ]; then
    echo "Three command-line arguments are expected. Exiting "script_with_tuned_parameters_for_512_string_length.sh" ."
    exit 1  # Exit the script with a non-zero status code indicating an error
fi

echo "benchmark_test: ${benchmark_test}, arr_size: ${arr_size}, unroll_fact: ${unroll_fact}"

#exit

#### These three are to be varied to create different set-ups. ####
string_size=512  #18   #34  #18 #100 125 150 175 200
LR_no_error_corr=1 # For prints related to data extraction.

# XXX below two are kept as same.
seed=7633   #24872 36578 (caused an error for 32 unrolling factor and message string of length 20)
LLC_NUM_BLOCKS=32768


sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h
sed -i.bak 's/\<LR_NO_ERR_CORR 0\>/LR_NO_ERR_CORR '${LR_no_error_corr}'/g' ../../inc/champsim.h

pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code

pin_output_file_sender=all_1_sender.txt
array_elements_in_one_cache_block=8
string_num=26  
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments

######### 1. Generate pintrace for sender
cd ${pin_files_path} 
./commands_sender_LR_NO_ERR_CORR_all1s.sh ${string_num} ${string_size} ${benchmark_test} ${path_to_copy_trace}  

########### 1.a Extract the number of instructions in the trace file.
sim_count_sender=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_sender} | awk '{print $2}'`  
echo "Number of instructions traced for sender: ${sim_count_sender}"
cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments
trace_sender=champsim.trace_sender_${string_size}_all1s_${sim_count_sender}.gz
echo "${sim_count_sender} ${trace_sender} ${string_size} ${seed}"


########### 2. Run script for sender
./run_build_1core.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed}

########### 2.a) Extract LLC occupancy of sender
result_file=result_random_1_${trace_sender}.txt
lines_to_skip=`grep -n "Cache state before clflush" ${result_file} | head -n 1 | awk -F ':' '{print $1}'`  #TODO this will not work, if all are zero in a string.
echo "lines_to_skip : ${lines_to_skip}"
buffer_lines=30
lines_to_skip=`expr $lines_to_skip + ${buffer_lines}`
echo "lines_to_skip : ${lines_to_skip}"
LLC_occupancy_of_sender=`cat ${result_file} | head -n ${lines_to_skip} | tail -n ${buffer_lines} | grep "Tag LLC Valid blocks" | awk '{print $5}' | head -2 | tail -1`
echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}"  # Sender's occupancy will not change for a specific trace-file.


#sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<LR_NO_ERR_CORR '${LR_no_error_corr}'\>/LR_NO_ERR_CORR 0/g' ../../inc/champsim.h


#exit

#sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h
#sed -i.bak 's/\<LR_NO_ERR_CORR 0\>/LR_NO_ERR_CORR '${LR_no_error_corr}'/g' ../../inc/champsim.h

######## 2.b) Decide other blocks count for unroll_fact. This is done after calibration for LR_NO_ERR_CORR.
if [[ ${unroll_fact} == 8 ]]; then
    other_blocks=326  #325 #19660
else
    other_blocks=327 #325 #19660
fi

######### 3. Run script for receiver

receiver_num_access_in_rdtsc=${unroll_fact}
echo "======================= starting for unrolling factor ${receiver_num_access_in_rdtsc} ======================="
#done
#receiver_num_access_in_rdtsc=4

pin_output_file_receiver=receiver_access_${receiver_num_access_in_rdtsc}.txt
trace_receiver=champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz

LLC_occupancy_of_receiver=0
Total=`expr $LLC_occupancy_of_receiver + $LLC_occupancy_of_sender`
echo "line 51, Total: $Total"

let a="($arr_size / $array_elements_in_one_cache_block) % $receiver_num_access_in_rdtsc" 
extra=${a} #`expr $arr_size % $receiver_num_access_in_rdtsc`

echo "line 54, receiver array size is: ${arr_size} extra cache blocks are : ${extra} "

tim=0

while [ ${Total} -ne ${LLC_NUM_BLOCKS} ]
do
   cd ${pin_files_path} 
   ./commands_receiver_LR_NO_ERR_CORR_with_benchmark_suit.sh ${receiver_num_access_in_rdtsc} ${arr_size} ${extra} ${other_blocks} ${string_size} ${path_to_copy_trace}
   sim_count_receiver=`grep "instrCount" ${pin_output_file_receiver} | awk '{print $4}'`
   echo "Number of instructions traced for receiver: ${sim_count_receiver}"
   cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments
   ./run_build_1core.sh ${sim_count_receiver} ${trace_receiver} ${string_size} ${seed} &
 
   # kill the process after 2 min.
   sleep 60
   process_name="bin/bimodal-no-no-random-1core"
   pid=`ps aux | grep $process_name | head -1 | awk '{print $2 }'`
   echo "pid is: $pid"
   killing_status=`kill -9 ${pid}`
   echo "killing_status: ${killing_status}"
 
 
  #3.a) Extract marker for region of interest
  result_file=result_random_1_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz.txt
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
      let result="$arr_size - ( ($Total - $LLC_NUM_BLOCKS) * 8)"
      arr_size=${result}
      echo "I am in if"
  elif [ ${Total} -lt ${LLC_NUM_BLOCKS} ];
  then
      let result="$arr_size + ( ( $LLC_NUM_BLOCKS - $Total) * 8)"
      arr_size=${result}
      echo "I am in elif" 
  fi

  let a="($arr_size / $array_elements_in_one_cache_block) % $receiver_num_access_in_rdtsc"
  extra=$a
  
  echo "new arr_size: ${arr_size}, extra: ${extra}"

  if [[ ${tim} -eq 3 ]] #3  ] #### Terminate the loop if it is running for more than 3 times.
  then
      sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
      sed -i.bak 's/\<LR_NO_ERR_CORR '${LR_no_error_corr}'\>/LR_NO_ERR_CORR 0/g' ../../inc/champsim.h
      exit
  fi
 
  tim=`expr ${tim} + 1`  
  echo $tim

done

echo "Receiver LLC occupancy: ${LLC_occupancy_of_receiver}, Sender LLC occupancy: ${LLC_occupancy_of_sender}" # Adjust Receiver occupancy, don't change Sender occupancy.


######### Run a 2-core simulation with wait_implementation off.
trace1=champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz
trace2=champsim.trace_sender_${string_size}_all1s_${sim_count_sender}.gz
./run_build_2core_all1s.sh ${sim_count_sender} ${trace1} ${trace2} ${seed} ${string_size} ${benchmark_test} &

sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
sed -i.bak 's/\<LR_NO_ERR_CORR '${LR_no_error_corr}'\>/LR_NO_ERR_CORR 0/g' ../../inc/champsim.h

# kill the process after 2 min.
sleep 60
process_name="bin/bimodal-no-no-random-2core"
pid=`ps aux | grep $process_name | head -1 | awk '{print $2 }'`
echo "pid is: $pid"
killing_status=`kill -9 ${pid}`
echo "killing_status: ${killing_status}"

exit 

