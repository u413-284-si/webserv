# webserv

A lightweight HTTP server written in C++98.

## Overview

This project consists of building a functional HTTP server from scratch using C++98.
It mirrors the core behavior of real servers such as NGINX, following the constraints of non-blocking I/O, a single multiplexing mechanism, and accurate HTTP request/response handling.

Although inspired by HTTP/1.0–1.1, it implements only the subset of features required by the 42 subject.

✅ Completed with 110/100

Made with ❤️ by

- [u413-284-si](https://github.com/u413-284-si)
- [QCHR1581](https://github.com/QCHR1581)
- [gwolf-011235](https://github.com/gwolf-011235)

## Objectives

- Understand how HTTP works on a low level
- Implement a fully functional web server
- Handle multiple simultaneous clients using a single event loop
- Parse and serve dynamic configurations
- Support CGI execution
- Build robust, fault-tolerant backend systems

## Features

### Mandatory

**Core HTTP Handling**

- Full support for GET, POST, and DELETE
- Serving static files efficiently
- Directory handling:
  - Custom index files
  - Optional directory listing
- Custom error pages (with fallback defaults)
- Upload handling for POST requests
- Correct HTTP status codes
- Chunked request body decoding
- Simultaneous monitoring of read/write events
- Immediate handling of client disconnections
- Built-in protection against infinite waits or hanging requests

**Networking**

- Single epoll() loop for all sockets
- Strict non-blocking behavior
- Multiple listening ports defined by configuration
- Configurable request body size limit

**CGI Support**

- Execution based on file extension (e.g., .php, .py)
- Proper environment variable setup
- Unchunked request body passed to CGI
- CGI output handled according to HTTP rules
- EOF detection in absence of Content-Length

### Bonus Features

- Handle multiple CGI types

## Architecture Overview

The server follows a modular architecture built around four main responsibilities:

### 1. Event Loop

- A single non-blocking I/O loop manages:
  - Listening sockets
  - Client connections
  - CGI pipes
- Ensures the server never blocks and remains responsive under heavy load.

### 2. Request Parsing

- Incremental parsing based on readiness notifications
- Handles:
  - Request line
  - Headers
  - Chunked bodies
  - Body size limits
- Detects malformed requests and returns appropriate responses.

### 3. Response Generation

- Mapping URL → Route configuration
- Serving files via non-blocking reads
- Generating directory listings
- Dynamic response building for CGI output
- Accurate Content-Length or chunked encoding (based on implementation)

### 4. Configuration Layer

- Parses a configuration file inspired by NGINX
- Applies route-level rules including:
  - Redirections
  - Method limits
  - Root paths
  - Index files
  - Upload directories
  - Directory listing rules
  - CGI settings

## Building the Project

### Prerequisites

#### Using the Dev Container (Recommended)

This repository includes a dev container configuration (`.devcontainer/devcontainer.json`) that provides a complete development environment with all necessary dependencies pre-installed:

- C++ compiler (`gcc`, `g++`, `clang`)
- `make`, `cmake`
- Google Test (libgtest, libgmock)
- `bear` for generating compile commands
- `kcov` for code coverage
- `valgrind` for memory checking
- `perf` for profiling
- Python 3 with `pytest` for integration tests
- Build tools: `cppcheck`, `lldb`, `llvm`, `gdb`
- Git and standard utilities

To use the dev container, open the repository in VS Code with the "Dev Containers" extension installed, and it will automatically set up the environment.

#### Manual Setup

If not using the dev container, ensure the following are installed:

- A C++ compiler supporting at least C++98 and C++20 (tests use C++20). Typical: `g++`/`clang++`
- `make`
- Google Test (libgtest, libgmock) for unit tests and headers installed for linking
- `pytest` and Python dependencies used by integration tests
- Optional tools (used by Makefile targets): `kcov`, `bear`, `perf`, `valgrind`, `siege`

If you're missing optional tools, the Makefile checks for some of them and prints instructions when required.

### Quick Start

**Build and run the server**

1. Build the default server binary:

```bash
make all
```

This produces a binary named `webserv` in the repository root (unless `SANI=1` is set, see below).

2. Start the server (foreground):

```bash
./webserv config_files/example.conf
```

3. Start the server in background and capture PID:

```bash
./webserv config_files/example.conf > webserv.log 2>&1 & echo $! > webserv.pid
```

To stop the server when started this way:

```bash
kill `cat webserv.pid` && rm -f webserv.pid
```

Configuration: The default config file used by multiple Makefile targets is `config_files/example.conf`.

### Makefile Targets

The `Makefile` includes many convenience targets. Below is a summary of the most-used targets and how to use them.

- `make all` (default): compile the server binary (`webserv`).
- `make test`: compile and run the unit tests (links with `gtest`/`gmock`).
- `make test2`: build the server and run integration tests with `pytest`. This runs pytest with `-v -m "not timeout"` and passes `--server-executable` and `--config-file` to the test runner.
- `make test3`: run a load test using `siege`. Starts the server, runs siege using settings in `test/siege`, then kills the server.
- `make test4`: a benchmark-style siege run for a single URL.
- `make test5`: run the server under `valgrind` and drive requests using `test/requester/requester.py` (useful for leak checking); opens the valgrind log with `less` at the end.
- `make coverage`: generate a coverage report for unit tests using `kcov` (requires `kcov`). Output directory is `.vscode/coverage` by default.
- `make coverage2`: generate coverage from integration tests using `pytest` with kcov output (requires pytest kcov support and `kcov`).
- `make comp`: uses `bear` to create a `compile_commands.json` for language servers (runs `bear -- make -j4 --no-print-directory $(UNIT)`). Requires `bear`.
- `make profile`: run the program with `perf` and produce a perf.data file (requires `perf`).
- `make clean`: remove object and dependency files (cleans `obj/`).
- `make fclean`: performs `clean` and also removes binaries, logs, coverage directory, and other temporary files.
- `make re`: perform `fclean` then `all`.
- `make help`: print a short help summary (targets and variables).

### Build Variables

- `VERBOSE=1` — show all commands during the build
- `SANI=1` — enable Address/Undefined behavior sanitizers; builds alternate binary `webserv_sani`
- `NODEBUG=1` — compile without debug messages

Example build with sanitizers enabled:

```bash
make all SANI=1
# binary will be named `webserv_sani`
```

## Testing

### Unit Tests

Build and run the unit tests with:

```bash
make test
```

This target builds a test binary (named `unittest` by default) linked against `gtest`/`gmock` and runs it.

If you prefer to run tests manually from the generated binary:

```bash
./unittest
```

### Integration Tests

Integration tests are driven by `pytest` under `test/integration/`. The Makefile target `test2` will build the server and run the tests, passing the server executable and configuration file to the pytest suite.

```bash
make test2
```

You can also run pytest directly and point it to the server binary and config file used in the Makefile, for example:

```bash
pytest -v ./test/integration --server-executable=./webserv --config-file=./config_files/example.conf
```

Note: Python dependencies for the integration tests are not installed by the Makefile. Ensure `pytest` is available in your environment.

### Code Coverage

The Makefile includes two coverage helpers:

- `make coverage` — run `kcov` against the unit test binary and write results to `.vscode/coverage`.
- `make coverage2` — run integration tests under pytest and produce kcov-compatible output.

Example:

```bash
make coverage
# open .vscode/coverage/index.html in a browser to view report
```

`kcov` is required for these targets.

### Profiling and Memory Checking

- `make profile` — runs the server with `perf` to record samples; `perf` must be installed.
- `make test5` — runs the server under `valgrind` and executes `test/requester/requester.py` for workload; requires `valgrind`.

### Load Testing

Load testing uses `siege` with configuration files under `test/siege/`. To run the included siege scenarios:

```bash
make test3   # or `make test4` for a single-URL benchmark
```

`test3` and `test4` start the server for you, run `siege`, then stop the server.

## Development Tasks

- Generate `compile_commands.json` for editor tooling:

```bash
make comp
```

- Clean build artifacts:

```bash
make clean
make fclean
```

## Project Structure and Notes

### File Organization

- Logs are written under `log/` by default when some targets create them.
- The server serves files from the `html/` directory in integration-test scenarios; example content and directories are provided.
- The `cgi-bin/` folder contains several helper scripts used by integration tests (Python and shell scripts).

