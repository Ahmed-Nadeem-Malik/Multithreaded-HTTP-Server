# MultithreadedServer

A high-performance HTTP server built from scratch in C++ with multithreading support. This project demonstrates systems programming concepts, concurrent design patterns, and network programming.

## Performance

I benchmarked the server using ApacheBench with 20,000 requests and 200 concurrent connections:

- **39,039 requests/second** - High throughput under load
- **0.026ms average response time** per concurrent request
- **Zero failed requests** - Reliable under stress
- **18.4 MB/sec transfer rate**
- **Sub-millisecond latency** - 99% of requests completed in under 5ms

## What I Built

### Core Technologies
- **C++11** with STL containers and modern language features
- **POSIX sockets** for low-level network programming
- **Custom thread pool** with synchronized task queues
- **CMake build system** for cross-platform compilation

### Key Features
- **Modular architecture** - Clean separation of headers and implementation
- **Thread pool implementation** - Efficient concurrent request handling
- **Dynamic routing system** - Easy to add new endpoints
- **Complete HTTP/1.1 support** - Proper request parsing with Content-Length handling
- **Robust request handling** - Multi-packet request assembly for reliable GET/POST support
- **Built-in metrics** - Real-time performance monitoring
- **Thread-safe operations** - Atomic counters and mutex-protected logging
- **Interactive web interface** - Test all endpoints directly from the browser

## System Architecture

The server uses a producer-consumer pattern where the main thread accepts connections and worker threads process requests:

```
┌─────────────────────────────────────────────────────────────────┐
│                    MultithreadedServer                          │
├─────────────────┬─────────────────┬─────────────────────────────┤
│   Socket Layer  │  Thread Pool    │     Routing Engine          │
│                 │                 │                             │
│ • POSIX Sockets │ • 14 Workers    │ • Dynamic Route Registration│
│ • IPv4/TCP      │ • Work Queue    │ • Lambda-based Handlers     │
│ • Accept Loop   │ • Condition Vars│ • Static & Dynamic Content  │
└─────────────────┴─────────────────┴─────────────────────────────┘
```

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET` | `/` | Main page |
| `GET` | `/hello` | Demo page |
| `GET` | `/time` | Current server time |
| `GET` | `/metrics` | Server performance stats |
| `GET` | `/css/style.css` | Stylesheet |
| `GET` | `/js/script.js` | JavaScript file |
| `POST` | `/echo` | Echo request body |

## Project Structure

I organized the code using standard C++ conventions:

```
MultithreadedServer/
├── include/              # Header files - Interface definitions
│   ├── config.h          # Server configuration constants
│   ├── globals.h         # Global state declarations
│   ├── http.h            # HTTP protocol handling
│   ├── routing.h         # Dynamic routing system
│   ├── server.h          # Network socket management
│   └── threadpool.h      # Thread pool implementation
├── src/                  # Source files - Implementation
│   ├── main.cpp          # Application entry point
│   ├── globals.cpp       # Global state definitions
│   ├── http.cpp          # HTTP utilities and request handling
│   ├── routing.cpp       # Route registration and dispatching
│   ├── server.cpp        # Socket setup and client handling
│   └── threadpool.cpp    # Worker thread management
├── static/               # Web content
│   ├── index.html        # Landing page
│   ├── hello.html        # Demo page
│   ├── css/style.css     # Styles
│   └── js/script.js      # Client-side code
└── CMakeLists.txt        # Build configuration
```

This structure separates interface definitions (headers) from implementation (source files), making the code easier to maintain and navigate.

## Building and Running

### Prerequisites
- C++11 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.16 or higher
- Unix-like system (Linux, macOS)

### Quick Start
```bash
git clone <your-repo-url>
cd MultithreadedServer

mkdir build && cd build
cmake ..
make

./MultithreadedServer
```

The server starts on port 8080. Visit `http://localhost:8080` in your browser.

### Testing Performance
```bash
# Basic functionality
curl http://localhost:8080/

# Load testing with ApacheBench
ab -n 20000 -c 200 http://localhost:8080/

# Check server metrics
curl http://localhost:8080/metrics
```

## Configuration

Server settings are centralized in `include/config.h`:

```cpp
namespace Config {
    constexpr const char* PORT = "8080";           // Server port
    constexpr int THREADS_NUM = 14;                // Worker threads
    constexpr int BUFFER_SIZE = 2048;              // Request buffer (2KB for POST support)
    constexpr int BACKLOG = 10;                    // Connection queue
}
```

## What I Learned

Building this server taught me about:

- **Low-level network programming** - Working directly with POSIX sockets and handling TCP packet boundaries
- **HTTP protocol implementation** - Parsing headers, handling Content-Length, and managing request/response cycles
- **Concurrent programming** - Thread synchronization and atomic operations
- **Network reliability** - Dealing with partial reads and multi-packet requests
- **Systems design** - Balancing performance with code maintainability
- **C++ best practices** - Modern language features and professional project organization
- **Performance optimization** - Achieving high throughput with efficient resource usage

## Technical Highlights

- **Zero external dependencies** - Built entirely with standard C++ and POSIX APIs
- **Robust HTTP handling** - Proper multi-packet request assembly and Content-Length parsing
- **Memory efficient** - 2KB buffer allocation with proper cleanup and boundary handling
- **Production patterns** - Thread pool, comprehensive error handling, graceful shutdown
- **Extensible design** - Easy to add new routes and features through lambda-based handlers
- **Interactive testing** - Built-in web interface for testing all endpoints
- **Industry-standard structure** - Professional C++ project organization with header/source separation

---

**Ahmed Nadeem Malik**  
*Systems Programming • C++ • Network Development*
