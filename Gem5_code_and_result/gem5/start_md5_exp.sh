#!/bin/bash

files=("file8.txt"  "file16.txt" "file32.txt" "file64.txt" "file128.txt" "file256.txt")
#files=("file8.txt"  "file16.txt")
num_repeats=5 #4
max_parallel=3

running_jobs=0

mkdir -p md5_hash_results

for i in $(seq 1 $num_repeats)
do
            for file in "${files[@]}"
            do
                directory_name="md5_results_${file}_wo_fl_iter${i}"

                mkdir -p "md5_hash_results/$directory_name"
                sudo ./build/X86/gem5.opt --outdir=/data/gem5_experiments_setup_ugp/gem5/md5_hash_results/${directory_name} configs/example/gem5_library/x86-fp-md5_wo_fl.py --file "$file" &

                ((running_jobs++))

                if [[ "$running_jobs" -ge "$max_parallel" ]]; then
                    wait
                    running_jobs=0
                fi
            done
done

wait

num_repeats=5 #4
max_parallel=3
running_jobs=0

mkdir -p md5_hash_results

for i in $(seq 1 $num_repeats)
do
            for file in "${files[@]}"
            do
                directory_name="md5_results_${file}_w_fl_iter${i}"

                mkdir -p "md5_hash_results/$directory_name"
                sudo ./build/X86/gem5.opt --outdir=/data/gem5_experiments_setup_ugp/gem5/md5_hash_results/${directory_name} configs/example/gem5_library/x86-fp-md5_w_fl.py --file "$file" &

                ((running_jobs++))

                if [[ "$running_jobs" -ge "$max_parallel" ]]; then
                    wait
                    running_jobs=0
                fi
            done
done
wait
echo "All experiments completed."

