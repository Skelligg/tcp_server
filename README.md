# Low-Latency Event-Driven TCP Server (Modern C++)

A Linux-first, low-latency TCP server implemented in modern C++, designed to explore networking internals, event-driven I/O, and systems-level correctness without hiding complexity behind frameworks.

This project is intentionally built close to the metal: raw sockets, binary protocols, and epoll, with a staged approach that mirrors real-world server evolution.

## Project Goals
### Functional Scope

- Accept N concurrent TCP connections

- Receive binary messages from clients

- Parse, process, and respond deterministically

- Maintain correct behavior under load

### Non-Functional Constraints (Deliberate)

- No frameworks

- No HTTP

- No JSON

- No Boost.Asio

- No TLS

- CMake-based build

- Linux-first (epoll)

- Explicit ownership and memory control

These constraints are intentional: they force understanding of syscalls, TCP behavior, and event-driven design, rather than hiding them behind abstractions.

## Current Status

✔ Stage 1 — Blocking TCP Server \
✔ Stage 2 — Binary protocol & framing \
✔ Stage 3 — Non-blocking I/O + epoll (in progress) 

❌ Multithreading (planned) \
❌ Instrumentation (planned)

The project is functional today and structured to evolve cleanly into a multi-threaded, production-style design.

## Architecture Overview
### Protocol

A minimal binary protocol with explicit framing:

```
struct ProtocolHeader {
    uint16_t type;
    uint16_t length;
};
```

Rules:

- Header is read first

- Body is read based on declared length

- Partial reads/writes are handled explicitly

- No assumptions about message boundaries

### I/O Model

- All sockets are non-blocking

- Central epoll event loop

- Per-connection state machines

- Explicit handling of:

    - EPOLLIN

    - EPOLLOUT

    - EPOLLERR

No threads are used at this stage, scalability comes from correct event-driven design.

## Build & Run
### Requirements

- Docker Compose (or native linux environment)

- C++20-capable compiler

- CMake ≥ 3.20

### Build
```
mkdir build
cd build
cmake ..
cmake --build .
```
### Run server
```
./tcp_server
```
### Run client
```
./tcp_client
```

## Testing Strategy

This project distinguishes between correctness testing and systems behavior testing.

### Unit Tests

- Protocol parsing

- Buffer handling

- Edge cases (partial reads, malformed input)

### Integration Tests

- Server launched as a real process

- Clients communicate over actual TCP sockets

- Validates end-to-end correctness

### Stress Tests

- External clients generate load

- No reuse of server internals

- Exercises:

    - epoll behavior

    - backpressure

    - socket lifecycle

    - protocol robustness

Stress tests intentionally behave like real clients, not shared code paths.

## Planned Work
### Stage 4 — Event Loop Refinement

- Move accept() into epoll-driven flow

- Improve connection lifecycle handling

- Cross-client message routing (optional)

### Stage 5 — Threading Model

One of:

- 1 acceptor + N worker event loops

- Connection sharding by hash

- Optional CPU pinning

Locks are avoided unless proven necessary.

### Stage 6 — Instrumentation

No frameworks:

- Active connection count

- Bytes in / out

- Latency histogram

- Backpressure detection

This enforces professional observability discipline.

## C++ Discipline
### Used

- RAII for sockets

- std::unique_ptr

- std::span

- constexpr

- enum class

- noexcept where appropriate

### Avoided

- Exceptions in hot paths

- Inheritance hierarchies

- shared_ptr without justification

- Heavy templates / metaprogramming

This mirrors real low-latency and infrastructure codebases.

## Why This Project Matters

This single project exercises:

- TCP internals

- Event-driven architecture

- Memory ownership & lifetimes

- Failure handling

- Performance-aware design

It maps directly to:

- High-performance backends

- Trading / simulation infrastructure

- Distributed systems

- Embedded or gateway software

- Networking-heavy C++ roles

## Design Philosophy

If I cannot explain what each syscall does, you are not ready to abstract it.

This repository prioritises clarity, correctness, and explicit design decisions over convenience.
