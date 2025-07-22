# LeakyRand evaluation on a Full-system simulator Gem5

## Setup
- Download Ubuntu disk image from Zenodo link(10.5281/zenodo.16237834). Keep it in the same path where Gem5 and vmlinux is.
- Ubuntu disk image already has all the compiled program binaries.
## Build Gem5
- pre-requisite: gcc 10 or above, scons
- cd gem5
- scons ./build/X86/gem5.opt -j8
## How to run
- 
## Results Generation
-
-
## How to access compiled program binaries (not necessary for above steps).
- ./mount\_image.sh
- sudo su
- cd tmpmount/home/gem5
