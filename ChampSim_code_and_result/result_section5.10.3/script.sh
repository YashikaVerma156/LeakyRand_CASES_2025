num_prl=1
str_num=3 #TODO str_num should be 500 to generate the error count reported in the paper. But, keep str_num=3 for a test run.
if [ $num_prl -eq 1 ];
then
        echo "Set the number of parallel simulations as num_prl and comment the below exit. Please note that for x value of $num_prl, 3x processes are launched parallely. "
        exit
fi

cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_footprint_analysis/error_calculation/LR_experiments

#Run experiments.
./start_experiment_md5_in_c.sh ${str_num} ${num_prl}

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/result_section5.10.3/results_analysis_scripts
python3 process_data_latest.py
python3 error_cal_best_threshold.py

