CXX = /opt/llvm/19.1.4/bin/clang++
CXXFLAGS = -Wall -Wextra -std=c++23 -O2
CXXFLAGSDBG = -Wall -Wextra -std=c++23 -O0

.PHONY: all clean

# Find all .cpp files and replace .cpp with .e
SOURCES = $(wildcard *.cpp)
EXECUTABLES = $(SOURCES:.cpp=.e)

all: $(EXECUTABLES)

%.e: %.cpp funclist.h
	$(CXX) $(CXXFLAGS) $< -o $@
	./$@

clean:
	rm -f *.e