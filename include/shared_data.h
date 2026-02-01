// include/shared_data.h
#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <pthread.h>

typedef struct {
    float temperature;
    float target_temp;
    pthread_mutex_t lock;
} SharedData;

/**
 * Initialize shared data
 * @param data Pointer to SharedData structure
 */
void shared_data_init(SharedData *data);

/**
 * Destroy shared data
 * @param data Pointer to SharedData structure
 */
void shared_data_destroy(SharedData *data);

/**
 * Get temperature
 * @param data Pointer to SharedData structure
 * @return Temperature
 */
float shared_data_get_temp(SharedData *data);

/**
 * Get target temperature
 * @param data Pointer to SharedData structure
 * @return Target temperature
 */
float shared_data_get_target_temp(SharedData *data);

/**
 * Get temperature and target temperature
 * @param data Pointer to SharedData structure
 * @param temp Pointer to temperature
 * @param target_temp Pointer to target temperature
 */
void shared_data_get_temp_and_target_temp(SharedData *data, float *temp, float *target_temp);

/**
 * Set temperature
 * @param data Pointer to SharedData structure
 * @param value Temperature
 */
void shared_data_set_temp(SharedData *data, float value);

/**
 * Set target temperature
 * @param data Pointer to SharedData structure
 * @param value Target temperature
 */
void shared_data_set_target_temp(SharedData *data, float value);

#endif