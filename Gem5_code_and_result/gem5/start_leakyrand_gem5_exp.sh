#!/bin/bash

# Check if exactly one argument is passed
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <total_experiments_per_configuration>"
    echo "total_experiments_per_configuration should 100"
    exit 1
fi


tot_exp=$1

cd ../
disk_path=$(pwd)
disk_image="${disk_path}/x86-ubuntu-24.04-img"
kernel="${disk_path}/vmlinux-vanilla-new"

cd gem5
path=$(pwd)

benchmark="test"
max_parallel_run_count=5
stringlen=512
buffer=500

for ds in 1500 3000 4500
do

    if [[ $benchmark == "test" ]]; then
        total_count=$tot_exp
    elif [[ $benchmark == "train" ]]; then
        total_count=250     
    else
        echo "wrong benchmark type, exiting!"
        exit
    fi

    echo "$benchmark ${total_count}"

    mkdir -p $path/results_gem5_lr
    mkdir -p $path/results_gem5_lr/result_strlen_${stringlen}
    mkdir -p $path/results_gem5_lr/result_strlen_${stringlen}/result_${benchmark}_ds_${ds}_buffer_${buffer}

    parallel_run_count=0

    for (( i=1; i<=${total_count}; i++ ));
    do
        mkdir -p $path/results_gem5_lr/result_strlen_${stringlen}/result_${benchmark}_ds_${ds}_buffer_${buffer}/results_strnum_$i
        result_dir="$path/results_gem5_lr/result_strlen_${stringlen}/result_${benchmark}_ds_${ds}_buffer_${buffer}/results_strnum_$i"

	echo "$result_dir"
	sudo ./build/X86/gem5.opt --outdir=${result_dir} configs/example/gem5_library/x86-lre2e_test_suite_ds_${ds}_buffer_${buffer}_strlen_${stringlen}.py --T $i --kernel $kernel --disk ${disk_image} &
        	
        #exit
	parallel_run_count=`expr $parallel_run_count + 1`

	if [[ ${parallel_run_count} -eq ${max_parallel_run_count} ]]; then
	    wait
            parallel_run_count=0
        fi
    done
done
exit
