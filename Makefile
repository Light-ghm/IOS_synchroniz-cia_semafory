CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic -pthread

proj2:proj2.o
	$(CC) $(CFLAGS) -o proj2 proj2.o

proj2.o: proj2.c
	$(CC) $(CFLAGS) -c proj2.c

clean:
	rm -f *.o *.out proj2
