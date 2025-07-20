CORE=2
warcount=0
simcount=$1 #1875 #1590 #1875 #1590 #1875 #590 #2359395
receiver_trace=$2
sender_trace=$3
seed=$4
string_size=$5
result_dir=$6
binary_path=$7

#extra_blocks=$6
#extra_for_llc_occ_chk=$7
#arr_start_addr=$8
#max_arr_len=$9
#extra_arr_max_addr=${10}

#repl_pol=(lru random)


repl_pol=(random)

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))

for ((k=0; k<${#repl_pol[@]}; k++))
do
	repl=${repl_pol[$k]}
  r_trace=$(basename ${receiver_trace})
  s_trace=$(basename ${sender_trace})
	echo " (./${binary_path}/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces ${receiver_trace} ${sender_trace} )"
	(./${binary_path}/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces ${receiver_trace} ${sender_trace}) > ${result_dir}/result_${repl}_${CORE}_${r_trace}_${s_trace}.txt

done
