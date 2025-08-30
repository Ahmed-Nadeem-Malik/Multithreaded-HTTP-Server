/**
 * @file routing.cpp
 * @brief HTTP request routing and route registration
 */

#include "../include/routing.h"

#include <chrono>
#include <sstream>

#include "../include/globals.h"
#include "../include/http.h"

static RouteMap ROUTES;

RouteMap& get_routes() { return ROUTES; }

void add_route(const std::string& path, const std::string& method, Handler h) 
{ 
    ROUTES[path][method] = std::move(h); 
}

Handler static_file(const std::string& file_path, const std::string& content_type)
{
    // Return a lambda that serves the static file with specified content type
    return [file_path, content_type](const std::string&) { return create_file_response(file_path, content_type); };
}

void init_routes()
{
    // Static file routes
    add_route("/", "GET", static_file("../static/index.html", "text/html"));
    add_route("/hello", "GET", static_file("../static/hello.html", "text/html"));
    add_route("/css/style.css", "GET", static_file("../static/css/style.css", "text/css"));
    add_route("/js/script.js", "GET", static_file("../static/js/script.js", "application/javascript"));

    // Current server time
    add_route("/time", "GET",
              [](const std::string&) { return create_http_response("200 OK", "text/plain", get_current_time()); });

    // Performance metrics endpoint
    add_route("/metrics", "GET",
              [](const std::string&)
              {
                  // Calculate server uptime since startup
                  auto now = std::chrono::system_clock::now();
                  auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - server_start).count();
                  int total = request_counter.load();
                  double rps = uptime > 0 ? static_cast<double>(total) / uptime : 0.0;

                  // Format metrics as plain text
                  std::ostringstream os;
                  os << "total_requests " << total << "\n";
                  os << "uptime_seconds " << uptime << "\n";
                  os << "requests_per_second " << rps << "\n";
                  return create_http_response("200 OK", "text/plain", os.str());
              });

    // Echo POST data back to client
    add_route("/echo", "POST", [](const std::string& body)
              { return create_http_response("200 OK", "text/plain", "You posted:\n" + body); });
}
