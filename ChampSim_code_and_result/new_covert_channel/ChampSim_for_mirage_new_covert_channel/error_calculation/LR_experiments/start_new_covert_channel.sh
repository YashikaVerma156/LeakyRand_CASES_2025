sender_disturbance=818 #409 #819 #1638 #9830 #6554 #3276 #1638
string_length=512
benchmark_test=1
receiver_array_size=262144
repeat_new_crfill=50
start_point=1
end_point=1
num_prl=10
for sender_disturbance in 5500 #4000 4500 5000 6000 #13108 16384  #409 819 1638 3276 6554 9830
do
    ./generate_sender_receiver_traces.sh ${sender_disturbance} ${string_length} ${benchmark_test} ${receiver_array_size} ${repeat_new_crfill} ${start_point} ${end_point} ${num_prl} 

done
