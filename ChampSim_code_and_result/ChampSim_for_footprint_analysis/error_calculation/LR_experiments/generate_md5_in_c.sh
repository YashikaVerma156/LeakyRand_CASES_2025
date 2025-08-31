receiver_array_size=$1
repeat_new_crfill=$2
lr_like_probe=$3
num_prl=$4
unroll_fact=$5
start_point=$6
end_point=$7
benchmark_test=$8
text_file_size=$9

seed=788444118   #1000
LLC_size=2
string_size=1
# Check if correct unroll_fact value is passed.
if [ $unroll_fact -ne 16 ] && [ $unroll_fact -ne 1 ]; then
    echo "Value for unrolling_factor is not 16."
    exit 1
fi

if [ $unroll_fact -eq 16 ]; then
    hit_miss_threshold=124
elif [ $unroll_fact -eq 1 ]; then
    hit_miss_threshold=60
fi

pin_files_path=/home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code

pin_output_file_md5=all_1_md5_in_c_1.txt
array_elements_in_one_cache_block=8
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_footprint_analysis/error_calculation/LR_experiments/pin_traces
back_to_exp_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_footprint_analysis/error_calculation/LR_experiments
Result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_footprint_analysis/error_calculation/LR_experiments/results
result_dir=/home/yashikav/Desktop/Mirage_project/result_section5.10.3/total_results

mkdir -p binary_new_cov_ch
mkdir -p binary_new_cov_ch/lr_1uf
mkdir -p binary_new_cov_ch/lr_16uf
mkdir -p binary_new_cov_ch/spp
mkdir -p binary_new_cov_ch/lr_gen
mkdir -p binary_new_cov_ch/lr_contr
mkdir -p results
mkdir -p results_new
mkdir -p pin_traces
mkdir -p binary
mkdir -p /home/yashikav/Desktop/Mirage_project/result_section5.10.3/total_results
rm ../../bin/bimodal-no-no-random*

#exit

####### Generating receiver trace that has controlled LLC setup exactly like LR. The fingerprint of md5hash is identified after CRFill + CRProbe. Hence the LLC is in controlled state. #######
controlled_array_size=259384  #Calculated array size for receiver array.
./generate_receiver_trace_for_controlled_lr_probe.sh ${benchmark_test} ${controlled_array_size} 16 514 326 4
trace_receiver_lr_controlled=champsim.trace_receiver_multiple_access_16_other_blocks_326_1_514_with_both_algo_4_algo_itr.gz
#exit
build_count=0

