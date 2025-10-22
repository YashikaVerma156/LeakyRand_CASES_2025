gcc parse_multiple_files_edited_error_calc_including_initial_bits_single_suite.c

mkdir -p different_optimal_n_for_different_subspace

for bench_test in 0
do
    if [ ${bench_test} -eq 0 ];
    then
        benchmark_str='train'
    else
        benchmark_str='test'
    fi

    for less_miss in 0
    do
        for ds in 2500 #5750 1000 3000
        do
            for uf in 16 32 64 128 #32 64 128 #16
            do
                mkdir -p different_optimal_n_for_different_subspace/DS_${ds}
                for hist_len in 1 2 3 4 #0 1 2 3 4
                do  
                    optimal_n_arr=()
                    optimal_n_err_arr=()
                    optimal_n_threshold=()
                    optimal_n_err_1to0_arr=()
                    optimal_n_err_0to1_arr=()
                    #TH_limit=`expr $uf - 60`

                    #Identify the start and end limit.
                    #Check if the limits are already defined.
                    check_var=$(grep ${ds}_${hist_len}_${uf}_${less_miss}_${bench_test} histlen_n_start_end_limit.txt | awk '{print $2}')
                    if [ -z "$check_var" ]; then #checking if the string is empty. Keep $check_var in quotes.
												echo "yes"
                        # Run the loop and identify the TH_start and TH_limit values.
                        for (( TH=1; TH<=uf; TH++));
                        do
												    if [ ${bench_test} -eq 0 ];
												    then
                                ./a.out benchmark_train.txt misses_results_${ds}_train/misses_results_${uf} ${hist_len} ${uf} ${TH} ${less_miss} 0 > temp.txt
                            else
                                ./a.out benchmark_test.txt misses_results_${ds}_test/misses_results_${uf} ${hist_len} ${uf} ${TH} ${less_miss} 0 > temp.txt
                            fi

                            str=$(grep -n "Total error:" temp.txt)
                            if [ ! -z "$str" ]; then
                                #echo "Yes happening"
                                TH_start=${TH}
                                break
                            fi

                        done
                        
                        for (( TH=uf; TH>=1; TH--));
                        do
                            if [ ${bench_test} -eq 0 ];
                            then
                                ./a.out benchmark_train.txt misses_results_${ds}_train/misses_results_${uf} ${hist_len} ${uf} ${TH} ${less_miss} 0 > temp.txt
                            else
                                ./a.out benchmark_test.txt misses_results_${ds}_test/misses_results_${uf} ${hist_len} ${uf} ${TH} ${less_miss} 0 > temp.txt
                            fi

                            str=$(grep -n "Total error:" temp.txt)
                            if [ ! -z "$str" ]; then
                                #echo "Yes happening"
                                TH_limit=${TH}
                                break
                            fi

                        done
                        echo "${ds}_${hist_len}_${uf}_${less_miss}_${bench_test} $TH_start $TH_limit" >> histlen_n_start_end_limit.txt

                    else
												TH_start=$(grep ${ds}_${hist_len}_${uf}_${less_miss}_${bench_test} histlen_n_start_end_limit.txt | awk '{print $2}')
												TH_limit=$(grep ${ds}_${hist_len}_${uf}_${less_miss}_${bench_test} histlen_n_start_end_limit.txt | awk '{print $3}')
												echo "no"
                        #echo "TH_start $TH_start $TH_limit"
                    fi

                    for (( TH=TH_start; TH<=TH_limit; TH++));
                    do
                        echo "$less_miss $hist_len $ds $uf $TH"
                        
												if [ ${bench_test} -eq 0 ];
												then
                            ./a.out benchmark_train.txt misses_results_${ds}_train/misses_results_${uf} ${hist_len} ${uf} ${TH} ${less_miss} 0 > temp.txt
                        else
                            ./a.out benchmark_test.txt misses_results_${ds}_test/misses_results_${uf} ${hist_len} ${uf} ${TH} ${less_miss} 0 > temp.txt
                        fi
                        limit=$((2 ** hist_len))
                        echo "limit= $limit"
                        #exit
                        #exit
                        for (( i =0; i<limit; i++));
                        do
                            string=min_error$i:
                            #echo "$string"
                            num_err=$(grep ${string} DS_${ds}_histlen_${hist_len}_${uf}_${TH}_single_${benchmark_str}_suite_${less_miss}_naive.txt | awk '{ print $2 }' | awk -F ',' '{ print $1 }')
                            num_err_threshold=$(grep ${string} DS_${ds}_histlen_${hist_len}_${uf}_${TH}_single_${benchmark_str}_suite_${less_miss}_naive.txt | awk '{ print $4 }' | awk -F ',' '{ print $1 }')
                            num_err_1to0=$(grep ${string} DS_${ds}_histlen_${hist_len}_${uf}_${TH}_single_${benchmark_str}_suite_${less_miss}_naive.txt | awk '{ print $6 }' | awk -F ',' '{ print $1 }')
                            num_err_0to1=$(grep ${string} DS_${ds}_histlen_${hist_len}_${uf}_${TH}_single_${benchmark_str}_suite_${less_miss}_naive.txt | awk '{ print $8 }' | awk -F ',' '{ print $1 }')
                            #echo $num_err $num_err_threshold $num_err_1to0 $num_err_0to1
                            if [ $TH -eq $TH_start ]; # This is first iteration.
                            then
                                optimal_n_arr[i]=$TH                           # optimal n
                                optimal_n_err_arr[i]=$num_err                  # total errors
                                optimal_n_threshold[i]=$num_err_threshold      # threshold observed for the subspace
                                optimal_n_err_1to0_arr[i]=$num_err_1to0        # total 1to0 error
                                optimal_n_err_0to1_arr[i]=$num_err_0to1        # total 0to1 error
                            else
                                #echo " line 111 ${num_err}  ${optimal_n_err_arr[$i]}"
                                if [ ${num_err} -lt ${optimal_n_err_arr[$i]} ]; 
                                then
                                    optimal_n_arr[i]=$TH                       # optimal n
                                    optimal_n_err_arr[i]=$num_err              # total errors
                                    optimal_n_threshold[i]=$num_err_threshold  # threshold observed for the subspace
                                    optimal_n_err_1to0_arr[i]=$num_err_1to0    # total 1to0 error
                                    optimal_n_err_0to1_arr[i]=$num_err_0to1    # total 0to1 error
                                fi
                            fi
                        done
                        mv DS_${ds}_histlen_${hist_len}_${uf}_${TH}_single_${benchmark_str}_suite_${less_miss}_naive.txt different_optimal_n_for_different_subspace/DS_${ds}
                        #exit
                    done
                    rm final_result_${ds}_histlen_${hist_len}_${uf}_${TH}_train_suite_${less_miss}_naive.txt
                    rm final_result_${ds}_histlen_${hist_len}_${uf}_${TH}_test_suite_${less_miss}_naive.txt
                    echo "For History_length: $hist_len UF: $uf ds: $ds ${benchmark_str}"
                    echo "For History_length: $hist_len UF: $uf ds: $ds ${benchmark_str}" >> final_result_${ds}_histlen_${hist_len}_${uf}_${TH}_train_suite_${less_miss}_naive.txt
                    for (( i =0; i<limit; i++));
                    do
                        echo "$i train ${optimal_n_arr[$i]} ${optimal_n_err_arr[$i]} ${optimal_n_threshold[$i]} ${optimal_n_err_1to0_arr[$i]} ${optimal_n_err_0to1_arr[$i]} "
                        echo "$i ${optimal_n_arr[$i]} ${optimal_n_err_arr[$i]} ${optimal_n_threshold[$i]} ${optimal_n_err_1to0_arr[$i]} ${optimal_n_err_0to1_arr[$i]} " >> final_result_${ds}_histlen_${hist_len}_${uf}_${TH}_train_suite_${less_miss}_naive.txt
                        gcc -o parse parse_multiple_files_edited_error_calc_including_initial_bits.c
                        ./parse benchmark_train.txt misses_results_${ds}_train/misses_results_${uf} ${hist_len} misses_results_${ds}_test/misses_results_${uf} benchmark_test.txt 0 ${uf} ${optimal_n_arr[$i]} ${less_miss} > temp.txt 
                        #TODO What is the output file ?
                        string=min_error$i:
                        grep $string DS_${ds}_histlen_${hist_len}_${uf}_${optimal_n_arr[$i]}_both_suite_naive_${less_miss}.txt | tail -1
                        num_err_test=$(grep ${string} DS_${ds}_histlen_${hist_len}_${uf}_${optimal_n_arr[$i]}_both_suite_naive_${less_miss}.txt | tail -1 | awk '{ print $2 }' | awk -F ',' '{ print $1 }')
                        num_err_threshold_test=$(grep ${string} DS_${ds}_histlen_${hist_len}_${uf}_${optimal_n_arr[$i]}_both_suite_naive_${less_miss}.txt | tail -1 | awk '{ print $4 }' | awk -F ',' '{ print $1 }')
                        num_err_1to0_test=$(grep ${string} DS_${ds}_histlen_${hist_len}_${uf}_${optimal_n_arr[$i]}_both_suite_naive_${less_miss}.txt | tail -1 | awk '{ print $6 }' | awk -F ',' '{ print $1 }')
                        num_err_0to1_test=$(grep ${string} DS_${ds}_histlen_${hist_len}_${uf}_${optimal_n_arr[$i]}_both_suite_naive_${less_miss}.txt | tail -1 | awk '{ print $8 }' | awk -F ',' '{ print $1 }')

                        echo "$i test ${optimal_n_arr[$i]} ${num_err_test} ${num_err_threshold_test} ${num_err_1to0_test} ${num_err_0to1_test} "
                        echo "$i ${optimal_n_arr[$i]} ${num_err_test} ${num_err_threshold_test} ${num_err_1to0_test} ${num_err_0to1_test} " >> final_result_${ds}_histlen_${hist_len}_${uf}_${TH}_test_suite_${less_miss}_naive.txt
                        #exit
                    done
                done
            done
        done
    done
done
