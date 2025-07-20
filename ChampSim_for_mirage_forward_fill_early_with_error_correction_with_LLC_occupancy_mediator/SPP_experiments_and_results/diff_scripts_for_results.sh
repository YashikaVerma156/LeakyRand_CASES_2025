file=$1

#grep -nri -m 2 "wait" $file| grep -o '^[^:]*' | awk '{print $1}'
#line_numbers_for_string_wait=`grep -nri "wait" $file| awk -F: 'NR>1 {print $1}'`
grep -nri -P "cpu 0 is on wait" $file | awk  '{print $1, $12}' | sed -s 's/:SSSSSSSS//'
line_numbers_for_string_wait=`grep -nri "cpu 0 is on wait" $file| awk -F: '{print $1}'`
wait_line_numbers=($(echo "$line_numbers_for_string_wait" | tr ' ' '\n'))

for ((kk=0; kk<${#wait_line_numbers[@]}-1; kk++))
do
    #echo "${wait_line_numbers[$kk]} ${wait_line_numbers[`expr $kk + 1`]} "
    start=${wait_line_numbers[$kk]}
    end=${wait_line_numbers[`expr $kk + 1`]}
    #awk '$start <=NR && NR <= $end && /found a miss for CPU: 1/ {print NR,$0}' $file | wc
    cpu0=`awk -v start="$start" -v end="$end" 'NR >= start && NR <= end && /found a miss for CPU: 0/ {print $0}' "$file" | wc -l`
    cpu1=`awk -v start="$start" -v end="$end" 'NR >= start && NR <= end && /found a miss for CPU: 1/ {print $0}' "$file" | wc -l`
    echo "${wait_line_numbers[$kk]} ${wait_line_numbers[`expr $kk + 1`]} ${cpu0}, ${cpu1}"
    cpu0=`awk -v start="$start" -v end="$end" 'NR >= start && NR <= end && /row buffer hit  open cpu: 0/ {print $0}' "$file" | wc -l`
    cpu1=`awk -v start="$start" -v end="$end" 'NR >= start && NR <= end && /row buffer hit  open cpu: 1/ {print $0}' "$file" | wc -l`
    echo "${wait_line_numbers[$kk]} ${wait_line_numbers[`expr $kk + 1`]} ${cpu0}, ${cpu1}"
    cpu0=`awk -v start="$start" -v end="$end" 'NR >= start && NR <= end && /row buffer miss  open cpu: 0/ {print $0}' "$file" | wc -l`
    cpu1=`awk -v start="$start" -v end="$end" 'NR >= start && NR <= end && /row buffer miss  open cpu: 1/ {print $0}' "$file" | wc -l`
    echo "${wait_line_numbers[$kk]} ${wait_line_numbers[`expr $kk + 1`]} ${cpu0}, ${cpu1}"
 
    echo "=============================="

    #awk -v start="${wait_line_numbers[$kk]}" -v end="${wait_line_numbers[$kk + 1]}" '$1 >= start && $1 <= end && /found a miss for CPU: 1/ {print NR, $0}' "$file" | wc

done
