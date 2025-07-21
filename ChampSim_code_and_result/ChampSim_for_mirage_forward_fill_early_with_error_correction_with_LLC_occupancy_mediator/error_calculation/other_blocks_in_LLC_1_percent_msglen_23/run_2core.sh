CORE=2
warcount=0
simcount=$1 #1875 #1590 #1875 #1590 #1875 #590 #2359395
trace1=$2
trace2=$3
seed=$4
string_size=$5
extra_blocks=$6
extra_for_llc_occ_chk=$7
arr_start_addr=$8
max_arr_len=$9
extra_arr_max_addr=${10}
msg_type=${11}

#repl_pol=(lru random)


repl_pol=(random)

TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}_${msg_type}


for ((k=0; k<${#repl_pol[@]}; k++))
do
	repl=${repl_pol[$k]}

	mix1=""
	mix2=""

	for ((i=0; i<1; i++))
	do
		mix1="$mix1 ${TRACE_DIR}/${trace1}"
		mix2="$mix2 ${TRACE_DIR}/${trace2}"
	done

	echo " (./binary/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces $mix1 $mix2)"
	(./binary/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces $mix1 $mix2) > error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}_${msg_type}/result_${repl}_${CORE}_${trace1}_${trace2}_${seed}.txt

done
