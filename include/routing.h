/**
 * @file routing.h
 * @brief HTTP route registration and handling system
 */

#pragma once

#include <functional>
#include <string>
#include <unordered_map>

using Handler = std::function<std::string(const std::string& body)>;
using RouteMap = std::unordered_map<std::string, std::unordered_map<std::string, Handler>>;

/**
 * Registers a route handler for a specific path and HTTP method
 * @param path URL path (e.g., "/api/users")
 * @param method HTTP method (e.g., "GET", "POST")
 * @param h Handler function to process requests
 */
void add_route(const std::string& path, const std::string& method, Handler h);

/**
 * Creates a handler that serves static files
 * @param file_path Path to the static file
 * @param content_type MIME type for the file
 * @return Handler function that serves the specified file
 */
Handler static_file(const std::string& file_path, const std::string& content_type);

/**
 * Gets reference to the global route map
 * @return Reference to the route map containing all registered routes
 */
RouteMap& get_routes();

/**
 * Sets up all application routes (static files, /time, /metrics, /echo)
 */
void init_routes();
