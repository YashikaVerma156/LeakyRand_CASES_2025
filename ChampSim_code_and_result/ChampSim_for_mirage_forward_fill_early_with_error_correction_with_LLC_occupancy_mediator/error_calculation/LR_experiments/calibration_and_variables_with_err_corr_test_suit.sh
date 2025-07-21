unroll_fact=$1    ## Specify unrolling factor
algo_trigger_point=$2    ## Specify bit number to trigger hole-filling algo. Accordingly occupancy check will be triggered.
START_POINT=1     ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
END_POINT=500  
num_prl=50        ## Number of parallel instances to be run, depends on the number of cores.
benchmark_test=1  ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
string_size=512   # Message Length.
other_blocks=327

# XXX Below setup is for 1% other blocks i.e. 327 in LLC.
if [ ${algo_trigger_point} -eq 128 ]; then
    arr_size=
elif [ ${algo_trigger_point} -eq 64 ]; then
    arr_size=
elif [ ${algo_trigger_point} -eq 32 ]; then
    arr_size=
elif [ ${algo_trigger_point} -eq 16 ]; then
    arr_size=
elif [ ${algo_trigger_point} -eq 8 ]; then
    arr_size=
fi

#echo "endpoint: ${END_POINT} startpoint: ${START_POINT} benchmark_test: ${benchmark_test} arr_size: ${arr_size}"

mkdir -p binary
mkdir -p LR_NO_ERR_CORR_2core_results
mkdir -p LR_NO_ERR_CORR_2core_results/LR_ERR_CORR_2core_results

########### XXX Below lines should be uncommented while calibration and generating receiver traces. XXX  ############
#1a. Remove older binaries from bin/
#rm ../../bin/bimodal-no-no-random*
#XXX DOING below things
#1b. Build 2-core LR binary, keeping a check on calibration constraints.
#./script_with_tuned_parameters_for_512_string_length_err_corr.sh ${benchmark_test} ${arr_size} ${unroll_fact} ${algo_trigger_point}

#1c. Copy to binary/ 
#cp ../../bin/bimodal-no-no-random-2core binary/
########## XXX Above lines should be uncommented while calibration and generating receiver traces. XXX  ############

./trigger_LR_WITH_ERR_CORR_BW_EXP_with_benchmark_suit.sh ${unroll_fact} ${START_POINT} ${END_POINT} ${num_prl} ${benchmark_test} ${string_size} ${other_blocks} ${arr_size}
