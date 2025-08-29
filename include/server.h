#pragma once

// Server functions
int setup_server_socket();
void handle_client(int client_fd);