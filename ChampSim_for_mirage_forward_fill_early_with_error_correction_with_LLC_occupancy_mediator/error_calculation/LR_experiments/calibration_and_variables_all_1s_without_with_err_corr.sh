unroll_fact=$1    ## Specify unrolling factor
algo_trigger_point=$2
benchmark_test=$3
with_err_corr=$4

if [ ${benchmark_test} -eq 1 ]; then
    START_POINT=1     ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
    END_POINT=1  
elif [ ${benchmark_test} -eq 0 ]; then
    START_POINT=1     ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
    END_POINT=1  
fi

num_prl=50        ## Number of parallel instances to be run, depends on the number of cores.
string_size=512   # Message Length.

if [ ${with_err_corr} -eq 0 ]; then

		# XXX Below setup is for 1% other blocks i.e. 327 in LLC.
		other_blocks=327
		if [ ${unroll_fact} -eq 32 ]; then
				arr_size=259256
		elif [ ${unroll_fact} -eq 16 ]; then
				arr_size=259288
		elif [ ${unroll_fact} -eq 8 ]; then
				arr_size=259304
				other_blocks=326
		elif [ ${unroll_fact} -eq 4 ]; then
				arr_size=259328
		elif [ ${unroll_fact} -eq 2 ]; then
				arr_size=259320
		elif [ ${unroll_fact} -eq 1 ]; then
				arr_size=259312
		fi

elif [ ${with_err_corr} -eq 1 ]; then
    other_blocks=326
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
				hit_miss_threshold=120
		fi
fi
echo "unroll_fact: ${unroll_fact} with_err_corr: ${with_err_corr} endpoint: ${END_POINT} startpoint: ${START_POINT} benchmark_test: ${benchmark_test} arr_size: ${arr_size} other_blocks: ${other_blocks}"

#exit 1

mkdir -p binary
mkdir -p LR_NO_ERR_CORR_2core_results
mkdir -p LR_NO_ERR_CORR_2core_results/LR_all_1s_WITHOUT_WITH_ERR_CORR_2core_results

########### XXX Below lines should be uncommented while calibration and generating receiver traces. XXX  ############
#1a. Remove older binaries from bin/
#rm ../../bin/bimodal-no-no-random*

#1b. Build 2-core LR binary, keeping a check on calibration constraints.
#./script_with_tuned_parameters_for_512_string_length.sh ${benchmark_test} ${arr_size} ${unroll_fact}

#1c. Copy to binary/ 
#cp ../../bin/bimodal-no-no-random-2core binary/
########## XXX Above lines should be uncommented while calibration and generating receiver traces. XXX  ############

if [ ${with_err_corr} -eq 1 ]; then
    ./trigger_LR_all_1s_WITH_ERR_CORR_EXP_both_algo.sh ${unroll_fact} ${START_POINT} ${END_POINT} ${num_prl} ${benchmark_test} ${string_size} ${other_blocks} ${arr_size} ${algo_trigger_point} ${hit_miss_threshold}
elif [ ${with_err_corr} -eq 0 ]; then
    ./trigger_LR_all_1s_NO_ERR_CORR.sh ${unroll_fact} ${START_POINT} ${END_POINT} ${num_prl} ${benchmark_test} ${string_size} ${other_blocks} ${arr_size} ${hit_miss_threshold}
fi
