
#Generate sender traces.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_to_mimick_DMA_invalidations/error_calculation/LR_experiments

#Run experiments.
./trigger_invalidation_program.sh

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/fig1/results_analysis_scripts
./result_extraction.sh
cd ..
cp results_analysis_scripts/extracted_result_llc_misses.txt .

#Generate plot data.
python3 generate_plot_data.py

#Generate plots.
gnuplot plot_llc_miss_io_calls.p
epstopdf replacement_llc_miss_io_calls.eps
mv replacement_llc_miss_io_calls.pdf fig1.pdf
