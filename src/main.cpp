/**
 * MultithreadedServer - A high-performance HTTP server implementation
 *
 * This server provides:
 * - Multithreaded request handling using a thread pool
 * - Static file serving with MIME type support
 * - Basic HTTP/1.1 protocol implementation
 * - Thread-safe logging and request counting
 *
 * Author: Ahmed
 * Version: 1.0
 */

// =============================================================================
// SYSTEM INCLUDES
// =============================================================================
#include <arpa/inet.h>   // Network address conversion
#include <netdb.h>       // Network database operations
#include <netinet/in.h>  // Internet address family
#include <sys/socket.h>  // Socket interface
#include <sys/types.h>   // Data types
#include <unistd.h>      // POSIX operating system API

// =============================================================================
// STANDARD LIBRARY INCLUDES
// =============================================================================
#include <atomic>              // Atomic operations
#include <chrono>              // Time utilities
#include <condition_variable>  // Thread synchronization
#include <cstdlib>             // General utilities
#include <cstring>             // String operations
#include <fstream>             // File stream operations
#include <iostream>            // Input/output stream
#include <mutex>               // Mutual exclusion
#include <queue>               // Queue container
#include <sstream>             // String stream
#include <stdexcept>           // Exception classes
#include <string>              // String class
#include <thread>              // Thread support library
#include <vector>              // Vector container

// =============================================================================
// CONFIGURATION CONSTANTS
// =============================================================================
namespace Config
{
constexpr const char* PORT = "8080";                     // Server port
constexpr const char* SERVER_NAME = "MyHTTPServer/1.0";  // Server identification
constexpr int BUFFER_SIZE = 256;                         // Client request buffer size
constexpr int BACKLOG = 10;                              // Maximum pending connections
constexpr int THREADS_NUM = 14;                          // Thread pool size
}  // namespace Config

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================
std::atomic<int> request_counter{0};  // Thread-safe request counter
std::mutex logging_mutex;             // Mutex for thread-safe logging

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Reads the entire content of a file into a string
 *
 * @param file_path Path to the file to read
 * @return File content as string, empty string if file cannot be opened
 */
std::string read_file_content(const std::string& file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * Creates a properly formatted HTTP response
 *
 * @param status HTTP status line (e.g., "200 OK", "404 Not Found")
 * @param content_type MIME type of the content (e.g., "text/html", "text/css")
 * @param content Response body content
 * @return Complete HTTP response string with headers and body
 */
std::string create_http_response(const std::string& status, const std::string& content_type, const std::string& content)
{
    return "HTTP/1.1 " + status +
           "\r\n"
           "Content-Length: " +
           std::to_string(content.size()) +
           "\r\n"
           "Server: " +
           std::string(Config::SERVER_NAME) +
           "\r\n"
           "Content-Type: " +
           content_type +
           "\r\n"
           "\r\n" +
           content;
}

/**
 * Creates an HTTP response by reading a file from disk
 *
 * @param file_path Path to the file to serve
 * @param content_type MIME type for the file
 * @return HTTP response with file content or 404 if file not found
 */
std::string create_file_response(const std::string& file_path, const std::string& content_type)
{
    std::string content = read_file_content(file_path);

    if (content.empty())
    {
        return create_http_response("404 Not Found", "text/html", "File not Found");
    }

    return create_http_response("200 OK", content_type, content);
}

/**
 * Gets the current system time as a formatted string
 *
 * @return Current time as a string (removes trailing newline from ctime)
 */
std::string get_current_time()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::string time_str = std::ctime(&time_t);
    time_str.pop_back();  // Remove trailing newline character
    return time_str;
}

// =============================================================================
// HTTP REQUEST HANDLING
// =============================================================================

/**
 * Processes an HTTP request and generates appropriate response
 *
 * Supports:
 * - GET / : Serves index.html
 * - GET /hello : Serves hello.html
 * - GET /time : Returns current server time
 * - GET /css/style.css : Serves CSS stylesheet
 * - GET /js/script.js : Serves JavaScript file
 *
 * @param request Raw HTTP request string from client
 * @return Formatted HTTP response string
 */
std::string handle_http_request(const std::string& request)
{
    ++request_counter;  // Increment request counter atomically

    std::istringstream iss(request);
    std::string method, path;
    iss >> method >> path;

    // Only support GET method
    if (method != "GET")
    {
        return create_http_response("405 Method Not Allowed", "text/html", "Method Not Allowed");
    }

    // Route handling
    if (path == "/")
    {
        return create_file_response("../static/index.html", "text/html");
    }
    else if (path == "/hello")
    {
        return create_file_response("../static/hello.html", "text/html");
    }
    else if (path == "/time")
    {
        return create_http_response("200 OK", "text/html", get_current_time());
    }
    else if (path == "/css/style.css")
    {
        return create_file_response("../static/css/style.css", "text/css");
    }
    else if (path == "/js/script.js")
    {
        return create_file_response("../static/js/script.js", "application/javascript");
    }
    else
    {
        return create_http_response("404 Not Found", "text/html", "Not Found");
    }
}

// =============================================================================
// CLIENT CONNECTION HANDLING
// =============================================================================

/**
 * Handles a single client connection in a thread-safe manner
 *
 * This function:
 * 1. Receives HTTP request from client
 * 2. Processes the request and generates response
 * 3. Sends response back to client
 * 4. Closes the connection
 *
 * @param client_fd File descriptor for the client socket
 */
