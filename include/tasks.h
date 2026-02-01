#ifndef TASKS_H
#define TASKS_H

#include <pthread.h>

extern pthread_mutex_t sock_lock;
extern pthread_mutex_t temp_lock;
extern pthread_mutex_t temp_consigne_lock;

extern pthread_mutex_t consigne_signal_lock;
extern pthread_cond_t consigne_signal_cond;

extern float temp;
extern float temp_consigne;
extern Socket sock;

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
void* task_target_temp();

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
void* task_power();

/**
 * Initialize real-time condition
 * @return 0 on success, -1 on failure
 */
int init_realtime_cond(void);

#endif
