#!/bin/bash

sen_dis=$1
string_size=$2
benchmark_test=$3
receiver_array_size=$4
repeat_new_crfill=$5
lr_like_probe=$6
start_point=$7
end_point=$8
num_prl=$9
unroll_fact=${10}
slide_width=${11}
arr_size_mul=${12}
history_length=${13}

echo "slide_width: $slide_width"

#exit

seed=476473680  #1000 
LLC_size=2 #How big of a role is this playing ?

# Check if correct unroll_fact value is passed.
if [ $unroll_fact -ne 16 ] && [ $unroll_fact -ne 32 ] && [ $unroll_fact -ne 64 ] && [ $unroll_fact -ne 128 ]; then
    echo "Value for unroll_fact is not 16, 32, 64, or 128."
    exit 1
fi

if [ $unroll_fact -eq 16 ]; then
    hit_miss_threshold=124
elif [ $unroll_fact -eq 32 ]; then
    hit_miss_threshold=134
elif [ $unroll_fact -eq 64 ]; then
    hit_miss_threshold=233
elif [ $unroll_fact -eq 128 ]; then
    hit_miss_threshold=316
fi

pin_files_path=/home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code
pin_output_file_sender=all_1_sender.txt
array_elements_in_one_cache_block=8
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments/pin_traces
back_to_exp_dir=/home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments
Result_dir=/home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments/results
result_dir=/home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments/NEW_COV_CH_2core_LR_PROBE_${sen_dis}_version2

mkdir -p NEW_COV_CH_2core_LR_PROBE_${sen_dis}_version2
mkdir -p binary_new_cov_ch
mkdir -p results
mkdir -p pin_traces
mkdir -p binary_new_cov_ch/${sen_dis}
rm ../../bin/bimodal-no-no-random*
string_num=$start_point
build_count=0

while [ $string_num -le $end_point ]
do
    cd ${pin_files_path}
    ./commands_sender_new_covert_channel_version2_general.sh ${sen_dis} ${string_size} ${string_num} ${path_to_copy_trace} ${benchmark_test} ${slide_width} ${arr_size_mul} 
    sim_count_sender=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_sender} | awk '{print $2}'`
    echo "sim_count: $sim_count_sender"

    if [ ${benchmark_test} -eq 1 ];
    then
        trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_arr_size_${sen_dis}_new_cov_ch_test.gz
    else
        trace_sender=champsim.trace_sender_${string_size}_${string_num}_${sim_count_sender}_arr_size_${sen_dis}_new_cov_ch_train.gz
    fi
    cd ${back_to_exp_dir}

    if [ ${build_count} -eq 0 ];
    then
        ./run_build_1core_err_corr.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir}
        build_count=`expr $build_count + 1 `
    else
        ./run_1core_err_corr.sh ${sim_count_sender} ${trace_sender} ${string_size} ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir}
    fi
    string_num=`expr $string_num + 1 `
done

###### generate receiver trace ######
pin_output_file_receiver=receiver_new_cov_ch.txt
trace_receiver=champsim.trace_receiver_${string_size}_new_cov_ch_lr_probe_uf_${unroll_fact}.gz

cd ${pin_files_path} 
./commands_receiver_new_covert_channel_lr_probe.sh ${receiver_array_size} ${string_size} ${path_to_copy_trace} ${LLC_size} ${repeat_new_crfill} ${unroll_fact}
sim_count_receiver=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver} | awk '{print $2}'`
echo "sim_count: $sim_count_receiver"
#exit
cd ${back_to_exp_dir}

./run_build_1core_err_corr.sh ${sim_count_receiver} ${trace_receiver} ${string_size} ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} &

echo "Completed till here."  
# kill the process after a minute.
sleep 60
process_name="bimodal-no-no-random-1core"
pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
echo "pid is: $pid"
killing_status=`kill -9 ${pid}`
echo "killing_status: ${killing_status}"

#exit

sed -i.bak 's/\<NEW_COVERT_CHANNEL 0\>/NEW_COVERT_CHANNEL 1/g' ../../inc/champsim.h
sed -i.bak 's/\<NEW_COVERT_CHANNEL_LR_PROBE 0\>/NEW_COVERT_CHANNEL_LR_PROBE '${lr_like_probe}'/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK 0\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'/g' ../../inc/champsim.h

./run_build_2core_err_corr.sh ${sim_count_sender} ${trace_receiver} ${trace_sender} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} &

sleep 60

sed -i.bak 's/\<NEW_COVERT_CHANNEL 1\>/NEW_COVERT_CHANNEL 0/g' ../../inc/champsim.h
sed -i.bak 's/\<NEW_COVERT_CHANNEL_LR_PROBE '${lr_like_probe}'\>/NEW_COVERT_CHANNEL_LR_PROBE 0/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h

process_name="bimodal-no-no-random-2core"
pid=`ps aux | grep $process_name | grep -v "grep" | head -1 | awk '{print $2 }'`
echo "pid is: $pid"
killing_status=`kill -9 ${pid}`
echo "killing_status: ${killing_status}"

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

            search_string1="champsim.trace_sender_${string_size}_${STR_NUM}_"
            if [ $benchmark_test == 0 ]
            then
                search_string2="_arr_size_${sen_dis}_new_cov_ch_train.gz"
            elif [ $benchmark_test == 1 ]
            then
                search_string2="_arr_size_${sen_dis}_new_cov_ch_test.gz"
            fi
            sim_count=0

            # Use a loop to iterate through each file in the directory
            for filename in $(ls "$path_to_copy_trace"); do

                # Check if the filename contains the search string
                if [[ $filename == *"$search_string1"* && $filename == *"$search_string2"* ]]; then
                    echo "Found match: $filename"

                    # Split the filename using both '_' and '.' as delimiters
                    IFS='_.' read -ra array <<< "$filename"

                    # Extract the desired part (index 5 in this case)
                    sim_count="${array[5]}"
                    break
                fi
            done
            #exit
            receiver_trace=champsim.trace_receiver_${string_size}_new_cov_ch_lr_probe_uf_${unroll_fact}.gz
            receiver_trace_path=/home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments/pin_traces/${receiver_trace}

            if [ $benchmark_test == 0 ]
            then
                sender_trace=champsim.trace_sender_${string_size}_${STR_NUM}_${sim_count}_arr_size_${sen_dis}_new_cov_ch_train.gz
            elif [ $benchmark_test == 1 ]
            then
                sender_trace=champsim.trace_sender_${string_size}_${STR_NUM}_${sim_count}_arr_size_${sen_dis}_new_cov_ch_test.gz
            fi
            sender_trace_path=/home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments/pin_traces/${sender_trace}

            ./run_2core_LR_ERR_CORR_both_algo.sh $sim_count $receiver_trace_path $sender_trace_path $seed $string_size $result_dir $binary_path &
            #./run_build_2core_err_corr.sh ${sim_count_sender} ${trace_receiver} ${trace_sender} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir}
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

