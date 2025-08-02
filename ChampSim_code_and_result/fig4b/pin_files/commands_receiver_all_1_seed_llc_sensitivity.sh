
#make champsim_tracer_for_covert_channel_extract_everything.test TARGET=intel64
array_size=$1
string_size=$2
repetetion_coding=$3
differential_signalling=$4
path=$5
LLC_size=$6

echo "Inside commands_receiver.sh; array size: $array_size, string size: $string_size"

sed -i.bak 's/\<NUM 0\>/NUM '${array_size}'/g' receiver_IEEE_SEED.c
sed -i.bak 's/\<NUM_BITS 0\>/NUM_BITS '${string_size}'/g' config.h
sed -i.bak 's/\<REPETETION_CODING 0\>/REPETETION_CODING '${repetetion_coding}'/g' config.h
sed -i.bak 's/\<DIFFERENTIAL_SIGNALLING 0\>/DIFFERENTIAL_SIGNALLING '${differential_signalling}'/g' config.h


#make champsim_tracer_for_covert_channel_extract_everything_with_error_correction.test TARGET=intel64
gcc -O3 -no-pie -fno-stack-protector --static --entry=_receiver_entry receiver_IEEE_SEED.c -o receiver_IEEE_SEED
../../../../pin -t ../obj-intel64/champsim_tracer_for_covert_channel_extract_everything_with_error_correction.so -- ./receiver_IEEE_SEED > receiver_seed.txt

mv champsim.trace champsim.trace_receiver_${string_size}_${LLC_size}_seed
gzip champsim.trace_receiver_${string_size}_${LLC_size}_seed
mv champsim.trace_receiver_${string_size}_${LLC_size}_seed.gz ${path}

# Restore to the default configuration
sed -i.bak 's/\<NUM '${array_size}'\>/NUM 0/g' receiver_IEEE_SEED.c
sed -i.bak 's/\<NUM_BITS '${string_size}'\>/NUM_BITS 0/g' config.h
sed -i.bak 's/\<REPETETION_CODING '${repetetion_coding}'\>/REPETETION_CODING 0/g' config.h
sed -i.bak 's/\<DIFFERENTIAL_SIGNALLING '${differential_signalling}'\>/DIFFERENTIAL_SIGNALLING 0/g' config.h

