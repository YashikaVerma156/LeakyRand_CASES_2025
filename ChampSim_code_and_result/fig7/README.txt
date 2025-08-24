1. This directory has scripts to generate the fig7.
2. Run script.sh, provide the count for parallel simulations( $num_prl in script.sh ) that can be scheduled in your system.
3. In script.sh provide $str_num=1 for a test run. $str_num should be 50 to calculate the bit error rate. The test run will take around an hour to complete. Please don't interrupt the ChampSim build process. 
4. Time and storage requirements.
a) Time requirements if $num_prl in script.sh is atleast 5, total experiments(str_num = 50 and 12 configurations i.e. 50*12) should complete in around 10 hours. 
b) Storage requirement for storing the result files and trace files: 100 GB.
5. At the end, the script.sh generate fig7.pdf which can be compared with the figure 7 plot in LeakyRand paper.
6. our_data.txt has the original data used to plot figure 7 in LeakyRand paper.
