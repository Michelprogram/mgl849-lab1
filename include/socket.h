#ifndef SOCKET_H
#define SOCKET_H

#include <stddef.h>

// Socket structure
typedef struct {
    int fd; 
    char ip[16];
    int port;
    int connected;
} Socket;

/**
 * Initialize a socket structure
 * @param s Pointer to Socket structure
 * @param ip Server IP address (e.g., "127.0.0.1")
 * @param port Server port number
 */
void socket_init(Socket *s, const char *ip, int port);

/**
 * Connect to the server
 * @param s Pointer to Socket structure
 * @return 0 on success, -1 on failure
 */
int socket_connect(Socket *s);

/**
 * Send raw data to the server
 * @param s Pointer to Socket structure
 * @param data Pointer to data buffer
 * @param len Length of data to send
 * @return Number of bytes sent, or -1 on failure
 */
int socket_send(Socket *s, const char *data, size_t len);

/**
 * Send a string to the server
 * @param s Pointer to Socket structure
 * @param str Null-terminated string to send
 * @return Number of bytes sent, or -1 on failure
 */
int socket_send_str(Socket *s, const char *str);

/**
 * Close the socket connection
 * @param s Pointer to Socket structure
 */
void socket_close(Socket *s);

/**
 * Check if socket is connected
 * @param s Pointer to Socket structure
 * @return 1 if connected, 0 if disconnected
 */
int socket_is_connected(Socket *s);

#endif // SOCKET_H