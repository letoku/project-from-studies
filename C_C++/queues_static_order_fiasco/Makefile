CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -O2 -std=c++17

ARTIFACTS = main strqueue_test_2 strqueue_test_2_dbg_a strqueue_test_2_dbg_b strqueue_test_2_nodbg strqueue_test_1_dbg strqueue_test_1_nodbg

.PHONY: all clean

compilation:
	@g++ -c -Wall -Wextra -O2 -std=c++17 strqueue.cpp -o strqueue_dbg.o
	@g++ -c -Wall -Wextra -O2 -std=c++17 -DNDEBUG strqueue.cpp -o strqueue_nodbg.o
	@gcc -c -Wall -Wextra -O2 -std=c17 strqueue_test_1.c -o strqueue_test_1.o
	@g++ -c -Wall -Wextra -O2 -std=c++17 strqueue_test_2.cpp -o strqueue_test_2.o


test1: compilation
	g++ strqueue_test_1.o strqueue_dbg.o -o strqueue_test_1_dbg

test2: compilation
	g++ strqueue_test_1.o strqueue_nodbg.o -o strqueue_test_1_nodbg

test3: compilation
	g++ strqueue_test_2.o strqueue_dbg.o -o strqueue_test_2_dbg_a

test4: compilation
	g++ strqueue_dbg.o strqueue_test_2.o -o strqueue_test_2_dbg_b

test5: compilation
	g++ strqueue_test_2.o strqueue_nodbg.o -o strqueue_test_2_nodbg

clean:
	rm -rf *.o *.so *.e
	rm -f ${ARTIFACTS}