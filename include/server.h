/**
 * @file server.h
 * @brief TCP server socket management and client handling
 */

#pragma once

/**
 * Creates and configures a TCP server socket
 * @return Socket file descriptor, or -1 on error
 */
int setup_server_socket();

/**
 * Processes client connection and HTTP request
 * @param client_fd Client socket file descriptor
 */
void handle_client(int client_fd);