void handle_client(int client_fd)
{
    char buffer[Config::BUFFER_SIZE];

    // Log client connection (thread-safe)
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cout << "Client connected" << std::endl;
    }

    // Receive data from client
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == -1)
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cerr << "Receive error" << std::endl;
        close(client_fd);
        return;
    }
    else if (bytes_received == 0)
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cout << "Client disconnected" << std::endl;
        close(client_fd);
        return;
    }

    // Process the request
    buffer[bytes_received] = '\0';  // Null-terminate the received data
    std::string request(buffer);
    std::string response = handle_http_request(request);

    // Send response to client
    int bytes_sent = send(client_fd, response.c_str(), response.length(), 0);
    if (bytes_sent == -1)
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cerr << "Send error" << std::endl;
    }
    else
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cout << "Response sent to client" << std::endl;
    }

    // Clean up connection
    close(client_fd);
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cout << "Client connection closed\n" << std::endl;
    }
}

// =============================================================================
// NETWORK SETUP FUNCTIONS
// =============================================================================

/**
 * Sets up and configures the server socket for accepting connections
 *
 * This function:
 * 1. Creates socket using getaddrinfo for IPv4 TCP
 * 2. Binds socket to specified port
 * 3. Sets socket to listening mode
 *
 * @return Server socket file descriptor
 * @throws std::runtime_error on any socket setup failure
 */
int setup_server_socket()
{
    struct addrinfo hints, *server_info;

    // Configure socket hints for IPv4 TCP socket
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_flags = AI_PASSIVE;      // Use local IP

    // Get address information for the specified port
    int status = getaddrinfo(NULL, Config::PORT, &hints, &server_info);
    if (status != 0)
    {
        throw std::runtime_error("getaddrinfo failed: " + std::string(gai_strerror(status)));
    }

    // Create socket
    int server_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_fd == -1)
    {
        freeaddrinfo(server_info);
        throw std::runtime_error("Socket creation failed");
    }

    // Bind socket to address and port
    if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        close(server_fd);
        freeaddrinfo(server_info);
        throw std::runtime_error("Bind failed");
    }

    // Start listening for connections
    if (listen(server_fd, Config::BACKLOG) == -1)
    {
        close(server_fd);
        freeaddrinfo(server_info);
        throw std::runtime_error("Listen failed");
    }

    freeaddrinfo(server_info);
    return server_fd;
}

// =============================================================================
// THREAD POOL IMPLEMENTATION
// =============================================================================

/**
 * Thread pool class for handling multiple client connections concurrently
 *
 * This class maintains a pool of worker threads that process client connections
 * from a shared queue. It provides thread-safe job submission and automatic
 * cleanup on destruction.
 */
class ThreadPool
{
   private:
    std::vector<std::thread> workers_;  // Pool of worker threads
    std::queue<int> client_queue_;      // Queue of client file descriptors
    std::mutex queue_mutex_;            // Mutex for queue access
    std::condition_variable queue_cv_;  // Condition variable for queue notifications
    std::atomic<bool> stop_{false};     // Flag to signal threads to stop

    /**
     * Worker thread function - continuously processes clients from the queue
     * Each worker thread runs this function in a loop until stop is signaled
     */
    void worker()
    {
        while (true)
        {
            int client_fd;
            {
                // Wait for work or stop signal
                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cv_.wait(lock, [this] { return stop_.load() || !client_queue_.empty(); });

                // Exit if stopping and no more work
                if (stop_.load() && client_queue_.empty())
                {
                    return;
                }

                // Get next client from queue
                client_fd = client_queue_.front();
                client_queue_.pop();
            }

            // Process the client connection
            handle_client(client_fd);
        }
    }

   public:
    /**
     * Constructor - creates the specified number of worker threads
     * @param num_threads Number of worker threads to create
     */
    ThreadPool(int num_threads)
    {
        for (int i = 0; i < num_threads; ++i)
        {
            workers_.emplace_back([this] { worker(); });
        }
    }

    /**
     * Destructor - signals all threads to stop and waits for them to finish
     */
    ~ThreadPool()
    {
        stop_.store(true);       // Signal all threads to stop
        queue_cv_.notify_all();  // Wake up all waiting threads

        // Wait for all worker threads to finish
        for (auto& worker : workers_)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    /**
     * Submits a client connection to the thread pool for processing
     * @param client_fd File descriptor of the client socket to process
     */
    void submit(int client_fd)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            client_queue_.push(client_fd);
        }
        queue_cv_.notify_one();  // Wake up one worker thread
    }
};

// =============================================================================
// MAIN SERVER FUNCTION
// =============================================================================

/**
 * Main server function - sets up the server and handles incoming connections
 *
 * This function:
 * 1. Initializes the server socket
 * 2. Creates a thread pool for handling clients
 * 3. Continuously accepts new client connections
 * 4. Delegates each client to the thread pool
 *
 * @return 0 on success, 1 on error
 */
int main()
{
    try
    {
        // Initialize server socket
        int server_fd = setup_server_socket();
        std::cout << "Server listening on port " << Config::PORT << std::endl;

        // Create thread pool for handling client connections
        ThreadPool pool(Config::THREADS_NUM);

        // Main server loop - accept and dispatch client connections
        while (true)
        {
            struct sockaddr_storage client_addr;
            socklen_t addr_size = sizeof(client_addr);

            // Accept new client connection
            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_size);
            if (client_fd == -1)
            {
                std::cerr << "Accept failed, continuing..." << std::endl;
                continue;  // Continue accepting other connections
            }

            // Submit client to thread pool for processing
            pool.submit(client_fd);
        }

        // Clean up server socket (unreachable in current implementation)
        close(server_fd);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
