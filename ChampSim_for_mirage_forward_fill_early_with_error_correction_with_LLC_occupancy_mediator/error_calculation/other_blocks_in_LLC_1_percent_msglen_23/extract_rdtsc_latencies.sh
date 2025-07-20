#1. Extract line number from which the grep should search for the pattern.
line=`grep -nri "cpu 0 is on wait" result_random_2_champsim.trace_receiver_multiple_access_32_other_blocks_325_5.gz_champsim.trace_sender_5_24479.gz.txt | head -1 | awk -F ':' '{print $1}'`
echo "$line"
line=$(echo "$line" | tr ' ' '\n')
echo "$line"

#2. 
#a=`awk "NR > $line" result_random_2_champsim.trace_receiver_multiple_access_32_other_blocks_325_5.gz_champsim.trace_sender_5_24479.gz.txt | grep -ni "found a miss for CPU: 0" | awk -F ':' '{print $1}'`

pat1="found a miss for CPU: 0"
pat2="rdtsc subtraction"

# Capture the output into an array
mapfile -t a < <(awk "NR > $line" result_random_2_champsim.trace_receiver_multiple_access_32_other_blocks_325_5.gz_champsim.trace_sender_5_24479.gz.txt | grep -E "$pat1|$pat2")

hit_time=()
miss_time=()
# Print each line separately (just for verification)
for line in "${a[@]}"; do
    if [[ $line == *"$pat1"* ]]; then
        mark=1
    else
        if [[ $mark == 1 ]]; then
            time=$(echo "$line" | awk '{print $3}')
            miss_time+=( "$time" )
            mark=0
        else
            time=$(echo "$line" | awk '{print $3}')
            hit_time+=( "$time" )
        fi
    fi
done
for n in "${hit_time[@]}"
do
  echo "$n" >> hit.txt
done

for n in "${miss_time[@]}"
do
  echo "$n" >> miss.txt
done

'''
######### Extract rdtsc latencies for misses
a=`grep -nri "found a miss for CPU: 0" result_random_2_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}.gz_champsim.trace_sender_old.gz.txt | awk -F ':' '{print $1}'`
as=($(echo "$a" | tr ' ' '\n'))

miss_arr=()
min=700000 #Any random value big enough than rdtsc values
for ((kk=0; kk<${#as[@]}; kk++))
do
  a=${as[$kk]}
  a=`expr ${a} + 40`
  miss=`cat result_random_2_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}.gz_champsim.trace_sender_old.gz.txt | head -n ${a} | tail -n 40 | grep -n "rdtsc_subtraction" | head -n 1 | awk '{print $2}'`
  #echo "$miss "
  if [ ${min} -gt ${miss} ]
  then
      min=${miss}
      #echo "$min"
  fi
  miss_arr[$kk]=${miss}
done
echo "Minimum miss latency is: $min"
echo "Miss array elements are: "
for n in "${miss_arr[@]}"
do
  echo "$n "
done

######### Extract rdtsc subtraction values. Set the hit/miss threshold.
grep -nri "rdtsc_subtraction" result_random_2_champsim.trace_receiver_multiple_access_${receiver_num_access_in_rdtsc}.gz_champsim.trace_sender_old.gz.txt | awk '{ print $2 }' > access_${receiver_num_access_in_rdtsc}_plot.txt
'''
#python3 cdf.py ${min} access_${receiver_num_access_in_rdtsc}_plot.txt ${receiver_num_access_in_rdtsc}