cd ${pin_files_path}
./commands_md5hash_in_c.sh ${path_to_copy_trace} ${text_file_size} 
sim_count_sender_md5=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_md5} | awk '{print $2}'`
echo "sim_count: $sim_count_sender_md5"
cd ${back_to_exp_dir}

trace_md5=champsim.trace_in_c_md5hash_${text_file_size}_${sim_count_sender_md5}.gz
./run_build_1core_err_corr.sh ${sim_count_sender_md5} ${trace_md5} 0 ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir}

run_all=1

if [ $run_all -eq 1 ]; then

    ####### generate LR_like receiver trace ######
    #pin_output_file_receiver=receiver_new_cov_ch.txt
    #unroll_fact=1
    #trace_receiver_lr_1=champsim.trace_receiver_${string_size}_new_cov_ch_lr_probe_uf_${unroll_fact}.gz
    #cd ${pin_files_path} 
    #./commands_receiver_new_covert_channel_lr_probe_for_footprint_analysis.sh ${receiver_array_size} ${string_size} ${path_to_copy_trace} ${LLC_size} ${repeat_new_crfill} ${unroll_fact}
    #sim_count_receiver_lr_like_1=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver} | awk '{print $2}'`
    #echo "sim_count: $sim_count_receiver_lr_like_1"
    #cd ${back_to_exp_dir}
    #./run_build_1core_err_corr.sh ${sim_count_receiver_lr_like_1} ${trace_receiver_lr_1} ${string_size} ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 

    #unroll_fact=16
    #trace_receiver_lr_16=champsim.trace_receiver_${string_size}_new_cov_ch_lr_probe_uf_${unroll_fact}.gz
    #cd ${pin_files_path}
    #./commands_receiver_new_covert_channel_lr_probe_for_footprint_analysis.sh ${receiver_array_size} ${string_size} ${path_to_copy_trace} ${LLC_size} ${repeat_new_crfill} ${unroll_fact}
    #sim_count_receiver_lr_like_16=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver} | awk '{print $2}'`
    #echo "sim_count: $sim_count_receiver_lr_like_16"
    #cd ${back_to_exp_dir}
    #./run_build_1core_err_corr.sh ${sim_count_receiver_lr_like_16} ${trace_receiver_lr_16} ${string_size} ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir}

    repeat_new_crfill_spp=1 #No need to occupy more than 63% LLC.
    pin_output_file_receiver=receiver_new_cov_ch.txt
    trace_receiver_spp_like=champsim.trace_receiver_${string_size}_new_cov_ch_${repeat_new_crfill_spp}.gz
    cd ${pin_files_path}
    ./commands_receiver_spp_like_probe.sh ${receiver_array_size} ${string_size} ${path_to_copy_trace} ${LLC_size} ${repeat_new_crfill_spp}
    sim_count_receiver_spp_like=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver} | awk '{print $2}'`
    echo "sim_count: $sim_count_receiver_spp_like"
    cd ${back_to_exp_dir}
    ./run_build_1core_err_corr.sh ${sim_count_receiver_spp_like} ${trace_receiver_spp_like} ${string_size} ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 

    pin_output_file_receiver=receiver_new_cov_ch.txt
    trace_receiver_lr_gen=champsim.trace_receiver_${string_size}_new_cov_ch_${repeat_new_crfill}.gz
    cd ${pin_files_path}
    ./commands_receiver_spp_like_probe.sh ${receiver_array_size} ${string_size} ${path_to_copy_trace} ${LLC_size} ${repeat_new_crfill}
    sim_count_receiver_lr_gen=`grep -oE 'instrCount: [0-9]+' ${pin_output_file_receiver} | awk '{print $2}'`
    echo "sim_count: $sim_count_receiver_lr_gen"
    cd ${back_to_exp_dir}
    ./run_build_1core_err_corr.sh ${sim_count_receiver_lr_gen} ${trace_receiver_lr_gen} ${string_size} ${seed} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir}
    ####### exit #######

    #hit_miss_threshold=60
    ##sed -i.bak 's/\<NEW_COVERT_CHANNEL 0\>/NEW_COVERT_CHANNEL 1/g' ../../inc/champsim.h
    ##sed -i.bak 's/\<NEW_COVERT_CHANNEL_LR_PROBE 0\>/NEW_COVERT_CHANNEL_LR_PROBE '${lr_like_probe}'/g' ../../inc/champsim.h
    #sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../../inc/champsim.h
    #sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK 0\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'/g' ../../inc/champsim.h

    #echo "${sim_count_receiver_lr_like_1}  ${sim_count_sender_md5}"
    #if [ ${sim_count_receiver_lr_like_1} -gt ${sim_count_sender_md5} ];
    #then
    #    echo "in if"
    #	sim_count1=${sim_count_receiver_lr_like_1}
    #else
    #	echo "in else"
    #	sim_count1=${sim_count_sender_md5}
    #fi

    #./run_build_2core_err_corr.sh ${sim_count1} ${trace_receiver_lr_1} ${trace_md5} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 
    #binary_path_lr_1uf=binary_new_cov_ch/lr_1uf
    #cp ../../bin/bimodal-no-no-random-2core binary_new_cov_ch/lr_1uf

    ##sed -i.bak 's/\<NEW_COVERT_CHANNEL 1\>/NEW_COVERT_CHANNEL 0/g' ../../inc/champsim.h
    ##sed -i.bak 's/\<NEW_COVERT_CHANNEL_LR_PROBE '${lr_like_probe}'\>/NEW_COVERT_CHANNEL_LR_PROBE 0/g' ../../inc/champsim.h
    #sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
    #sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h



    #hit_miss_threshold=124
    ##sed -i.bak 's/\<NEW_COVERT_CHANNEL 0\>/NEW_COVERT_CHANNEL 1/g' ../../inc/champsim.h
    ##sed -i.bak 's/\<NEW_COVERT_CHANNEL_LR_PROBE 0\>/NEW_COVERT_CHANNEL_LR_PROBE '${lr_like_probe}'/g' ../../inc/champsim.h
    #sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../../inc/champsim.h
    #sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK 0\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'/g' ../../inc/champsim.h

    #echo "${sim_count_receiver_lr_like_16}  ${sim_count_sender_md5}"
    #if [ ${sim_count_receiver_lr_like_16} -gt ${sim_count_sender_md5} ];
    #then
    #    echo "in if"
    #	sim_count2=${sim_count_receiver_lr_like_16}
    #else
    #	echo "in else"
    #	sim_count2=${sim_count_sender_md5}
    #fi

    #./run_build_2core_err_corr.sh ${sim_count2} ${trace_receiver_lr_16} ${trace_md5} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir}
    #binary_path_lr_16uf=binary_new_cov_ch/lr_16uf
    #cp ../../bin/bimodal-no-no-random-2core binary_new_cov_ch/lr_16uf

    ##sed -i.bak 's/\<NEW_COVERT_CHANNEL 1\>/NEW_COVERT_CHANNEL 0/g' ../../inc/champsim.h
    ##sed -i.bak 's/\<NEW_COVERT_CHANNEL_LR_PROBE '${lr_like_probe}'\>/NEW_COVERT_CHANNEL_LR_PROBE 0/g' ../../inc/champsim.h
    #sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
    #sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h

    echo "${sim_count_receiver_spp_like}  ${sim_count_sender_md5}"
    if [ ${sim_count_receiver_spp_like} -gt ${sim_count_sender_md5} ];
    then
        echo "in if"
	sim_count3=${sim_count_receiver_spp_like}
    else
        echo "in else"
	sim_count3=${sim_count_sender_md5}
    fi

    ./run_build_2core_err_corr.sh ${sim_count3} ${trace_receiver_spp_like} ${trace_md5} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 
    binary_path_spp=binary_new_cov_ch/spp
    cp ../../bin/bimodal-no-no-random-2core binary_new_cov_ch/spp

    echo "${sim_count_receiver_lr_gen}  ${sim_count_sender_md5}"
    if [ ${sim_count_receiver_lr_gen} -gt ${sim_count_sender_md5} ];
    then
        echo "in if"
	sim_count4=${sim_count_receiver_lr_gen}
    else
	echo "in else"
	sim_count4=${sim_count_sender_md5}
    fi

    ./run_build_2core_err_corr.sh ${sim_count4} ${trace_receiver_lr_gen} ${trace_md5} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir} 
    #exit
    binary_path_lr_gen=binary_new_cov_ch/lr_gen
    cp ../../bin/bimodal-no-no-random-2core binary_new_cov_ch/lr_gen
fi


hit_miss_threshold=124
sim_count_receiver_lr_controlled=350365
#sed -i.bak 's/\<NEW_COVERT_CHANNEL 0\>/NEW_COVERT_CHANNEL 1/g' ../../inc/champsim.h
#sed -i.bak 's/\<NEW_COVERT_CHANNEL_LR_PROBE 0\>/NEW_COVERT_CHANNEL_LR_PROBE '${lr_like_probe}'/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD 0\>/HIT_MISS_THRESHOLD '${hit_miss_threshold}'/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK 0\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'/g' ../../inc/champsim.h

echo "${sim_count_receiver_lr_controlled}  ${sim_count_sender_md5}"
if [ ${sim_count_receiver_lr_controlled} -gt ${sim_count_sender_md5} ];
then
		echo "in if"
		sim_count5=${sim_count_receiver_lr_controlled}
else
		echo "in else"
		sim_count5=${sim_count_sender_md5}
fi

./run_build_2core_err_corr.sh ${sim_count5} ${trace_receiver_lr_controlled} ${trace_md5} ${seed} ${string_size} 0 0 0 0 0 ${path_to_copy_trace} ${Result_dir}
binary_path_lr_contr=binary_new_cov_ch/lr_contr
cp ../../bin/bimodal-no-no-random-2core binary_new_cov_ch/lr_contr

#sed -i.bak 's/\<NEW_COVERT_CHANNEL 1\>/NEW_COVERT_CHANNEL 0/g' ../../inc/champsim.h
#sed -i.bak 's/\<NEW_COVERT_CHANNEL_LR_PROBE '${lr_like_probe}'\>/NEW_COVERT_CHANNEL_LR_PROBE 0/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD 0/g' ../../inc/champsim.h
sed -i.bak 's/\<HIT_MISS_THRESHOLD_LLC_OCC_CHK '${hit_miss_threshold}'\>/HIT_MISS_THRESHOLD_LLC_OCC_CHK 0/g' ../../inc/champsim.h


STR_NUM=${start_point}
echo "STR_NUM: $STR_NUM"

#exit

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

            if [ $run_all -eq 1 ]; then    
		#./run_2core_LR_ERR_CORR_both_algo.sh ${sim_count1} ${path_to_copy_trace}/${trace_receiver_lr_1} ${path_to_copy_trace}/${trace_md5} $seed $string_size $result_dir $binary_path_lr_1uf &
		#sleep 1
		#./run_2core_LR_ERR_CORR_both_algo.sh ${sim_count2} ${path_to_copy_trace}/${trace_receiver_lr_16} ${path_to_copy_trace}/${trace_md5} $seed $string_size $result_dir $binary_path_lr_16uf &
		#sleep 1
		./run_2core_LR_ERR_CORR_both_algo.sh ${sim_count3} ${path_to_copy_trace}/${trace_receiver_spp_like} ${path_to_copy_trace}/${trace_md5} $seed $string_size $result_dir $binary_path_spp &
		sleep 2
	        ./run_2core_LR_ERR_CORR_both_algo.sh ${sim_count4} ${path_to_copy_trace}/${trace_receiver_lr_gen} ${path_to_copy_trace}/${trace_md5} $seed $string_size $result_dir $binary_path_lr_gen &
		sleep 2
            fi
            ./run_2core_LR_ERR_CORR_both_algo.sh ${sim_count5} ${path_to_copy_trace}/${trace_receiver_lr_controlled} ${path_to_copy_trace}/${trace_md5} $seed $string_size $result_dir $binary_path_lr_contr &

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

