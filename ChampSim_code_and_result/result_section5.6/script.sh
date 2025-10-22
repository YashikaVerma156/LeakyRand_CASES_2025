num_prl=1
str_num_test=1 #TODO str_num_test should be 500 to generate the results reported in the paper. But, keep str_num_test=1 for a test run.
str_num_train=1 #TODO str_num_train should be 250 to generate the results reported in the paper. But, keep str_num_train=1 for a test run.

cd /home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments
./start_new_covert_channel_intelligent_sender_version2.sh 0 ${num_prl} ${str_num_train}
./start_new_covert_channel_intelligent_sender_version2.sh 1 ${num_prl} ${str_num_test}

cd /home/yashikav/Desktop/Mirage_project/new_covert_channel/ChampSim_for_mirage_new_covert_channel/error_calculation/LR_experiments/results_generation_scripts
python3 process_data.py 0 6250
python3 process_data.py 1 6250

gcc parse_multiple_files_edited_error_calc_including_initial_bits.c -o parse
./parse benchmark_train.txt data_6250_train_suite_version2_corrected 3 data_6250_test_suite_version2_corrected benchmark_test.txt 2

#Channel Bandwidth observed.
python3 extract_numbers_for_bandwidth.py
