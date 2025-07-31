num_prl=1

if [ $num_prl -eq 1 ];
then
        echo "Set the number of parallel simulations as num_prl and comment the below exit."
        exit
fi

cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks_for_high_UF/error_calculation/LR_experiments

str_num=2 #500  #XXX For a test run and to quickly check the bandwidth trend in the results, keep $str_num as 2. Give the script 5 to 6 hours to complete for the test run. Please don't interrupt the build process of ChampSim simulator.

#Generate sender traces.
./generating_sender_traces_fig10.sh 1 $str_num

#Run simulations
./start_fig10.sh 163 16384 $num_prl $str_num
./start_fig10.sh 652 65536 $num_prl $str_num
./start_fig10_2mb.sh 326 1 $str_num $num_prl


cd /home/yashikav/Desktop/Mirage_project/fig10/results_analysis_scripts
python3 data_processing_with_err_corr_31_other_blocks.py 0 163
python3 data_processing_with_err_corr_31_other_blocks.py 0 326
python3 data_processing_with_err_corr_31_other_blocks.py 0 652

cd ../

cp /home/yashikav/Desktop/Mirage_project/fig10/results_analysis_scripts/Total_result_*.txt .

##Run SPP simulations and generate results.
##for 1MB and 4MB LLC.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4

./trigger_fig10.sh 1 1 1 1 1 $str_num $num_prl 
##./trigger_fig10.sh 1 0 1 1 1 $str_num $num_prl 
./trigger_fig10.sh 4 1 1 1 1 $str_num $num_prl 
##./trigger_fig10.sh 4 0 1 1 1 $str_num $num_prl 

##for 2MB LLC.
##./trigger_2mb_fig10_train.sh 1 $str_num $num_prl #not needed for bandwidth calculation.
./trigger_2mb_fig10_test.sh 1 $str_num $num_prl

#cd /home/yashikav/Desktop/Mirage_project/fig10/spp_results_analysis_scripts_train
##python3 process_data.py
##python3 generate_min_max_latency_diff.py

#cd /home/yashikav/Desktop/Mirage_project/fig10/spp_results_analysis_scripts_train/calculate_optimal_threshold
##python3 extract_optimal_threshold.py
##python3 extract_min_error_count.py

cd /home/yashikav/Desktop/Mirage_project/fig10/spp_results_analysis_scripts_test
python3 process_data_only_bw.py
python3 extract_numbers_for_bandwidth.py

#Generate plot data.
cd /home/yashikav/Desktop/Mirage_project/fig10/
python3 generate_plot_data.py

#Generate plots.
gnuplot plot_lr_spp_llc_size_high_UF.p
epstopdf bw_llc_size_sensitivity_high_UF.eps
mv bw_llc_size_sensitivity_high_UF.pdf fig10.pdf
