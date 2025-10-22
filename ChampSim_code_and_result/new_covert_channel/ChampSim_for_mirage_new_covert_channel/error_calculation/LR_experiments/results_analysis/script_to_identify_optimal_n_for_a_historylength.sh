gcc parse_multiple_files_edited_error_calc_including_initial_bits_single_suite.c

mkdir -p different_optimal_n_for_different_subspace

for less_miss in 0
do
    for ds in 5750 #3000 #5750 #1000 
    do
        for hist_len in 1 #0 1 2 3 4
        do
            mkdir -p different_optimal_n_for_different_subspace/DS_${ds}
            for uf in 64 #16 32 64 128
            do  
                optimal_n_arr=()
                optimal_n_err_arr=()
                optimal_n_threshold=()
                optimal_n_err_1to0_arr=()
                optimal_n_err_0to1_arr=()
                #TH_limit=`expr $uf - 60`
                TH_limit=17
                TH_start=9
                for (( TH=TH_start; TH<=TH_limit; TH++));
                do
                    echo "$less_miss $hist_len $ds $uf $TH"
                    ./a.out benchmark_train.txt misses_results_${ds}_train/misses_results_${uf} ${hist_len} ${uf} ${TH} ${less_miss} 0 > temp.txt
                    #./a.out benchmark_test.txt misses_results_${ds}_test/misses_results_${uf} ${hist_len} ${uf} ${TH} ${less_miss} 0 #> temp.txt
                    #exit
                    limit=$((2 ** hist_len))
                    #echo "limit= $limit"
                    #exit
                    for (( i =0; i<limit; i++));
                    do
                        string=min_error$i
                        #echo "$string"
                        num_err=$(grep -nr ${string} DS_${ds}_histlen_${hist_len}_${uf}_${TH}_single_train_suite_${less_miss}_naive.txt | awk '{ print $2 }' | awk -F ',' '{ print $1 }')
                        num_err_threshold=$(grep -nr ${string} DS_${ds}_histlen_${hist_len}_${uf}_${TH}_single_train_suite_${less_miss}_naive.txt | awk '{ print $4 }' | awk -F ',' '{ print $1 }')
                        num_err_1to0=$(grep -nr ${string} DS_${ds}_histlen_${hist_len}_${uf}_${TH}_single_train_suite_${less_miss}_naive.txt | awk '{ print $6 }' | awk -F ',' '{ print $1 }')
                        num_err_0to1=$(grep -nr ${string} DS_${ds}_histlen_${hist_len}_${uf}_${TH}_single_train_suite_${less_miss}_naive.txt | awk '{ print $8 }' | awk -F ',' '{ print $1 }')
                        #echo $num_err $num_err_threshold $num_err_1to0 $num_err_0to1
                        if [ $TH -eq $TH_start ];
                        then
                            optimal_n_arr[i]=$TH
                            optimal_n_err_arr[i]=$num_err
                            optimal_n_threshold[i]=$num_err_threshold
                            optimal_n_err_1to0_arr[i]=$num_err_1to0
                            optimal_n_err_0to1_arr[i]=$num_err_0to1
                        else
                            #echo " ${num_err}  ${optimal_n_err_arr[$i]}"
                            if [ ${num_err} -lt ${optimal_n_err_arr[$i]} ]; 
                            then
                                optimal_n_arr[i]=$TH
                                optimal_n_err_arr[i]=$num_err
                                optimal_n_threshold[i]=$num_err_threshold
                                optimal_n_err_1to0_arr[i]=$num_err_1to0
                                optimal_n_err_0to1_arr[i]=$num_err_0to1
                            fi
                        fi
                    done
                    mv DS_${ds}_histlen_${hist_len}_${uf}_${TH}_single_train_suite_${less_miss}_naive.txt different_optimal_n_for_different_subspace/DS_${ds}
                    #exit
                done
            done
            echo "For History_length: $hist_len"
            for (( i =0; i<limit; i++));
            do
                echo "$i ${optimal_n_arr[$i]} ${optimal_n_err_arr[$i]} ${optimal_n_threshold[$i]} ${optimal_n_err_1to0_arr[$i]} ${optimal_n_err_0to1_arr[$i]} "
            done
        done
    done
done

