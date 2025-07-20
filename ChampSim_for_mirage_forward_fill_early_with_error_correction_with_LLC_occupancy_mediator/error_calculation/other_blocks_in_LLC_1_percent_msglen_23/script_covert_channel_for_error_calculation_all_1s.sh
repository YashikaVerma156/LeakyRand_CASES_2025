##################### This script is for error calculation plot ###########################
check=1
NUM_EXP=1    # How many number of experiments you want to run.
msg_type='all1s'    #'all1s' 'rand'

#### These three are to be varied to create different set-ups. ####
unroll_fact=(32) #16 8 4 2 1
unroll_fact_for_llc_occ_chk=1
string_size=10  #18   #34  #18 #100 125 150 175 200
do_llc_occ_chk_at=19   #17
max_arr_len=0
extra=0
extra_for_llc_occ_chk=0
trigger_extra_access_at=69
start_arr_addr=0
extra_arr_max_addr=0

# XXX below two are kept as same.
err_corr_iterations=1
max_holes=1

# XXX Holes should be triggered earlier or during the transmission of same bit as that of error correction( i.e. holes filling).
trigger_holes_at=69
do_err_corr_at=18  #16  #18 # 21 #20  #3 #4 #20  #XXX This can vary, but should be such that there is atleast a single increase in array length.

seed=36578  #24872  #36578  #24872  #36578 #24872  #7633 XXX This generates row buffer hits at receiver side.
LLC_NUM_BLOCKS=32768

### Calculating maximum increase in array after every round of error-correction.
max_inc_in_array=`expr ${string_size} / ${do_err_corr_at}`
let max_inc_in_array="${max_inc_in_array} * ${err_corr_iterations}"

echo "max_inc_in_array: ${max_inc_in_array} "

sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../../inc/config.h
sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'/g' ../../inc/champsim.h

pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/error_correction_and_occupancy_check_string_length_${string_size}_${msg_type}

original_binary_path=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/bin
bin_2core='bimodal-no-no-random-2core'
pin_output_file_sender=all_1_sender.txt

array_elements_in_one_cache_block=8
is_it_sender=1

string_num=31  #24479  #24618
trace_sender=champsim.trace_sender_${string_size}_${string_num}.gz
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}_${msg_type}

######### 1. Generate pintrace for sender
cd ${pin_files_path} 

if [ $is_it_sender == 1 ]
then
    echo "It is sender's turn to run. Is_it_sender: ${is_it_sender} do_err_corr_at: ${do_err_corr_at} do_llc_occ_chk_at: ${do_llc_occ_chk_at}"
fi

./commands_sender_all_1.sh ${string_num} ${string_size} ${do_err_corr_at} ${trigger_holes_at} ${max_holes} ${trigger_extra_access_at} ${do_llc_occ_chk_at} ${path_to_copy_trace}  

#### run single-core experiment to estimate sender's LLC occupancy ####

########### 1.a Extract the number of instructions in the trace file.
sim_count_sender=`grep "instrCount" ${pin_output_file_sender} | awk '{print $4}'`

echo "Number of instructions traced for sender: ${sim_count_sender}"

cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}

echo "${max_arr_len} ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr}"

########### 2. Run script for sender
./run_build_1core.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr}

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

#sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
#sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h

#exit

#sed -i.bak 's/\<MAX_ARR_INC 0\>/MAX_ARR_INC '${max_inc_in_array}'/g' ../../inc/config.h
#sed -i.bak 's/\<UNROLLING_FACTOR 0\>/UNROLLING_FACTOR '${unroll_fact}'/g' ../../inc/champsim.h
#sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'/g' ../../inc/champsim.h

######### 3. Run script for receiver
is_it_sender=0
if [ $check -eq 1 ]
then
if [ $is_it_sender == 1 ]
then
    echo "It is receiver's turn to run. Is_it_sender: ${is_it_sender} do_err_corr_at: ${do_err_corr_at}"
fi

other_blocks=325 #19660  #326 # 19660  #326  #19659 #326 #19660 #327  #19659 #19660 #328 #19660 #327     #1638 #XXX edited it for testing purpose.
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

receiver_arr_size=259240  #259240  #104568 #259240 #104544 # 259232  #104568 #259232  #104552 #259248 #104560 #104316 #259232 #259304 #259336  #248848 #261048  #some number to start with.
let a="($receiver_arr_size / $array_elements_in_one_cache_block) % $receiver_num_access_in_rdtsc" 
extra=${a} #`expr $receiver_arr_size % $receiver_num_access_in_rdtsc`

