CORE=2
warcount=0
repetetion_coding=$1
differential_signalling=$2

#repl_pol=(lru random)

repl_pol=(random)

#TRACE_DIR=/home/yashikav/Desktop/Mirage_project/ChampSim/traces
#trace_string=$(ls /home/yashikav/Desktop/Mirage_project/ChampSim/traces)
#traces=($(echo "$trace_string" | tr ' ' '\n'))



cd ../../

#sed -i.bak 's/\<SEED_NUMBER 0\>/SEED_NUMBER '${seed}'/g' inc/champsim.h
#sed -i.bak 's/\<EXTRA_ARR_SIZE 0\>/EXTRA_ARR_SIZE '${extra_blocks}'/g' inc/config.h
#sed -i.bak 's/\<ARR_MAX_LIMIT 0\>/ARR_MAX_LIMIT '${max_arr_len}'/g' inc/config.h
sed -i.bak 's/\<REPETETION_CODING 0\>/REPETETION_CODING '${repetetion_coding}'/g' inc/config.h
sed -i.bak 's/\<DIFFERENTIAL_SIGNALLING 0\>/DIFFERENTIAL_SIGNALLING '${differential_signalling}'/g' inc/config.h

for ((k=0; k<${#repl_pol[@]}; k++))
do
    repl=${repl_pol[$k]}
    ./build_champsim.sh bimodal no no ${repl} $CORE

#    sed -i.bak 's/\<EXTRA_ARR_SIZE '${extra_blocks}'\>/EXTRA_ARR_SIZE 0/g' inc/config.h
#    sed -i.bak 's/\<ARR_MAX_LIMIT '${max_arr_len}'\>/ARR_MAX_LIMIT 0/g' inc/config.h
    sed -i.bak 's/\<REPETETION_CODING '${repetetion_coding}'\>/REPETETION_CODING 0/g' inc/config.h
    sed -i.bak 's/\<DIFFERENTIAL_SIGNALLING '${differential_signalling}'\>/DIFFERENTIAL_SIGNALLING 0/g' inc/config.h

done

#sed -i.bak 's/\<SEED_NUMBER '${seed}'\>/SEED_NUMBER 0/g' inc/champsim.h
