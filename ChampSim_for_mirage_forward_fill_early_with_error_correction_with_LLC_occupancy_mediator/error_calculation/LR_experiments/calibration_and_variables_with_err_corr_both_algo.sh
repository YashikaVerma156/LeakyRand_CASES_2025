unroll_fact=$1    ## Specify unrolling factor
algo_trigger_point=$2    ## Specify bit number to trigger hole-filling algo. Accordingly occupancy check will be triggered.
benchmark_test=$3  ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
other_blocks=$4

if [ ${benchmark_test} -eq 1 ]; then
  START_POINT=1     ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
  END_POINT=500    #500  
elif [ ${benchmark_test} -eq 0 ]; then
  START_POINT=1     ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
  END_POINT=250     #250 
fi
num_prl=12        ## Number of parallel instances to be run, depends on the number of cores.
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
			hit_miss_threshold=134
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
			fi
			hit_miss_threshold=116
	fi

elif [ $other_blocks -eq 31 ]; then

	if [ $unroll_fact -eq 16 ]; then
      echo " other blocks are: $other_blocks"
			if [ ${algo_trigger_point} -eq 128 ]; then
					arr_size=261456
			elif [ ${algo_trigger_point} -eq 64 ]; then
					arr_size=261448
			elif [ ${algo_trigger_point} -eq 32 ]; then
					arr_size=261440
			elif [ ${algo_trigger_point} -eq 16 ]; then
					arr_size=261424
			elif [ ${algo_trigger_point} -eq 8 ]; then
					arr_size=261352
      fi
			hit_miss_threshold=116
	fi

fi

echo "algo_trigger_point: ${algo_trigger_point}, endpoint: ${END_POINT}, startpoint: ${START_POINT}, benchmark_test: ${benchmark_test}, arr_size: ${arr_size}, other_blocks: ${other_blocks}"

mkdir -p binary
mkdir -p LR_NO_ERR_CORR_2core_results
mkdir -p LR_NO_ERR_CORR_2core_results/LR_ERR_CORR_2core_results_both_algo

########### XXX Below lines should be uncommented while calibration and generating receiver traces. XXX  ############
#1a. Remove older binaries from bin/
#rm ../../bin/bimodal-no-no-random*

#1b. Build 2-core LR binary, keeping a check on calibration constraints.
#./script_for_512_string_length_err_corr_both_algo.sh ${benchmark_test} ${arr_size} ${unroll_fact} ${algo_trigger_point} ${other_blocks} ${END_POINT} ${hit_miss_threshold}

#1c. Copy to binary/ 
#cp ../../bin/bimodal-no-no-random-2core binary/
########## XXX Above lines should be uncommented while calibration and generating receiver traces. XXX  ############

#Below command is used when running general experiments.
./trigger_LR_WITH_ERR_CORR_both_algo_with_benchmark_suit.sh ${unroll_fact} ${START_POINT} ${END_POINT} ${num_prl} ${benchmark_test} ${string_size} ${other_blocks} ${arr_size} ${algo_trigger_point} ${hit_miss_threshold}

#Below commands are used when running only a few specific experiments.
#mkdir -p LR_NO_ERR_CORR_2core_results/LR_ERR_CORR_2core_results_both_algo_specific_exp
#./trigger_LR_WITH_ERR_CORR_both_algo_with_benchmark_suit_run_specific_experiments.sh ${unroll_fact} ${START_POINT} ${END_POINT} ${num_prl} ${benchmark_test} ${string_size} ${other_blocks} ${arr_size} ${algo_trigger_point} ${hit_miss_threshold}

