CORE=2
warcount=0
simcount=$1 #1875 #1590 #1875 #1590 #1875 #590 #2359395
trace1=$2
trace2=$3
seed=$4
string_size=$5
extra_blocks=$6
max_arr_len=$7
repetetion_coding=$8
differential_signalling=$9
LLC_size=${10}
llc_sets=${11}
TRACE_DIR=${12}
#repl_pol=(lru random)

repl_pol=(random)

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))

echo "llc_sets: $llc_sets, LLC_size: $LLC_size"
echo "${trace1} ${trace2}"



#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early_with_error_correction_with_LLC_occupancy_mediator/error_calculation/other_blocks_in_LLC_1_percent_msglen_4

cd ../../

sed -i.bak 's/\<IEEE_SEED 0\>/IEEE_SEED 1/g' inc/champsim.h
sed -i.bak 's/\<EXTRA_ARR_SIZE 0\>/EXTRA_ARR_SIZE '${extra_blocks}'/g' inc/config.h
sed -i.bak 's/\<ARR_MAX_LIMIT 0\>/ARR_MAX_LIMIT '${max_arr_len}'/g' inc/config.h
sed -i.bak 's/\<REPETETION_CODING 0\>/REPETETION_CODING '${repetetion_coding}'/g' inc/config.h
sed -i.bak 's/\<DIFFERENTIAL_SIGNALLING 0\>/DIFFERENTIAL_SIGNALLING '${differential_signalling}'/g' inc/config.h
#sed -i 's/#define LLC_SET 1\*2048/#define LLC_SET 1\*'${llc_sets}'/' inc/cache.h
#sed -i 's/#define data_LLC_WAY 16\*2048\*1/#define data_LLC_WAY 16\*'${llc_sets}'\*1/' inc/cache.h

for ((k=0; k<${#repl_pol[@]}; k++))
do
	repl=${repl_pol[$k]}
./build_champsim.sh bimodal no no ${repl} $CORE

sed -i.bak 's/\<IEEE_SEED 1\>/IEEE_SEED 0/g' inc/champsim.h
sed -i.bak 's/\<EXTRA_ARR_SIZE '${extra_blocks}'\>/EXTRA_ARR_SIZE 0/g' inc/config.h
sed -i.bak 's/\<ARR_MAX_LIMIT '${max_arr_len}'\>/ARR_MAX_LIMIT 0/g' inc/config.h
sed -i.bak 's/\<REPETETION_CODING '${repetetion_coding}'\>/REPETETION_CODING 0/g' inc/config.h
sed -i.bak 's/\<DIFFERENTIAL_SIGNALLING '${differential_signalling}'\>/DIFFERENTIAL_SIGNALLING 0/g' inc/config.h
#sed -i 's/#define LLC_SET 1\*'${llc_sets}'/#define LLC_SET 1\*2048/' inc/cache.h
#sed -i 's/#define data_LLC_WAY 16\*'${llc_sets}'\*1/#define data_LLC_WAY 16\*2048\*1/' inc/cache.h

#for ((j=0; j<${#traces[@]}; j++))
for ((j=0; j<1; j++))
do
	mix1=""
	mix2=""

	for ((i=0; i<1; i++))
	do
		mix1="$mix1 ${TRACE_DIR}/${trace1}"
		mix2="$mix2 ${TRACE_DIR}/${trace2}"
	done

	echo " (./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces $mix1 $mix2)"
	(./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces $mix1 $mix2) > error_calculation/other_blocks_in_LLC_1_percent_msglen_4/results/result_${repl}_${CORE}_${trace1}_${trace2}.txt
#	(./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed} -traces $mix1 $mix2) > error_calculation/other_blocks_in_LLC_1_percent_msglen_${string_size}/result_2_recent.txt

done
done

#sed -i.bak 's/\<SEED_NUMBER '${seed}'\>/SEED_NUMBER 0/g' inc/champsim.h
