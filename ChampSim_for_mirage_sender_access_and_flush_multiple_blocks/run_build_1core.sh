CORE=1
warcount=0
simcount=$1
trace1=$2 

repl_pol=(random)

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))

TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_forward_fill_early


echo "$traces"

for ((k=0; k<${#repl_pol[@]}; k++))
do
	repl=${repl_pol[$k]}
 ./build_champsim.sh bimodal no no ${repl} $CORE

#for ((j=0; j<${#traces[@]}; j++))
for ((j=0; j<1; j++))
do
	mix1=""
	for ((i=0; i<1; i++))
	do
		mix1="$mix1 ${TRACE_DIR}/${trace1}"
	done

	echo " (./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -traces $mix1)"
	(./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -traces $mix1) > result_${repl}_${CORE}_${trace1}.txt

done
done
