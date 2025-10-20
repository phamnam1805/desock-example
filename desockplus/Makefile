# Makefile for desockplus shared library

CC = gcc
CFLAGS = -Wall -Wno-unused-parameter -Wno-unused-but-set-variable -Wextra -fPIC
LDFLAGS = -shared -ldl -lpthread

TARGET = desockplus.so
SRCS = desockplus.c logging.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Dependencies
desockplus.o: desockplus.c logging.h
logging.o: logging.c logging.h
