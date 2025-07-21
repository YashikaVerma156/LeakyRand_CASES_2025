################################################################################################################
# 1) What does it do: run multiple experiments to generate SPP error and bandwidth results.                    #
# 2) Pre-requisites:                                                                                           #
# a) /binary folder should have the 2-core binary ready for SPP.                                               #
# b) The receiver trace should be generated a priori. As the receiver trace does not change unless the string  #
#    size is changed.                                                                                          #
# c) This script uses two more scripts i) SPP_error_bandwidth_experiment_multiple_run.sh and ii) run_2core.sh. # 
#    These two should be present in the current dir.                                                           #
################################################################################################################


NUM_EXP=50
exp_ran=0
sender_arr_size_list=(3276 6554 9830 13108 16384)

while [ ${exp_ran} -ne ${NUM_EXP} ]
do
    
    seed=$((RANDOM % 100000 + 1))
    msg_num=$((RANDOM % 100000 + 1))

    #for ((kk=0; kk<${#sender_arr_size_list[@]}; kk++))
    #do
    #    ./SPP_error_bandwidth_experiment_multiple_run.sh ${sender_arr_size_list[${kk}]} ${seed} ${msg_num} 
    #done

        ./SPP_error_bandwidth_experiment_multiple_run.sh ${sender_arr_size_list[0]} ${seed} ${msg_num} &
        ./SPP_error_bandwidth_experiment_multiple_run.sh ${sender_arr_size_list[1]} ${seed} ${msg_num} &
        ./SPP_error_bandwidth_experiment_multiple_run.sh ${sender_arr_size_list[2]} ${seed} ${msg_num} &
        ./SPP_error_bandwidth_experiment_multiple_run.sh ${sender_arr_size_list[3]} ${seed} ${msg_num} &
        ./SPP_error_bandwidth_experiment_multiple_run.sh ${sender_arr_size_list[4]} ${seed} ${msg_num} 
        wait
    	exp_ran=`expr ${exp_ran} + 1`
done
