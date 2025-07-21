CORE=1
warcount=0
simcount=$1 #1875 #1590 #1875 #1590 #1875 #590 #2359395
trace1=$2
seed=$3
string_size=$4
repetetion_coding=$5
differential_signalling=$6
llc_sets=$7
TRACE_DIR=$8


repl_pol=(random)

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))

cd ../../

echo "$traces"

sed -i.bak 's/\<REPETETION_CODING 0\>/REPETETION_CODING '${repetetion_coding}'/g' inc/config.h
sed -i.bak 's/\<DIFFERENTIAL_SIGNALLING 0\>/DIFFERENTIAL_SIGNALLING '${differential_signalling}'/g' inc/config.h

# Commented becuse it is evaluated for 2MB LLC only.
#sed -i 's/#define LLC_SET 1\*2048/#define LLC_SET 1\*'${llc_sets}'/' inc/cache.h
#sed -i 's/#define data_LLC_WAY 16\*2048\*1/#define data_LLC_WAY 16\*'${llc_sets}'\*1/' inc/cache.h


for ((k=0; k<${#repl_pol[@]}; k++))
do
	repl=${repl_pol[$k]}
 ./build_champsim.sh bimodal no no ${repl} $CORE

sed -i.bak 's/\<REPETETION_CODING '${repetetion_coding}'\>/REPETETION_CODING 0/g' inc/config.h
sed -i.bak 's/\<DIFFERENTIAL_SIGNALLING '${differential_signalling}'\>/DIFFERENTIAL_SIGNALLING 0/g' inc/config.h

# Commented becuse it is evaluated for 2MB LLC only.
#sed -i 's/#define LLC_SET 1\*'${llc_sets}'/#define LLC_SET 1\*2048/' inc/cache.h
#sed -i 's/#define data_LLC_WAY 16\*'${llc_sets}'\*1/#define data_LLC_WAY 16\*2048\*1/' inc/cache.h

#for ((j=0; j<${#traces[@]}; j++))
  for ((j=0; j<1; j++))
  do
 	  mix1=""
	  for ((i=0; i<1; i++))
	  do
		  mix1="$mix1 ${TRACE_DIR}/${trace1}"
	  done

	  echo " (./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed}  -traces $mix1)"
	  (./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -random_seed ${seed}  -traces $mix1) > error_calculation/other_blocks_in_LLC_1_percent_msglen_4/results/result_${repl}_${CORE}_${trace1}.txt

  done
done
