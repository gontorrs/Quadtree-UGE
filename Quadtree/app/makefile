# ======================
# app/Makefile
# ======================
CC        = gcc
CFLAGS    = -Wall -Wextra -O2 -I../libqtc/include
LDFLAGS   = -L../libqtc -lqtc -lm
TARGET    = codec
SRCS      = main.c
OBJS      = $(SRCS:.c=.o)

all: $(TARGET) libqtc.so

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

libqtc.so: ../libqtc/libqtc.so
	cp ../libqtc/libqtc.so .

clean:
	rm -f $(OBJS) $(TARGET) libqtc.so *.qtc
