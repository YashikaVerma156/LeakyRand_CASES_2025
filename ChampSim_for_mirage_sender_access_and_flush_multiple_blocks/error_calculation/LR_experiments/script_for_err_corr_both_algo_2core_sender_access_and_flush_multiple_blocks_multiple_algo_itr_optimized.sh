#### These three are to be varied to create different set-ups. ####

benchmark_test=$1
receiver_arr_size=$2
unroll_fact=$3
do_err_corr_at=$4  
other_blocks=$5
string_num=$6
hit_miss_threshold=$7
err_corr_iterations=$8  #XXX Defines multiple iterations of error correction. If we assume sender create multiple holes.
do_llc_occ_chk_at=0   #XXX This flag is not needed to be transferred separately as this is triggered along with the do_err_corr_at.

unroll_fact_for_llc_occ_chk=${unroll_fact}  #XXX Keeping the unroll_fact and unroll_fact_for_llc_occ_chk the same. 
string_size=512  #11   #512  
max_arr_len=0
extra=0
extra_for_llc_occ_chk=0
start_arr_addr=0
extra_arr_max_addr=0
LR_error_corr=1

# Check if exactly one argument is passed
if [ "$#" -ne 8 ]; then
    echo "Appropriate number of arguments not passed, exiting from script_for_err_corr_both_algo_2core_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh. $#"
    
   echo "benchmark_test: $benchmark_test, receiver_arr_size: $receiver_arr_size, unroll_fact: $unroll_fact, do_err_corr_at: $do_err_corr_at, other_blocks: $other_blocks, string_num: $string_num, hit_miss_threshold: $hit_miss_threshold, err_corr_iterations: $err_corr_iterations"

    exit 1
fi

# XXX below two are kept as same.

LLC_NUM_BLOCKS=32768

line_number=`expr ${string_num} + 1`

if [ $benchmark_test == 0 ]
then
		specific_line=$(sed -n "${line_number}p" seeds_train.txt)
		IFS=',' read -ra columns <<< "$specific_line"
		seed=${columns[32]} # Hard coded inside seeds_train.txt.
		echo "Seed: $seed"
else
		specific_line=$(sed -n "${line_number}p" seeds_test.txt)
		IFS=',' read -ra columns <<< "$specific_line"
		seed=${columns[32]}
		echo "Seed: $seed"
fi

mkdir -p results

### Calculating maximum increase in array after every round of error-correction.

number=`expr ${do_err_corr_at} - 1` # This is needed as we trigger error correction algo at that bit-position itself.
echo "number: $number string_size: ${string_size}"
max_inc_in_array=`expr ${string_size} / ${number}`
let max_inc_in_array="${max_inc_in_array} * ${err_corr_iterations}"  # max_inc_in_array calculation should be different for the champsim directory and for the pin directory i.e. for champsim directory the max_inc_in_array should be multiplied with err_corr_iterations but not in pin directory.
#In champsim directory max_inc_in_array decide how many maximum addresses are needed to be flushed. While in pin directory, max_inc_in_array decide how many total message bits to be transferred.

echo "max_inc_in_array: ${max_inc_in_array} "

#exit

sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../../inc/config.h
sed -i.bak 's/\<NUM_SENDER_DISTURBANCES 0\>/NUM_SENDER_DISTURBANCES '${err_corr_iterations}'/g' ../../inc/config.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK 0\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'/g' ../../inc/champsim.h
sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO 0\>/LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'/g' ../../inc/champsim.h

pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code

pin_output_file_sender=all_1_sender.txt
array_elements_in_one_cache_block=8
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks/error_calculation/LR_experiments/pin_traces
back_to_exp_dir=../../../../../ChampSim_for_mirage_sender_access_and_flush_multiple_blocks/error_calculation/LR_experiments
Result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks/error_calculation/LR_experiments/results


######### 1. Generate pintrace for sender
cd ${pin_files_path} 
./commands_sender_LR_ERR_CORR_both_algo_with_benchmark_suit_multiple_access_optimized.sh ${string_num} ${string_size} ${do_err_corr_at} ${do_llc_occ_chk_at} ${path_to_copy_trace} ${benchmark_test} ${err_corr_iterations}

#exit

#### run single-core experiment to estimate sender's LLC occupancy ####

