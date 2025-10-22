
for sender_disturbance in 3000 #4250 4500 4750 5000 5250 5500 5750 #2500 2750 3000 3250 3500 3750 4000 #1500 1750 2000 2250 #500 750 1000 1250 #0 1
do
		echo "$sender_disturbance"
		#sender_disturbance=500 #9830 #6554 #3276 #1638
		string_length=512
		start_point=1
		end_point=2
		benchmark_test=1
		receiver_array_size=262144
		repeat_new_crfill=51
		lr_like_probe=1
		num_prl=10
    slide_width=1000
    arr_size_mul=3  # 3 number is chosen randomly.
    history_length=4

    for unroll_fact in 16 #32 64 128 # Try for higher unroll_fact as well, such as 32, 64, and 128.
    do
        ./generate_sender_receiver_traces_lr_probe_version2.sh ${sender_disturbance} ${string_length} ${benchmark_test} ${receiver_array_size} ${repeat_new_crfill} ${lr_like_probe} ${start_point} ${end_point} ${num_prl} ${unroll_fact} ${slide_width} ${arr_size_mul} ${history_length}
    done
done
