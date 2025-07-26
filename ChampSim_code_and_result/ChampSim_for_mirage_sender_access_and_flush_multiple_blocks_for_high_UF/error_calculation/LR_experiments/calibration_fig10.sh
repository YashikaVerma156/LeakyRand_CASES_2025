unroll_fact=$1    ## Specify unrolling factor
algo_trigger_point=$2    ## Specify bit number to trigger hole-filling algo. Accordingly occupancy check will be triggered.
benchmark_test=$3  ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
other_blocks=$4
err_corr_iterations=$5
LLC_num_blocks=$6
str_num=$7
num_prl=$8        ## Number of parallel instances to be run, depends on the number of cores.



if [ ${benchmark_test} -eq 1 ]; then
  START_POINT=1     ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
  END_POINT=${str_num}       #500  
elif [ ${benchmark_test} -eq 0 ]; then
  START_POINT=1     #1     ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
  END_POINT=${str_num}     #250 
fi
string_size=512   # Message Length.


if [ $LLC_num_blocks -eq 16384 ]; then
    if [ $other_blocks -eq 16 ]; then
          arr_size=130680
    elif [ $other_blocks -eq 163 ]; then
          arr_size=129504
    fi
		if [ $unroll_fact -eq 128 ]; then
	      hit_miss_threshold=340
	      hit_miss_threshold_for_err_corr=124
		fi

elif [ $LLC_num_blocks -eq 65536 ]; then
    if [ $other_blocks -eq 64 ]; then
          arr_size=523512
    elif [ $other_blocks -eq 652 ]; then
              arr_size=518808
    fi
		if [ $unroll_fact -eq 128 ]; then
	      hit_miss_threshold=340
	      hit_miss_threshold_for_err_corr=124
		fi
fi
other_blocks_previous=${other_blocks}

echo "algo_trigger_point: ${algo_trigger_point}, endpoint: ${END_POINT}, startpoint: ${START_POINT}, benchmark_test: ${benchmark_test}, arr_size: ${arr_size}, other_blocks: ${other_blocks} LLC_num_blocks: ${LLC_num_blocks}"

#./update_arr_size.sh ${unroll_fact} ${algo_trigger_point} ${err_corr_iterations} ${other_blocks_previous} ${arr_size}

mkdir -p binary
mkdir -p pin_traces
mkdir -p LR_ERR_CORR_2core_results_both_algo_${err_corr_iterations}_access

# XXX NOTE: other_blocks_previous is given to identify if the other blocks are 0.1% or 1% in LLC. But other_blocks may change as per the calibration.
# Call the script and capture its output
output=$(./check_for_arr_size_existence.sh ${unroll_fact} ${algo_trigger_point} ${err_corr_iterations} ${other_blocks_previous})

echo "output is: $output"

# Check if the output is "found" or "not found"
if [ "$output" == "arr_size is found" ]; then
    echo "The output is 'found'"

    # Define the file name
    file_name="output.txt"

    # Define the pattern to search for
    pattern="${unroll_fact}_${algo_trigger_point}_${err_corr_iterations}_${other_blocks_previous}"

    # Use grep to search for the pattern in the file
    match=$(grep -o "$pattern\s[0-9]*" "$file_name")

    # Extract the value of arr_size
    arr_size=$(echo "$match" | awk '{print $2}')
    echo "arr_size found is: ${arr_size}"

elif [ "$output" == "arr_size is not found" ]; then
    echo "The output is 'not found'"
		########### XXX Below lines should be uncommented while calibration and generating receiver traces. XXX  ############
		#1a. Remove older binaries from bin/
		rm ../../bin/bimodal-no-no-random*

		#1b. Build 2-core LR binary, keeping a check on calibration constraints.
		./script_update_arr_size_high_UF_with_LLC_sensitivity.sh ${benchmark_test} ${arr_size} ${unroll_fact} ${algo_trigger_point} ${other_blocks} ${END_POINT} ${hit_miss_threshold} ${err_corr_iterations} ${hit_miss_threshold_for_err_corr} ${LLC_num_blocks}

    # Define the file name
    file_name="output.txt"

    # Define the pattern to search for
    pattern="${unroll_fact}_${algo_trigger_point}_${err_corr_iterations}_${other_blocks_previous}"

    # Use grep to search for the pattern in the file
    match=$(grep -o "$pattern\s[0-9]*" "$file_name")

    # Extract the value of arr_size
    arr_size=$(echo "$match" | awk '{print $2}')

		#1c. Copy to binary/ 
		#cp ../../bin/bimodal-no-no-random-2core binary/
		########## XXX Above lines should be uncommented while calibration and generating receiver traces. XXX  ############
else
    echo "The output is neither 'found' nor 'not found'"
    echo "Something is wrong."
    exit 
fi

#exit 

#Below command is used when running general experiments.
./trigger_fig10.sh ${unroll_fact} ${START_POINT} ${END_POINT} ${num_prl} ${benchmark_test} ${string_size} ${other_blocks} ${arr_size} ${algo_trigger_point} ${hit_miss_threshold} ${err_corr_iterations} ${hit_miss_threshold_for_err_corr} ${LLC_num_blocks}
