#!/bin/bash
err_corr_iterations=1
start_point=1   #500
stop_point=$1 #50 #500   #500

for benchmark_test in 1
do
		for unroll_fact in 1
		do
				for algo_trigger_point in 514
				do
					      echo "==== RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test}, err_corr_iterations: ${err_corr_iterations} ===="
					      ./calibration_fa_lru_final.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${err_corr_iterations} ${start_point} ${stop_point}
				done
		done
done
