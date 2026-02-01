#ifndef TASKS_H
#define TASKS_H

#include <pthread.h>
#include "shared_data.h"
#include "socket.h"  

extern pthread_mutex_t consigne_signal_lock;
extern pthread_cond_t consigne_signal_cond;

typedef struct {
    SharedData *data;
    Socket *sock;
} TaskArgs;

typedef struct {
    SharedData *data;
    int i2c_fd;
    Socket *sock;
} SensorTaskArgs;

typedef struct {
    SharedData *data;
    int keyboard_fd;
} KeyboardTaskArgs;

/**
 * Task to read temperature from LPS25H sensor
 * @param arg I2C file descriptor
 * @return NULL
 */
void* task_temperature(void* arg);

/**
 * Task to read pressure from LPS25H sensor
 * @param arg I2C file descriptor
 * @return NULL
 */
void* task_pressure(void* arg);

/**
 * Task to read humidity from HTS221 sensor
 * @param arg I2C file descriptor
 * @return NULL
 */
void* task_humidity(void* arg);

/**
 * Task to read target temperature
 * @return NULL
 */
void* task_target_temp(void* arg);

/**
 * Task to read keyboard events
 * @param arg Keyboard file descriptor
 * @return NULL
 */
void* task_keyboard(void* arg);

/**
 * Task to calculate power
 * @return NULL
 */
void* task_power(void* arg);

/**
 * Initialize real-time condition
 * @return 0 on success, -1 on failure
 */
int init_realtime_cond(void);

#endif
