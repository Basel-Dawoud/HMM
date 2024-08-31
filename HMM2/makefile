CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared

# Targets
all: libhmm.so test_hmm

libhmm.so: HMM.o
	$(CC) $(LDFLAGS) -o $@ $^

HMM.o: HMM.c heap.h
	$(CC) $(CFLAGS) -c HMM.c

test_hmm: test_hmm.c libhmm.so
	$(CC) -o $@ $^ -ldl

clean:
	rm -f *.o *.so test_hmm
