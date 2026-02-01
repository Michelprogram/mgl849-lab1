#ifndef CONFIG_H
#define CONFIG_H

// Network
#define DEFAULT_SERVER_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT 1234

// Task periods (seconds)
#define PERIOD_SENSOR_READ  2
#define PERIOD_TARGET_TEMP  5

// Temperature limits
#define TEMP_MIN            5.0f
#define TEMP_MAX            50.0f
#define TEMP_DEFAULT        30.0f

// Scheduling
#define PRIORITY_HIGH       90
#define PRIORITY_NORMAL     50

#endif