CC = gcc
CFLAGS = -Wall -O3 -Iinclude

SRC = src/main.c src/cpu.c src/isa.c src/memory.c src/interrupts.c src/dma.c
OBJS = $(SRC:.c=.o)

prog: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) prog

