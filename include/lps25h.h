#ifndef LPS25H_H
#define LPS25H_H

// Register addresses (from datasheet page 20)
#define TEMP_OUT_L  0x2B  // Temperature LSB
#define TEMP_OUT_H  0x2C  // Temperature MSB

#define PRESS_OUT_XL       0x28
#define PRESS_OUT_L        0x29
#define PRESS_OUT_H        0x2A

/**
 * Read temperature from LPS25H sensor
 * Formula from datasheet: T(°C) = 42.5 + (TEMP_OUT / 480)
 * @param file I2C file descriptor
 * @return Temperature in Celsius, or -999.0 on error
 */
float lps25h_read_temperature(int file);

float lps25h_read_pressure(int file);

#endif