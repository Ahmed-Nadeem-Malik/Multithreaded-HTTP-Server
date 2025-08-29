#include "../include/globals.h"

// Global state definitions
std::atomic<int> request_counter{0};
std::mutex logging_mutex;
const auto server_start = std::chrono::system_clock::now();