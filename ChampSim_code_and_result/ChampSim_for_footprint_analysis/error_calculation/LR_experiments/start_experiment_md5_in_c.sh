
str_num=$1
num_prl=$2

echo "$sender_disturbance"
#sender_disturbance=500 #9830 #6554 #3276 #1638
string_length=512
start_point=1
end_point=$str_num
benchmark_test=0
receiver_array_size=262144
repeat_new_crfill=51
lr_like_probe=1

a=`cat /proc/cpuinfo | grep processor | wc -l` ;

unroll_fact=1 # Try for higher unroll_fact as well, such as 32, 64, and 128.

for benchmark_test in 0 1
do
    for text_file_size in 8 20 # 8 9 10 12 16 20
    do
        ./generate_md5_in_c.sh ${receiver_array_size} ${repeat_new_crfill} ${lr_like_probe} ${num_prl} ${unroll_fact} ${start_point} ${end_point} ${benchmark_test} ${text_file_size}
    done
done
