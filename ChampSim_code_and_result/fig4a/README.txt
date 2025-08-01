1. This directory has scripts to generate figure 4a.
2. Run script.sh, provide the count for parallel simulations( $num_prl in script.sh ) that can be scheduled in your system.
3. In script.sh provide $str_num=1 for a test run. With $str_num = 1, the trend in bandwidth obtained will be similar to what is reported in the paper. $str_num should be 500 to calculate the bit error rate. The test run will take around a day to complete. Please don't interrupt the ChampSim build process. 
4. Time and storage requirements.
a) Time requirements if $num_prl in script.sh is 5, total experiments(str_num = 500 and 14 configurations i.e. 500*14) should complete in around 3 weeks. 
b) Storage requirement for storing the result files and trace files: 2TB.
5. At the end, the script.sh generate fig4a.pdf which can be compared with the figure 4a in our paper.
