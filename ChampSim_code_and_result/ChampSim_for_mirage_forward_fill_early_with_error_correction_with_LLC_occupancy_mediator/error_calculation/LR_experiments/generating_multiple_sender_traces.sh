benchmark_test=1       ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).
i=500

#XXX Make sure to uncomment the build in "run_build_1core_to_generate_sender_traces.sh" for one execution.

## File to capture the occupancy of traces.
rm occupancy_tracker.txt
touch occupancy_tracker.txt

mkdir -p sender_test_benchmark_traces
mkdir -p single_core_results


while [ ${i} -gt 0 ]
do
        echo "string_num: $i"
        ./generate_sender_trace_no_error_correction.sh ${i} ${benchmark_test} # XXX Sim count to be identified in the pin directory and the trace name should have this information.
        i=`expr ${i} - 1`
done
