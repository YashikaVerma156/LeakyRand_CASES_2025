###################### This script is for error calculation plot ###########################
check=1
string_size=25
LLC_NUM_BLOCKS=32768
NUM_EXP=300
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}
RANDOM=$$  #Initialise the variable RANDOM to process id.
for ((ii=0; ii<${NUM_EXP}; ii++))
do

    echo "Experiment number: ${ii} " >> error_calculation_report.txt
    current_date_time=$(date)
    echo "Current date and time: $current_date_time" >> error_calculation_report.txt
    data_array=()
		pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/error_correction_string_length_${string_size}
		pin_output_file_sender=all_1_sender.txt
		array_elements_in_one_cache_block=8
		is_it_sender=1

		string_num=${RANDOM}  #1048576  #RANDOM is a shell maintained variable, gives a random number between 0 to 32767.
    seed=${RANDOM}
    echo "Random number choosen is: $string_num" >> error_calculation_report.txt 
    string_num1=${string_num}
    for ((num_bits=0; num_bits<${string_size}; num_bits++))
    do
        data_array[$num_bits]=`expr ${string_num1} % 2`
        string_num1=`expr ${string_num1} / 2`

        if [ ${string_num1} -eq 0 ]
        then
            string_num1=${string_num}
        fi
    done
    
    ## Printing message
    for ((num_bits=0; num_bits<${string_size}; num_bits++))
    do 
    echo -n "${data_array[$num_bits]}" >> error_calculation_report.txt
    done

    echo " " >> error_calculation_report.txt

		######### 1. Generate pintrace for sender
		cd ${pin_files_path} 

		./commands_sender_all_1.sh ${string_num} ${string_size} ${path_to_copy_trace}


		#### run single-core experiment to estimate sender's LLC occupancy ####

		########### 1.a Extract the number of instructions in the trace file.
		sim_count_sender=`grep "instrCount" ${pin_output_file_sender} | awk '{print $4}'`

		echo "Number of instructions traced for sender: ${sim_count_sender}" >> error_calculation_report.txt

		cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}


		########### 2. Run script for sender
		trace_sender=champsim.trace_sender_${string_size}_${string_num}.gz
		./run_build_1core.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed}


		########### 2.a) Extract LLC occupancy of sender
		#result_file=`ls -lthr | tail -1 | awk '{ print $9 }'`
		result_file=result_random_1_champsim.trace_sender_${string_size}_${string_num}.gz.txt
		lines_to_skip=`grep -n "Cache state before clflush" ${result_file} | head -n 1 | awk -F ':' '{print $1}'`  #XXX this will not work, if all are zero in a string.
		buffer_lines=40
		lines_to_skip=`expr $lines_to_skip + ${buffer_lines}`
		LLC_occupancy_of_sender=`cat ${result_file} | head -n ${lines_to_skip} | tail -n ${buffer_lines} | grep "Tag LLC Valid blocks" | awk '{print $5}' | head -2 | tail -1`
		echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}" >> error_calculation_report.txt  # Sender's occupancy will not change for a specific trace-file.

		if [ $is_it_sender == 1 ]
		then
				echo "Sender's turn is over." >> error_calculation_report.txt
		fi


######### 3. Run script for receiver
#is_it_sender=0
#if [ $check -eq 1 ]
#then
#if [ $is_it_sender == 1 ]
#then
#    echo "It is receiver's turn to run. Is_it_sender: ${is_it_sender}"
#fi

#unroll_fact=(32)
receiver_num_access_in_rdtsc=32
other_blocks=328     #1638
#for ((ki=0; ki<${#unroll_fact[@]}; ki++))
#do
#  receiver_num_access_in_rdtsc=${unroll_fact[$ki]}
#  echo "======================= starting for unrolling factor ${receiver_num_access_in_rdtsc} ======================="


#pin_output_file_receiver=1_multiple_access_${receiver_num_access_in_rdtsc}_for_all_1.txt
#trace_receiver=champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_for_all_1_other_blocks_${other_blocks}.gz

#LLC_occupancy_of_receiver=0
#Total=`expr $LLC_occupancy_of_receiver + $LLC_occupancy_of_sender`
#echo "line 51, Total: $Total"

#receiver_arr_size=259288  #248848 #261048  #some number to start with.
#let a="($receiver_arr_size / $array_elements_in_one_cache_block) % $receiver_num_access_in_rdtsc" 
#extra=${a} #`expr $receiver_arr_size % $receiver_num_access_in_rdtsc`
#echo "line 54, receiver array size is: ${receiver_arr_size} extra cache blocks are : ${extra}"

