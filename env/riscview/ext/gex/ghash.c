/*  ghash.c 1.4 2018/09/11

    Copyright 2004-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered 
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
	Hash Table (untemplated functions)
    ===========================================================================
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <stddef.h>	/* for offsetof() */
#include "ghash.h"

/* ===========================================================================
 * Implement GeHash
 *
 * The Hash Table is implemented by a list of buckets (bucketList). Each
 * bucket is implemented by a single-linked list of hash entries.
 * The correct bucket is calculated by a modulo division of the
 * hash number by the bucketList length (nBuckets).  The bucketList
 * length is a prime number in order to get the load well distributed over
 * the buckets.  The initial bucketList length is zero, but can be preallocated
 * with ...Alloc. In any case, the bucketList length grows (in steps
 * of 4X) whenever the average bucket load exceeds 5 (see GeHashInsert).
 *
 * Each hash entry consists of a KEY and a VALUE.  The hash entries are
 * stored in GeHashMemory - the memory is managed by GeHashFreeMemory
 * and GeHashMoreMemory.
 * ===========================================================================
 */

struct GeHashMemory {
    struct GeHashMemory* next;
    double buf[1];		/* enforce double alignment on some machines */
};

/* ===========================================================================
 * GeHashGetPrime
 *
 * calculate a prime number equal to or greater then the given
 * size.  We use a fixed small set of primes with a distance
 * of about 2X between two of them.
 * ===========================================================================
 */

unsigned GeHashGetPrime(unsigned size) {
    static const unsigned primes[] = {
        9, 19, 37, 79, 157, 317, 631, 1279, 2557, 5119, 10223,
        20479, 40949, 81919, 163819, 327673, 655351, 1310719,
        2621603, 5243207, 10486417, 20972839, 41945689, 83891383,
        167782799, 335565611, 671131259, 1342262563,
        0
    };
    const unsigned* pp;
    for(pp=primes; *pp && *pp < size; pp++) {
        if (*pp == 0) { pp--; break; }  /* stop with the largest prime */
    }
    return *pp;
}


void GeHashFreeMemory(struct GeHashMemory* memoryBuffer)
{
    struct GeHashMemory*  memBlock;
    struct GeHashMemory*  next;

    for (memBlock = memoryBuffer; memBlock; memBlock = next) {
        next = memBlock->next;
        free(memBlock);
    }
}

struct GeHashEntry {
    struct GeHashEntry* next;
};

void* GeHashMoreMemory(struct GeHashMemory** memoryBufferPtr,
    int e_size, unsigned count)
{
    unsigned bufSize;			/* size of memory block */
    unsigned i;				/* loop counter */
    const unsigned headSize = offsetof(struct GeHashMemory,buf);
    struct GeHashMemory* memBlock;    /* the new memory block */
    char*		 startP;
    struct GeHashEntry*  entry;
    void*		   freeList = NULL;

    bufSize = e_size * count + headSize;

    /* don't allocate more then 64k and search for best size */
    if (bufSize > 0x10000) {
        bufSize = 0x10000/e_size*e_size + headSize;	/*LCOV_EXCL_LINE*/
    }

    memBlock = (struct GeHashMemory*)malloc(bufSize);
    count = (bufSize - headSize) / e_size;

    memBlock->next = *memoryBufferPtr;
    *memoryBufferPtr = memBlock;

    /* insert count entries into the freeList */
    freeList = NULL;
    startP = (char*)memBlock->buf;
    for(i=0; i < count; i++) {
        entry = (struct GeHashEntry*)(startP + i*e_size);
        entry->next = freeList;
        freeList = entry;
    }
    return freeList;
}
