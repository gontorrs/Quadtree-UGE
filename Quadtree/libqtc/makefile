# ======================
# libqtc/Makefile
# ======================
CC        = gcc
CFLAGS    = -Wall -Wextra -O2 -fPIC -I./include
LDFLAGS   = -shared
TARGET    = libqtc.so

SRC_DIR   = src
OBJ_DIR   = obj
INCLUDE_DIR = include

SRCS      = $(wildcard $(SRC_DIR)/*.c)
OBJS      = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

