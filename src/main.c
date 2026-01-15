#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "socket.h"
#include "lps25h.h"

int main(int argc, char *argv[]) {
    Socket sock;
    int sensor;
    float temperature;
    
    const char *server_ip = (argc > 1) ? argv[1] : "127.0.0.1";
    int server_port = (argc > 2) ? atoi(argv[2]) : 8888;
    
    socket_init(&sock, server_ip, server_port);
    
    printf("Connecting to %s:%d...\n", sock.ip, sock.port);
    if (socket_connect(&sock) < 0) {
        fprintf(stderr, "✗ Failed to connect\n");
        return 1;
    }
    printf("Connected!\n\n");

    sensor = lps25h_init();
    if (sensor < 0) {
        fprintf(stderr, "Failed to initialize LPS25H sensor\n");
        return 1;
    }

    while (1) {
        temperature = lps25h_read_temperature(sensor);
        
        if (temperature != -999.0) {
            char buffer[100];
            sprintf(buffer, "TP%.2f\n\n", temperature);
            if (socket_send_str(&sock, buffer) < 0) {
                fprintf(stderr, "✗ Send failed\n");
            }
            printf("Temperature: %.2f °C\n", temperature);
        } else {
            printf("Error reading temperature\n");
        }
        
        sleep(2);
    }
    
    socket_close(&sock);
    lps25h_close(sensor);
    return 0;
}