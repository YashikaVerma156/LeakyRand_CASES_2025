################################################################################################################
# 1) What does it do: run multiple experiments to generate SPP error and bandwidth results.                    #
# 2) Pre-requisites:                                                                                           #
# a) /binary folder should have the 2-core binary ready for SPP.                                               #
# b) The receiver trace should be generated a priori. As the receiver trace does not change unless the string  #
#    size is changed.                                                                                          #
# c) MAKE SURE that in this script the seeds_t*.txt file is set as per the requirement.                        #
# d) Make SURE the benchmark_t*.txt file is set as per the requirement in the config.h file in read_message()  #
# e) This script uses two more scripts i) SPP_error_bandwidth_experiment_multiple_run.sh and ii) run_2core.sh. #
#    These two should be present in the current dir.                                                           #
################################################################################################################

# This define which traces will be ran by which script.
i=-1
START_POINT=$1
STOP_POINT=$2
num_prl=$3

# Check if exactly three arguments are passed.
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <START_POINT> <STOP_POINT> <num_prl>"
    echo "START_POINT tells the start of string number for experiment."
    echo "STOP_POINT tells the stop of string number for experiment."
    echo "num_prl number of parallel experiments that can be run."
    exit 1
fi

sender_arr_size_list=(9830)

for sender_arr_size in 9830
do
	#echo "generated traces"
        ./generate_sender_receiver_trace_test.sh ${sender_arr_size} ${START_POINT} ${STOP_POINT} 
	./SPP_error_bandwidth_experiment_multiple_run_with_benchmark_suit_2mb_fig10_test.sh ${sender_arr_size} 100 ${START_POINT} 1
done
#exit
j=${num_prl}
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
        #echo "$i , $column_value"
	seed=$column_value
	msg_num=$i

        ./SPP_error_bandwidth_experiment_multiple_run_with_benchmark_suit_2mb_fig10_test.sh ${sender_arr_size_list[0]} ${seed} ${msg_num} 0 &

        j=`expr ${j} - 1`
        echo "j is $j"
        if [ $j == 0 ]
        then
            j=${num_prl}
            wait
        fi
done < seeds_test.txt
