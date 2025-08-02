string_num=$1
string_size=$2
arr_size=$3
repetetion_coding=$4
differential_signalling=$5
path=$6

echo "string number in pin directory is: $string_num"

sed -i.bak 's/\<STR_NUM 0\>/STR_NUM '${string_num}'/g' config_train.h
sed -i.bak 's/\<NUM_BITS 0\>/NUM_BITS '${string_size}'/g' config_train.h
sed -i.bak 's/\<ARR_SIZE_SENDER 0\>/ARR_SIZE_SENDER '${arr_size}'/g' config_train.h
sed -i.bak 's/\<REPETETION_CODING 0\>/REPETETION_CODING '${repetetion_coding}'/g' config_train.h
sed -i.bak 's/\<DIFFERENTIAL_SIGNALLING 0\>/DIFFERENTIAL_SIGNALLING '${differential_signalling}'/g' config_train.h

#make champsim_tracer_for_covert_channel_extract_everything_with_error_correction.test TARGET=intel64

gcc -O3 -no-pie -fno-stack-protector --static --entry=_sender_entry testing_all_1_IEEE_SEED_with_benchmark_suit_train.c -o sender_all_1_seed
#cd ../
#../../../pin -t obj-intel64/champsim_tracer_for_covert_channel_extract_everything.so -- ./string_length_25/sender_all_1 > string_length_25/all_1_sender.txt

../../../../pin -t ../obj-intel64/champsim_tracer_for_covert_channel_extract_everything_with_error_correction.so -- ./sender_all_1_seed > all_1_sender_seed.txt

sim_count=`grep "instrCount" all_1_sender_seed.txt | awk '{print $4}'`

echo "sim_count: $sim_count"

mv champsim.trace champsim.trace_sender_${string_size}_${string_num}_${sim_count}_arr_size_${arr_size}_seed_train
gzip champsim.trace_sender_${string_size}_${string_num}_${sim_count}_arr_size_${arr_size}_seed_train
mv champsim.trace_sender_${string_size}_${string_num}_${sim_count}_arr_size_${arr_size}_seed_train.gz ${path}

#objdump -D sender_all_1_seed > sender_all_1_seed.txt

sed -i.bak 's/\<STR_NUM '${string_num}'\>/STR_NUM 0/g' config_train.h
sed -i.bak 's/\<NUM_BITS '${string_size}'\>/NUM_BITS 0/g' config_train.h
sed -i.bak 's/\<ARR_SIZE_SENDER '${arr_size}'\>/ARR_SIZE_SENDER 0/g' config_train.h
sed -i.bak 's/\<REPETETION_CODING '${repetetion_coding}'\>/REPETETION_CODING 0/g' config_train.h
sed -i.bak 's/\<DIFFERENTIAL_SIGNALLING '${differential_signalling}'\>/DIFFERENTIAL_SIGNALLING 0/g' config_train.h
