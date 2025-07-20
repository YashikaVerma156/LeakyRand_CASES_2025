CORE=2
warcount=0
simcount=$1 #1875 #1590 #1875 #1590 #1875 #590 #2359395
trace1=$2
trace2=$3
echo "$CORE"

#repl_pol=(lru random)

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
	#trace=${traces[$j]}
  #trace1=champsim.trace_receiver_multiple_access_8.gz #champsim.trace_receiver.gz #champsim.trace_receiver_without_dependency.gz #champsim.trace_receiver.gz #champsim.trace_sender.gz  #champsim.trace.gz  #605.mcf_s-1554B.champsimtrace.xz
	#trace2=champsim.trace_sender_old.gz
	mix1=""
	mix2=""

	for ((i=0; i<1; i++))
	do
		mix1="$mix1 ${TRACE_DIR}/${trace1}"
		mix2="$mix2 ${TRACE_DIR}/${trace2}"
	done

	echo " (./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -traces $mix1 $mix2)"
	(./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -traces $mix1 $mix2) > result_${repl}_${CORE}_${trace1}_${trace2}.txt &
	#(./bin/bimodal-no-no-${repl}-${CORE}core -warmup_instructions $warcount -simulation_instructions $simcount ${option} -traces $mix1 $mix2) > result_32_44.txt &
#	exit

done
done
