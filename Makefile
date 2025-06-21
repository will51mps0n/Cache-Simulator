# Makefile

# Compiler
CXX = gcc

# Compiler flags (including debug info)
CXXFLAGS = -std=c99 -Wall -Werror -g3
LINKFLAGS = -lm
# -std=c99 restricts us to using C and not C++
# -lm links with libm, which includes math.h (may be used in P4)
# -Wall and -Werror catch extra warnings as errors to decrease the chance of undefined behaviors on CAEN
# -g3 or -g includes debug info for gdb


# Compile Simulator with your 1S Simulator and Cache. Change my_p1s_sim.o to inst_p1s_sim.<system>.o if using ours
simulator: cache.c my_p1s_sim.o
	$(CXX) $(CXXFLAGS) $^ $(LINKFLAGS) -o $@

# Compile your 1S Simulator to link with Cache
my_p1s_sim.o: my_p1s_sim.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile Assembler
assembler: assembler.c
	$(CXX) $(CXXFLAGS) $< -o $@

# Compile any C program
%.exe: %.c
	$(CXX) $(CXXFLAGS) $< -o $@

# Assemble an LC2K file into Machine Code
%.mc: %.as assembler
	./assembler $< $@

# Assemble an LC2K file into Machine Code
%.mc: %.s assembler
	./assembler $< $@

# Assemble an LC2K file into Machine Code
%.mc: %.lc2k assembler
	./assembler $< $@

# Simulate a Machine Code program to a file
%.out: %.mc simulator
	./simulator $< $(wordlist 2, 4, $(subst ., ,$*)) > $@

# Compare output to a *.mc.correct or *.out.correct file
%.diff: % %.correct
	diff $^ > $@

# Compare output to a *.mc.correct or *.out.correct file with full output
%.sdiff: % %.correct
	sdiff $^ > $@

# Remove anything created by a makefile
clean:
	rm -f *.obj *.mc *.out *.exe *.diff *.sdiff assembler simulator simulator.o
