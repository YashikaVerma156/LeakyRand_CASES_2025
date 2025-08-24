benchmark_train=0
for err_trigger_point in 1 2 4 8 16 32
do
    for err_corr_iterations in 100 800
    do
        python3 data_processing_with_err_corr_31_other_blocks.py ${benchmark_train} ${err_trigger_point} ${err_corr_iterations}
    done
done

