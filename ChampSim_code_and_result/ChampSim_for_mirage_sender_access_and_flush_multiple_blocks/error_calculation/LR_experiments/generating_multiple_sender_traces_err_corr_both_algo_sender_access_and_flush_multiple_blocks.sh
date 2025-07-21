benchmark_test=$1       ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
other_block=$2       ## 31 : 0.1% LLC unoccupancy. 326 : 1% LLC unoccupancy.


# Check if exactly one argument is passed
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <benchmark_test> <other_block>"
    echo "benchmark_test should be either 1 or 0, 1: if sender traces for test_benchmark_set is to be generated, 0: if sender traces for train_benchmark_set is to be generated. other_block should be either 31 or 326, 31 : 0.1% LLC unoccupancy. 326 : 1% LLC unoccupancy."
    exit 1
fi

# Check if the argument is either 0 or 1
if [ "$1" -ne 0 ] && [ "$1" -ne 1 ]; then
    echo "The value of benchmark_test must be either 0 or 1."
    exit 1
fi

# Check if the argument is either 326 or 31
if [ "$2" -ne 31 ] && [ "$2" -ne 326 ]; then
    echo "The value of other_block must be either 31 or 326."
    exit 1
fi

# The commandline argument should be an integer.
integer_regex='^[0-9]+$'

# Check if the argument matches the integer pattern
if ! [[ $1 =~ $integer_regex ]]; then
    echo "The value of benchmark_test must be an integer."
    exit 1
fi

# Check if the argument matches the integer pattern
if ! [[ $2 =~ $integer_regex ]]; then
    echo "The value of benchmark_test must be an integer."
    exit 1
fi

echo "benchmark_test: $benchmark_test"
echo "other_block: $other_block"

#exit

pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code

err_corr_iterations=4
for err_corr_interval in 128 #8 16 32 64 128 256 512 #8 16 32 64 128
do
        if [ ${other_block} -eq 31 ];
        then

                if [ "$err_corr_interval" -eq 8 ]; then
                        err_corr_iterations=3
                elif [ "$err_corr_interval" -eq 16 ]; then
                        err_corr_iterations=3
                elif [ "$err_corr_interval" -eq 32 ]; then
                        err_corr_iterations=3
                elif [ "$err_corr_interval" -eq 64 ]; then
                        err_corr_iterations=4
                elif [ "$err_corr_interval" -eq 128 ]; then
                        err_corr_iterations=4
                elif [ "$err_corr_interval" -eq 256 ]; then
                        err_corr_iterations=5
                elif [ "$err_corr_interval" -eq 512 ]; then
                        err_corr_iterations=9
                fi

        elif [ ${other_block} -eq 326 ];
        then
                if [ "$err_corr_interval" -eq 8 ]; then
                        err_corr_iterations=4
                elif [ "$err_corr_interval" -eq 16 ]; then
                        err_corr_iterations=5
                elif [ "$err_corr_interval" -eq 32 ]; then
                        err_corr_iterations=6
                elif [ "$err_corr_interval" -eq 64 ]; then
                        err_corr_iterations=9
                elif [ "$err_corr_interval" -eq 128 ]; then
                        err_corr_iterations=12 #9  #12
                elif [ "$err_corr_interval" -eq 256 ]; then
                        err_corr_iterations=25
                elif [ "$err_corr_interval" -eq 512 ]; then
                        err_corr_iterations=98
                fi
        fi
       
				if [ ${benchmark_test} -eq 1 ];
				then
						i=500
				else
						i=250  #250    #250
				fi

				iteration_num=0
				# File to capture the occupancy of traces.
				rm occupancy_tracker_${err_corr_interval}_${err_corr_iterations}_${benchmark_test}.txt
				touch occupancy_tracker_${err_corr_interval}_${err_corr_iterations}_${benchmark_test}.txt

				mkdir -p sender_err_corr_both_algo_sender_access_and_flush_multiple_blocks
				mkdir -p single_core_results
				while [ ${i} -gt 0 ]
				do
							echo "string_num: $i"
							./generate_sender_trace_err_corr_both_algo_sender_access_and_flush_multiple_blocks.sh ${i} ${benchmark_test} ${err_corr_interval} ${iteration_num} ${err_corr_iterations} # XXX Sim count to be identified in the pin directory and the trace name should have this information.
							#exit
							i=`expr ${i} - 1`
							iteration_num=`expr ${iteration_num} + 1`
							gap_number=`cat ${pin_files_path}/gap_number.txt`

              if [ $gap_number == 1440 ]
              then
                  echo "gap_number: $gap_number"
              else
                  echo "Gap number is not the same."
                  exit
              fi

				done
done
