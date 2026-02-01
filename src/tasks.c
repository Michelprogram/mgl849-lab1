#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "socket.h"
#include "lps25h.h"
#include "hts221.h"
#include "i2c.h"
#include "tasks.h"
#include "keyboard.h"
#include "config.h"
#include <signal.h> 

// Define mutexes
pthread_mutex_t sock_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t temp_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t temp_consigne_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t consigne_signal_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t consigne_signal_cond;


int init_realtime_cond(void) {
    pthread_condattr_t cond_attr;
    
    if (pthread_condattr_init(&cond_attr) != 0) {
        perror("pthread_condattr_init");
        return -1;
    }
    
    if (pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC) != 0) {
        perror("pthread_condattr_setclock");
        pthread_condattr_destroy(&cond_attr);
        return -1;
    }
    
    if (pthread_cond_init(&consigne_signal_cond, &cond_attr) != 0) {
        perror("pthread_cond_init");
        pthread_condattr_destroy(&cond_attr);
        return -1;
    }
    
    pthread_condattr_destroy(&cond_attr);
    return 0;
}

static void shutdown_on_error(const char *msg) {
    fprintf(stderr, "%s - initiating shutdown\n", msg);
    raise(SIGINT);
    pthread_exit(NULL);
}

void* task_temperature(void* arg) {
    int file = *(int*)arg;
    while (1) {
        pthread_mutex_lock(&temp_lock);
        temp = lps25h_read_temperature(file);
        pthread_mutex_unlock(&temp_lock);

        pthread_mutex_lock(&sock_lock);
        if (socket_send_temperature(&sock, temp) < 0) {
            pthread_mutex_unlock(&sock_lock);
            shutdown_on_error("Failed to send temperature");
        }
        pthread_mutex_unlock(&sock_lock);

        printf("Temperature: %.2f °C\n\n", temp);
        sleep(PERIOD_SENSOR_READ);
    }
    return NULL;
}

void* task_pressure(void* arg) {
    int file = *(int*)arg;
    while (1) {
        float pressure = lps25h_read_pressure(file);
        pthread_mutex_lock(&sock_lock);
        if (socket_send_pressure(&sock, pressure) < 0) {
            pthread_mutex_unlock(&sock_lock);
            shutdown_on_error("Failed to send pressure");
        }
        pthread_mutex_unlock(&sock_lock);
        printf("Pressure: %.2f hPa\n\n", pressure);
        sleep(PERIOD_SENSOR_READ);
    }
    return NULL;
}

void* task_humidity(void* arg) {
    int file = *(int*)arg;
    while (1) {
        float humidity = hts221_read_humidity(file);
        pthread_mutex_lock(&sock_lock);
        if (socket_send_humidity(&sock, humidity) < 0) {
            pthread_mutex_unlock(&sock_lock);
            shutdown_on_error("Failed to send humidity");
        }
        pthread_mutex_unlock(&sock_lock);
        printf("Humidity: %.2f %%\n\n", humidity);
        sleep(PERIOD_SENSOR_READ);
    }
    return NULL;
}

void* task_keyboard(void* arg) {
    int keyboard_fd = *(int*)arg;
    keyboard_event_t event;

    while (1) {
        if (keyboard_read_event(keyboard_fd, &event) == 0) {
            if (event.value == 1) {
                printf("[Keyboard] Key %d pressed\n", event.code);
                
                pthread_mutex_lock(&temp_consigne_lock);

                if (event.is_plus) {
                    temp_consigne ++;
                    if (temp_consigne > TEMP_MAX) temp_consigne = TEMP_DEFAULT;
                } else {
                    temp_consigne --;
                    if (temp_consigne < TEMP_MIN) temp_consigne = TEMP_DEFAULT;
                }

                pthread_mutex_unlock(&temp_consigne_lock);
                
                // Signal the target_temp task to send immediately
                pthread_mutex_lock(&consigne_signal_lock);
                pthread_cond_signal(&consigne_signal_cond);
                pthread_mutex_unlock(&consigne_signal_lock);
            }
        }
    }
    return NULL;
}

void* task_target_temp() {
    int policy;
    struct sched_param param;

    pthread_getschedparam(pthread_self(), &policy, &param);
    
    while (1) {
        struct timespec ts;
        
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_sec += PERIOD_TARGET_TEMP;
        
        pthread_mutex_lock(&consigne_signal_lock);
        int wait_result = pthread_cond_timedwait(&consigne_signal_cond, 
                                                  &consigne_signal_lock, &ts);
        pthread_mutex_unlock(&consigne_signal_lock);
        
        // Boost priority ONLY if keyboard triggered (wait_result == 0)
        if (wait_result == 0) {
            printf("Switching to SCHED_FIFO\n");
            
            struct sched_param rt_param;
            rt_param.sched_priority = PRIORITY_HIGH;
            
            if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &rt_param) != 0) {
                perror("Failed to set SCHED_FIFO");
            }
        }

        pthread_mutex_lock(&temp_consigne_lock);
        float current_consigne = temp_consigne;
        pthread_mutex_unlock(&temp_consigne_lock);
        
        printf("Target temperature : %.2f°C\n\n", current_consigne);
        
        pthread_mutex_lock(&sock_lock);
        if (socket_send_consigne(&sock, current_consigne) < 0) {
            fprintf(stderr, "Failed to send consigne\n");
        }
        pthread_mutex_unlock(&sock_lock);

        if (wait_result == 0) {
            struct sched_param normal_param;
            normal_param.sched_priority = PRIORITY_NORMAL;
            pthread_setschedparam(pthread_self(), SCHED_OTHER, &normal_param);
        }
    }
    return NULL;
}

void* task_power() {
    while (1) {
        pthread_mutex_lock(&temp_consigne_lock);
        pthread_mutex_lock(&temp_lock);
        float power = ((temp_consigne - temp)/6.0)*100.0;
        if (power > 100.0) power = 100.0;
        if (power < 0.0) power = 0.0;
        pthread_mutex_unlock(&temp_consigne_lock);
        pthread_mutex_unlock(&temp_lock);

        pthread_mutex_lock(&sock_lock);

        printf("Power: %.2f %%\n\n", power);
        if (socket_send_power(&sock, power) < 0) {
            pthread_mutex_unlock(&sock_lock);
            shutdown_on_error("Failed to send power");
        }
        pthread_mutex_unlock(&sock_lock);

        sleep(PERIOD_SENSOR_READ);
    }
    return NULL;
}
