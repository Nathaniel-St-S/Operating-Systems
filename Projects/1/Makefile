CC = gcc
CFLAGS = -Wall -O2 -Iinclude

SRC = src/main.c src/cpu.c src/isa.c src/memory.c src/interrupts.c
OBJS = $(SRC:.c=.o)

prog: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) prog

