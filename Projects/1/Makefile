CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -fsanitize=address -O2
OBJS = main.o mem.o cpu.o

prog: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c mem.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) prog

