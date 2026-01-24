#include "i2c.h"
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

int i2c_init_lps25h(void) {
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

int i2c_init_hts221(void) {
    int file = open("/dev/i2c-1", O_RDWR);
    if (file < 0) return -1;

    // Fixer l'adresse pour le reste de la session
    if (ioctl(file, I2C_SLAVE, HTS221_ADDR) < 0) {
        close(file);
        return -1;
    }

    // Config : Power ON (0x80) + 1Hz (0x01) = 0x81
    i2c_smbus_write_byte_data(file, CTRL_REG1_HTS221, 0x81);
    
    return file;
}

void i2c_close(int file) {
    if (file >= 0) close(file);
}