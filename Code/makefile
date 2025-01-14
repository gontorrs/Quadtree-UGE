CC=gcc
CFLAGS=-Wall -g -Iinclude
SRCS=src/quad.c src/compress.c src/main.c
OBJS=$(SRCS:.c=.o)

all: codec

codec: $(OBJS)
	$(CC) $(CFLAGS) -o codec $(OBJS) -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o codec *.qtc