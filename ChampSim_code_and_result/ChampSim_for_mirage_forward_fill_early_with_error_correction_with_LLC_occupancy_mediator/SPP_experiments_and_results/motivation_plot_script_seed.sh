##################### This script is for SPP experiments ###########################

#####XXX Variables #####

check=1

string_size=4 #100 125 150 175 200  #XXX this needs to be set dynamically, remove the dependency of the folder name.
seed=7633  #24872  #36578 #24872  #7633
LLC_NUM_BLOCKS=32768
sender_arr_size_list=(6554) # (3276 6554 9830 13108 16384)  #8192  #32768  #16384 # 16384  #500
repetition_coding=1
differential_signalling=2

pin_files_path=/home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_code_folder
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/SPP_experiments_and_results
pin_output_file_sender=all_1_sender_seed.txt
array_elements_in_one_cache_block=8
is_it_sender=1

sim_count_sender_arr=()

## Build single-core and multi-core binaries of SPP and copy to a newer location.

sed -i.bak 's/\<IEEE_SEED 0\>/IEEE_SEED 1/g' ../../inc/champsim.h

            ### Generate sender traces ###
for ((kk=0; kk<${#sender_arr_size_list[@]}; kk++))
do
			for ((kk=0; kk<${#sender_arr_size_list[@]}; kk++))
			do
						sender_arr_size=${sender_arr_size_list[$kk]}
						string_num=24469  #31 #  24469  #24618
						trace_sender=champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed.gz

						######### 1. Generate pintrace for sender
						cd ${pin_files_path} 

						if [ $is_it_sender == 1 ]
						then
									echo "It is sender's turn to run. Is_it_sender: ${is_it_sender} "
						fi

						./commands_sender_all_1_seed.sh ${string_num} ${string_size} ${sender_arr_size} ${repetition_coding} ${differential_signalling} ${path_to_copy_trace}  

						########### 1.a Extract the number of instructions in the trace file.
						sim_count_sender=`grep "instrCount" ${pin_output_file_sender} | awk '{print $4}'`
						sim_count_sender_arr+=(${sim_count_sender})

						echo "Number of instructions traced for sender: ${sim_count_sender}"

						cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}

						########### 2. Run script for sender
						./run_build_1core.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} 0 0 ${repetition_coding} ${differential_signalling}

						########### 2.a) Extract LLC occupancy of sender
						result_file=result_random_1_champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed.gz.txt
						LLC_occupancy_of_sender=`grep "Tag LLC Valid blocks" ${result_file}| tail -7 | head -1 | awk '{print $5}'`
						echo "LLC_occupancy_of_sender: ${LLC_occupancy_of_sender}"  # Sender's occupancy will not change for a specific trace-file.

      done
done

if [ $is_it_sender == 1 ]
then
    echo "Sender's turn is over."
fi

#exit

######### 3. Run script for receiver
is_it_sender=0
if [ $check -eq 1 ]
then
if [ $is_it_sender == 1 ]
then
    echo "It is receiver's turn to run. Is_it_sender: ${is_it_sender}"
fi

pin_output_file_receiver=receiver_seed.txt
trace_receiver=champsim.trace_receiver_${string_size}_seed.gz

LLC_occupancy_of_receiver=0
Total=`expr $LLC_occupancy_of_receiver + $LLC_occupancy_of_sender`
echo "line 51, Total: $Total"

receiver_arr_size=262080 # 259232  #104568 #259232  #104552 #259248 #104560 #104316 #259232 #259304 #259336  #248848 #261048  #some number to start with.

cd ${pin_files_path} 
./commands_receiver_all_1_seed.sh ${receiver_arr_size} ${string_size} ${repetition_coding} ${differential_signalling} ${path_to_copy_trace}
sim_count_receiver=`grep "instrCount" ${pin_output_file_receiver} | awk '{print $4}'`
echo "Number of instructions traced for receiver: ${sim_count_receiver}"
cd ../../../../../ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}

./run_build_1core.sh ${sim_count_receiver} ${trace_receiver} ${string_size} ${seed} 0 0 ${repetition_coding} ${differential_signalling}
echo "Completed till here."  
 
#3.a) Extract marker for region of interest
result_file=result_random_1_champsim.trace_receiver_${string_size}_seed.gz.txt
LLC_occupancy_of_receiver=`grep "Tag LLC Valid blocks" ${result_file}| tail -7 | head -1 | awk '{print $5}'`
Total=`expr $LLC_occupancy_of_receiver + $LLC_occupancy_of_sender`
echo "line 81, Total $Total"

echo "Receiver LLC occupancy: ${LLC_occupancy_of_receiver}, Sender LLC occupancy: ${LLC_occupancy_of_sender}"

result_file=result_random_1_champsim.trace_receiver_${string_size}_seed.gz.txt
receiver_setup_wait=`grep -nri "wait" ${result_file} | head -n 1 | awk '{ print $12 }'`

echo "receiver_setup_wait ${receiver_setup_wait}"
 

for ((kk=0; kk<${#sender_arr_size_list[@]}; kk++))
do
      sender_arr_size=${sender_arr_size_list[$kk]}
			######### Run a 2-core simulation with wait_implementation off.
			trace1=champsim.trace_receiver_${string_size}_seed.gz
			trace2=champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed.gz
      sim_count_sender=$((sim_count_sender_arr[kk]))
			./run_build_2core.sh ${sim_count_sender} ${trace1} ${trace2} ${seed} ${string_size} 0 0 ${repetition_coding} ${differential_signalling}
			result_file=result_random_2_champsim.trace_receiver_${string_size}_seed.gz_champsim.trace_sender_${string_size}_${string_num}_arr_size_${sender_arr_size}_seed.gz.txt

      python3 collect_seed_results.py ${result_file} ${sender_arr_size}
done
sed -i.bak 's/\<IEEE_SEED 1\>/IEEE_SEED 0/g' ../../inc/champsim.h

##### Generating motivation results


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


######### Set wait_implementation on.
fi
