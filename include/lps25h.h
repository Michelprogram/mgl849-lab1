#ifndef LPS25H_H
#define LPS25H_H

#include <stdint.h>

// LPS25H I2C Addresses
#define LPS25H_ADDR 0x5C

// Register addresses (from datasheet page 20)
#define WHO_AM_I    0x0F  // Should return 0xBD
#define CTRL_REG1   0x20  // Control register
#define TEMP_OUT_L  0x2B  // Temperature LSB
#define TEMP_OUT_H  0x2C  // Temperature MSB
#define STATUS_REG  0x27  // Status register

// Expected device ID
#define LPS25H_ID   0xBD

/**
 * Initialize LPS25H sensor
 * Searches for the sensor on available I2C buses and configures it
 * @return File descriptor for I2C device, or -1 on failure
 */
int lps25h_init(void);

/**
 * Read temperature from LPS25H sensor
 * Formula from datasheet: T(°C) = 42.5 + (TEMP_OUT / 480)
 * @param file I2C file descriptor
 * @return Temperature in Celsius, or -999.0 on error
 */
float lps25h_read_temperature(int file);

/**
 * Close LPS25H sensor connection
 * @param file I2C file descriptor
 */
void lps25h_close(int file);

#endif