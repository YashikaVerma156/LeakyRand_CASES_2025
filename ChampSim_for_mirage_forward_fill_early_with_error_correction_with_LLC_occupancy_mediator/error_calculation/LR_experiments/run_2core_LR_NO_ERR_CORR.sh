CORE=2
warcount=0
simcount=$1 #1875 #1590 #1875 #1590 #1875 #590 #2359395
trace1=$2
trace2=$3
seed=$4
string_size=$5

#repl_pol=(lru random)


repl_pol=(random)

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))

SENDER_TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments/sender_test_benchmark_traces
RECEIVER_TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments

#cd ../../

for ((k=0; k<${#repl_pol[@]}; k++))
do
	repl=${repl_pol[$k]}

for ((j=0; j<1; j++))
do
	mix1=""
	mix2=""

	for ((i=0; i<1; i++))
	do
		mix1="$mix1 ${RECEIVER_TRACE_DIR}/${trace1}"
		mix2="$mix2 ${SENDER_TRACE_DIR}/${trace2}"
	done

	echo " (./binary/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces $mix1 $mix2)"
	(./binary/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces $mix1 $mix2) > LR_NO_ERR_CORR_2core_results/result_${repl}_${CORE}_${trace1}_${trace2}.txt
#	(./binary/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces $mix1 $mix2) > error_calculation/other_blocks_in_LLC_1_percent_msglen_23/result_2_recent.txt

done
done

