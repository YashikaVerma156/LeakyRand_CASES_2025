str_num=500 #str_num should be 500 to generate the bandwidth reported in the paper. But, keep str_num=1 for a test run. str_num=1 shows a similar trend in bandwidth.

cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_fa_lru_multithreaded_receiver_with_coherence_details/fa_lru_covert_channel/

#Run experiments.
./start_fa_lru_final.sh $str_num

#Generate results from raw result files.
cd /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_fa_lru_multithreaded_receiver_with_coherence_details/fa_lru_covert_channel/result_analysis_script

python3 data_processing_with_err_corr_31_other_blocks.py 0

# Print bandwidth.
echo ""
echo ""
echo ""
a1=`grep Total_error Total_result__train_0.txt | awk '{ print $8 }'`
echo "Bandwidth observed is: $a1"