#tim=0

#while [ ${Total} -ne ${LLC_NUM_BLOCKS} ]
#do
#   cd ${pin_files_path} 
#   ./commands_receiver_all_1.sh ${receiver_num_access_in_rdtsc} ${receiver_arr_size} ${extra} ${other_blocks}
#   sim_count_receiver=`grep "instrCount" ${pin_output_file_receiver} | awk '{print $4}'`
   
#   echo "Number of instructions traced for receiver: ${sim_count_receiver}"
#   cd ../../../../ChampSim_for_mirage_forward_fill_early/error_calculation
#  ./run_build_1core.sh ${sim_count_receiver} ${trace_receiver}

#  if [ ${tim} -eq 3  ] #### Terminate the loop if it is running for more than 5 times.
#  then
#      exit
#  fi
 
#  tim=`expr ${tim} + 1`  
#  echo $tim
  
 
  #3.a) Extract marker for region of interest
#  result_file=result_random_1_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_for_all_1_other_blocks_${other_blocks}.gz.txt
#  lines_to_skip=`grep -n "wait" ${result_file} | head -n 1 | awk -F ':' '{print $1}'`
#  buffer_lines=50
#  lines_to_skip=`expr $lines_to_skip + ${buffer_lines}`
#  LLC_occupancy_of_receiver=`cat ${result_file} | head -n ${lines_to_skip} | tail -n ${buffer_lines} | grep "Tag LLC Valid blocks" | awk '{print $5}' | head -n 1 | tail -n 1`
#  Total=`expr $LLC_occupancy_of_receiver + $LLC_occupancy_of_sender`
#  echo "line 81, Total $Total"

#  if [ ${Total} -gt ${LLC_NUM_BLOCKS} ];
#  then
#      let result="$receiver_arr_size - ( ($Total - $LLC_NUM_BLOCKS) * 8)"
#      receiver_arr_size=${result}
#      echo "I am in if"
#  elif [ ${Total} -lt ${LLC_NUM_BLOCKS} ];
#  then
#      let result="$receiver_arr_size + ( ( $LLC_NUM_BLOCKS - $Total) * 8)"
#      receiver_arr_size=${result}
#      echo "I am in elif" 
#  fi

#  let a="($receiver_arr_size / $array_elements_in_one_cache_block) % $receiver_num_access_in_rdtsc"
#  extra=$a
  
#  echo "new receiver_arr_size: ${receiver_arr_size}, extra: ${extra}"

#done

#echo "Receiver LLC occupancy: ${LLC_occupancy_of_receiver}, Sender LLC occupancy: ${LLC_occupancy_of_sender}" # Adjust Receiver occupancy, don't change Sender occupancy.


#result_file=result_random_1_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_for_all_1_other_blocks_${other_blocks}.gz.txt
#receiver_setup_wait=`grep -nri "wait" ${result_file} | head -n 1 | awk '{ print $12 }'`

#echo "receiver_setup_wait ${receiver_setup_wait}"

######### Run a 2-core simulation with wait_implementation off.
trace1=champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz
trace2=champsim.trace_sender_${string_size}_${string_num}.gz

    echo "Seed: ${seed}" >> error_calculation_report.txt
    ./run_build_2core.sh ${sim_count_sender} ${trace1} ${trace2} ${seed} ${string_size}
    result_file=result_random_2_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}_other_blocks_${other_blocks}_${string_size}.gz_champsim.trace_sender_${string_size}_${string_num}.gz.txt


    received_bits=(`grep -nri "bit received is" ${result_file} | awk '{print $7}'`)
   
    for ((num_bits=0; num_bits<${string_size}; num_bits++))
    do
    echo -n "${received_bits[$num_bits]}" >> error_calculation_report.txt
    done
    echo " " >> error_calculation_report.txt
    count=0
    for ((num_bits=0; num_bits<${string_size}; num_bits++))
    do 
        if [ ${data_array[$num_bits]} -eq  ${received_bits[$num_bits]} ]
        then
            count=`expr $count + 1`
        fi
    done 

    echo "Error: `expr ${string_size} - ${count}` " >> error_calculation_report.txt
    echo -e " \n \n \n" >> error_calculation_report.txt

done

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

#done

######### Set wait_implementation on.
#fi
