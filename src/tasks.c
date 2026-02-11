#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "shared_data.h"
#include "socket.h"
#include "lps25h.h"
#include "hts221.h"
#include "i2c.h"
#include "tasks.h"
#include "keyboard.h"
#include "config.h"
#include <signal.h>

#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))

// Define mutexes
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
}

void* task_temperature(void* arg) {
    SensorTaskArgs *args = (SensorTaskArgs*)arg;

    while (1) {
        float temp = lps25h_read_temperature(args->i2c_fd);
        shared_data_set_temp(args->data, temp);

        if (socket_send_temperature(args->sock, temp) < 0) {
            shutdown_on_error("Failed to send temperature");
            break;
        }

        printf("Temperature: %.2f °C\n\n", temp);
        sleep(PERIOD_SENSOR_READ);
    }
    return NULL;
}

void* task_pressure(void* arg) {
    SensorTaskArgs *args = (SensorTaskArgs*)arg;

    while (1) {
        float pressure = lps25h_read_pressure(args->i2c_fd);

        if (socket_send_pressure(args->sock, pressure) < 0) {
            shutdown_on_error("Failed to send pressure");
            break;
        }

        printf("Pressure: %.2f hPa\n\n", pressure);
        sleep(PERIOD_SENSOR_READ);
    }
    return NULL;
}

void* task_humidity(void* arg) {
    SensorTaskArgs *args = (SensorTaskArgs*)arg;
    while (1) {
        float humidity = hts221_read_humidity(args->i2c_fd);

        if (socket_send_humidity(args->sock, humidity) < 0) {
            shutdown_on_error("Failed to send humidity");
            break;
        }

        printf("Humidity: %.2f %%\n\n", humidity);
        sleep(PERIOD_SENSOR_READ);
    }
    return NULL;
}

void* task_keyboard(void* arg) {
    KeyboardTaskArgs *args = (KeyboardTaskArgs*)arg;
    keyboard_event_t event;

    while (1) {
        if (keyboard_read_event(args->keyboard_fd, &event) == 0) {
            if (event.value == 1) {
                printf("[Keyboard] Key %d pressed\n", event.code);

                float target_temp = shared_data_get_target_temp(args->data);
                
                if (event.is_plus) {
                    shared_data_set_target_temp(args->data, MIN(target_temp + TEMP_STEP, TEMP_MAX));
                } else {
                    shared_data_set_target_temp(args->data, MAX(target_temp - TEMP_STEP, TEMP_MIN));
                }

                // Signal the target_temp task to send immediately
                pthread_mutex_lock(&consigne_signal_lock);
                pthread_cond_broadcast(&consigne_signal_cond); 
                pthread_mutex_unlock(&consigne_signal_lock);
            }
        }
    }
    return NULL;
}

void* task_target_temp(void* arg) {
    int policy;
    struct sched_param param;
    TaskArgs *args = (TaskArgs*)arg;

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

        float target_temp = shared_data_get_target_temp(args->data);
        
        printf("Target temperature : %.2f°C\n\n", target_temp);
        
        if (socket_send_target_temp(args->sock, target_temp) < 0) {
            shutdown_on_error("Failed to send target temperature");
            break;
        }

        if (wait_result == 0) {
            struct sched_param normal_param;
            normal_param.sched_priority = PRIORITY_NORMAL;
            pthread_setschedparam(pthread_self(), SCHED_OTHER, &normal_param);
        }
    }
    return NULL;
}

void* task_power(void* arg) {
    int policy;
    struct sched_param param;
    TaskArgs *args = (TaskArgs*)arg;

    pthread_getschedparam(pthread_self(), &policy, &param);

    while (1) {
        
        struct timespec ts;
        
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_sec += PERIOD_TARGET_TEMP;
        
        pthread_mutex_lock(&consigne_signal_lock);
        int wait_result = pthread_cond_timedwait(&consigne_signal_cond, 
                                                  &consigne_signal_lock, &ts);
        pthread_mutex_unlock(&consigne_signal_lock);
        
        if (wait_result == 0) {
            printf("Switching to SCHED_FIFO\n");
            
            struct sched_param rt_param;
            rt_param.sched_priority = PRIORITY_HIGH;
            
            if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &rt_param) != 0) {
                perror("Failed to set SCHED_FIFO");
            }
        }
        
        float temp, target_temp;
        shared_data_get_temp_and_target_temp(args->data, &temp, &target_temp);

        float power = ((target_temp - temp)/6.0)*100.0;
        if (power > 100.0) power = 100.0;
        if (power < 0.0) power = 0.0;

        printf("Power: %.2f %%\n\n", power);
        if (socket_send_power(args->sock, power) < 0) {
            shutdown_on_error("Failed to send power");
            break;
        }
        
        if (wait_result == 0) {
            struct sched_param normal_param;
            normal_param.sched_priority = PRIORITY_NORMAL;
            pthread_setschedparam(pthread_self(), SCHED_OTHER, &normal_param);
        }
    }
    return NULL;
}
