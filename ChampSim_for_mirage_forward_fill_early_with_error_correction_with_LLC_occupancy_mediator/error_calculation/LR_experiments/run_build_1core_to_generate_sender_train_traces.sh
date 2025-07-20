CORE=1
warcount=0
simcount=$1
trace1=$2 
string_size=$3
seed=$4
repl_pol=(random)

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))

TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/LR_experiments/sender_train_benchmark_traces

cd ../../

for ((k=0; k<${#repl_pol[@]}; k++))
do
	repl=${repl_pol[$k]}
# ./build_champsim.sh bimodal no no ${repl} $CORE

for ((j=0; j<1; j++))
do
	mix1=""
	for ((i=0; i<1; i++))
	do
		mix1="$mix1 ${TRACE_DIR}/${trace1}"
	done

	echo " (./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed}  -traces $mix1)"
	(./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed}  -traces $mix1) > error_calculation/LR_experiments/single_core_results/result_${repl}_${CORE}_${trace1}.txt

done
done
