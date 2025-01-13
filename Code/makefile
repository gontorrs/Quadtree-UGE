CC=gcc
CFLAGS=-Wall -g -Iinclude
SRCS=src/quad.c src/compress.c src/decode.c
OBJS=$(SRCS:.c=.o)

all: quadtree

quadtree: $(OBJS)
	$(CC) $(CFLAGS) -o quadtree $(OBJS) -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o quadtree *.qtc