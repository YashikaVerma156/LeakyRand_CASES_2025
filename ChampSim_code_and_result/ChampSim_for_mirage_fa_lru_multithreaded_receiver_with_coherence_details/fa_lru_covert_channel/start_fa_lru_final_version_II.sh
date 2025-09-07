#!/bin/bash
err_corr_iterations=1
start_point=1   #500
stop_point=500 #50 #500   #500
string_num=23
for benchmark_test in 1
do
		for unroll_fact in 1
		do
				for algo_trigger_point in 514
				do
            #while [ ${string_num} -le $stop_point ]
            #for string_num in 23 107 128 174 333 359 363 392 419 456 482 96 #1 2 3 4 5
            #do
					      echo "==== RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test}, err_corr_iterations: ${err_corr_iterations}, string_num: ${string_num} ===="
					#exit 1
					      ./calibration_fa_lru_final_version_II.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${err_corr_iterations} ${start_point} ${stop_point}
             #   string_num=`expr ${string_num} + 1`
           #done
				done
		done
done
