num_prl=1

if [ $num_prl -eq 1 ];
then
        echo "Set the number of parallel simulations as num_prl and comment the below exit."
        exit
fi

cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_sender_access_and_flush_multiple_blocks/error_calculation/LR_experiments

str_num=500  #1 #500

#Generate sender traces.
./generating_sender_traces_fig5.sh 1 326 $str_num

#Run simulations
./start_fig5.sh 326 $num_prl $str_num

cd /home/yashikav/Desktop/Mirage_project/fig5/results_analysis_scripts
python3 data_processing.py

cd ../

cp /home/yashikav/Desktop/Mirage_project/fig5/results_analysis_scripts/Total_result_*.txt .

#Generate plot data.
python3 generate_plot_data.py

#Generate plots.
gnuplot lr_overhead_script.p
epstopdf overhead_lr.eps
mv overhead_lr.pdf fig5.pdf
