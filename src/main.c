#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "socket.h"
#include "lps25h.h"
#include "i2c.h"
#include "hts221.h"
#include "joystick.h"

Socket sock;
float temp;
float temp_consigne = 30.0;

pthread_mutex_t sock_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t temp_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t temp_consigne_lock = PTHREAD_MUTEX_INITIALIZER;


void* task_read_temperature(void* arg) {
    int file = *(int*)arg;
    while (1) {
        pthread_mutex_lock(&temp_lock);
        temp = lps25h_read_temperature(file);
        pthread_mutex_unlock(&temp_lock);

        pthread_mutex_lock(&sock_lock);
        if (socket_send_temperature(&sock, temp) < 0) {
            fprintf(stderr, "Failed to send temperature\n");
        }
        pthread_mutex_unlock(&sock_lock);
        
        printf("Temperature: %.2f °C\n", temp);
        sleep(2);
    }
    return NULL;
}

void* task_read_pressure(void* arg) {
    int file = *(int*)arg;
    while (1) {
        float pressure = lps25h_read_pressure(file);
        pthread_mutex_lock(&sock_lock);
        if (socket_send_pressure(&sock, pressure) < 0) {
            fprintf(stderr, "Failed to send pressure\n");
        }
        pthread_mutex_unlock(&sock_lock);
        printf("Pressure: %.2f hPa\n", pressure);
        sleep(2);
    }
    return NULL;
}

void* task_read_humidity(void* arg) {
    int file = *(int*)arg;
    while (1) {
        float humidity = hts221_read_humidity(file);
        pthread_mutex_lock(&sock_lock);
        if (socket_send_humidity(&sock, humidity) < 0) {
            fprintf(stderr, "Failed to send humidity\n");
        }
        pthread_mutex_unlock(&sock_lock);
        printf("Humidity: %.2f %%\n", humidity);
        sleep(2);
    }
    return NULL;
}

void* task_setpoint_logic(void* arg) {
    while (1) {
        sleep(5); 
        pthread_mutex_lock(&temp_consigne_lock);
        temp_consigne += 2.0;
        pthread_mutex_unlock(&temp_consigne_lock);
        
        if (temp_consigne > 40.0) temp_consigne = 5.0; 
        
        printf("[T1] Nouvelle consigne : %.2f°C\n", temp_consigne);
        pthread_mutex_lock(&sock_lock);
        if (socket_send_consigne(&sock, temp_consigne) < 0) {
            fprintf(stderr, "Failed to send consigne\n");
        }
        pthread_mutex_unlock(&sock_lock);
    }
    return NULL;
}

void* task_power(void* arg) {
    while (1) {
        pthread_mutex_lock(&temp_consigne_lock);
        pthread_mutex_lock(&temp_lock);
        float power = ((temp_consigne - temp)/6.0)*100.0;
        if (power > 100.0) power = 100.0;
        if (power < 0.0) power = 0.0;
        pthread_mutex_unlock(&temp_consigne_lock);
        pthread_mutex_unlock(&temp_lock);

        pthread_mutex_lock(&sock_lock);

        printf("Power: %.2f %%\n", power);
        if (socket_send_power(&sock, power) < 0) {
            fprintf(stderr, "Failed to send power\n");
        }
        pthread_mutex_unlock(&sock_lock);

        sleep(2);
    }
    return NULL;
}

int main(int argc, char *argv[]) {

    pthread_t th_read_temperature;
    pthread_t th_read_pressure;
    pthread_t th_read_humidity;
    pthread_t th_setpoint_logic;
    pthread_t th_power;

    const char *server_ip = (argc > 1) ? argv[1] : "127.0.0.1";
    int server_port = (argc > 2) ? atoi(argv[2]) : 1234;
    
    socket_init(&sock, server_ip, server_port);

    printf("Connecting to %s:%d...\n", sock.ip, sock.port);
    if (socket_connect(&sock) < 0) {
        fprintf(stderr, "✗ Failed to connect\n");
        return 1;
    }
    printf("Connected!\n\n");

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

    pthread_create(&th_read_temperature, NULL, task_read_temperature, &file);
    pthread_create(&th_read_pressure, NULL, task_read_pressure, &file);
    pthread_create(&th_read_humidity, NULL, task_read_humidity, &file2);
    pthread_create(&th_setpoint_logic, NULL, task_setpoint_logic, NULL);
    pthread_create(&th_power, NULL, task_power, NULL);

    pthread_join(th_read_temperature, NULL);
    pthread_join(th_read_pressure, NULL);
    pthread_join(th_read_humidity, NULL);
    pthread_join(th_setpoint_logic, NULL);
    pthread_join(th_power, NULL);

    socket_close(&sock);
    i2c_close(file);
    i2c_close(file2);

    return 0;
}

