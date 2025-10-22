#######################################################################################################
# This script is to analyse the changed cache region identification algorithm. The decided technique  #
# is to count all the LLC hits anyway and check if the difference observed between the counted misses #
# and actual occupancy is significant.                                                                #
#######################################################################################################

pin_files_path=/home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code

pin_output_file_sender=new_cache_reg_ident.txt
array_elements_in_one_cache_block=8
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments/pin_traces
back_to_exp_dir=/home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments
Result_dir=/home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments/results

num=1
num_repeat=10
arr_size_probed=1

Total_iterations=$num_repeat

######### 1. Generate pintrace for sender
cd ${pin_files_path} 
./commands_changed_cache_region_identification_1x.sh ${num} ${num_repeat} ${path_to_copy_trace} ${arr_size_probed}

#### run single-core experiment to estimate sender's LLC occupancy ####

########### 1.a Extract the number of instructions in the trace file.
simulation_count=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_sender} | awk '{print $2}'`

echo "Number of instructions traced for sender: ${sim_count_sender}"

trace=champsim.trace_${num}_${num_repeat}_probe_${arr_size_probed}.gz
echo "$trace"
#exit
cp array_start_address.txt ${back_to_exp_dir}
cd ${back_to_exp_dir}

arr_start_addr=`cat array_start_address.txt`
echo "arr_start_addr: $arr_start_addr"

########### 2. Run script for sender
sed -i.bak 's/\<CACHE_REGION_IDENTIFICATION 0\>/CACHE_REGION_IDENTIFICATION 1/g' ../../inc/champsim.h
sed -i.bak 's/\<ARR_SIZE_PROBED 0\>/ARR_SIZE_PROBED '${arr_size_probed}'/g' ../../inc/config.h
./run_build_1core_err_corr.sh ${simulation_count} ${trace} 0 1086043 0 0 ${arr_start_addr} 0 0 ${path_to_copy_trace} ${Result_dir}
sed -i.bak 's/\<CACHE_REGION_IDENTIFICATION 1\>/CACHE_REGION_IDENTIFICATION 0/g' ../../inc/champsim.h
sed -i.bak 's/\<ARR_SIZE_PROBED '${arr_size_probed}'\>/ARR_SIZE_PROBED 0/g' ../../inc/config.h

########### 2.a) Extract LLC occupancy of sender
#result_file=`ls -lthr | tail -1 | awk '{ print $9 }'`
result_file=result_random_1_${trace}.txt
result_file_path=results/${result_file}
percentage=`grep -nri "cnt" ${result_file_path} | head -n ${Total_iterations} | tail -n ${Total_iterations}`
echo "$percentage"
