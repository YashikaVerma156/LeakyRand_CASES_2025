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
import re
from m5.objects import (
    BadAddr,
    BaseXBar,
    Cache,
    L2XBar,
    Port,
    SystemXBar,
)

from ....isas import ISA
from ....utils.override import *
from ...boards.abstract_board import AbstractBoard
from ..abstract_cache_hierarchy import AbstractCacheHierarchy
from ..abstract_three_level_cache_hierarchy import (
    AbstractThreeLevelCacheHierarchy,
)
from ..abstract_two_level_cache_hierarchy import AbstractTwoLevelCacheHierarchy
from .abstract_classic_cache_hierarchy import AbstractClassicCacheHierarchy
from .caches.l1dcache import L1DCache
from .caches.l1icache import L1ICache
from .caches.l2cache import L2Cache
from .caches.l3cache import L3Cache
from .caches.mmu_cache import MMUCache


class PrivateL1PrivateL2L3CacheHierarchy(
    AbstractClassicCacheHierarchy, AbstractThreeLevelCacheHierarchy
):
    """
    A cache setup where each core has a private L1 Data and Instruction Cache,
    and a private L2 cache.
    """

    @staticmethod
    def _get_default_membus() -> SystemXBar:
        """
        A method used to obtain the default memory bus of 64 bit in width for
        the PrivateL1PrivateL2 CacheHierarchy.

        :returns: The default memory bus for the PrivateL1PrivateL2
                  CacheHierarchy.

        """
        membus = SystemXBar(width=64)
        membus.badaddr_responder = BadAddr()
        membus.default = membus.badaddr_responder.pio
        return membus

    def __init__(
        self,
        l1d_size: str,
        l1i_size: str,
        l2_size: str,
        l3_size: str,
        membus: BaseXBar = _get_default_membus.__func__(),
    ) -> None:
        """
        :param l1d_size: The size of the L1 Data Cache (e.g., "32kB").

        :param  l1i_size: The size of the L1 Instruction Cache (e.g., "32kB").

        :param l2_size: The size of the L2 Cache (e.g., "256kB").

        :param membus: The memory bus. This parameter is optional parameter and
                       will default to a 64 bit width SystemXBar is not specified.
        """

        AbstractClassicCacheHierarchy.__init__(self=self)
        AbstractThreeLevelCacheHierarchy.__init__(
            self,
            l1i_size=l1i_size,
            l1i_assoc=8,
            l1d_size=l1d_size,
            l1d_assoc=8,
            l2_size=l2_size,
            l2_assoc=16,
            #l2_assoc=4,
            l3_size=l3_size,
            l3_assoc=32768,
            #l3_assoc=65536,
            #l3_assoc=16,
        )

        self.membus = membus

    @overrides(AbstractClassicCacheHierarchy)
    def get_mem_side_port(self) -> Port:
        return self.membus.mem_side_ports

    @overrides(AbstractClassicCacheHierarchy)
    def get_cpu_side_port(self) -> Port:
        return self.membus.cpu_side_ports

    @overrides(AbstractCacheHierarchy)
    def incorporate_cache(self, board: AbstractBoard) -> None:
        # Set up the system port for functional access from the simulator.
        board.connect_system_port(self.membus.cpu_side_ports)

        for _, port in board.get_memory().get_mem_ports():
            self.membus.mem_side_ports = port

        self.l1icaches = [
            L1ICache(size=self._l1i_size)
            for i in range(board.get_processor().get_num_cores())
        ]
        self.l1dcaches = [
            L1DCache(size=self._l1d_size)
            for i in range(board.get_processor().get_num_cores())
        ]
        self.l2buses = [
            L2XBar() for i in range(board.get_processor().get_num_cores())
        ]
        self.l2caches = [
            L2Cache(size=self._l2_size)
            for i in range(board.get_processor().get_num_cores())
        ]
        # ITLB Page walk caches
        self.iptw_caches = [
            MMUCache(size="8KiB")
            for _ in range(board.get_processor().get_num_cores())
        ]
        # DTLB Page walk caches
        self.dptw_caches = [
            MMUCache(size="8KiB")
            for _ in range(board.get_processor().get_num_cores())
        ]

        if board.has_coherent_io():
            self._setup_io_cache(board)

        self.l3bus = L2XBar()
        #print("======================================l3_size:",self._l3_size)

        # Extract the numeric part (e.g., "256" of 256kB)
        num_match = re.match(r"(\d+)", self._l3_size) # check for digits in the beginning of the string.
        if not num_match:
            raise ValueError(f"Invalid size format: {size_str}")
    
        number = int(num_match.group(1))

        # Detect the cache size unit (case-insensitive)
        unit = self._l3_size.lower()
        if "mb" in unit:
            unit = "MB"
        elif "kb" in unit:
            unit = "kB"
        else:
            raise ValueError(f"Unknown unit in size string: {size_str}")

        # Previous code to calculate LLC size.
        #self.l3cache = L3Cache(
        #        size=str(int(self._l3_size[:1]) * (board.get_processor().get_num_cores()))+'MB',
        #    mshrs=64 * (board.get_processor().get_num_cores()),
        #)
        
        # New code to calculate LLC size.
        self.l3cache = L3Cache(
                size=str(number * (board.get_processor().get_num_cores()))+unit,
            mshrs=64 * (board.get_processor().get_num_cores()),
        )
        
        for i, cpu in enumerate(board.get_processor().get_cores()):
            cpu.connect_icache(self.l1icaches[i].cpu_side)
            cpu.connect_dcache(self.l1dcaches[i].cpu_side)

            self.l1icaches[i].mem_side = self.l2buses[i].cpu_side_ports
            self.l1dcaches[i].mem_side = self.l2buses[i].cpu_side_ports
            self.iptw_caches[i].mem_side = self.l2buses[i].cpu_side_ports
            self.dptw_caches[i].mem_side = self.l2buses[i].cpu_side_ports

            self.l2buses[i].mem_side_ports = self.l2caches[i].cpu_side

            self.l2caches[i].mem_side = self.l3bus.cpu_side_ports

            cpu.connect_walker_ports(
                self.iptw_caches[i].cpu_side, self.dptw_caches[i].cpu_side
            )

            if board.get_processor().get_isa() == ISA.X86:
                int_req_port = self.membus.mem_side_ports
                int_resp_port = self.membus.cpu_side_ports
                cpu.connect_interrupt(int_req_port, int_resp_port)
            else:
                cpu.connect_interrupt()
        self.l3bus.mem_side_ports = self.l3cache.cpu_side
        self.membus.cpu_side_ports = self.l3cache.mem_side

    def _setup_io_cache(self, board: AbstractBoard) -> None:
        """Create a cache for coherent I/O connections"""
        self.iocache = Cache(
            assoc=8,
            tag_latency=50,
            data_latency=50,
            response_latency=50,
            mshrs=20,
            size="1kB",
            tgts_per_mshr=12,
            addr_ranges=board.mem_ranges,
        )
        self.iocache.mem_side = self.membus.cpu_side_ports
        self.iocache.cpu_side = board.get_mem_side_coherent_io_port()
