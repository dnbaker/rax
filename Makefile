DEBUG?= -g -ggdb
CFLAGS?= -O2 -Wall -W -std=c99
LDFLAGS= -lm

CXX?=g++-9

# Uncomment the following two lines for coverage testing
#
# CFLAGS+=-fprofile-arcs -ftest-coverage
# LDFLAGS+=-lgcov

all: rax-test rax-oom-test

rax.o: rax.h
rax-test.o: rax.h
rax-oom-test.o: rax.h

rax-test: rax-test.o rax.o rc4rand.o crc16.o
	$(CC) -o $@ $^ $(LDFLAGS) $(DEBUG)

rax-oom-test: rax-oom-test.o rax.o
	$(CC) -o $@ $^ $(LDFLAGS) $(DEBUG)

raxpp: raxpp.cpp raxpp.h rax.o
	$(CXX) -std=c++14 -o $@ $< rax.o -O3 -fsanitize=undefined -fsanitize=address

.c.o:
	$(CC) -c $(CFLAGS) $(DEBUG) $<

clean:
	rm -f rax-test rax-oom-test *.gcda *.gcov *.gcno *.o
