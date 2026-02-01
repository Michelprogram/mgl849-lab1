#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

void socket_init(Socket *s, const char *ip, int port) {
    s->fd = -1;
    s->port = port;
    strncpy(s->ip, ip, sizeof(s->ip) - 1);
    s->ip[sizeof(s->ip) - 1] = '\0';
    pthread_mutex_init(&s->lock, NULL);
}

int socket_connect(Socket *s) {
    struct sockaddr_in server_addr;
    
    s->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (s->fd < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(s->port);
    
    if (inet_pton(AF_INET, s->ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address: %s\n", s->ip);
        close(s->fd);
        s->fd = -1;
        return -1;
    }
    
    if (connect(s->fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(s->fd);
        s->fd = -1;
        return -1;
    }
    
    return 0;
}

int socket_send(Socket *s, const char *data, size_t len) {
    if (s->fd < 0) {
        fprintf(stderr, "Socket not connected\n");
        return -1;
    }
    
    ssize_t sent = send(s->fd, data, len, 0);
    if (sent < 0) {
        perror("Send failed");
        return -1;
    }
    
    return sent;
}

int socket_send_temperature(Socket *s, float temperature) {
    char buffer[100];
    sprintf(buffer, "TP%.2f\n", temperature);

    pthread_mutex_lock(&s->lock);
    int result = socket_send(s, buffer, strlen(buffer));
    pthread_mutex_unlock(&s->lock);
    
    return result;
}

int socket_send_pressure(Socket *s, float pressure) {
    char buffer[100];
    sprintf(buffer, "PR%.2f\n", pressure);

    pthread_mutex_lock(&s->lock);
    int result = socket_send(s, buffer, strlen(buffer));
    pthread_mutex_unlock(&s->lock);
    
    return result;
}

int socket_send_humidity(Socket *s, float humidity) {
    char buffer[100];
    sprintf(buffer, "HU%.2f\n", humidity);

    pthread_mutex_lock(&s->lock);
    int result = socket_send(s, buffer, strlen(buffer));
    pthread_mutex_unlock(&s->lock);
    
    return result;
}

int socket_send_target_temp(Socket *s, float target_temp) {
    char buffer[100];
    sprintf(buffer, "TD%.2f\n", target_temp);

    pthread_mutex_lock(&s->lock);
    int result = socket_send(s, buffer, strlen(buffer));
    pthread_mutex_unlock(&s->lock);
    
    return result;
}

int socket_send_power(Socket *s, float power) {
    char buffer[100];
    sprintf(buffer, "PW%.2f\n", power);

    pthread_mutex_lock(&s->lock);
    int result = socket_send(s, buffer, strlen(buffer));
    pthread_mutex_unlock(&s->lock);
    
    return result;
}

void socket_close(Socket *s) {
    if (s->fd >= 0) {
        close(s->fd);
        s->fd = -1;
    }
}

void socket_destroy(Socket *s) {
    socket_close(s);
    pthread_mutex_destroy(&s->lock);
}