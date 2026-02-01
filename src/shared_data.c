#include "shared_data.h"
#include <pthread.h>

void shared_data_init(SharedData *data) {
    data->temperature = 0.0f;
    data->target_temp = 30.0f;

    pthread_mutex_init(&data->lock, NULL);
}

void shared_data_destroy(SharedData *data) {
    pthread_mutex_destroy(&data->lock);
}

float shared_data_get_temp(SharedData *data) {
    pthread_mutex_lock(&data->lock);
    float value = data->temperature;
    pthread_mutex_unlock(&data->lock);
    return value;
}

float shared_data_get_target_temp(SharedData *data) {
    pthread_mutex_lock(&data->lock);
    float value = data->target_temp;
    pthread_mutex_unlock(&data->lock);
    return value;
}

void shared_data_get_temp_and_target_temp(SharedData *data, float *temp, float *consigne) {
    pthread_mutex_lock(&data->lock);
    *temp = data->temperature;
    *consigne = data->target_temp;
    pthread_mutex_unlock(&data->lock);
}

void shared_data_set_temp(SharedData *data, float value) {
    pthread_mutex_lock(&data->lock);
    data->temperature = value;
    pthread_mutex_unlock(&data->lock);
}

void shared_data_set_target_temp(SharedData *data, float value) {
    pthread_mutex_lock(&data->lock);
    data->target_temp = value;
    pthread_mutex_unlock(&data->lock);
}