let a1="($receiver_arr_size / $array_elements_in_one_cache_block) % $unroll_fact_for_llc_occ_chk"
extra_for_llc_occ_chk=${a1} #`expr $receiver_arr_size % $receiver_num_access_in_rdtsc`

echo "line 54, receiver array size is: ${receiver_arr_size} extra cache blocks are : ${extra} do_err_corr_at: ${do_err_corr_at} extra_for_llc_occ_chk: ${extra_for_llc_occ_chk}"

tim=0

while [ ${Total} -ne ${LLC_NUM_BLOCKS} ]
do
   cd ${pin_files_path} 
   ./commands_receiver_all_1.sh ${receiver_num_access_in_rdtsc} ${receiver_arr_size} ${extra} ${other_blocks} ${string_size} ${do_err_corr_at} ${do_llc_occ_chk_at} ${err_corr_iterations} ${path_to_copy_trace}
   sim_count_receiver=`grep "instrCount" ${pin_output_file_receiver} | awk '{print $4}'`


   echo "Number of instructions traced for receiver: ${sim_count_receiver}"
   cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}

   # Update array start point, needed while llc_occ_chk.   
	 typeset -i variable1=$(cat array_end_address.txt)
   start_arr_addr=${variable1}

   # Update array maximum size, needed while llc_occ_chk.   
	 typeset -i variable=$(cat array_start_address.txt)
   max_arr_len=${variable}

   # Update extra array maximum size used during error check.   
	 typeset -i variable=$(cat test2.txt | cut -d ' ' -f 1)
   extra_arr_max_addr=${variable}

   ./run_build_1core.sh ${sim_count_receiver} ${trace_receiver} ${string_size} ${seed} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr}

   echo "Completed till here."  
 
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

result_file=result_random_1_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz.txt
receiver_setup_wait=`grep -nri "wait" ${result_file} | head -n 1 | awk '{ print $12 }'`

echo "receiver_setup_wait ${receiver_setup_wait}"
#exit 

######### Run a 2-core simulation with wait_implementation off.
trace1=champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz
trace2=champsim.trace_sender_${string_size}_${string_num}.gz

# Update array start point, needed while llc_occ_chk.   
typeset -i start_arr_addr=$(cat array_end_address.txt)

# Update array maximum size, needed while llc_occ_chk.   
typeset -i max_arr_len=$(cat array_start_address.txt)

# Update extra array maximum size used during error check.   
typeset -i variable=$(cat test2.txt | cut -d ' ' -f 1)
extra_arr_max_addr=${variable}

./run_build_2core.sh ${sim_count_sender} ${trace1} ${trace2} ${seed} ${string_size} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr}
result_file=result_random_2_${trace1}_${trace2}_${seed}.txt

###### Update multi-core setup. ######

###### Copy the 2-core binary at below path.
cp ${original_binary_path}/${bin_2core} binary/

exp_ran=0
while [ ${exp_ran} -ne ${NUM_EXP} ]
do
    seed=$((RANDOM % 100000 + 1))
    #seed=20095
    echo "rand_seed : ${seed}"
		./run_2core.sh ${sim_count_sender} ${trace1} ${trace2} ${seed} ${string_size} ${extra} ${extra_for_llc_occ_chk} ${start_arr_addr} ${max_arr_len} ${extra_arr_max_addr} ${msg_type}
		result_file=result_random_2_${trace1}_${trace2}_${seed}.txt
    msg_received=($(grep -nri "bit received is" ${result_file} | awk '{print $7}' | tr '\n' ' '))
    printf "%s" "seed: ${seed}" >> result_file_all_1s.txt
    echo " " >> result_file_all_1s.txt
    printf "%s" "msg_received: ${msg_received[@]}" >> result_file_all_1s.txt
    echo " " >> result_file_all_1s.txt
    #### Error Calculation ##########
    err_count=0
    for i in $msg_received; do
      if [ "$i" = "0" ]; then
        err_count=$((err_count + 1))
      fi
    done
    printf "%s" "error_calculated: ${err_count}" >> result_file_all_1s.txt
    echo " " >> result_file_all_1s.txt
		exp_ran=`expr ${exp_ran} + 1`
done


sed -i.bak 's/\<MAX_ARR_INC '${max_inc_in_array}'\>/MAX_ARR_INC 0/g' ../../inc/config.h
sed -i.bak 's/\<UNROLLING_FACTOR '${unroll_fact}'\>/UNROLLING_FACTOR 0/g' ../../inc/champsim.h
sed -i.bak 's/\<UNROLLING_FACTOR_FOR_LLC_OCC_CHK '${unroll_fact_for_llc_occ_chk}'\>/UNROLLING_FACTOR_FOR_LLC_OCC_CHK 0/g' ../../inc/champsim.h

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
