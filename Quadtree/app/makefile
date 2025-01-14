# ======================
# app/Makefile
# ======================
CC        = gcc
CFLAGS    = -Wall -Wextra -O2 -I../libqtc/include
LDFLAGS   = -L../libqtc -lqtc -lm
TARGET    = codec
SRCS      = main.c
OBJS      = $(SRCS:.c=.o)

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET) *.qtc

