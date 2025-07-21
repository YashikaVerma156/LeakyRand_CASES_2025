#!/bin/bash

X_values=(1.2)
T_values=(1 2 3 4 5 6 7 8 9)
num_repeats=5

max_parallel=4

running_jobs=0

for X in "${X_values[@]}"
do
    for T in "${T_values[@]}"
    do
        for i in $(seq 1 $num_repeats)
        do
            timestamp=$(date +%Y%m%d_%H%M%S)
            directory_name="dynamic_results_X${X}_T${T}_iter${i}_${timestamp}"

            echo "Launching experiment: X=$X, T=$T, Iteration $i -> Directory: $directory_name"
            mkdir -p "$directory_name"
            build/X86/gem5.opt -d "$directory_name" configs/example/gem5_library/x86-ugp-fs-dynamic.py --X "$X" --T "$T" &

            ((running_jobs++))

            # If max_parallel is reached, wait for any to finish
            if [[ "$running_jobs" -ge "$max_parallel" ]]; then
                wait -n  # Wait for any one job to finish
                ((running_jobs--))
            fi

            sleep 1  # Ensure unique timestamps
        done
    done
done

# Wait for all remaining background jobs
wait

echo "All experiments completed."
