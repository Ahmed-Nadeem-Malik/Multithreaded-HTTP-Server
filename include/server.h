#pragma once

// Creates and configures a TCP server socket
int setup_server_socket();

// Processes client connection and HTTP request
void handle_client(int client_fd);
