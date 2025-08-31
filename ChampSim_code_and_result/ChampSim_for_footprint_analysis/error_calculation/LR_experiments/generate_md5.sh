receiver_array_size=$1
repeat_new_crfill=$2
lr_like_probe=$3
num_prl=$4
unroll_fact=$5

benchmark_test=0

start_point=1
end_point=10

seed=476473680   #1000
LLC_size=2
string_size=1
# Check if correct unroll_fact value is passed.
if [ $unroll_fact -ne 16 ]; then
    echo "Value for unrolling_factor is not 16."
    exit 1
fi

if [ $unroll_fact -eq 16 ]; then
    hit_miss_threshold=124
fi

pin_files_path=/home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code

pin_output_file_aes=all_1_aes.txt
pin_output_file_rsa=all_1_rsa.txt
array_elements_in_one_cache_block=8
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_footprint_analysis/error_calculation/LR_experiments/pin_traces
back_to_exp_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_footprint_analysis/error_calculation/LR_experiments
Result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_footprint_analysis/error_calculation/LR_experiments/results
result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_footprint_analysis/error_calculation/LR_experiments/results_new

mkdir -p binary_new_cov_ch
mkdir -p results
mkdir -p results_new
mkdir -p pin_traces
mkdir -p binary
rm ../../bin/bimodal-no-no-random*

build_count=0

		cd ${pin_files_path}
		./commands_md5hash.sh ${path_to_copy_trace} #Reusing the already generated traces.
		trace_md51=champsim.trace_md5hash1.gz
		trace_md52=champsim.trace_md5hash2.gz

		cd ${back_to_exp_dir}

   sim_count=10000000

    ./run_build_1core_err_corr.sh ${sim_count} ${trace_md51} 0 ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir}
    sim_count=50000000
    ./run_build_1core_err_corr.sh ${sim_count} ${trace_md52} 0 ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir}

exit #Below code is not sorted yet.

###### generate receiver trace ######
pin_output_file_receiver=receiver_new_cov_ch.txt
trace_receiver_lr=champsim.trace_receiver_${string_size}_new_cov_ch_lr_probe.gz

cd ${pin_files_path} 
./commands_receiver_new_covert_channel_lr_probe.sh ${receiver_array_size} ${string_size} ${path_to_copy_trace} ${LLC_size} ${repeat_new_crfill}
sim_count_receiver_lr_like=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver} | awk '{print $2}'`
echo "sim_count: $sim_count_receiver_lr_like"

cd ${back_to_exp_dir}

./run_build_1core_err_corr.sh ${sim_count_receiver_lr_like} ${trace_receiver_lr} ${string_size} ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 

echo "Completed till here."  
# kill the process after a minute.
sleep 40
process_name="bimodal-no-no-random-1core"
pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
echo "pid is: $pid"
killing_status=`kill -9 ${pid}`
echo "killing_status: ${killing_status}"

###### generate receiver trace ######
repeat_new_crfill=1 #No need to occupy more than 63% LLC.
pin_output_file_receiver=receiver_new_cov_ch.txt
trace_receiver_spp_like=champsim.trace_receiver_${string_size}_new_cov_ch_${repeat_new_crfill}.gz
cd ${pin_files_path}
./commands_receiver_spp_like_probe.sh ${receiver_array_size} ${string_size} ${path_to_copy_trace} ${LLC_size} ${repeat_new_crfill}
sim_count_receiver_spp_like=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver} | awk '{print $2}'`
echo "sim_count: $sim_count_receiver_spp_like"
cd ${back_to_exp_dir}
./run_build_1core_err_corr.sh ${sim_count_receiver_spp_like} ${trace_receiver_spp_like} ${string_size} ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 

#exit

sed -i.bak 's/\<NEW_COVERT_CHANNEL 0\>/NEW_COVERT_CHANNEL 1/g' ../../inc/champsim.h
sed -i.bak 's/\<NEW_COVERT_CHANNEL_LR_PROBE 0\>/NEW_COVERT_CHANNEL_LR_PROBE '${lr_like_probe}'/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK 0\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'/g' ../../inc/champsim.h

