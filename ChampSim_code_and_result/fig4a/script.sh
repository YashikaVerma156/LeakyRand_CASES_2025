
num_prl=1
str_num=1 # XXX str_num should be 500 to generate the error count and bandwidth reported in the paper. But str_num=1 gives a similar bandwidth trend and can be used for a test run.

if [ $num_prl -eq 1 ];
then
	echo "Set the number of parallel simulations as num_prl and comment the below exit."
	exit
fi

#Generate sender traces.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks/error_calculation/LR_experiments

./generating_sender_traces_fig4a.sh 1 326 ${str_num}
./generating_sender_traces_fig4a.sh 1 31 ${str_num}

#Run experiments.
./start_fig4a.sh 326 $str_num $num_prl
./start_fig4a.sh 31 $str_num $num_prl

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/fig4a/results_analysis_scripts
python3 data_processing.py 0
python3 data_processing_31.py 0

#Generate plot data.
cd ../
python3 generate_plot_data.py

#Generate plots.
gnuplot plot_lr_double_bar.p
epstopdf bwerror-99-999.eps
mv bwerror-99-999.pdf fig4a.pdf
