prioritize_invalid_blocks=$1

# Check if exactly one argument is passed
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <prioritize_invalid_blocks>"
    echo "prioritize_invalid_blocks should be either 1 or 0"
    exit 1
fi

array_size=524144
# The below variable says that array size to LLC size ratio is 1.
array_to_llc_size=2 #XXX A special case, where array size is 2X the LLC size.
pin_files_path=../../../pin-3.21-98484-ge7cd811fd-gcc-linux/source/tools/ManualExamples/LR_sender_receiver_code
pin_output_file=logs.txt
Result_dir=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_to_mimick_DMA_invalidations/error_calculation/LR_experiments/single_core_results
path_to_copy_trace=/home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_to_mimick_DMA_invalidations/error_calculation/LR_experiments/sender_err_corr_both_algo_sender_access_and_flush_multiple_blocks
back_to_exp_dir=../../../../../ChampSim_for_mirage_to_mimick_DMA_invalidations/error_calculation/LR_experiments
result_dir=/home/yashikav/Desktop/Mirage_project/fig1/invalidation_results

mkdir -p /home/yashikav/Desktop/Mirage_project/ChampSim_for_mirage_to_mimick_DMA_invalidations/error_calculation/LR_experiments/sender_err_corr_both_algo_sender_access_and_flush_multiple_blocks

mkdir -p /home/yashikav/Desktop/Mirage_project/fig1/invalidation_results

#for num_io_calls in 20 #2   
#do
#    for num_arr_accesses in 2 4 6 8
#    do
#        for num_mid_points in 1  #XXX The mid_point is made so as the whole 2x is accessed.
#	#do
#	    num_access=${num_arr_accesses} # 2, 4, 6, 8  #5    
#	    num_mid=${num_mid_points}
#	    io_calls=${num_io_calls}  #5
#
#            ######### 1.a Generate pintrace for sender.  #############
#	    cd ${pin_files_path}
#	    ./commands_invalidations_program.sh ${num_access} ${array_size} ${num_mid} ${io_calls} ${array_to_llc_size} ${path_to_copy_trace}
#
#	    ########### 1.b Extract the number of instructions in the trace file.
#	    sim_count=`grep -oE 'instrCount: [0-9]+' ${pin_output_file} | awk '{print $2}'`
#	    echo "$sim_count"
#
#	    cd ${back_to_exp_dir}
#	    trace=champsim.trace_invalidation_${num_mid}_io_calls_${io_calls}_arr_access_${num_access}_array_size_${array_to_llc_size}.gz
#	    ./run_build_1core_err_corr_IO_invalidation.sh ${sim_count} ${trace} 0 100451 0 0 0 0 0 ${prioritize_invalid_blocks} ${path_to_copy_trace} ${result_dir}
#            exit
#        done
#    done 
#done

array_size=262072
array_to_llc_size=1 # This variable says that array size to LLC size ratio is 1.
for num_io_calls in 20 #2   
do
    for num_arr_accesses in 2 4 6 8
    do
        for num_mid_points in 0.125 0.25 0.5 0.75 1  #2 #TODO a special case, where array size is needed to be increased.
        do
            num_access=${num_arr_accesses} # 2, 4, 6, 8  #5    
            num_mid=${num_mid_points}
            io_calls=${num_io_calls}  #5

            ######### 1.a Generate pintrace for sender.  #############
            cd ${pin_files_path}
            ./commands_invalidations_program.sh ${num_access} ${array_size} ${num_mid} ${io_calls} ${array_to_llc_size} ${path_to_copy_trace}

            ########### 1.b Extract the number of instructions in the trace file.
            sim_count=`grep -oE 'instrCount: [0-9]+' ${pin_output_file} | awk '{print $2}'`
            echo "$sim_count"

            cd ${back_to_exp_dir}
            trace=champsim.trace_invalidation_${num_mid}_io_calls_${io_calls}_arr_access_${num_access}_array_size_${array_to_llc_size}.gz

            ./run_build_1core_err_corr_IO_invalidation.sh ${sim_count} ${trace} 0 100451 0 0 0 0 0 ${prioritize_invalid_blocks} ${path_to_copy_trace} ${result_dir}
        done
    done
done
