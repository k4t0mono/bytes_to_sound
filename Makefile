# I'm just a Makefile

CC=clang
FLAGS=-Wall -Wextra -O0

bts: main.o encoder.o
	$(CC) $(FLAGS) -g encoder.o main.o -o bts.out

main.o: main.c
	$(CC) $(FLAGS) -c main.c -o main.o

encoder.o: configs.h encoder.c
	$(CC) $(FLAGS) -c encoder.c -o encoder.o

clean:
	$(RM) *.o bts.out
