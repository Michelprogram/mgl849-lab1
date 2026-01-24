#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "socket.h"
#include "lps25h.h"
#include "i2c.h"
#include "hts221.h"
#include "joystick.h"

//#define TESTSOCKET
//#define TESTJOYSTICK
//#define TESTTEMPERATURE
#define TESTI2C


#ifdef TESTI2C
int main(void) {
    int file = i2c_init_lps25h();
    int file2 = i2c_init_hts221();

    if (file < 0) {
        fprintf(stderr, "Failed to initialize I2C bus\n");
        return 1;
    }
    if (file2 < 0) {
        fprintf(stderr, "Failed to initialize I2C bus\n");
        return 1;
    }

    printf("I2C buses initialized\n");

    while (1) {
        float temperature = lps25h_read_temperature(file);
        float pressure = lps25h_read_pressure(file);
        float humidity = hts221_read_humidity(file2);
        printf("Temperature: %.2f °C, Pressure: %.2f hPa, Humidity: %.2f %%\n", temperature, pressure, humidity);
        sleep(2);
    }


    i2c_close(file);
    i2c_close(file2);

    return 0;
}
#endif

#ifdef TESTSOCKET
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
#endif

#ifdef TESTJOYSTICK
int main(void) {
    int js_fd;
    joystick_event_t event;
    
    // Initialiser le joystick
    js_fd = joystick_init();
    if (js_fd < 0) {
        fprintf(stderr, "Failed to initialize joystick\n");
        return 1;
    }
    
    printf("Joystick ready. Press buttons...\n");
    
    // Lire les événements
    while (1) {
        if (joystick_read_event(js_fd, &event) == 0) {
            printf("Joystick %s: %s\n",
                   joystick_code_to_string(event.code),
                   joystick_value_to_string(event.value));
            
            // Sortir si ENTER est pressé
            if (event.code == JS_ENTER && event.value == JS_PRESSED) {
                printf("ENTER pressed, exiting...\n");
                break;
            }
        }
    }
    
    joystick_close(js_fd);
    return 0;
}
#endif

#ifdef TESTTEMPERATURE
int main(void) {
    int sensor;
    float temperature;
    
    sensor = lps25h_init();
    if (sensor < 0) {
        fprintf(stderr, "Failed to initialize LPS25H sensor\n");
        return 1;
    }
    
    temperature = lps25h_read_temperature(sensor);
    if (temperature != -999.0) {
        printf("Temperature: %.2f °C\n", temperature);
    } else {
        printf("Error reading temperature\n");
    }
    
    lps25h_close(sensor);
    return 0;
}
#endif