#!/bin/bash
#benchmark_test=$1

#if [ "$1" > 1 ] || [ "$1" < 0 ]; then
#    echo " ERROR: The value of benchmark_test should be either 0 or 1."
#    exit 1
#fi

other_blocks=$1
start_point=$2
end_point=$3
num_prl=$4

# Check if three arguments are passed
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <other_blocks> <start_point> <end_point> <num_prl>"
    echo "other_blocks should be either 31 or 326"
    exit 1
fi

# Check if the argument is either 326 or 31
if [ "$1" -ne 326 ] && [ "$1" -ne 31 ]; then
    echo "The value of other_blocks must be either 326 or 31"
    exit 1
fi

# The commandline argument should be an integer.
integer_regex='^[0-9]+$'

# Check if the argument matches the integer pattern
if ! [[ $1 =~ $integer_regex ]]; then
    echo "The value of other_blocks must be an integer"
    exit 1
fi

echo "other blocks: $other_blocks"
err_corr_iterations=4

#exit 1

if [ "$other_blocks" -eq 31 ]; then
for err_corr_iterations in 4   #2 3 4 ##2 4 8 16 17 #1 2 3 4  # 5 6 7 8  # 1 2 3 4
do
		for benchmark_test in 0   #1         #0  1
		do
				for unroll_fact in 256 128 64 32 #16   #16 32
				do
						for algo_trigger_point in 256 512   #8 16 32 64 128
						do
								# XXX Keep the below numbers for UF 16 and calculate the different sender_disturbance numbers for UF 32.
								if [ "$algo_trigger_point" -eq 8 ]; then 
									err_corr_iterations=3
								elif [ "$algo_trigger_point" -eq 16 ]; then
									err_corr_iterations=3
								elif [ "$algo_trigger_point" -eq 32 ]; then
									err_corr_iterations=3
								elif [ "$algo_trigger_point" -eq 64 ]; then
									err_corr_iterations=4
								elif [ "$algo_trigger_point" -eq 128 ]; then
									err_corr_iterations=4
								elif [ "$algo_trigger_point" -eq 256 ]; then
									err_corr_iterations=5
								elif [ "$algo_trigger_point" -eq 512 ]; then
									err_corr_iterations=9
								fi

								echo "====================RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test} other_blocks: ${other_blocks} ======================="   
								 #exit 1 
								./calibration_fig10_2mb.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${other_blocks} ${err_corr_iterations} ${start_point} ${end_point} ${num_prl}
						done
				done
		done
done

elif [ "$other_blocks" -eq 326 ]; then

                for benchmark_test in 1   #1         #0  1
                do
                                for unroll_fact in 128 #64 32   #256 128 64 32  
                                do
                                                for algo_trigger_point in 128 #8 16 32 64 128
                                                do
								if [ "$algo_trigger_point" -eq 8 ]; then
									 err_corr_iterations=4
								elif [ "$algo_trigger_point" -eq 16 ]; then
									 err_corr_iterations=5
								elif [ "$algo_trigger_point" -eq 32 ]; then
									 err_corr_iterations=6
								elif [ "$algo_trigger_point" -eq 64 ]; then
									 err_corr_iterations=9
								elif [ "$algo_trigger_point" -eq 128 ]; then
								      if [ "$unroll_fact" -eq 32 ]; then
									 err_corr_iterations=12    
								      elif [ "$unroll_fact" -eq 64 ]; then
									 err_corr_iterations=13    #18
								      elif [ "$unroll_fact" -eq 128 ]; then
									 err_corr_iterations=13    #18
								      fi
								elif [ "$algo_trigger_point" -eq 256 ]; then
									 err_corr_iterations=25
								elif [ "$algo_trigger_point" -eq 512 ]; then
									 err_corr_iterations=98
								fi

								if [ "$unroll_fact" -eq 256 ]; then
								    for err_corr_iterations in 18 30
								    do
                                                                        echo "====================RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test} other_blocks: ${other_blocks} ======================= err_corr_iterations: ${err_corr_iterations}"   
                                                                        ./calibration_fig10_2mb.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${other_blocks} ${err_corr_iterations} ${start_point} ${end_point} ${num_prl}

								    done
                                                                else
                                                                    echo "====================RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test} other_blocks: ${other_blocks} ======================= err_corr_iterations: ${err_corr_iterations}"   
                                                                    ./calibration_fig10_2mb.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${other_blocks} ${err_corr_iterations} ${start_point} ${end_point} ${num_prl} 
								fi
                                                done
                                done
                done
fi
