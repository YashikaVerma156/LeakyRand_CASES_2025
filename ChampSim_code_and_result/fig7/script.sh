
num_prl=1
str_num=1 #str_num should be 50 to generate the error count reported in the paper. But, keep str_num=1 for a test run.

if [ $num_prl -eq 1 ];
then
	echo "Set the number of parallel simulations as num_prl and comment the below exit."
	exit
fi

cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_noise_sensitivity_study_LR/error_calculation/LR_experiments

#Generate sender traces.
./generating_multiple_sender_traces_err_corr_both_algo_sender_access_and_flush_multiple_blocks.sh 1 326 ${str_num}

#Run experiments.
./start_LR_WITH_ERR_CORR_EXP_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized.sh 326 $str_num $num_prl

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/fig7/champsim_results
./script_to_trigger_python_programs.sh

cd ../
#Generate plot data.
python3 generate_plot_data.py ${str_num}

#Generate plots.
gnuplot plot_lr_error_with_noise_ds_sensitivity.p
epstopdf error-lr-noise_ds_sensitivity.eps
mv error-lr-noise_ds_sensitivity.pdf fig7.pdf
