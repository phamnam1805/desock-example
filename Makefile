CC ?= gcc
CFLAGS ?= -O2 -Wall -Wno-unused-result -Wextra -pedantic -std=c11
LDFLAGS ?=

BIN_DIR := bin
SERVER := $(BIN_DIR)/server
CLIENT := $(BIN_DIR)/client
SIMPLE_FUZZER := $(BIN_DIR)/simple_fuzzer

SRC_SERVER := server.c
SRC_CLIENT := client.c
SRC_SIMPLE_FUZZER := simple-fuzzer.c

.PHONY: all clean

all: $(SERVER) $(CLIENT) $(SIMPLE_FUZZER)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(SERVER): $(SRC_SERVER) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT): $(SRC_CLIENT) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SIMPLE_FUZZER): $(SRC_SIMPLE_FUZZER) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -Wno-implicit-function-declaration

clean:
	rm -rf $(BIN_DIR)
