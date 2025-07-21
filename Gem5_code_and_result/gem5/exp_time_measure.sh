#!/bin/bash

X_values=(1.2)
T_values=(9)
#sleep_times=(50000000 60000000 70000000 80000000 90000000 100000000)  # in milliseconds
num_repeats=10
max_parallel=4

running_jobs=0

for i in $(seq 1 $num_repeats)
do
    for X in "${X_values[@]}"
    do
        for T in "${T_values[@]}"
        do
            
            
                timestamp=$(date +%Y%m%d_%H%M%S)
                directory_name="time20Apr_iter${i}_${timestamp}"

                echo "Launching experiment: X=$X, T=$T,  Iteration $i -> Directory: $directory_name"
                mkdir -p "$directory_name"

                build/X86/gem5.opt -d "$directory_name" configs/example/gem5_library/x86-ugp-fs-generic.py --exec "time" --X "$X" --T "$T" &

                ((running_jobs++))

                if [[ "$running_jobs" -ge "$max_parallel" ]]; then
                    wait -n
                    ((running_jobs--))
                fi

                sleep 1  # Ensure unique timestamps
            done
        done
    done


wait
echo "All sleep experiments completed."

