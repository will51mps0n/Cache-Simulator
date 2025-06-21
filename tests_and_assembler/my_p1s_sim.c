#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Machine Definitions
#define MEMORYSIZE 65536 /* maximum number of words in memory (maximum number of lines in a given file)*/
#define NUMREGS 8        /*total number of machine registers [0,7]*/

// File Definitions
#define MAXLINELENGTH 1000 /* MAXLINELENGTH is the max number of characters we read */

static inline int convertNum(int32_t);
// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int32_t num)
{
    return num - ((num & (1 << 15)) ? 1 << 16 : 0);
}

typedef struct
    stateStruct
{
    int pc;
    int mem[MEMORYSIZE];
    int reg[NUMREGS];
    int numMemory;
} stateType;

extern void cache_init(int blockSize, int numSets, int blocksPerSet);
extern int cache_access(int addr, int write_flag, int write_data);
extern void printStats();
static stateType state;
static int num_mem_accesses = 0;
int mem_access(int addr, int write_flag, int write_data)
{
    ++num_mem_accesses;
    if (write_flag)
    {
        state.mem[addr] = write_data;
        if (state.numMemory <= addr)
        {
            state.numMemory = addr + 1;
        }
    }
    return state.mem[addr];
}
int get_num_mem_accesses()
{
    return num_mem_accesses;
}

void printState(stateType *);

// static inline int convertNum(int32_t);

int main(int argc, char **argv)
{
    char line[MAXLINELENGTH];
    FILE *filePtr;

    if (argc < 4)
    {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL)
    {
        printf("error: can't open file %s , please ensure you are providing the correct path", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; ++state.numMemory)
    {
        if (sscanf(line, "%d", state.mem + state.numMemory) != 1)
        {
            printf("error in reading address  %d\n. Please ensure you are providing a machine code file.", state.numMemory);
            perror("sscanf");
            exit(1);
        }
        //printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

stateType *statePtr = &state;
for (int i = 0; i < 8; i++)
{
    statePtr->reg[i] = 0;
}
    /*
    OPCODES:
    0 add & .fill
    1 nor
    2 lw
    3 sw
    4 beq
    5 jalr
    6 halt
    7 noop
    */

    state.pc = 0;
    int opcode = (state.mem[0] >> 22) & 0x7;
    bool halt = false;
    int instructionCount = 0;
    // assuming all are well written and have a halt

    cache_init(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

    while (!halt)
    {
        // print state and incrememnt needed variables
        instructionCount++;

        // set variables shared by all commands
        int instruction = cache_access(state.pc, 0, 0);
        opcode = (instruction >> 22) & 0x7; // Right shift 22 bits, then mask with 0x7 to get the last 3 bits;
        int A = (instruction >> 19) & 0x7;
        int B = (instruction >> 16) & 0x7;
        
        // have to declare these here to avoid uninit. variable warning
        int destination = 0;
        int offSet = 0;
        int memoryAddress = 0;

        // set destination or offset
        if (opcode == 0 || opcode == 1)
        { // add and nor use dest.
            // get just the last 3 bits of of regC
            destination = instruction & 0x7;
        }
        if (opcode > 1 && opcode < 4)
        { // lw sw bew use offset
            // offset mem can be 16 bits, get all:

            offSet = instruction & 0xffff;
            offSet = convertNum(offSet);
            // printf("%d\n",offSet);
            // lw or sw
            if (opcode == 2 || opcode == 3)
            {
                memoryAddress = state.reg[A] + offSet;
                // printf("%d\n",memoryAddress);
            }
        }
        if (opcode == 4)
        {
            destination = instruction & 0xffff;
            destination = convertNum(destination);
        }

        // switch cases: handle particular functionality for each opcode
        switch (opcode)
        {
        case 0: // add & .fill
            state.reg[destination] = state.reg[A] + state.reg[B];
            break;
        case 1: // nor
            // not sure if size of and * 8 are neccesssary
            state.reg[destination] = ~(state.reg[A] | state.reg[B]);
            break;
        case 2: // lw
            state.reg[B] = cache_access(memoryAddress, 0, state.reg[B]);
            break;
        case 3: // sw
            cache_access(memoryAddress, 1, state.reg[B]);
            break;
        case 4: // beq
            if (state.reg[A] == state.reg[B])
            { // if equal
                state.pc = state.pc + 1 + destination;
                continue;
            }
            break;
        case 5: // jalr
            state.reg[B] = state.pc + 1;
            state.pc = state.reg[A];
            continue;
        case 6:
            halt = true;
            state.pc++;
            printf("machine halted\n");
            printf("total of ");
            printf("%d", instructionCount);
            printf(" instructions executed\n");
            printf("final state of machine:\n");
            printState(&state);
            break;
        case 7: // noop
            break;
        }
        state.pc++;
    }
    printStats();
    return (0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++)
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++)
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    printf("end state\n");
}

// convert a 16-bit number into a 32-bit Linux integer
/*static inline int convertNum(int num)
{
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}*/

/*
 * Write any helper functions that you wish down here.
 */
