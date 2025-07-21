#!/bin/bash
disk_image="/home/ugp/x86-ubuntu-24.04-img"  # path to operating system disk image
#kernel="/home/ugp/os3/linux-6.8/vmlinux"
kernel="/home/ugp/os4/linux-6.12.11/vmlinux"
#sudo ./build/X86/gem5.opt --debug-flags=cac configs/example/gem5_library/x86-ugp-fs.py --l2cache --image $disk_image --kernel $kernel
#sudo ./build/X86/gem5.opt  configs/example/gem5_library/x86-ugp-fs.py --image $disk_image --kernel $kernel
#sudo ./build/X86/gem5.opt configs/example/gem5_library/x86-ugp-fs-dynamic.py --X 1.2 --T 9
#sudo ./build/X86/gem5.opt configs/example/gem5_library/ugp-generic.py --exec pallav --X 1.2 --T 9 --file file16.txt
#sudo ./build/X86/gem5.opt  configs/example/gem5_library/x86-ugp-fs.py --image $disk_image --kernel $kernel --valu 5
#sudo ./build/X86/gem5.opt --debug-flags=invalidate configs/example/gem5_library/x86-ugp-fs.py --image $disk_image --kernel $kernel
#sudo ./build/X86/gem5.opt --debug-flags=cac configs/example/gem5_library/x86-ugp-fs.py --image $disk_image --kernel $kernel
sudo ./build/X86/gem5.opt --outdir=/home/ugp/gem5/results_gem5_lr/results_for_100_zeroes configs/example/gem5_library/x86-lre2e_all_0.py
#sudo ./build/X86/gem5.opt --outdir=/home/ugp/gem5/results_gem5_lr/results_for_100_zeroes configs/example/gem5_library/x86-ubuntu-run-with-kvm.py
