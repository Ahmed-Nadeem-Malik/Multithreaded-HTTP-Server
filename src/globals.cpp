/**
 * @file globals.cpp
 * @brief Global state variable definitions
 */

#include "../include/globals.h"

#include <chrono>

std::atomic<int> request_counter{0};
std::mutex logging_mutex;
const std::chrono::system_clock::time_point server_start = std::chrono::system_clock::now();
