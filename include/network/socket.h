#ifndef SOCKET_H
#define SOCKET_H

#include <stddef.h>

// Socket structure
typedef struct {
    int fd; 
    char ip[16];
    int port;
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
 * Send temperature to the server
 * @param s Pointer to Socket structure
 * @param temperature Temperature to send
 * @return Number of bytes sent, or -1 on failure
 */
int socket_send_temperature(Socket *s, float temperature);

/**
 * Send pressure to the server
 * @param s Pointer to Socket structure
 * @param pressure Pressure to send
 * @return Number of bytes sent, or -1 on failure
 */
int socket_send_pressure(Socket *s, float pressure);

/**
 * Send humidity to the server
 * @param s Pointer to Socket structure
 * @param humidity Humidity to send
 * @return Number of bytes sent, or -1 on failure
 */
int socket_send_humidity(Socket *s, float humidity);

/**
 * Send consigne to the server
 * @param s Pointer to Socket structure
 * @param consigne Consigne to send
 * @return Number of bytes sent, or -1 on failure
 */
int socket_send_consigne(Socket *s, float consigne);

/**
 * Send power to the server
 * @param s Pointer to Socket structure
 * @param power Power to send
 * @return Number of bytes sent, or -1 on failure
 */
int socket_send_power(Socket *s, float power);

/**
 * Close the socket connection
 * @param s Pointer to Socket structure
 */
void socket_close(Socket *s);

#endif