./run_build_2core_err_corr.sh ${sim_count_sender_aes} ${trace_receiver_lr} ${trace_sender_aes_with_wait} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 
binary_path=binary_new_cov_ch
cp ../../bin/bimodal-no-no-random-2core binary_new_cov_ch/
#./run_build_2core_err_corr.sh ${sim_count_sender_aes} ${trace_receiver_spp_like} ${trace_sender_aes_with_wait} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 
./run_2core_LR_ERR_CORR_both_algo.sh $sim_count_sender_aes ${path_to_copy_trace}/${trace_receiver_spp_like} ${path_to_copy_trace}/${trace_sender_aes_with_wait} $seed $string_size $result_dir $binary_path &
sleep 5
#./run_build_2core_err_corr.sh ${sim_count_receiver_lr_like} ${trace_receiver_lr} ${trace_sender_rsa_with_wait} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 
./run_2core_LR_ERR_CORR_both_algo.sh ${sim_count_receiver_lr_like} ${path_to_copy_trace}/${trace_receiver_lr} ${path_to_copy_trace}/${trace_sender_rsa_with_wait} $seed $string_size $result_dir $binary_path &
sleep 5
#./run_build_2core_err_corr.sh ${sim_count_receiver_spp_like} ${trace_receiver_spp_like} ${trace_sender_rsa_with_wait} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 
./run_2core_LR_ERR_CORR_both_algo.sh $sim_count_receiver_spp_like ${path_to_copy_trace}/${trace_receiver_spp_like} ${path_to_copy_trace}/${trace_sender_rsa_with_wait} $seed $string_size $result_dir $binary_path 
sleep 40

sed -i.bak 's/\<NEW_COVERT_CHANNEL 1\>/NEW_COVERT_CHANNEL 0/g' ../../inc/champsim.h
sed -i.bak 's/\<NEW_COVERT_CHANNEL_LR_PROBE '${lr_like_probe}'\>/NEW_COVERT_CHANNEL_LR_PROBE 0/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h

process_name="bimodal-no-no-random-2core"
pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
echo "pid is: $pid"
#killing_status=`kill -9 ${pid}`
echo "killing_status: ${killing_status}"

#exit

binary_path=binary_new_cov_ch/${sen_dis}
cp ../../bin/bimodal-no-no-random-2core binary_new_cov_ch/${sen_dis}

STR_NUM=${start_point}
echo "STR_NUM: $STR_NUM"

# Total Number of Experiments to run.
EXP_NUM=`expr \( ${end_point} - ${start_point} + 1 \) `

while [ ${EXP_NUM} -gt 0 ]
do
    #3. Run 'num_prl' experiments parallely.
    i=${num_prl}
    echo "Running parallel experiments"
    while [ ${i} -gt 0 ]
    do
            echo "i: $i STR_NUM: $STR_NUM EXP_NUM: $EXP_NUM"
            line_number=`expr $STR_NUM + 1`
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

						./run_2core_LR_ERR_CORR_both_algo.sh $sim_count_sender_aes ${path_to_copy_trace}/${trace_receiver_lr} ${path_to_copy_trace}/${trace_sender_aes_with_wait} $seed $string_size $result_dir $binary_path &
						sleep 3
						./run_2core_LR_ERR_CORR_both_algo.sh $sim_count_sender_aes ${path_to_copy_trace}/${trace_receiver_spp_like} ${path_to_copy_trace}/${trace_sender_aes_with_wait} $seed $string_size $result_dir $binary_path &
						sleep 3
						./run_2core_LR_ERR_CORR_both_algo.sh ${sim_count_receiver_lr_like} ${path_to_copy_trace}/${trace_receiver_lr} ${path_to_copy_trace}/${trace_sender_rsa_with_wait} $seed $string_size $result_dir $binary_path &
						sleep 3
						./run_2core_LR_ERR_CORR_both_algo.sh $sim_count_receiver_spp_like ${path_to_copy_trace}/${trace_receiver_spp_like} ${path_to_copy_trace}/${trace_sender_rsa_with_wait} $seed $string_size $result_dir $binary_path 

            wait

            STR_NUM=`expr ${STR_NUM} + 1`
            i=`expr ${i} - 1`
            EXP_NUM=`expr $EXP_NUM - 1`
            if [ $EXP_NUM == 0 ]
            then
                break
            fi
    done
    wait
done

