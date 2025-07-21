CORE=2
warcount=0
simcount=$1
trace1=$2
trace2=$3
seed=$4
string_size=$5
extra_blocks=$6
max_arr_len=$7
repetetion_coding=$8
differential_signalling=$9
sender_arr_size=${10}
path=${11}
repl_pol=(random)

echo "${trace1} ${trace2}"

TRACE_DIR=${path}

for ((k=0; k<${#repl_pol[@]}; k++))
do
        repl=${repl_pol[$k]}
for ((j=0; j<1; j++))
do
        mix1=""
        mix2=""

        for ((i=0; i<1; i++))
        do
                mix1="$mix1 ${TRACE_DIR}/${trace1}"
                mix2="$mix2 ${TRACE_DIR}/${trace2}"
        done

#       (./binary/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces $mix1 $mix2) > error_calculation/other_blocks_in_LLC_1_percent_msglen_4/sender_arr_size_${sender_arr_size}/result_${repl}_${CORE}_${trace1}_${trace2}.txt
        (./binary/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces $mix1 $mix2) > sender_arr_size_${sender_arr_size}/all1s/result_${repl}_${CORE}_${trace1}_${trace2}_${seed}.txt

done
done

