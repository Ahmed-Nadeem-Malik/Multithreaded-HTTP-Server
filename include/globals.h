#pragma once

#include <atomic>
#include <mutex>
#include <chrono>

// Global state declarations
extern std::atomic<int> request_counter;
extern std::mutex logging_mutex;
extern const std::chrono::system_clock::time_point server_start;