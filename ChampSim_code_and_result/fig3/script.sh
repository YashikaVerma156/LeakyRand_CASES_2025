
#Generate sender traces.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

#Run experiments.
./fig3.sh
./fig3_multiple.sh

#Generate results from raw result files.
cp latest_clear_cache_stats_1_new.txt /home/yashikav/Desktop/Mirage_project/fig3
cp latest_clear_cache_stats_1_new_5_itr.txt /home/yashikav/Desktop/Mirage_project/fig3

cd /home/yashikav/Desktop/Mirage_project/fig3

#Generate plot data.
python3 generate_plot_data.py

#Generate plots.
gnuplot plot_lr_region_identification_99_percent.p
epstopdf region_identification_99_percent.eps
mv region_identification_99_percent.pdf fig3.pdf
