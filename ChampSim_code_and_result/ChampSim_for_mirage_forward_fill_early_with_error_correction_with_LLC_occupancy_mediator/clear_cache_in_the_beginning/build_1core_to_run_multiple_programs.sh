CORE=1
extra_blocks=$1
extra_for_llc_occ_chk=$2
arr_start_addr=$3
max_arr_len=$4

repl_pol=(random)

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))

TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/clear_cache_in_the_beginning

cd ../


sed -i.bak 's/\<EXTRA_ARR_SIZE 0\>/EXTRA_ARR_SIZE '${extra_blocks}'/g' inc/config.h
sed -i.bak 's/\<EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK 0\>/EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK '${extra_for_llc_occ_chk}'/g' inc/config.h
sed -i.bak 's/\<ARR_MAX_LIMIT 0\>/ARR_MAX_LIMIT '${max_arr_len}'/g' inc/config.h
sed -i.bak 's/\<ARR_START_ADDR 0\>/ARR_START_ADDR '${arr_start_addr}'/g' inc/config.h

for ((k=0; k<${#repl_pol[@]}; k++))
do
	repl=${repl_pol[$k]}
 ./build_champsim.sh bimodal no no ${repl} $CORE

sed -i.bak 's/\<EXTRA_ARR_SIZE '${extra_blocks}'\>/EXTRA_ARR_SIZE 0/g' inc/config.h
sed -i.bak 's/\<EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK '${extra_for_llc_occ_chk}'\>/EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK 0/g' inc/config.h
sed -i.bak 's/\<ARR_MAX_LIMIT '${max_arr_len}'\>/ARR_MAX_LIMIT 0/g' inc/config.h
sed -i.bak 's/\<ARR_START_ADDR '${arr_start_addr}'\>/ARR_START_ADDR 0/g' inc/config.h

done
