benchmark_test=$1       ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
msg_string_end_num=$2      


# Check if exactly one argument is passed
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <benchmark_test> <msg_string_end_num>"
    echo "benchmark_test should be either 1 or 0, 1: if sender traces for test_benchmark_set is to be generated, 0: if sender traces for train_benchmark_set is to be generated. msg_string_end_num  1>= msg_string_end_num <= 250 for train_benchmark and 1>= msg_string_end_num <= 500 for test benchmark."
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
    echo "The value of msg_string_end_num must be an integer."
    exit 1
fi

# Check if the argument is either 0 or 1
if [ "$1" -ne 0 ] && [ "$1" -ne 1 ]; then
    echo "The value of benchmark_test must be either 0 or 1."
    exit 1
fi

if [ $1 -eq 0 ]; then

    # Check if the argument is within range.
    if [ "$2" -lt 1 ] && [ "$2" -gt 250 ]; then
        echo "The value of msg_string_end_num is not correct."
        exit 1
    fi

else

    # Check if the argument is within range.
    if [ "$2" -lt 1 ] && [ "$2" -gt 500 ]; then
        echo "The value of msg_string_end_num is not correct."
        exit 1
    fi

fi


echo "benchmark_test: $benchmark_test"

#exit

pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code

err_corr_interval=128
for err_corr_iterations in 17 13 15
do
	    i=${msg_string_end_num} #500
	    
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
