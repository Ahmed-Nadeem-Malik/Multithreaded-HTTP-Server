# ==============================================================================
# MultithreadedServer - High-performance HTTP Server
# ==============================================================================
# 
# A multithreaded HTTP server built in C++ with the following features:
# - Thread pool for concurrent client handling  
# - HTTP/1.1 protocol support with keep-alive
# - Static file serving with proper MIME types
# - RESTful API endpoints (/time, /metrics, /echo)
# - Configurable server settings
#
# Requirements:
# - CMake 3.16 or later
# - C++11 compatible compiler (GCC 4.8+, Clang 3.3+, MSVC 2015+)
# - POSIX threads (pthread) on Linux
#
# Build Instructions:
#   mkdir build && cd build
#   cmake ..
#   make
#   ./MultithreadedServer
#
# ==============================================================================

cmake_minimum_required(VERSION 3.16)
project(MultithreadedServer 
        VERSION 1.0.0 
        DESCRIPTION "High-performance multithreaded HTTP server"
        LANGUAGES CXX)

# C++ Standard Configuration
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for LSP support (clangd, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Compiler Configuration
# ==============================================================================

# Debug build: Enable debugging symbols, disable optimization, strict warnings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -pedantic")
    message(STATUS "Debug build - enabling debug symbols and strict warnings")
# Release build: Maximum optimization, remove debug info
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
    message(STATUS "Release build - enabling maximum optimization")
endif()

# ==============================================================================
# Project Files
# ==============================================================================

# Source files - Core implementation
set(SOURCES
    src/main.cpp        # Application entry point
    src/http.cpp        # HTTP protocol handling
    src/routing.cpp     # URL routing and handlers
    src/threadpool.cpp  # Thread pool implementation
    src/server.cpp      # TCP server socket management
    src/globals.cpp     # Global state variables
)

# Header files - API declarations (included for IDE support)
set(HEADERS
    include/config.h      # Server configuration constants
    include/http.h        # HTTP utilities and request handling
    include/routing.h     # Route registration and mapping
    include/threadpool.h  # Thread pool class definition
    include/server.h      # Server socket functions
    include/globals.h     # Global variable declarations
)

# ==============================================================================
# Target Configuration
# ==============================================================================

# Create the main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Set include directories for LSP and IDE support
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Ensure C++11 standard compliance
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

# Platform-specific threading library linking
if(UNIX AND NOT APPLE)
    # Linux systems require explicit pthread linking
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
    message(STATUS "Linking pthread library for Linux")
elseif(APPLE)
    # macOS has threading built into the system libraries
    message(STATUS "Using system threading on macOS")
elseif(WIN32)
    # Windows threading is built into the runtime
    message(STATUS "Using system threading on Windows")
endif()

# ==============================================================================
# Build Information
# ==============================================================================

message(STATUS "=== MultithreadedServer Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Project version: ${PROJECT_VERSION}")
message(STATUS "Source files: ${SOURCES}")
message(STATUS "===============================================")
