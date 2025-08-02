num_prl=1
num_prl_mul5=1

if [ $num_prl -eq 1 ];
then
	echo "Set the number of parallel simulations as num_prl and comment the below exit. Also set num_prl_mul5 appropriately."
	echo "num_prl_mul5 = 1 launch 5 experiments ; num_prl_mul5 = 2 launch 10 experiments and so on."
	exit
fi

#XXX For test run keep $str_num_test and $str_num_train as 2, give the script around 5 to 6 hours for a test run to complete. 
#XXX To generate the exact results the str_num_test should be 500, str_num_train should be 250.
str_num_test=2 
str_num_train=2

#Generate sender traces.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4

#Run experiments.
 ./trigger_fig4b_train_with_ref.sh 1 ${str_num_train} $num_prl_mul5 # num_prl_mul5 = 1 launch 5 experiments ; num_prl_mul5 = 2 launch 10 experiments and so on.
 ./trigger_fig4b_test_with_ref.sh 1 ${str_num_test} $num_prl_mul5 # num_prl_mul5 = 1 launch 5 experiments ; num_prl_mul5 = 2 launch 10 experiments and so on.
 ./trigger_fig4b_without_ref.sh 2 0 1 1 1 ${num_prl} ${str_num_train}
 ./trigger_fig4b_without_ref.sh 2 1 1 1 1 ${num_prl} ${str_num_test}
#exit

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_with_ref
python3 process_data.py
python3 generate_min_max_latency_diff.py

cd /home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_with_ref/calculate_optimal_threshold
python3 extract_optimal_threshold.py
python3 extract_min_error_count.py

cd /home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref
python3 process_data.py
python3 extract_numbers_for_bandwidth.py

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_without_ref
python3 process_data.py
python3 generate_min_max_latency_diff.py

cd /home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_without_ref/calculate_optimal_threshold
python3 extract_optimal_threshold.py
python3 extract_min_error_count.py

cd /home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref
python3 process_data.py
python3 extract_numbers_for_bandwidth.py

cd ../

#Generate plot data.
python3 generate_plot_data.py

#Generate plots.
gnuplot plot_spp_double_bar.p
epstopdf bwerror-spp-with-without.eps
mv bwerror-spp-with-without.pdf fig4b.pdf
