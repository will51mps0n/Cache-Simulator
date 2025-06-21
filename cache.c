#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_CACHE_SIZE 256
#define MAX_BLOCK_SIZE 256

// Powers of 2 have exactly one 1 and the rest 0's, and 0 isn't a power of 2.
#define is_power_of_2(val) (val && !(val & (val - 1)))

/*
 * Accesses 1 word of memory.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads and 1 for writes.
 * write_data is a word, and is only valid if write_flag is 1.
 * If write flag is 1, mem_access does: state.mem[addr] = write_data.
 * The return of mem_access is state.mem[addr].
 */
extern int mem_access(int addr, int write_flag, int write_data);

/*
 * Returns the number of times mem_access has been called.
 */
extern int get_num_mem_accesses(void);

// Use this when calling printAction
enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

typedef struct blockStruct
{
    int data[MAX_BLOCK_SIZE];
    int dirty;
    int lruLabel;
    int tag;
    int valid;
} blockStruct;

typedef struct cacheStruct
{
    blockStruct blocks[MAX_CACHE_SIZE];
    int blockSize;
    int numSets;
    int blocksPerSet;
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;

void printAction(int, int, enum actionType);
void printCache(void);

/*
 * Set up the cache with given command line parameters. This is
 * called once in main().
 */
void cache_init(int blockSize, int numSets, int blocksPerSet)
{
    if (blockSize <= 0 || numSets <= 0 || blocksPerSet <= 0)
    {
        printf("error: input parameters must be positive numbers\n");
        exit(1);
    }
    if (blocksPerSet * numSets > MAX_CACHE_SIZE)
    {
        printf("error: cache must be no larger than %d blocks\n", MAX_CACHE_SIZE);
        exit(1);
    }
    if (blockSize > MAX_BLOCK_SIZE)
    {
        printf("error: blocks must be no larger than %d words\n", MAX_BLOCK_SIZE);
        exit(1);
    }
    if (!is_power_of_2(blockSize))
    {
        printf("warning: blockSize %d is not a power of 2\n", blockSize);
    }
    if (!is_power_of_2(numSets))
    {
        printf("warning: numSets %d is not a power of 2\n", numSets);
    }
    printf("Simulating a cache with %d total lines; each line has %d words\n",
           numSets * blocksPerSet, blockSize);
    printf("Each set in the cache contains %d lines; there are %d sets\n",
           blocksPerSet, numSets);

    cache.blockSize = blockSize;
    cache.numSets = numSets;
    cache.blocksPerSet = blocksPerSet;

    for (int i = 0; i < numSets; i++)
    {
        for (int j = 0; j < blocksPerSet; j++)
        {
            cache.blocks[i * blocksPerSet + j].valid = 0;
            cache.blocks[i * blocksPerSet + j].dirty = 0;    // Also initializing dirty to 0
            cache.blocks[i * blocksPerSet + j].lruLabel = 0; // Initial LRU label
        }
    }

    return;
}

/*
 * Access the cache. This is the main part of the project,
 * and should call printAction as is appropriate.
 * It should only call mem_access when absolutely necessary.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads (fetch/lw) and 1 for writes (sw).
 * write_data is a word, and is only valid if write_flag is 1.
 * The return of mem_access is undefined if write_flag is 1.
 * Thus the return of cache_access is undefined if write_flag is 1.
 */

typedef struct CurrentAddress
{
    int blockOffset;
    int setNum;
    int tag;
} CurrentAddress;

blockStruct *findCacheHit(int currAddr);
blockStruct *cacheMiss(int addr);
blockStruct *findLRU(int setNum);
void updateLRUs(blockStruct *block, int setNum);
int cacheToProcess(blockStruct *block, int addr);
void processToCache(blockStruct *block, int addr, int write_data);
void writeBackData(int addr, blockStruct *newBlock);

int getOffset(int addr);
int getSetNum(int addr);
int getTag(int addr);
int cache_access(int addr, int write_flag, int write_data)
{
    // Initialization for cache functionality

    // 0 is a cashe miss, 1 is a cache hit
    // first check : if data not in current cache:

    blockStruct *block = findCacheHit(addr);

    if (block == NULL)
    {
        block = cacheMiss(addr);
    }

    updateLRUs(block, getSetNum(addr));

    // processor to cache only for sw:
    if (write_flag == 1)
    {
        processToCache(block, addr, write_data);
        return 1;
    }
    else
    { // for lw and instr - this will be cache to processor
        return cacheToProcess(block, addr);
    }
    // for lw and sw there is cache to processor.
}

blockStruct *findCacheHit(int currAddr)
{
    for (int i = 0; i < cache.blocksPerSet; i++)
    {
        blockStruct *currentBlock = &cache.blocks[getSetNum(currAddr) * cache.blocksPerSet + i];
        if (currentBlock->valid && currentBlock->tag == getTag(currAddr))
        {
            return currentBlock;
        }
    }
    return NULL;
}

blockStruct *cacheMiss(int addr)
{
    blockStruct *newBlock = NULL;

    for (int i = 0; i < cache.blocksPerSet; i++)
    {
        if (cache.blocks[getSetNum(addr) * cache.blocksPerSet + i].valid == 0)
        {
            newBlock = &cache.blocks[getSetNum(addr) * cache.blocksPerSet + i];
            break;
        }
    }

    // we only writeback if we ARE evicting something
    if (newBlock == NULL)
    {
        newBlock = findLRU(getSetNum(addr));
        writeBackData(addr, newBlock);
    }

    if (!newBlock)
    {
        fprintf(stderr, "Error: No block found for replacement.\n");
        exit(1);
    }

    // Load data from memory
    for (int i = 0; i < cache.blockSize; i++)
    {
        newBlock->data[i] = mem_access(addr - (addr % cache.blockSize) + i, 0, 0);
    }
    newBlock->valid = 1;
    newBlock->dirty = 0;
    newBlock->tag = getTag(addr);

    printAction(addr - (addr % cache.blockSize), cache.blockSize, memoryToCache); // Log action

    return newBlock;
}

void writeBackData(int addr, blockStruct *newBlock)
{
    int setNum = getSetNum(addr);
    // Compute the starting address of the block being replaced:
    int blockStartingAddress = (newBlock->tag << (int)(log2(cache.numSets) + log2(cache.blockSize))) + (setNum << (int)log2(cache.blockSize));

    // if we are evicting something:
    if (newBlock->dirty)
    {
        for (int i = 0; i < cache.blockSize; i++)
        {
            int memAddr = blockStartingAddress + i;
            mem_access(memAddr, 1, newBlock->data[i]); // Write back if dirty
        }
        printAction(blockStartingAddress, cache.blockSize, cacheToMemory);
    }
    else
    {

        printAction(blockStartingAddress, cache.blockSize, cacheToNowhere);
    }
}
void processToCache(blockStruct *block, int addr, int write_data)
{
    block->valid = 1;
    block->dirty = 1; // Mark the block as dirty
    block->data[getOffset(addr)] = write_data;
    printAction(addr, 1, processorToCache);
}

int cacheToProcess(blockStruct *block, int addr)
{
    printAction(addr, 1, cacheToProcessor);
    return block->data[getOffset(addr)];
}

void updateLRUs(blockStruct *block, int setNum)
{

    int changedBlockLru = block->lruLabel;
    for (int i = 0; i < cache.blocksPerSet; i++)
    {
        if (cache.blocks[setNum * cache.blocksPerSet + i].lruLabel <= changedBlockLru)
        {
            cache.blocks[setNum * cache.blocksPerSet + i].lruLabel++;
        }
    }
    // 0 is the MRU
    block->lruLabel = 0;
}

blockStruct *findLRU(int setNum)
{
    int lruIndex = -1;
    int highestLRU = -1; // use the highest LRU to find the least recently used.
    for (int i = 0; i < cache.blocksPerSet; i++)
    {
        if (cache.blocks[setNum * cache.blocksPerSet + i].lruLabel > highestLRU)
        {
            highestLRU = cache.blocks[setNum * cache.blocksPerSet + i].lruLabel;
            lruIndex = i;
        }
    }
    return &cache.blocks[setNum * cache.blocksPerSet + lruIndex];
}

int getOffset(int addr)
{
    int offsetBits = (int)log2(cache.blockSize);

    int offsetMask = (1 << offsetBits) - 1;

    return addr & offsetMask;
}

int getSetNum(int addr)
{
    int offsetBits = (int)log2(cache.blockSize);
    int indexBits = (int)log2(cache.numSets);

    int indexMask = ((1 << indexBits) - 1) << offsetBits;

    return (addr & indexMask) >> offsetBits;
}

int getTag(int addr)
{
    int offsetBits = (int)log2(cache.blockSize);
    int indexBits = (int)log2(cache.numSets);

    return addr >> (offsetBits + indexBits);
}

/*
 * print end of run statistics 
 */

void printStats(void)
{
    return;
}

/*
 * Log the specifics of each cache action.
 *  -    cacheToProcessor: reading data from the cache to the processor
 *  -    processorToCache: writing data from the processor to the cache
 *  -    memoryToCache: reading data from the memory to the cache
 *  -    cacheToMemory: evicting cache data and writing it to the memory
 *  -    cacheToNowhere: evicting cache data and throwing it away
 */
void printAction(int address, int size, enum actionType type)
{
    printf("$$$ transferring word [%d-%d] ", address, address + size - 1);

    if (type == cacheToProcessor)
    {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache)
    {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache)
    {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory)
    {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere)
    {
        printf("from the cache to nowhere\n");
    }
    else
    {
        printf("Error: unrecognized action\n");
        exit(1);
    }
}

/*
 * Prints the cache based on the configurations of the struct
 */
void printCache(void)
{
    printf("\ncache:\n");
    for (int set = 0; set < cache.numSets; ++set)
    {
        printf("\tset %i:\n", set);
        for (int block = 0; block < cache.blocksPerSet; ++block)
        {
            printf("\t\t[ %i ]: {", block);
            for (int index = 0; index < cache.blockSize; ++index)
            {
                printf(" %i", cache.blocks[set * cache.blocksPerSet + block].data[index]);
            }
            printf(" }\n");
        }
    }
    printf("end cache\n");
}
