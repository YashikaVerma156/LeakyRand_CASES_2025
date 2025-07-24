# LeakyRand evaluation on a Full-system simulator Gem5

## Setup
- Download Ubuntu disk image (x86-ubuntu-24.04-img) from Zenodo link( https://doi.org/10.5281/zenodo.16237834 ). Keep it in the same directory where Gem5 and vmlinux is i.e. in LeakyRand\_CASES\_2025/Gem5\_code\_and\_result.
- Ubuntu disk image already has all the compiled program binaries.

## Build Gem5
- prerequisites: python3.6 or above, gcc 10 or above, scons
- cd gem5
- scons ./build/X86/gem5.opt -j8

## How to run
- start\_leakyrand\_gem5\_exp.sh run the simulations for which the results are reported in section 5.3.
- $max\_parallel\_run\_count is currently set to 5. This parameter controls the number of Gem5 simulations that can be run in parallel. You can change it as per your machine.
- Use sudo ./start\_leakyrand\_gem5\_exp.sh ${total\_experiments} to run the simulations.

## Results Generation
- cd result\_generation\_scripts
- ./script.sh

## How to access compiled program binaries (not necessary for above steps).
- ./mount\_image.sh
- sudo su
- cd tmpmount/home/gem5
