1. This directory has scripts to generate the fig10.

SETUP:
2. Copy the required pin tool files, given in pin_files/ by following below steps.
   cd /home/yashikav/Desktop/Mirage_project/fig10/pin_files
   cp * /home/yashikav/Desktop/Mirage_project/pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/SPP_code_sender_receiver_sender_arr_size_9830/

3. Run the following command to update numexpr. This might be needed to generate results via python script. 
   pip install --upgrade numexpr

4. For a test run and to quickly check the bandwidth trend in the results, keep $str_num as 2 in script.sh,
   Give the script 5 to 6 hours to complete for the test run. Please don't interrupt the build process of ChampSim simulator.
   The bandwidth trend observed in test run will be similar to what is reported in the paper.
   To generate the original reported results, the $str_num should be 500 (i.e. a total of 6*500 experiments). All 6*500 experiments may take around a week to complete depending on the value of $num_prl.

5. set ${num_prl} as per your machine in script.sh. The value for ${num_prl} should be less than the number of logical cores available on your machine.

RUN SIMULATIONS
 ./script.sh 

RESULTS: 
6. The data used to generate the plot for fig10 is given in our_data.txt .

7. The plot generated after experiments are complete is named as fig10.pdf .
