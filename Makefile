CC ?= gcc
CFLAGS ?= -O2 -Wall -Wno-unused-result -Wextra -pedantic -std=c11
LDFLAGS ?=

BIN_DIR := bin
SERVER := $(BIN_DIR)/server
CLIENT := $(BIN_DIR)/client

SRC_SERVER := server.c
SRC_CLIENT := client.c

.PHONY: all clean

all: $(SERVER) $(CLIENT)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(SERVER): $(SRC_SERVER) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT): $(SRC_CLIENT) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)
