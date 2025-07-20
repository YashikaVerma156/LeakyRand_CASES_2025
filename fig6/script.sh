
num_prl=1

if [ $num_prl -eq 1 ];
then
	echo "Set num_prl as number of parallel simulations that can be run in your system and comment the below exit."
	echo "num_prl should be less than the number of logical cores."
	exit
fi



#Generate sender traces.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks/error_calculation/LR_experiments

./generating_multiple_sender_traces_for_multiple_ds.sh 1 326

#Run experiments.
./start_LR_WITH_ERR_CORR_multiple_ds.sh 326 $num_prl

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/fig6/results_analysis_scripts
python3 data_processing.py 0 1
python3 data_processing.py 0 3
python3 data_processing.py 0 6
python3 data_processing.py 0 9
python3 data_processing.py 0 12

cd ../
cp results_analysis_scripts/Total_result_*.txt .

#Generate plot data.
python3 generate_plot_data.py

#Generate plots.
gnuplot plot_lr_sd.p
epstopdf bwerror-lr-ds.eps
mv bwerror-lr-ds.pdf fig6.pdf
