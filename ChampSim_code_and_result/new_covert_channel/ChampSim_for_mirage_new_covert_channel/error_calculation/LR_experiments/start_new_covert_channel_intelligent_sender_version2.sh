benchmark_test=$1
num_prl=$2
end_point=$3

# Check if exactly two arguments are passed.
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <benchmark_test> <num_prl> <end_point>"
    echo "benchmark_test should be either 1 or 0."
    echo "num_prl should be set as per the number of cores available in the machine."
    echo "end_point signifies how many experiments are to be run."
    exit 1
fi

string_length=512
receiver_array_size=262144
start_point=1
slide_width=1000
history_length=3 # This is used in pin directory to decide which sender trace to use.
repeat_new_crfill=`expr 50 + $history_length `
#echo "repeat_new_crfill: ${repeat_new_crfill}"

for sender_disturbance in 6250 
do
    arr_size_mul=3  # 3 number is chosen randomly.
    ./generate_sender_receiver_traces_version2.sh ${sender_disturbance} ${string_length} ${benchmark_test} ${receiver_array_size} ${repeat_new_crfill} ${start_point} ${end_point} ${num_prl} ${slide_width} ${arr_size_mul} ${history_length}

done
