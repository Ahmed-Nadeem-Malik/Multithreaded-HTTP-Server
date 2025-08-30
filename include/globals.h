/**
 * @file globals.h
 * @brief Global state variable declarations and definitions
 */

#pragma once

#include <atomic>
#include <chrono>
#include <mutex>

// Global state declarations
extern std::atomic<int> request_counter;        // Thread-safe counter for total HTTP requests
extern std::mutex logging_mutex;                // Synchronizes console output from multiple threads
extern const std::chrono::system_clock::time_point server_start;  // Server startup time for uptime calculation
