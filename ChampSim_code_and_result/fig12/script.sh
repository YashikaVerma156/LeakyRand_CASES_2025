num_prl=1

if [ $num_prl -eq 1 ];
then
        echo "Set the number of parallel simulations as num_prl and comment the below exit."
        exit
fi

cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks_for_high_UF/error_calculation/LR_experiments

str_num=500 #500

#Generate sender traces.
./generating_sender_traces_fig10.sh 1 $str_num

#Run simulations
./start_fig12.sh 163 16384 $num_prl $str_num
./start_fig12.sh 652 65536 $num_prl $str_num
./start_fig12_2mb.sh 326 1 $str_num $num_prl


cd /home/yashikav/Desktop/Mirage_project/fig12/results_analysis_scripts
python3 data_processing_with_err_corr_31_other_blocks.py 0 163
python3 data_processing_with_err_corr_31_other_blocks.py 0 326
python3 data_processing_with_err_corr_31_other_blocks.py 0 652

cd ../

cp /home/yashikav/Desktop/Mirage_project/fig12/results_analysis_scripts/Total_result_*.txt .

cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning/llc_size_sensitivity/1MB

./clear_cache_basic_script_dynamic_strategy_combination_of_three_array_sizes.sh 1 1 1.1 2 1.2 6

cp temp_1MB.txt /home/yashikav/Desktop/Mirage_project/fig12/

cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

./clear_cache_basic_script_dynamic_strategy_combination_of_three_array_sizes.sh 1 1 1.1 2 1.2 6

cp temp_2MB.txt /home/yashikav/Desktop/Mirage_project/fig12/

cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning/llc_size_sensitivity/4MB

./clear_cache_basic_script_dynamic_strategy_combination_of_three_array_sizes.sh 1 1 1.1 2 1.2 6

cp temp_4MB.txt /home/yashikav/Desktop/Mirage_project/fig12/

cd /home/yashikav/Desktop/Mirage_project/fig12/

#Generate plot data.
python3 generate_plot_data.py

#Generate plots.
gnuplot lr_overhead_script_diffr_llc_size.p
epstopdf overhead_lr_diffr_llc_size.eps
mv overhead_lr_diffr_llc_size.pdf fig12.pdf
