
#Generate results from raw result files.
cp /home/yashikav/Desktop/Mirage_project/fig4a/results_analysis_scripts/Total_result_*.txt .

#Generate plot data.
python3 generate_plot_data.py

#Generate plots.
gnuplot lr_overhead_script.p
epstopdf overhead_lr.eps
mv overhead_lr.pdf fig5.pdf
