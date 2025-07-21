#!/bin/bash

X_values=(1.2)
T_values=(9)
files=("file8.txt"  "file12.txt" "file16.txt" "file20.txt")
num_repeats=4
max_parallel=4

running_jobs=0

for i in $(seq 1 $num_repeats)
do
    for X in "${X_values[@]}"
    do
        for T in "${T_values[@]}"
        do
            for file in "${files[@]}"
            do
                timestamp=$(date +%Y%m%d_%H%M%S)
                directory_name="Apr15_sleep_md5_results_X${X}_T${T}_${file}_iter${i}_${timestamp}"

                echo "Launching experiment: X=$X, T=$T, File=$file, Iteration $i -> Directory: $directory_name"
                mkdir -p "$directory_name"
                build/X86/gem5.opt -d "$directory_name" configs/example/gem5_library/x86-ugp-fs-wf_md5.py --X "$X" --T "$T" --file "$file" &

                ((running_jobs++))

                if [[ "$running_jobs" -ge "$max_parallel" ]]; then
                    wait -n
                    ((running_jobs--))
                fi

                sleep 1  # Ensure unique timestamps
            done
        done
    done
done

wait
echo "All experiments completed."