########### 1.a Extract the number of instructions in the trace file.
sim_count_sender=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_sender} | awk '{print $2}'`

echo "Number of instructions traced for sender: ${sim_count_sender}"

if [ ${benchmark_test} -eq 1 ];
then
    trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${do_err_corr_at}_with_both_algo_${err_corr_iterations}_access_test.gz
else
    trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_${do_err_corr_at}_with_both_algo_${err_corr_iterations}_access_train.gz
fi

cd ${back_to_exp_dir}

echo "${max_arr_len} ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr}"


########### 2. Run script for sender
./run_build_1core_err_corr.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr} ${path_to_copy_trace} ${Result_dir}


########### 2.a) Extract LLC occupancy of sender
#result_file=`ls -lthr | tail -1 | awk '{ print $9 }'`
result_file=result_random_1_${trace_sender}.txt
result_file_path=results/${result_file}
start_line=`grep -n "cpu 0 is on wait" ${result_file_path} | head -n 3 | tail -n 1 | awk -F ":" '{print $1}'`
end_line=`grep -n "cpu 0 is on wait" ${result_file_path} | head -n 4 | tail -n 1 | awk -F ":" '{print $1}'`
echo "start_line: $start_line end_line: $end_line"
LLC_occupancy_of_sender=`sed -n "${start_line},${end_line}p" ${result_file_path} | grep "Tag LLC Valid blocks" | tail -n 1 | awk '{print $5}'` #surround variables name with double quotes.
echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}"  # Sender's occupancy will not change for a specific trace-file.

gap_number=`cat ${pin_files_path}/gap_number.txt`

if [ $gap_number == 1440 ]
then
    echo "gap_number: $gap_number"
else
    sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
    sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../../inc/config.h
    sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
    sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h
    sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
    sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h
    sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../../inc/champsim.h
    echo "Gap number is not the same."
    exit

fi


#sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
#sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../../inc/config.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../../inc/champsim.h


#exit

#sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../../inc/config.h
#sed -i.bak 's/\<NUM_SENDER_DISTURBANCES 0\>/NUM_SENDER_DISTURBANCES '${err_corr_iterations}'/g' ../../inc/config.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../../inc/champsim.h
#sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK 0\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'/g' ../../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'/g' ../../inc/champsim.h
#sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO 0\>/LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'/g' ../../inc/champsim.h


######### 3. Run script for receiver

for ((ki=0; ki<${#unroll_fact[@]}; ki++))
do
  receiver_num_access_in_rdtsc=${unroll_fact[$ki]}
  echo "======================= starting for unrolling factor ${receiver_num_access_in_rdtsc} ======================="
#done
#receiver_num_access_in_rdtsc=4

pin_output_file_receiver=1_multiple_access_${receiver_num_access_in_rdtsc}_for_all_1.txt
trace_receiver=champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}_${do_err_corr_at}_with_both_algo_${err_corr_iterations}_algo_itr.gz

LLC_occupancy_of_receiver=0
Total=`expr $LLC_occupancy_of_receiver + $LLC_occupancy_of_sender`
echo "line 51, Total: $Total"

let a="($receiver_arr_size / $array_elements_in_one_cache_block) % $receiver_num_access_in_rdtsc" 
extra=${a} #`expr $receiver_arr_size % $receiver_num_access_in_rdtsc`

let a1="($receiver_arr_size / $array_elements_in_one_cache_block) % $unroll_fact_for_llc_occ_chk"
extra_for_llc_occ_chk=${a1} #`expr $receiver_arr_size % $receiver_num_access_in_rdtsc`

echo "line 54, receiver array size is: ${receiver_arr_size} extra cache blocks are : ${extra} do_err_corr_at: ${do_err_corr_at} extra_for_llc_occ_chk: ${extra_for_llc_occ_chk}"

tim=0

while [ ${Total} -ne ${LLC_NUM_BLOCKS} ]
do
   cd ${pin_files_path} 
   ./commands_receiver_LR_ERR_CORR_both_algo_with_benchmark_suit_multiple_algo_itr_optimized.sh ${receiver_num_access_in_rdtsc} ${receiver_arr_size} ${extra} ${other_blocks} ${string_size} ${do_err_corr_at} ${do_llc_occ_chk_at} ${err_corr_iterations} ${path_to_copy_trace}
   sim_count_receiver=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver} | awk '{print $2}'`


   echo "Number of instructions traced for receiver: ${sim_count_receiver}"
   cd ${back_to_exp_dir}

   # Update array start point, needed while llc_occ_chk.   
	 typeset -i variable1=$(cat pin_traces/array_end_address.txt)
   start_arr_addr=${variable1}

   # Update array maximum size, needed while llc_occ_chk.   
	 typeset -i variable=$(cat pin_traces/array_start_address.txt)
   max_arr_len=${variable}

   # Update extra array maximum size used during error check.   
	 typeset -i variable=$(cat pin_traces/test2.txt | cut -d ' ' -f 1)
   extra_arr_max_addr=${variable}

   ./run_build_1core_err_corr.sh ${sim_count_receiver} ${trace_receiver} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr} ${path_to_copy_trace} ${Result_dir} &

   echo "Completed till here."  
   # kill the process after a minute.
   sleep 60
   process_name="bin/bimodal-no-no-random-1core"
   pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
   echo "pid is: $pid"
   killing_status=`kill -9 ${pid}`
   echo "killing_status: ${killing_status}"

  #3.a) Extract marker for region of interest
  result_file=result_random_1_${trace_receiver}.txt
  result_file_path=results/${result_file}
  start_line=`grep -nri "wait" ${result_file_path} | head -n 1 | tail -n 1 | awk -F ":" '{print $1}'` 
  end_line=`grep -nri "wait" ${result_file_path} | head -n 2 | tail -n 1 | awk -F ":" '{print $1}'` 
  LLC_occupancy_of_receiver=`sed -n "${start_line},${end_line}p" ${result_file_path} | grep "Tag LLC Valid blocks" | head -n 4 | tail -n 1 | awk '{print $5}'`

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
      sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
      sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../../inc/config.h
      sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
      sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h
      sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
      sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h
      sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../../inc/champsim.h
      exit
  fi

  gap_number=`cat ${pin_files_path}/gap_number.txt`

  if [ $unroll_fact == 16 ]
  then
      if [ $gap_number == 1456 ]
      then
          echo "gap_number: $gap_number"
      else
          sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
          sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../../inc/config.h
          sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
          sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h
          sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
          sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h
          sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../../inc/champsim.h
          echo "Gap number is not the same."
          exit
      fi
   fi

  tim=`expr ${tim} + 1`  
  echo $tim

