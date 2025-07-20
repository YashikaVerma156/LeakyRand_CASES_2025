unroll_fact=$1    ## Specify unrolling factor
START_POINT=19   ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
END_POINT=19
num_prl=13        ## Number of parallel instances to be run, depends on the number of cores.
benchmark_test=1  ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
string_size=512   # Message Length.
other_blocks=327

# XXX Below setup is for 1% other blocks i.e. 327 in LLC.
if [ ${unroll_fact} -eq 32 ]; then
    arr_size=259256
elif [ ${unroll_fact} -eq 16 ]; then
    arr_size=259288
elif [ ${unroll_fact} -eq 8 ]; then
    arr_size=259304
elif [ ${unroll_fact} -eq 4 ]; then
    arr_size=259328
elif [ ${unroll_fact} -eq 2 ]; then
    arr_size=259320
elif [ ${unroll_fact} -eq 1 ]; then
    arr_size=259312
fi

#echo "endpoint: ${END_POINT} startpoint: ${START_POINT} benchmark_test: ${benchmark_test} arr_size: ${arr_size}"

mkdir -p binary
mkdir -p LR_NO_ERR_CORR_2core_results

#1a. Remove older binaries from bin/
rm ../../bin/bimodal-no-no-random*

#1b. Build 2-core LR binary, keeping a check on calibration constraints.
./script_with_tuned_parameters_for_512_string_length.sh ${benchmark_test} ${arr_size} ${unroll_fact}

#1c. Copy to binary/ 
cp ../../bin/bimodal-no-no-random-2core binary/


./trigger_LR_NO_ERR_CORR_BW_EXP_with_benchmark_suit.sh ${unroll_fact} ${START_POINT} ${END_POINT} ${num_prl} ${benchmark_test} ${string_size} ${other_blocks} ${arr_size}
