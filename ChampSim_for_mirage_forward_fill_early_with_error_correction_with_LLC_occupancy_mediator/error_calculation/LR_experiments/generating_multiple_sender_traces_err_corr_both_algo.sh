benchmark_test=$1       ## 0 : train_benchmark is used(250 records). 1 : test_benchmark is used(500 records).

for err_corr_interval in 8 16 32 64 128
do
  if [ ${benchmark_test} -eq 1 ];
  then
      i=500
  else
      i=250
  fi

  #XXX Make sure to uncomment the build in "run_build_1core_to_generate_sender_traces.sh" for one execution.

  # File to capture the occupancy of traces.
  rm occupancy_tracker_${err_corr_interval}_${benchmark_test}.txt
  touch occupancy_tracker_${err_corr_interval}_${benchmark_test}.txt

  mkdir -p sender_err_corr_both_algo

  while [ ${i} -gt 0 ]
  do
        echo "string_num: $i"
        ./generate_sender_trace_err_corr_both_algo.sh ${i} ${benchmark_test} ${err_corr_interval} # XXX Sim count to be identified in the pin directory and the trace name should have this information.
        i=`expr ${i} - 1`
  done

done
