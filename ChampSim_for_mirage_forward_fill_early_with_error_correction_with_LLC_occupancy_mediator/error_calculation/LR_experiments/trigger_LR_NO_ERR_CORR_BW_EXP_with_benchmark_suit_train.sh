########################################################################################
# 1. This script is to launch LR experiments, without error correction.                #
#    Total 'itr' iterations, each runs 'num_prl' LR experiments in parallel.           #
#    User should decide 'num_prl' by the number of cores in the system.                #
# 2. This script is called from calibration_and_variables.sh.                          #
########################################################################################

unroll_fact=$1    ## Specify unrolling factor
START_POINT=$2   ## Read START_POINT and END_POINT of the benchmark, as a reference for the seed and the benchmark to be used.
END_POINT=$3
num_prl=$4        ## Number of parallel instances to be run, depends on the number of cores.
benchmark_test=$5  ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
string_size=$6   # Message Length.
other_blocks=$7
arr_size=$8

sender_traces_directory="sender_train_benchmark_traces"
 
STR_NUM=${START_POINT}
echo "STR_NUM: $STR_NUM"

# Total Number of Experiments to run.
EXP_NUM=`expr \( ${END_POINT} - ${START_POINT} + 1 \) `

while [ ${EXP_NUM} -gt 0 ]
do
    #3. Run 'num_prl' experiments parallely.
    i=${num_prl}
    echo "Running parallel experiments"
    while [ ${i} -gt 0 ]
    do
            echo "i: $i STR_NUM: $STR_NUM EXP_NUM: $EXP_NUM"
            line_number=`expr $STR_NUM + 1`
            if [ $benchmark_test == 0 ] 
            then
                specific_line=$(sed -n "${line_number}p" seeds_train.txt)
                IFS=',' read -ra columns <<< "$specific_line"
                seed=${columns[32]} # Hard coded inside seeds_train.txt.
                echo "Seed: $seed"
            else
                specific_line=$(sed -n "${line_number}p" seeds_test.txt)
                IFS=',' read -ra columns <<< "$specific_line"
                seed=${columns[32]}
                echo "Seed: $seed"
            fi
            
            search_string="champsim.trace_sender_512_${STR_NUM}_"
            sim_count=0

            # Use a loop to iterate through each file in the directory
            for filename in $(ls "$sender_traces_directory"); do

                # Check if the filename contains the search string
                if [[ $filename == *"$search_string"* ]]; then
                    echo "Found match: $filename"

                    # Split the filename using both '_' and '.' as delimiters
                    IFS='_.' read -ra array <<< "$filename"

                    # Extract the desired part (index 5 in this case)
                    sim_count="${array[5]}"
                    break
                fi
            done
            if [[ ${unroll_fact} == 8 ]]; then
                receiver_trace=champsim.trace_receiver_multiple_access_${unroll_fact}_other_blocks_326_${string_size}.gz
            else     
                receiver_trace=champsim.trace_receiver_multiple_access_${unroll_fact}_other_blocks_327_${string_size}.gz
            fi
            sender_trace=champsim.trace_sender_${string_size}_${STR_NUM}_${sim_count}_train.gz
            ./run_2core_LR_NO_ERR_CORR_train.sh $sim_count $receiver_trace $sender_trace $seed $string_size &

            STR_NUM=`expr ${STR_NUM} + 1`
            i=`expr ${i} - 1`
            EXP_NUM=`expr $EXP_NUM - 1`
            if [ $EXP_NUM == 0 ]
            then
                break
            fi
    done 
    wait
done
