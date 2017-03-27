CC = gcc

CFLAGS = -c -Wall -std=c99 -D_BSD_SOURCE

all: udp_test_client

udp_test_client: main.o aux.o
	$(CC) main.o aux.o $(LDFLAGS) -o udp_test_client

main.o: main.c
	$(CC) $(CFLAGS) main.c

aux.o: aux.c
	$(CC) $(CFLAGS) aux.c

clean:
	rm -rf *o udp_test_client

