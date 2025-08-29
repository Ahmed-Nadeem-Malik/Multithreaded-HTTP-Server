# MultithreadedServer

A high-performance, multithreaded HTTP server implementation in C++ that serves static files with proper MIME type support.

## Features

- **Multithreaded Architecture**: Uses a thread pool to handle multiple client connections concurrently
- **Static File Serving**: Serves HTML, CSS, and JavaScript files with proper MIME types
- **HTTP/1.1 Support**: Basic HTTP/1.1 protocol implementation
- **Thread-Safe Operations**: Thread-safe logging and request counting
- **Configurable**: Easy-to-modify configuration constants

## Architecture

The server is built with the following components:

- **Thread Pool**: Manages a pool of worker threads for handling client connections
- **Request Router**: Routes HTTP requests to appropriate handlers
- **File Server**: Reads and serves static files from the filesystem
- **Socket Management**: Handles network socket setup and client connections

## Supported Routes

- `GET /` - Serves `static/index.html`
- `GET /hello` - Serves `static/hello.html`  
- `GET /time` - Returns current server time
- `GET /css/style.css` - Serves CSS stylesheet
- `GET /js/script.js` - Serves JavaScript file

## Configuration

The server configuration can be modified in the `Config` namespace:

```cpp
namespace Config {
    constexpr const char* PORT = "8080";                    // Server port
    constexpr const char* SERVER_NAME = "MyHTTPServer/1.0"; // Server identification
    constexpr int BUFFER_SIZE = 256;                        // Client request buffer size
    constexpr int BACKLOG = 10;                             // Maximum pending connections
    constexpr int THREADS_NUM = 14;                         // Thread pool size
}
```

## Building

### Prerequisites

- C++11 compatible compiler (GCC, Clang, etc.)
- CMake (recommended) or make
- POSIX-compliant system (Linux, macOS, etc.)

### Using CMake

```bash
mkdir build
cd build
cmake ..
make
```

### Using g++ directly

```bash
g++ -std=c++11 -pthread -o MultithreadedServer src/main.cpp
```

## Running

1. Create a `static` directory in the project root
2. Add your HTML, CSS, and JavaScript files to the `static` directory
3. Run the server:

```bash
./MultithreadedServer
```

The server will start listening on port 8080 (or the configured port).

## Directory Structure

```
MultithreadedServer/
├── src/
│   └── main.cpp          # Main server implementation
├── static/               # Static files directory
│   ├── index.html        # Main page
│   ├── hello.html        # Hello page
│   ├── css/
│   │   └── style.css     # Stylesheet
│   └── js/
│       └── script.js     # JavaScript file
├── build/                # Build directory
└── README.md             # This file
```

## Usage Examples

### Basic Usage

```bash
# Start the server
./MultithreadedServer

# In another terminal, test with curl
curl http://localhost:8080/
curl http://localhost:8080/hello
curl http://localhost:8080/time
```

### Testing with Browser

Open your browser and navigate to:
- `http://localhost:8080/` - Main page
- `http://localhost:8080/hello` - Hello page
- `http://localhost:8080/time` - Current server time

## Performance

- **Concurrent Connections**: Handles multiple clients simultaneously using a thread pool
- **Thread Pool Size**: Default 14 worker threads (configurable)
- **Non-blocking**: Each client connection is handled in a separate thread
- **Memory Efficient**: Fixed buffer size with proper resource cleanup

## Error Handling

The server handles various error conditions:

- **File Not Found**: Returns 404 HTTP status
- **Method Not Allowed**: Returns 405 for non-GET requests
- **Socket Errors**: Logs errors and continues accepting connections
- **Thread Safety**: All logging operations are thread-safe

This is ApacheBench, Version 2.3 <$Revision: 1913912 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/
Benchmarking localhost (be patient)
Completed 2000 requests
Completed 4000 requests
Completed 6000 requests
Completed 8000 requests
Completed 10000 requests
Completed 12000 requests
Completed 14000 requests
Completed 16000 requests
Completed 18000 requests
Completed 20000 requests
Finished 20000 requests
Server Software:        MyHTTPServer/1.0
Server Hostname:        localhost
Server Port:            8080
Document Path:          /
Document Length:        391 bytes
Concurrency Level:      200
Time taken for tests:   0.512 seconds
Complete requests:      20000
Failed requests:        0
Total transferred:      9640000 bytes
HTML transferred:       7820000 bytes
Requests per second:    39039.47 [#/sec] (mean)
Time per request:       5.123 [ms] (mean)
Time per request:       0.026 [ms] (mean, across all concurrent requests)
Transfer rate:          18376.00 [Kbytes/sec] received
Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.3      0       5
Processing:     0    0   0.4      0       6
Waiting:        0    0   0.3      0       5
Total:          0    0   0.6      0       8
Percentage of the requests served within a certain time (ms)
  50%      0
  66%      0
  75%      0
  80%      0
  90%      0
  95%      0
  98%      1
  99%      5
 100%      8 (longest request)



## Author

Ahmed Nadeem Malik
