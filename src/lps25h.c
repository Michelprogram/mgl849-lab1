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

int lps25h_init(void) {
    int file;
    int device_id;
    
    
    printf("Opening I2C bus...\n");
    file = open("/dev/i2c-1", O_RDWR);
    if (file < 0) {
        fprintf(stderr, "Failed to open I2C bus: %s\n", strerror(errno));
        return -1;
    }

    printf("Setting I2C slave address to 0x%02X...\n", LPS25H_ADDR);
    if (ioctl(file, I2C_SLAVE, LPS25H_ADDR) < 0) {
        fprintf(stderr, "Failed to set I2C slave address: %s\n", strerror(errno));
        close(file);
        return 1;
    }

    printf("Slave address set successfully\n\n");
 
    device_id = i2c_smbus_read_byte_data(file, WHO_AM_I);
    if (device_id < 0) {
        fprintf(stderr, "Failed to read WHO_AM_I: %s\n", strerror(errno));
        close(file);
        return 1;
    }

    printf("Device ID: 0x%02X (expected 0x%02X)\n", device_id, LPS25H_ID);
    printf("Configuring sensor (CTRL_REG1 at 0x%02X)...\n", CTRL_REG1);

    // Configure sensor: Power ON, 1Hz sampling
    // CTRL_REG1: PD=1, ODR=001 (Binary: 10010000 = 0x90)
    if (i2c_smbus_write_byte_data(file, CTRL_REG1, 0x90) < 0) {
        fprintf(stderr, "Failed to configure sensor\n");
        close(file);
        return -1;
    }     
    printf("Sensor configured\n");
    sleep(1);
                
    return file;
}

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
    
    // Apply formula from datasheet (page 33): T(°C) = 42.5 + (TEMP_OUT / 480)
    return 42.5 + (temp_raw / 480.0);
}


void lps25h_close(int file) {
    if (file >= 0) {
        close(file);
    }
}