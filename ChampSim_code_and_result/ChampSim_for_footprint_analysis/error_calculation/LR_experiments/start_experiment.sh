
		echo "$sender_disturbance"
		#sender_disturbance=500 #9830 #6554 #3276 #1638
		string_length=512
		start_point=1
		end_point=1
		benchmark_test=1
		receiver_array_size=262144
		repeat_new_crfill=51
		lr_like_probe=1
		num_prl=10

		unroll_fact=16 # Try for higher unroll_fact as well, such as 32, 64, and 128.

    ./generate_sender_receiver_traces.sh ${receiver_array_size} ${repeat_new_crfill} ${lr_like_probe} ${num_prl} ${unroll_fact}

