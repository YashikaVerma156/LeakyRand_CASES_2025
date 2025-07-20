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
binary_path=$9

repl_pol=(random)

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))

TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

cd ../

echo "$traces $max_arr_len"

sed -i.bak 's/\<EXTRA_ARR_SIZE 0\>/EXTRA_ARR_SIZE '${extra_blocks}'/g' inc/config.h
sed -i.bak 's/\<EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK 0\>/EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK '${extra_for_llc_occ_chk}'/g' inc/config.h
sed -i.bak 's/\<ARR_MAX_LIMIT 0\>/ARR_MAX_LIMIT '${max_arr_len}'/g' inc/config.h
sed -i.bak 's/\<ARR_START_ADDR 0\>/ARR_START_ADDR '${arr_start_addr}'/g' inc/config.h

for ((k=0; k<${#repl_pol[@]}; k++))
do
	repl=${repl_pol[$k]}
	# ./build_champsim.sh bimodal no no ${repl} $CORE

	sed -i.bak 's/\<EXTRA_ARR_SIZE '${extra_blocks}'\>/EXTRA_ARR_SIZE 0/g' inc/config.h
	sed -i.bak 's/\<EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK '${extra_for_llc_occ_chk}'\>/EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK 0/g' inc/config.h
	sed -i.bak 's/\<ARR_MAX_LIMIT '${max_arr_len}'\>/ARR_MAX_LIMIT 0/g' inc/config.h
	sed -i.bak 's/\<ARR_START_ADDR '${arr_start_addr}'\>/ARR_START_ADDR 0/g' inc/config.h

	#for ((j=0; j<${#traces[@]}; j++))
	for ((j=0; j<1; j++))
	do
		mix1=""
		for ((i=0; i<1; i++))
		do
			mix1="$mix1 ${TRACE_DIR}/${trace1}"
		done

    cd ${binary_path}
    pwd
		echo " (./bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed}  -traces $mix1)"
		(./bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed}  -traces $mix1) > ${TRACE_DIR}/result_${repl}_${CORE}_${trace1}.txt

	done
done
