/*
 * Copyright (c) 2012-2014, 2023-2024 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2003-2005,2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * Definitions of a conventional tag store.
 */

#include "mem/cache/tags/base_set_assoc.hh"

#include <string>

#include "base/intmath.hh"
#include "debug/invalidate.hh"
namespace gem5
{

BaseSetAssoc::BaseSetAssoc(const Params &p)
    :BaseTags(p), allocAssoc(p.assoc), blks(p.size / p.block_size),
     sequentialAccess(p.sequential_access),
     replacementPolicy(p.replacement_policy)
{
    // There must be a indexing policy
    fatal_if(!p.indexing_policy, "An indexing policy is required");

    // Check parameters
    if (blkSize < 4 || !isPowerOf2(blkSize)) {
        fatal("Block size must be at least 4 and a power of 2");
    }
}

void
BaseSetAssoc::tagsInit()
{
    // Initialize all blocks
    for (unsigned blk_index = 0; blk_index < numBlocks; blk_index++) {
        // Locate next cache block
        CacheBlk* blk = &blks[blk_index];

        // Link block to indexing policy
        indexingPolicy->setEntry(blk, blk_index);

        // Associate a data chunk to the block
        blk->data = &dataBlks[blkSize*blk_index];
 //  indexingPolicy->insertInvalid(blk);

        // Associate a replacement data entry to the block
        blk->replacementData = replacementPolicy->instantiateEntry();

        // This is not used as of now but we set it for security
        blk->registerTagExtractor(genTagExtractor(indexingPolicy));
    }
}

void
BaseSetAssoc::invalidate(CacheBlk *blk)
{
    
   int cach=1;
    if(allocAssoc==32768) cach=3;
    else if(allocAssoc==16) cach=2;
	// Notify partitioning policies of release of ownership
    if (partitionManager) {
        partitionManager->notifyRelease(blk->getPartitionId());
    }
   /* if(blk->getSet()==0&&allocAssoc==16)
    {
            for(int i=0; i<16; i++)
                 {
                         printf("%ld ", blks[i].getTag());
                 }
                 printf("\n");
                 printf("This is geting invalidated %ld\n",blk->getTag());
    }
*/
    unsigned long int tagss=blk->getTag();
    unsigned long int addr=regenerateBlkAddr(blk);
    BaseTags::invalidate(blk);

    // Decrease the number of tags in use
    stats.tagsInUse--;

    // Invalidate replacement data
    replacementPolicy->invalidate(blk->replacementData);
   if(cach==3) indexingPolicy->insertInvalid(blk);

    {
        DPRINTF(invalidate,"l%d cache: This is geting invalidated tag %#lx and address %#x with way %#lx\n",cach,tagss,addr,blk->getWay());
    }
}

void
BaseSetAssoc::moveBlock(CacheBlk *src_blk, CacheBlk *dest_blk)
{
    BaseTags::moveBlock(src_blk, dest_blk);

    // Since the blocks were using different replacement data pointers,
    // we must touch the replacement data of the new entry, and invalidate
    // the one that is being moved.
    if(src_blk->getSet()==0&&allocAssoc==16)
    {
            for(int i=0; i<16; i++)
                 {
                         printf("%ld ", blks[i].getTag());
                 }
                 printf("\n");
		 printf("%ld\n",src_blk->getTag());
    }

    replacementPolicy->invalidate(src_blk->replacementData);
    indexingPolicy->insertInvalid(src_blk);
    
    replacementPolicy->reset(dest_blk->replacementData);
}

} // namespace gem5
