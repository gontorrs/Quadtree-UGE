CC=gcc
CFLAGS=-Wall -g
OBJS=quad.o compress.o decode.o

all: quadtree

quadtree: $(OBJS)
	$(CC) $(CFLAGS) -o quadtree $(OBJS) -lm

quad.o: quad.c quad.h compress.h
	$(CC) $(CFLAGS) -c quad.c

compress.o: compress.c compress.h
	$(CC) $(CFLAGS) -c compress.c

decode.o: decode.c decode.h
	$(CC) $(CFLAGS) -c decode.c

clean:
	rm -f *.o quadtree