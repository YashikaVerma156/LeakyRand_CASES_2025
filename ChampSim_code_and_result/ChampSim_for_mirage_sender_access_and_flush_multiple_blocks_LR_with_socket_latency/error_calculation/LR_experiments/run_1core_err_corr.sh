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
extra_arr_max_addr=$9
Trace_dir=${10}
Result_dir=${11}

echo "$max_arr_len $extra_arr_max_addr"
repl_pol=(random)

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))


cd ../../

echo "$traces $max_arr_len"


for ((k=0; k<${#repl_pol[@]}; k++))
do
	repl=${repl_pol[$k]}

	for ((j=0; j<1; j++))
	do
			mix1=""
			for ((i=0; i<1; i++))
			do
				mix1="$mix1 ${Trace_dir}/${trace1}"
			done

			echo " (./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed}  -traces $mix1)"
			(./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed}  -traces $mix1) > ${Result_dir}/result_${repl}_${CORE}_${trace1}.txt

	done
done
