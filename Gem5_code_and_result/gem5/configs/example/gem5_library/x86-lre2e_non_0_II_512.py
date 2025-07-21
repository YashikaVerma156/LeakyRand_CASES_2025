# Copyright (c) 2021 The Regents of the University of California
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""

This script shows an example of running a full system Ubuntu boot simulation
using the gem5 library. This simulation boots Ubuntu 18.04 using 2 KVM CPU
cores. The simulation then switches to 2 Timing CPU cores before running an
echo statement.

Usage
-----

```
scons build/X86/gem5.opt
./build/X86/gem5.opt configs/example/gem5_library/x86-ubuntu-run-with-kvm.py
```
"""
import argparse
import sys
parser = argparse.ArgumentParser()
#parser.add_argument('--X', type=float, default=1, help="array access size")
#parser.add_argument('--T', type=int, default=1, help="iterations of access")
#parser.add_argument('--file', type=str, required=True, help="filename for MD5 hash")
args = parser.parse_args(sys.argv[1:])
#print(f"Using {args.valu} cores.")
from gem5.coherence_protocol import CoherenceProtocol
from gem5.components.boards.x86_board import X86Board
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.cpu_types import CPUTypes
from gem5.components.processors.simple_switchable_processor import (
    SimpleSwitchableProcessor,
)
from gem5.isas import ISA
#from gem5.resources.resource import obtain_resource
from gem5.simulate.exit_event import ExitEvent
from gem5.simulate.simulator import Simulator
from gem5.utils.requires import requires
from m5.objects import Terminal
from gem5.resources.resource import (
    DiskImageResource,
    KernelResource,
    Resource,
    obtain_resource,
)

# This runs a check to ensure the gem5 binary is compiled to X86 and to the
# MESI Two Level coherence protocol.
requires(
    isa_required=ISA.X86,
    coherence_protocol_required=CoherenceProtocol.MESI_TWO_LEVEL,
    kvm_required=True,
)

from gem5.components.cachehierarchies.ruby.mesi_two_level_cache_hierarchy import (
    MESITwoLevelCacheHierarchy,
)

# Here we setup a MESI Two Level Cache Hierarchy.
#'''
#cache_hierarchy = MESITwoLevelCacheHierarchy(
#    l1d_size="16KiB",
#    l1d_assoc=8,
#    l1i_size="16KiB",
#    l1i_assoc=8,
#    l2_size="256KiB",
#    l2_assoc=16,
#    num_l2_banks=1,
#)
#'''
from gem5.components.cachehierarchies.classic.private_l1_private_l2_shared_l3_cache_hierarchy import(
    PrivateL1PrivateL2L3CacheHierarchy,
)


#cache_hierarchy = PrivateL1PrivateL2L3CacheHierarchy(l1d_size="32kB",l1i_size="32kB",l2_size="256kB",l3_size="16MB")
cache_hierarchy = PrivateL1PrivateL2L3CacheHierarchy(l1d_size="32kB",l1i_size="32kB",l2_size="256kB",l3_size="512kB")
#cache_hierarchy = PrivateL1PrivateL2L3CacheHierarchy(l1d_size="32kB",l1i_size="32kB",l2_size="256kB",l3_size="1MB")
#cache_hierarchy = PrivateL1PrivateL2L3CacheHierarchy(l1d_size="32kB",l1i_size="32kB",l2_size="256kB",l3_size="16MB",l3_assoc=1)
# Setup the system memory.
memory = SingleChannelDDR3_1600(size="3GiB")

# Here we setup the processor. This is a special switchable processor in which
# a starting core type and a switch core type must be specified. Once a
# configuration is instantiated a user may call `processor.switch()` to switch
# from the starting core types to the switch core types. In this simulation
# we start with KVM cores to simulate the OS boot, then switch to the Timing
# cores for the command we wish to run after boot.
processor = SimpleSwitchableProcessor(
    starting_core_type=CPUTypes.KVM,
    switch_core_type=CPUTypes.TIMING,
   # switch_core_type=CPUTypes.O3,
    isa=ISA.X86,
    num_cores=4,
)

# Here we setup the board. The X86Board allows for Full-System X86 simulations.
board = X86Board(
    clk_freq="3GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)
#board.pc.com_1.device = Terminal()
#board.pc.com_1.device.file = "serial_output.txt"
#board.pc.com_1.device = Terminal(stdout="serial_output.txt")
command = (
        "echo 'Before calling m5 exit';\n"\
        + "ls /sys/bus/cpu/devices;\n" \
        + "sleep 20;\n"\
        + "m5 exit;\n" \
        + "ls /sys/bus/cpu/devices;\n" \
        # + "echo 'This is running on Timing CPU cores.';\n" \
        + "sleep 10;\n"\
        + f"./dynamic_lr_end2end_non_zero_II_512;\n" \
        #+ "./fill_and_probe;\n" \
        + "m5 exit;"
        )

#workload = obtain_resource("x86-ubuntu-24.04-boot-with-systemd")
#board.set_workload(workload)


def exit_event_handler():
    print("First exit: kernel booted")
    yield False  # gem5 is now executing systemd startup
    print("Second exit: Started `after_boot.sh` script")
    # The after_boot.sh script is executed after the kernel and systemd have
    # booted.
    # Here we switch the CPU type to Timing.
    print("Switching to Timing CPU")
    processor.switch()
    yield False  # gem5 is now executing the `after_boot.sh` script
    print("Third exit: Finished `after_boot.sh` script")
    # The after_boot.sh script will run a script if it is passed via
    # m5 readfile. This is the last exit event before the simulation exits.
    yield True


board.set_kernel_disk_workload(
    #kernel=KernelResource("/home/ugp/os4/linux-6.12.11/vmlinux"),
    #kernel=KernelResource("/home/ugp/x86-linux-kernel-6.8.0-35-generic"),
    kernel=KernelResource("/home/ugp/vmlinux-vanilla-new"),  #Kernel provided by Arun.
    disk_image=DiskImageResource("/home/ugp/x86-ubuntu-24.04-img",
    ),
    #kernel_args=["root=/dev/sda2", "console=ttyS0"],
    kernel_args=["root=/dev/hda2", "console=ttyS0"],  # Used for kernel provided by Arun.
    readfile_contents=command,
)

simulator = Simulator(
    board=board,
    on_exit_event={
        # Here we want override the default behavior for the first m5 exit
        # exit event.
        ExitEvent.EXIT: exit_event_handler()
    },
)

simulator.run()
