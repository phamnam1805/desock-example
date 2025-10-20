# Desock Example

A toy example to explore desock from the [preeny](https://github.com/zardus/preeny) library.

## Structure

- `server.c` and `client.c`: Simple TCP socket chat between client and server
- `desock/`: Desock code from [preeny](https://github.com/zardus/preeny/blob/master/src/desock.c)
- `desockplus/`: Desockplus code from [GreenFuzz](https://github.com/behnamandarz/GreenFuzz/blob/main/GreenFuzz/preeny%2B/desockplus.c)

## Build

Compile server and client:
```bash
make
```

Compile desock:
```bash
cd desock
make
```

Compile desockplus:
```bash
cd desockplus
make
```

## Usage

Run normally:
```bash
# Terminal 1
./bin/server 5555

# Terminal 2
./bin/client 5555
```

Run client with desock:
```bash
LD_PRELOAD=./desock/desock.so ./bin/client 5555
```

Run client with desockplus:
```bash
LD_PRELOAD=./desockplus/desockplus.so ./bin/client 5555
```

## How Desock Works

When running the client with `LD_PRELOAD` desock, the client will **not** connect to the actual server. The client runs successfully even without starting the server. Desock hooks into and modifies the behavior of socket creation and connection functions, redirecting `write`/`read` operations to `stdin`/`stdout` instead.

### Race Condition Behavior

Since the client uses pthreads for sending and receiving messages, an interesting race condition occurs:

- **send_thread**: Uses `fgets` to read from stdin and `write` to send messages to the socket
- **recv_thread**: Uses `read` to receive messages from the socket and `printf` to output to stdout

With desock, `write` is redirected to stdout and `read` is redirected to stdin. When you type input into the terminal:

- **If send_thread wins the race**: Desock's `write` function will echo what you typed back to stdout (terminal)
- **If recv_thread wins the race**: The `read` function will read from stdin and `printf` it to the terminal with the "Server: " prefix

## Fuzzer (simple-fuzzer.c)

This repository includes a small driver `simple-fuzzer.c` that injects inputs into the client process by wiring a pipe to the child's stdin and setting `LD_PRELOAD=./desock/desock.so`.

- Behavior:
	- Parent (fuzzer) writes payloads into the pipe. Each payload contains an increasing counter so you can track inputs.
	- Child execs `./bin/client` with its stdin redirected from the pipe and `LD_PRELOAD` set to the desock library.
	- Because desock redirects socket read/write to stdin/stdout, the client will process the injected payloads as if they came from a server.

- Signals and shutdown:
	- `simple-fuzzer.c` installs a SIGINT handler in the parent so pressing Ctrl+C will stop sending, close the pipe (causing EOF on the child's stdin), and wait for the client to exit. This ensures a clean shutdown and a clear end-of-input.

- Build & run:
```bash
# build client/server/simple-fuzzer
make

# build desock (library)
cd desock && make && cd ..

# run the fuzzer (it will spawn the client with LD_PRELOAD)
./simple-fuzzer
```

You can stop the fuzzer with Ctrl+C; the child will receive EOF on stdin.