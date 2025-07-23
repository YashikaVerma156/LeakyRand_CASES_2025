
#1. Enter the code directory.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

echo "steps.csv is pre-computed"

#2. To verify the generation of steps.csv
./fig11a.sh
echo "For arr_size 1 , num_iterations 1 : The results of latest_clear_cache_stats_1_new_fig11a.txt are same as that of steps.csv"
echo "For arr_size 1.2 , num_iterations 9 : The results of latest_clear_cache_stats_1_new_fig11a.txt are same as that of steps.csv"

#3. Calculating dynamic steps for expected LLC occupancy 32522 and cpu_cycles budget of 7404054. 7404054 is the CRFill and CRProbe budget(in cpu cyles) of arr_size 1.2 and num_iterations 9.
cd /home/yashikav/Desktop/Mirage_project/fig11
gcc dynamic_search.c
./a.out steps.csv 32522 7404054 > program_output_for_1_percent_unoccupied_llc.txt

#4. The best budget_saved in dynamic algo is 168930 (can be verified in program_output_for_1_percent_unoccupied_llc.txt ) for occupancy sequence (1c)^1 (1.1c)^2 (1.2c)^6 . This implies that the cycles saved

#5. Run the ChampSim simulation of the best occupancy sequence observed i.e. (1c)^1 (1.1c)^2 (1.2c)^6 .
# Enter the code directory.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning
./clear_cache_basic_script_dynamic_strategy_combination_of_three_array_sizes.sh 1 1 1.1 2 1.2 6
simulated_number=$(cat temp.txt)
estimated_number=$(( 7404054 - 168930 ))
echo "${simulated_number} ${estimated_number}"

cp simulated_occ.txt /home/yashikav/Desktop/Mirage_project/fig11/

cd /home/yashikav/Desktop/Mirage_project/fig11

#Generate plot data.
./process_estimated_occupancy.sh
python3 generate_plot_data.py

#Generate plots.
gnuplot plot_lr_region_identification.p
epstopdf region_identification_best_dyn.eps
mv region_identification_best_dyn.pdf fig11.pdf
