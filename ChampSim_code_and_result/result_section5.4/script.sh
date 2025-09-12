str_num=1 #str_num should be 500 to generate the errors reported in the paper. But, keep str_num=1 for a test run. 
num_prl=1

if [ $num_prl -eq 1 ];
then
        echo "Set the number of parallel simulations as num_prl and comment the below exit. num_prl should be set as per your system."
        exit
fi

cd /home/yashikav/Desktop/Mirage_project/ChampSim_outofsync_sender_receiver/error_calculation/LR_experiments

#Generate sender traces.
./generating_multiple_sender_traces_err_corr_both_algo_sender_access_and_flush_multiple_blocks.sh 1 326 $str_num

#Run experiments.
./start_LR_WITH_ERR_CORR_EXP_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh 326 86 1 1 $str_num $num_prl
./start_LR_WITH_ERR_CORR_EXP_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh 326 86 2 1 $str_num $num_prl
./start_LR_WITH_ERR_CORR_EXP_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh 326 86 3 1 $str_num $num_prl
./start_LR_WITH_ERR_CORR_EXP_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh 326 86 4 1 $str_num $num_prl
./start_LR_WITH_ERR_CORR_EXP_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh 326 86 5 1 $str_num $num_prl
./start_LR_WITH_ERR_CORR_EXP_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh 326 86 6 1 $str_num $num_prl

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_outofsync_sender_receiver/error_calculation/LR_experiments/results_analysis_scripts
./script.sh

# Print bandwidth.
echo ""
echo ""
echo ""
a1=`grep "Total_error" Total_result_16_train_0_128_with_err_corr_both_algo_12_access_other_blocks_326_86_1_reverified.txt | awk '{print $2}'`
echo "Total errors observed for 1% bits with receiver array probe percentage 86%: $a1"
a1=`grep "Total_error" Total_result_16_train_0_128_with_err_corr_both_algo_12_access_other_blocks_326_86_2_reverified.txt | awk '{print $2}'`
echo "Total errors observed for 2% bits with receiver array probe percentage 86%: $a1"
a1=`grep "Total_error" Total_result_16_train_0_128_with_err_corr_both_algo_12_access_other_blocks_326_86_3_reverified.txt | awk '{print $2}'`
echo "Total errors observed for 3% bits with receiver array probe percentage 86%: $a1"
a1=`grep "Total_error" Total_result_16_train_0_128_with_err_corr_both_algo_12_access_other_blocks_326_86_4_reverified.txt | awk '{print $2}'`
echo "Total errors observed for 4% bits with receiver array probe percentage 86%: $a1"
a1=`grep "Total_error" Total_result_16_train_0_128_with_err_corr_both_algo_12_access_other_blocks_326_86_5_reverified.txt | awk '{print $2}'`
echo "Total errors observed for 5% bits with receiver array probe percentage 86%: $a1"
a1=`grep "Total_error" Total_result_16_train_0_128_with_err_corr_both_algo_12_access_other_blocks_326_86_6_reverified.txt | awk '{print $2}'`
echo "Total errors observed for 6% bits with receiver array probe percentage 86%: $a1"

