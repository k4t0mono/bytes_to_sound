# I'm just a Makefile

CC=clang
OL=0
FLAGS=-Wall -Wextra -O$(OL)
DEPS=configs.h encoder.h decoder.h
OBJS=main.o encoder.o decoder.o configs.o

%.o: %.c $(DEPS)
	$(CC) $(FLAGS) -c $< -o $@

bts.out: $(OBJS)
	$(CC) $(FLAGS) -g $^ -o $@


clean:
	$(RM) *.o bts.out
