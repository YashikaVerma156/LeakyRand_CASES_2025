1. This directory has scripts to generate the fig4b.

SETUP:
2. Copy the required pin tool files, given in pin_files/ by following below steps.
   cd /home/yashikav/Desktop/Mirage_project/fig4/pin_files
   cp * /home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_code_sender_receiver_sender_arr_size_3276/
   cp * /home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_code_sender_receiver_sender_arr_size_6554/
   cp * /home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_code_sender_receiver_sender_arr_size_9830/
   cp * /home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_code_sender_receiver_sender_arr_size_13108/
   cp * /home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_code_sender_receiver_sender_arr_size_16384/

3. Run the following command to update numexpr. This might be needed to generate results via python script. 
   pip install --upgrade numexpr

4. For a test run and to quickly check the bandwidth trend in the results, keep $str_num_test and $str_num_train as 2 in script.sh,
   Give the script 5 to 6 hours to complete for the test run. Please don't interrupt the build process of ChampSim simulator.
   The bandwidth trend observed in test run will be similar to what is reported in the paper in fig4b.
   To generate the original reported results, the $str_num_test should be 500 and $str_num_train should be 250 (i.e. a total of 10*(500+250) experiments). 
   All 10*750 experiments may take around 2 to 3 weeks to complete depending on the value of $num_prl.

5. set ${num_prl} and ${num_prl_mul5} as per your machine in script.sh. 
   The value for ${num_prl} should be less than the number of logical cores available on your machine.
   num_prl_mul5 = 1 launch 5 experiments ; num_prl_mul5 = 2 launch 10 experiments and so on. 
   The num_prl_mul5 should be set appropriately to not overburden the resources of your system.

RUN SIMULATIONS
 ./script.sh 

RESULTS: 
6. The data used to generate the plot for fig4b is given in our_data.txt .

7. The plot generated after experiments are complete is named as fig4b.pdf .
