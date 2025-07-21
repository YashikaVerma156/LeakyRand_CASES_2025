#!/bin/bash
#benchmark_test=$1

#if [ "$1" > 1 ] || [ "$1" < 0 ]; then
#    echo " ERROR: The value of benchmark_test should be either 0 or 1."
#    exit 1
#fi

other_blocks=$1
LLC_num_blocks=$2


# Check if exactly one argument is passed
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <other_blocks> <LLC_num_blocks>"
    echo "other_blocks should be either 16 or 163 if LLC_num_blocks are 16384."
    echo "other_blocks should be either 64 or 652 if LLC_num_blocks are 65536."
    exit 1
fi

# Check if the LLC size is either 16384 or 65536
if [ "$2" -ne 16384 ] && [ "$2" -ne 65536 ]; then
    echo "The value of LLC_num_blocks must be either 16384 or 65536."
    exit 1
fi

if [ "$LLC_num_blocks" -eq 16384 ]; then
		# Check if the argument is either 326 or 31
		if [ "$1" -ne 16 ] && [ "$1" -ne 163 ]; then
				echo "The value of other_blocks must be either 163 or 16 for LLC_num_blocks 16384."
				exit 1
		fi
fi

if [ "$LLC_num_blocks" -eq 65536 ]; then
		# Check if the argument is either 326 or 31
		if [ "$1" -ne 652 ] && [ "$1" -ne 64 ]; then
				echo "The value of other_blocks must be either 652 or 64 for LLC_num_blocks 65536."
				exit 1
		fi
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

if [ "$LLC_num_blocks" -eq 16384 ]; then

  if [ "$other_blocks" -eq 16 ]; then

		for benchmark_test in 1   #1         #0  1
		do
				for unroll_fact in 16 #128 64  #16   #16 32
				do
						for algo_trigger_point in 128 #256 512   #8 16 32 64 128
						do
								echo "====================RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test} other_blocks: ${other_blocks} ======================="   
								 exit 1 
								./calibration_and_variables_with_err_corr_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized_LLC_size_sensitivity.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${other_blocks} ${err_corr_iterations} ${LLC_num_blocks}
						done
				done
		done

   elif [ "$other_blocks" -eq 163 ]; then

     for benchmark_test in 1   #1         #0  1
     do
         for unroll_fact in 16   #16 32
         do
             for algo_trigger_point in 128
             do
                 if [ "$algo_trigger_point" -eq 128 ]; then
                         err_corr_iterations=14
                 fi
                 echo "====================RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test} other_blocks: ${other_blocks} ======================= err_corr_iterations: ${err_corr_iterations}"   
                 #exit 1 
                 ./calibration_and_variables_with_err_corr_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized_LLC_size_sensitivity.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${other_blocks} ${err_corr_iterations} ${LLC_num_blocks}
              done
          done
      done
    fi

elif [ "$LLC_num_blocks" -eq 65536 ]; then

  if [ "$other_blocks" -eq 64 ]; then

		for benchmark_test in 1   #1         #0  1
		do
				for unroll_fact in 16 #128 64  #16   #16 32
				do
						for algo_trigger_point in 512 8  #256 512   #8 16 32 64 128
						do

								echo "====================RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test} other_blocks: ${other_blocks} ======================="   
								 exit 1 
								./calibration_and_variables_with_err_corr_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized_LLC_size_sensitivity.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${other_blocks} ${err_corr_iterations} ${LLC_num_blocks}
						done
				done
		done

   elif [ "$other_blocks" -eq 652 ]; then

     for benchmark_test in 1   #1         #0  1
     do
         for unroll_fact in 16   #16 32
         do
             for algo_trigger_point in 128 
             do
                 if [ "$algo_trigger_point" -eq 128 ]; then
                         err_corr_iterations=12
                 fi
                 echo "====================RUNNING FOR unroll_fact: ${unroll_fact}, algo_trigger_point: ${algo_trigger_point}, benchmark_test: ${benchmark_test} other_blocks: ${other_blocks} ======================= err_corr_iterations: ${err_corr_iterations}"   
                 #exit 1 
                 ./calibration_and_variables_with_err_corr_both_algo_sender_access_and_flush_multiple_blocks_multiple_algo_itr_optimized_LLC_size_sensitivity.sh ${unroll_fact} ${algo_trigger_point} ${benchmark_test} ${other_blocks} ${err_corr_iterations} ${LLC_num_blocks}
              done
          done
      done
    fi
fi
