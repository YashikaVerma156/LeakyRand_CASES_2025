#!/bin/bash

X_values=(1.2)
T_values=(9)
#sleep_times=(50 60 70 80 90 100)  # in milliseconds
sleep_times=(150 200)  # in milliseconds
num_repeats=4
max_parallel=8

running_jobs=0

for i in $(seq 1 $num_repeats)
do
    for X in "${X_values[@]}"
    do
        for T in "${T_values[@]}"
        do
            for sleep_ms in "${sleep_times[@]}"
            do
                timestamp=$(date +%Y%m%d_%H%M%S)
                directory_name="rdtsc16Apr_${sleep_ms}ms_iter${i}_${timestamp}"

                echo "Launching experiment: X=$X, T=$T, Sleep=${sleep_ms}ms, Iteration $i -> Directory: $directory_name"
                mkdir -p "$directory_name"

                build/X86/gem5.opt -d "$directory_name" configs/example/gem5_library/ugp-generic.py --exec "sleep_rdtsc" --X "$X" --T "$T" --time "$sleep_ms" &

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
echo "All sleep experiments completed."

