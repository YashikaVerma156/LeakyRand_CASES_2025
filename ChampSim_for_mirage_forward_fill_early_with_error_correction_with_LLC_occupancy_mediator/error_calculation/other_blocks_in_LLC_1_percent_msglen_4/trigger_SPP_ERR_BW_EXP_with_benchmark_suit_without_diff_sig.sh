
#XXX The script has a mixture of both sh and bash compatible syntax.
#XXX TRICK TO EXECUTE: first time for a LLCsize, generate 2-core binary and receiver trace. Generate sender trace on the fly.

################################################################################################################
# 1) What does it do: run multiple experiments to generate SPP error and bandwidth results.                    #
# 2) Pre-requisites:                                                                                           #
# a) /binary folder should have the 2-core binary ready for SPP. Receiver trace should be present for a        #
#    particular cache size                                                                                     #
################################################################################################################


LLC_size=$1
bench_test=$2
build_2_core_binary=$3  #One time task for a particular LLC size.
generate_receiver_trace=$4  #One time task for a particular LLC size.
generate_sender_trace=$5  #One time task for a particular LLC size.

# Check if exactly five arguments are passed
if [ "$#" -ne 5 ]; then
    echo "Usage: $0 <LLC_size> <bench_test> <build_2_core_binary> <generate_receiver_trace> <generate_sender_trace>"
    echo "LLC_size should be '2' for 2MB."
    echo "bench_test should be either '1' for test_benchmark_suite or '0' for train_benchmark_suite."
    echo "build_2_core_binary should be either '1' or '0' ; 1 is to generate binary."
    echo "generate_receiver_trace should be either '1' or '0' ; 1 is to generate receiver trace."
    echo "generate_sender_trace should be either '1' or '0' ; 1 is to generate sender trace."
    exit 1
fi

# Check if the LLC_size is 2MB
if [ "$1" -ne 2 ]; then
    echo "The value of LLC_size must be 2 for 2MB."
    exit 1
fi

# Check if the bench_test is either 1 or 0
if [ "$2" -ne 1 ] && [ "$2" -ne 0 ]; then
    echo "The value of bench_test must be either 1 for test_benchmark_suite or 0 for train_benchmark_suite."
    exit 1
fi

# Check if the build_2_core_binary is either 1 or 0
if [ "$3" -ne 1 ] && [ "$3" -ne 0 ]; then
    echo "The value of build_2_core_binary must be either 1 or 0."
    exit 1
fi

# Check if the generate_receiver_trace is either 1 or 0
if [ "$4" -ne 1 ] && [ "$4" -ne 0 ]; then
    echo "The value of generate_receiver_trace must be either 1 or 0."
    exit 1
fi

# Check if the generate_sender_trace is either 1 or 0
if [ "$5" -ne 1 ] && [ "$5" -ne 0 ]; then
    echo "The value of generate_sender_trace must be either 1 or 0."
    exit 1
fi

# The commandline argument should be an integer.
integer_regex='^[0-9]+$'

# Check if the second argument matches the integer pattern
if ! echo "$1" | grep -qE "$integer_regex"; then
    echo "The value of LLC_size must be an integer."
    exit 1
fi

# Check if the second argument matches the integer pattern
if ! echo "$2" | grep -qE "$integer_regex"; then
    echo "The value of bench_test must be an integer."
    exit 1
fi

# This define which traces will be ran by which script.
i=-1

#Set the benchmark type.
if [ "$bench_test" -eq 1 ];  #Test benchmark suite
then 
    START_POINT=1
    STOP_POINT=1  #500
elif [ "$bench_test" -eq 0 ];  #Train benchmark suite
then
    START_POINT=1
    STOP_POINT=1  #250
fi

#Set the sender arr size.
if [ "$LLC_size" -eq 2 ];
then 
    sender_arr_size_list=(3276 6554 9830 13108 16380) # We are testing for all sender sizes
    #sender_arr_size_list=(3276) # We are testing for all sender sizes
    llc_sets=2048
fi

# Generate sender trace, one time task for a particular LLC size.

mkdir -p seed_pin_traces
mkdir -p results

if [ "$generate_sender_trace" -eq 1 ];
then
    # Iterate over each element in the array
    for size in "${sender_arr_size_list[@]}"
    do
        echo "Processing size: $size"
        mkdir -p sender_arr_size_${size}
        ./generate_sender_trace_without_diff_sig.sh ${size} $START_POINT $STOP_POINT $bench_test $llc_sets $LLC_size 224576
    done 
    #exit
fi

# Generate receiver trace and build 2-core binary for SPP, one time task for a particular LLC size.

if [ "$build_2_core_binary" -eq 1 ] || [ "$generate_receiver_trace" -eq 1 ];
then
    echo "llc_sets: $llc_sets"
    ./SPP_error_bandwidth_experiment_multiple_run_with_benchmark_suit_without_diff_sig.sh ${sender_arr_size_list[0]} 22456 $START_POINT $bench_test $build_2_core_binary $generate_receiver_trace $LLC_size $llc_sets 
fi

#exit

for size in "${sender_arr_size_list[@]}"
do
    sender_array_size=$size
    echo "Starting for sender size: $sender_array_size"
    #exit
		# This define which traces will be ran by which script.
		i=-1
    num_prl=10
    # Run simulations
    if [[ $bench_test -eq 1 ]];
    then
        j=$num_prl
        while read line; do
            i=`expr ${i} + 1`

            if [ $i -lt $START_POINT ]; then
                echo "Skip"
						    continue
            fi
            if [ $i -gt $STOP_POINT ]; then
						    echo "Exiting"
                break
            fi
            #echo "$line"
            # Split the line into columns using the delimiter.
            IFS="," read -ra columns <<< "$line"

            # Read the desired column from the split line.
            column_value=${columns[32]}

            # Do something with the column value.
            echo "$i , $column_value"
            seed=$column_value
            msg_num=$i

            ./SPP_error_bandwidth_experiment_multiple_run_with_benchmark_suit_without_diff_sig.sh ${sender_array_size} ${seed} ${msg_num} $bench_test 0 0 $LLC_size $llc_sets &
            j=`expr ${j} - 1`

            if [ $j -eq 0 ];
            then
                echo "entered wait"
                wait
                j=$num_prl
            fi
        done < seeds_test.txt
    fi

    if [[ $bench_test -eq 0 ]];
    then
        j=$num_prl
        while read line; do
            i=`expr ${i} + 1`

            if [ $i -lt $START_POINT ]; then
                echo "Skip"
						    continue
            fi
            if [ $i -gt $STOP_POINT ]; then
						    echo "Exiting"
                break
            fi
            #echo "$line"
            # Split the line into columns using the delimiter.
            IFS=',' read -ra columns <<< "$line"

            # Read the desired column from the split line.
            column_value=${columns[32]}

            # Do something with the column value.
            echo "$i , $column_value"
            seed=$column_value
            msg_num=$i

            ./SPP_error_bandwidth_experiment_multiple_run_with_benchmark_suit_without_diff_sig.sh ${sender_array_size} ${seed} ${msg_num} $bench_test 0 0 $LLC_size $llc_sets &
            j=`expr ${j} - 1`

            if [ $j -eq 0 ];
            then
                echo "entered wait"
                wait
                j=$num_prl
            fi
        done < seeds_train.txt
    fi
done