#ex
done

echo "Receiver LLC occupancy: ${LLC_occupancy_of_receiver}, Sender LLC occupancy: ${LLC_occupancy_of_sender} extra_for_llc_occ_chk: ${extra_for_llc_occ_chk}" # Adjust Receiver occupancy, don't change Sender occupancy.

#fi

result_file=result_random_1_${trace_receiver}.txt
receiver_setup_wait=`grep -nri "wait" ${result_file_path} | head -n 1 | awk '{ print $12 }'`

echo "receiver_setup_wait ${receiver_setup_wait}"
#exit 

######### Run a 2-core simulation with wait_implementation off.

# Update array start point, needed while llc_occ_chk.   
typeset -i start_arr_addr=$(cat pin_traces/array_end_address.txt)

# Update array maximum size, needed while llc_occ_chk.   
typeset -i max_arr_len=$(cat pin_traces/array_start_address.txt)

# Update extra array maximum size used during error check.   
typeset -i variable=$(cat pin_traces/test2.txt | cut -d ' ' -f 1)
extra_arr_max_addr=${variable}

./run_build_2core_err_corr.sh ${sim_count_sender} ${trace_receiver} ${trace_sender} ${seed} ${string_size} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr} ${path_to_copy_trace} ${Result_dir} &
result_file=result_random_2_${trace_receiver}_${trace_sender}.txt
# kill the process after 15 min.
#Sleep is needed so that the build can be finished.
sleep 90   #60  #900
process_name="./bin/bimodal-no-no-random-2core"
pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
#pid=$(ps -eo pid,lstart,cmd | grep "$process_name" | sort -rk 3,3 | head -1 | awk '{print $1}')
echo "kill pid is: $pid"
killing_status=`kill -9 ${pid}`
echo "killing_status: ${killing_status}"
sleep 10   #60  #900

sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
sed -i.bak 's/\<NUM_SENDER_DISTURBANCES '${err_corr_iterations}'\>/NUM_SENDER_DISTURBANCES 0/g' ../../inc/config.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h
sed -i.bak 's/\<LR_ERR_CORR_BOTH_ALGO '${LR_error_corr}'\>/LR_ERR_CORR_BOTH_ALGO 0/g' ../../inc/champsim.h

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
