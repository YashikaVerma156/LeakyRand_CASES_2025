unroll_fact=$1    ## Specify unrolling factor
algo_trigger_point=$2    ## Specify bit number to trigger hole-filling algo. Accordingly occupancy check will be triggered.
benchmark_test=$3  ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
other_blocks=$4
err_corr_iterations=$5
num_prl=$6        ## Number of parallel instances to be run, depends on the number of cores.

other_blocks_previous=${other_blocks}

if [ ${benchmark_test} -eq 1 ]; then
  START_POINT=1     ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
  END_POINT=500       #500  
elif [ ${benchmark_test} -eq 0 ]; then
  START_POINT=1     #1     ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
  END_POINT=250     #250 
fi
string_size=512   # Message Length.

if [ $other_blocks -eq 326 ]; then
	if [ $unroll_fact -eq 32 ]; then
			# XXX Below setup is for 1% other blocks i.e. 326 or 327 in LLC.
			if [ ${algo_trigger_point} -eq 128 ]; then
					arr_size=259056
			elif [ ${algo_trigger_point} -eq 64 ]; then
					arr_size=259048
					other_blocks=327
			elif [ ${algo_trigger_point} -eq 32 ]; then
					arr_size=259024
					other_blocks=327
			elif [ ${algo_trigger_point} -eq 16 ]; then
					arr_size=259016
			elif [ ${algo_trigger_point} -eq 8 ]; then
					arr_size=258968
			fi
			hit_miss_threshold=153
	fi

	if [ $unroll_fact -eq 16 ]; then
			# XXX Below setup is for 1% other blocks i.e. 326 or 327 in LLC.
			if [ ${algo_trigger_point} -eq 128 ]; then
					arr_size=259104
			elif [ ${algo_trigger_point} -eq 64 ]; then
					arr_size=259096
			elif [ ${algo_trigger_point} -eq 32 ]; then
					arr_size=259072
					other_blocks=327
			elif [ ${algo_trigger_point} -eq 16 ]; then
					arr_size=259048
			elif [ ${algo_trigger_point} -eq 8 ]; then
					arr_size=259000
			elif [ ${algo_trigger_point} -eq 256 ]; then
					arr_size=259264
			elif [ ${algo_trigger_point} -eq 512 ]; then
					arr_size=259264
			fi
			hit_miss_threshold=124
	fi

elif [ $other_blocks -eq 31 ]; then
	if [ $unroll_fact -eq 16 ]; then
	    #if [ $err_corr_iterations -eq 2 ]; then
      echo " other blocks are: $other_blocks"
					if [ ${algo_trigger_point} -eq 128 ]; then
							arr_size=261448
					elif [ ${algo_trigger_point} -eq 64 ]; then
							arr_size=261432
							#other_blocks=32
					elif [ ${algo_trigger_point} -eq 32 ]; then
							arr_size=261440
							#other_blocks=29
					elif [ ${algo_trigger_point} -eq 16 ]; then
							arr_size=261360
					elif [ ${algo_trigger_point} -eq 8 ]; then
							arr_size=261272    
					elif [ ${algo_trigger_point} -eq 256 ]; then
							arr_size=261624
					elif [ ${algo_trigger_point} -eq 512 ]; then
							arr_size=261624
					fi
      #fi
			hit_miss_threshold=124
	elif [ $unroll_fact -eq 32 ]; then
          if [ ${algo_trigger_point} -eq 128 ]; then
              arr_size=261624
          fi
			    hit_miss_threshold=153
	elif [ $unroll_fact -eq 64 ]; then
          if [ ${algo_trigger_point} -eq 128 ]; then
              arr_size=261624
          fi
			    hit_miss_threshold=250

	elif [ $unroll_fact -eq 128 ]; then
          if [ ${algo_trigger_point} -eq 128 ]; then
              arr_size=261624
          fi
			    hit_miss_threshold=340
	elif [ $unroll_fact -eq 256 ]; then
          if [ ${algo_trigger_point} -eq 128 ]; then
              arr_size=261624
          fi
			    hit_miss_threshold=390
	fi
fi

echo "algo_trigger_point: ${algo_trigger_point}, endpoint: ${END_POINT}, startpoint: ${START_POINT}, benchmark_test: ${benchmark_test}, arr_size: ${arr_size}, other_blocks: ${other_blocks}"

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
    ./script_for_err_corr_both_algo_2core_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized_update_arr_size.sh ${benchmark_test} ${arr_size} ${unroll_fact} ${algo_trigger_point} ${other_blocks} ${END_POINT} ${hit_miss_threshold} ${err_corr_iterations}

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
./trigger_multiple_ds.sh ${unroll_fact} ${START_POINT} ${END_POINT} ${num_prl} ${benchmark_test} ${string_size} ${other_blocks} ${arr_size} ${algo_trigger_point} ${hit_miss_threshold} ${err_corr_iterations}
