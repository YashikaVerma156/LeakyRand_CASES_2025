for with_err_corr in 0    #0 1
do
		for benchmark_test in 0         #0  1
		do
				for unroll_fact in 16 32  #16 32
				do
            if [ $with_err_corr -eq 0 ]; then
						    echo "=========== RUNNING FOR unroll_fact: ${unroll_fact}, benchmark_test: ${benchmark_test}, with_err_corr: ${with_err_corr} ================="
								./calibration_and_variables_all_1s_without_with_err_corr.sh ${unroll_fact} 0 ${benchmark_test} ${with_err_corr}
            else
						    for algo_trigger_point in 8 16 32 64 128
						    do
								    echo "=========== RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test}, with_err_corr: ${with_err_corr} ================="
								    ./calibration_and_variables_all_1s_without_with_err_corr.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${with_err_corr}
						    done
            fi
				done
		done
done
