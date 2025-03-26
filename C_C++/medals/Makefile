# Define variables
COMPILER = g++
COMPILER_FLAGS = -Wall -Wextra -O2 -std=c++20
LD = ld
LD_FLAGS = --fatal-warnings -g

# Define targets
all: medals

medals:
	$(COMPILER) $(COMPILER_FLAGS) medals.cpp -o medals

run_again: clean medals
	./medals < test_1.in

clean:
	rm -fv medals

.PHONY: all clean