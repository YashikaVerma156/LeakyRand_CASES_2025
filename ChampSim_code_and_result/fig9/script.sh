num_prl=1

if [ ${num_prl} -eq 1 ];
then
        echo "Set the number of parallel simulations as num_prl and comment the below exit."
        echo "num_prl should be less than logical cores."
        exit
fi

#Generate sender traces.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks_for_high_UF/error_calculation/LR_experiments

#Trace generation
./generating_multiple_sender_traces_err_corr_both_algo_sender_access_and_flush_multiple_blocks.sh 1 500

#Run experiments.
./start_LR_WITH_ERR_CORR_EXP_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh 326 1 500 ${num_prl}

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/fig9/results_analysis_scripts

python3 data_processing_with_err_corr_31_other_blocks.py 0 32 12 
python3 data_processing_with_err_corr_31_other_blocks.py 0 64 13
python3 data_processing_with_err_corr_31_other_blocks.py 0 128 13
python3 data_processing_with_err_corr_31_other_blocks.py 0 256 18
python3 data_processing_with_err_corr_31_other_blocks.py 0 256 30

cp Total_result_32_train_0_128_with_err_corr_both_algo_12_access_other_blocks_326.txt ../
cp Total_result_64_train_0_128_with_err_corr_both_algo_13_access_other_blocks_326.txt ../
cp Total_result_128_train_0_128_with_err_corr_both_algo_13_access_other_blocks_326.txt ../
cp Total_result_256_train_0_128_with_err_corr_both_algo_18_access_other_blocks_326.txt ../
cp Total_result_256_train_0_128_with_err_corr_both_algo_30_access_other_blocks_326.txt ../

#Generate plot data.
cd /home/yashikav/Desktop/Mirage_project/fig9
python3 generate_plot_data.py 500

#Generate plots.
gnuplot plot_unroll.p
epstopdf bwerror-lr-unroll.eps
mv bwerror-lr-unroll.pdf fig9.pdf
