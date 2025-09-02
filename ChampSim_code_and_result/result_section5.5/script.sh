num_prl=1
str_num=1 #str_num should be 100 to generate the bandwidth reported in the paper. But, keep str_num=1 for a test run.

if [ $num_prl -eq 1 ];
then
	echo "Set the number of parallel simulations as num_prl and comment the below exit."
	exit
fi

cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks_LR_with_socket_latency/error_calculation/LR_experiments

#Generate sender traces.
./generating_multiple_sender_traces_err_corr_both_algo_sender_access_and_flush_multiple_blocks.sh 1 326 ${str_num}
./generating_multiple_sender_traces_err_corr_both_algo_sender_access_and_flush_multiple_blocks.sh 1 31 ${str_num}

#Run experiments.
./start_LR_WITH_ERR_CORR_EXP_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh 326 0 1 $str_num $num_prl
./start_LR_WITH_ERR_CORR_EXP_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh 31 0 1 $str_num $num_prl

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/result_section5.5/results_analysis_scripts

python3 data_processing_with_err_corr_31_other_blocks.py 0 0 1

# Print bandwidth.
echo ""
echo ""
echo ""
a1=`grep Total_error Total_result_16_train_0_128_with_err_corr_both_algo_12_access_other_blocks_326_5600_0_1.txt | awk '{ print $12 }'`
echo "Bandwidth observed for 98.88% other block population(OBP): $a1"
a2=`grep Total_error Total_result_16_train_0_512_with_err_corr_both_algo_9_access_other_blocks_31_5600_0_1.txt | awk '{ print $12 }'`
echo "Bandwidth observed for 99.76% other block population(OBP): $a2"
