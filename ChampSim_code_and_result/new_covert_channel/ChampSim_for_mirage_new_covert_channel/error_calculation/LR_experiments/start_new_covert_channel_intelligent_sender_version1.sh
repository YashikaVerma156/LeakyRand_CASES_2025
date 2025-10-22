sender_disturbance=818 #409 #819 #1638 #9830 #6554 #3276 #1638
string_length=512
benchmark_test=1
receiver_array_size=262144
repeat_new_crfill=50
start_point=1
end_point=2
num_prl=10
slide_width=1000
#echo "size: $size"
#exit

for sender_disturbance in 6250 #5500 #4000 4500 5000 6000 #13108 16384  #409 819 1638 3276 6554 9830
do
    arr_size_mul=3  # 3 number is chosen randomly.
    ./generate_sender_receiver_traces_version1.sh ${sender_disturbance} ${string_length} ${benchmark_test} ${receiver_array_size} ${repeat_new_crfill} ${start_point} ${end_point} ${num_prl} ${slide_width} ${arr_size_mul}

done
