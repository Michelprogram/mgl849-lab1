#include "lps25h.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <i2c/smbus.h>

float lps25h_read_temperature(int file) {
    int temp_l, temp_h;
    int16_t temp_raw;
    
    temp_l = i2c_smbus_read_byte_data(file, TEMP_OUT_L);
    if (temp_l < 0) {
        fprintf(stderr, "Failed to read TEMP_OUT_L: %s\n", strerror(errno));
        return -999.0;
    }
    
    temp_h = i2c_smbus_read_byte_data(file, TEMP_OUT_H);
    if (temp_h < 0) {
        fprintf(stderr, "Failed to read TEMP_OUT_H: %s\n", strerror(errno));
        return -999.0;
    }
    
    temp_raw = (int16_t)((temp_h << 8) | temp_l);
    
    return 42.5 + (temp_raw / 480.0);
}

float lps25h_read_pressure(int file) {
    uint8_t xl = i2c_smbus_read_byte_data(file, PRESS_OUT_XL);
    uint8_t l = i2c_smbus_read_byte_data(file, PRESS_OUT_L);
    uint8_t h = i2c_smbus_read_byte_data(file, PRESS_OUT_H);
    
    int32_t raw = (int32_t)((h << 16) | (l << 8) | xl);
    
    return (float)raw / 4096.0;
}