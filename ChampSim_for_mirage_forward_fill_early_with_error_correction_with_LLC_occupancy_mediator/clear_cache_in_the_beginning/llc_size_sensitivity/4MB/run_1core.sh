CORE=1
warcount=0
simcount=$1
trace1=$2 
string_size=$3
seed=$4
extra_blocks=$5
extra_for_llc_occ_chk=$6
arr_start_addr=$7
max_arr_len=$8

echo "$max_arr_len"
repl="random"

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))

TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning/llc_size_sensitivity/4MB

cd ../../../

echo "$traces $max_arr_len"

#for ((j=0; j<${#traces[@]}; j++))
for ((j=0; j<1; j++))
do
	mix1=""
	for ((i=0; i<1; i++))
	do
		mix1="$mix1 ${TRACE_DIR}/${trace1}"
	done

	echo " (./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed}  -traces $mix1)"
	(./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed}  -traces $mix1) > clear_cache_in_the_beginning/llc_size_sensitivity/4MB/result_${repl}_${CORE}_${trace1}.txt

done
