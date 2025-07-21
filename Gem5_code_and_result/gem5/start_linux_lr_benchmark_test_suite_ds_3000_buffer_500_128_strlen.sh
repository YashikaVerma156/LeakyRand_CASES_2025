#!/bin/bash
disk_image="/data/gem5_experiments_setup_ugp/x86-ubuntu-24.04-img"  # path to operating system disk image
#kernel="/home/ugp/os3/linux-6.8/vmlinux"
kernel="/data/gem5_experiments_setup_ugp/vmlinux-vanilla-new"

benchmark="test"
max_parallel_run_count=5
stringlen=128
ds=3000
buffer=500

if [[ $benchmark == "test" ]]; then
    total_count=50    #500
    echo "I am in if"
elif [[ $benchmark == "train" ]]; then
    total_count=4      #250
    echo "I am in else"
else
    echo "wrong benchmark type, exiting!"
    exit
fi

echo "$benchmark ${total_count}"

mkdir -p results_gem5_lr
mkdir -p results_gem5_lr/result_strlen_${stringlen}
mkdir -p results_gem5_lr/result_strlen_${stringlen}/result_${benchmark}_ds_${ds}_buffer_${buffer}

parallel_run_count=0

    for (( i=26; i<=${total_count}; i++ ));
    do
        mkdir -p results_gem5_lr/result_strlen_${stringlen}/result_${benchmark}_ds_${ds}_buffer_${buffer}/results_strnum_$i

	#sudo ./build/X86/gem5.opt --debug-flags=cac configs/example/gem5_library/x86-ugp-fs.py --l2cache --image $disk_image --kernel $kernel
	#sudo ./build/X86/gem5.opt  configs/example/gem5_library/x86-ugp-fs.py --image $disk_image --kernel $kernel
	#sudo ./build/X86/gem5.opt configs/example/gem5_library/x86-ugp-fs-dynamic.py --X 1.2 --T 9
	#sudo ./build/X86/gem5.opt configs/example/gem5_library/ugp-generic.py --exec pallav --X 1.2 --T 9 --file file16.txt
	#sudo ./build/X86/gem5.opt  configs/example/gem5_library/x86-ugp-fs.py --image $disk_image --kernel $kernel --valu 5
	#sudo ./build/X86/gem5.opt --debug-flags=invalidate configs/example/gem5_library/x86-ugp-fs.py --image $disk_image --kernel $kernel
	#sudo ./build/X86/gem5.opt --debug-flags=cac configs/example/gem5_library/x86-ugp-fs.py --image $disk_image --kernel $kernel
	sudo ./build/X86/gem5.opt --outdir=/data/gem5_experiments_setup_ugp/gem5/results_gem5_lr/result_strlen_${stringlen}/result_${benchmark}_ds_${ds}_buffer_${buffer}/results_strnum_$i configs/example/gem5_library/x86-lre2e_test_suite_ds_${ds}_buffer_${buffer}_strlen_${stringlen}.py --T $i &
        	
	#sudo ./build/X86/gem5.opt --outdir=/home/ugp/gem5/results_gem5_lr/results_for_100_zeroes configs/example/gem5_library/x86-ubuntu-run-with-kvm.py

	parallel_run_count=`expr $parallel_run_count + 1`

	if [[ ${parallel_run_count} -eq ${max_parallel_run_count} ]]; then
	    wait
            parallel_run_count=0
	fi
    done
exit
