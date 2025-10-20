# Makefile for desock shared library

CC = gcc
CFLAGS = -Wall -Wno-unused-parameter -Wextra -fPIC
LDFLAGS = -shared -ldl -lpthread

TARGET = desock.so
SRCS = desock.c logging.c
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
desock.o: desock.c logging.h
logging.o: logging.c logging.h
