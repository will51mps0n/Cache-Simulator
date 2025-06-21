# Cache Simulator

This project implements a configurable cache simulator in C for a simulated architecture. It models a direct-mapped or set-associative cache with LRU replacement, dirty bit tracking, and write-back/write-allocate behavior.

---

## Features

- **Configurable parameters**:
  - Block size
  - Number of sets
  - Blocks per set
- **LRU Replacement Policy**
- **Write-back and write-allocate**
- **Integrated with simulated memory model**
- **Assembly support for test cases**

---

## Build Instructions

Run the following commands from the root directory:
make clean
make

This compiles:
The assembler (from assembler.c)
The simulator (from cache.c + my_p1s_sim.c)

## Running Tests
Use the included Bash script to compile and run test cases:
./runtests.sh

Each test will:
Assemble .as to .mc
Run the simulator with proper cache params
diff the output with correct output (*.txt)

## File Structure
cache.c: Core cache simulator (implements LRU, dirty/writeback)
my_p1s_sim.c: Custom 1-stage pipeline simulator 
assembler.c: Assembler to convert .as -> .mc
tests_and_assembler/: All test inputs and outputs

## Concepts Implemented
Address decomposition: tag, set index, block offset
Block fetching and eviction logic
LRU aging across accesses
Dirty block writebacks on eviction
Memory interface via mem_access and printAction

## Usage
./simulator <prog.mc> <blockSize> <numSets> <blocksPerSet>

## Example:
./simulator testLoop.2.4.5.mc 2 4 5

## Sample output: 
$$$ transferring word [24-27] from the memory to the cache
$$$ transferring word [25-25] from the cache to the processor

## Notes
Max cache size: 256 blocks
Max block size: 256 words
Supports both instruction and data caching
Correctness is verified using test .txt outputs