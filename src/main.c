#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "socket.h"
#include "lps25h.h"
#include "i2c.h"
#include "hts221.h"
#include "keyboard.h"
#include "config.h"
#include "tasks.h"
#include <signal.h>

Socket sock;
int file, file2, keyboard_fd;
pthread_t th_temperature, th_pressure, th_humidity, th_target_temp, th_power, th_keyboard;
SharedData shared_data;

void graceful_shutdown(int signum) {
    printf("\nReceived signal %d. Cancelling threads...\n", signum);

    pthread_cancel(th_temperature);
    pthread_cancel(th_pressure);
    pthread_cancel(th_humidity);
    pthread_cancel(th_target_temp);
    pthread_cancel(th_power);
    pthread_cancel(th_keyboard);

    printf("Closing socket...\n");
    socket_close(&sock);
    socket_destroy(&sock);

    shared_data_destroy(&shared_data);

    printf("Closing I2C connections...\n");
    i2c_close(file);
    i2c_close(file2);

    printf("Closing keyboard...\n");
    keyboard_close(keyboard_fd);

    printf("Cleanup complete. Exiting...\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    pthread_attr_t attr;
    struct sched_param sched_param;

    const char *server_ip = (argc > 1) ? argv[1] : DEFAULT_SERVER_IP;
    int server_port = (argc > 2) ? atoi(argv[2]) : DEFAULT_SERVER_PORT;

    signal(SIGINT, graceful_shutdown);

    socket_init(&sock, server_ip, server_port);
    shared_data_init(&shared_data);

    printf("Connecting to %s:%d...\n", sock.ip, sock.port);
    
    if (socket_connect(&sock) < 0) {
        fprintf(stderr, "✗ Failed to connect\n");
        return 1;
    }

    printf("Connected!\n\n");

    file = i2c_init_lps25h();
    file2 = i2c_init_hts221();

    if (file < 0) {
        fprintf(stderr, "Failed to initialize I2C bus\n");
        return 1;
    }
    if (file2 < 0) {
        fprintf(stderr, "Failed to initialize I2C bus\n");
        return 1;
    }

    printf("I2C buses initialized\n");

    keyboard_fd = keyboard_init();

    if (keyboard_fd < 0) {
        fprintf(stderr, "Failed to initialize keyboard\n");
        return 1;
    }

    printf("Keyboard initialized\n");

    if (init_realtime_cond() < 0) {
        fprintf(stderr, "Failed to initialize real-time condition\n");
        return 1;
    }
    printf("Real-time condition variable initialized (CLOCK_MONOTONIC)\n");

    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    sched_param.sched_priority = PRIORITY_NORMAL;
    pthread_attr_setschedparam(&attr, &sched_param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    SensorTaskArgs temp_args = { .data = &shared_data, .i2c_fd = file, .sock = &sock };
    SensorTaskArgs pressure_args = { .data = &shared_data, .i2c_fd = file, .sock = &sock };
    SensorTaskArgs humidity_args = { .data = &shared_data, .i2c_fd = file2, .sock = &sock };
    KeyboardTaskArgs keyboard_args = { .data = &shared_data, .keyboard_fd = keyboard_fd };
    
    TaskArgs power_args = { .data = &shared_data, .sock = &sock };
    TaskArgs target_args = { .data = &shared_data, .sock = &sock };

    pthread_create(&th_temperature, &attr, task_temperature, &temp_args);
    pthread_create(&th_pressure, &attr, task_pressure, &pressure_args);
    pthread_create(&th_humidity, &attr, task_humidity, &humidity_args);
    pthread_create(&th_power, &attr, task_power, &power_args);
    pthread_create(&th_target_temp, &attr, task_target_temp, &target_args);
    pthread_create(&th_keyboard, &attr, task_keyboard, &keyboard_args);

    pthread_attr_destroy(&attr);

    pthread_join(th_temperature, NULL);
    pthread_join(th_pressure, NULL);
    pthread_join(th_humidity, NULL);
    pthread_join(th_target_temp, NULL);
    pthread_join(th_power, NULL);
    pthread_join(th_keyboard, NULL);

    graceful_shutdown(0);

    return 0;
}