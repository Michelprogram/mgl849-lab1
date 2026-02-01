#ifndef HTS221_H
#define HTS221_H

#define HUMIDITY_OUT_L     0x28
#define HUMIDITY_OUT_H     0x29

/**
 * Read humidity from HTS221 sensor
 * Formula from datasheet: H = raw / 655.36
 * @param file I2C file descriptor
 * @return Humidity in percentage, or -999.0 on error
 */
float hts221_read_humidity(int file);

#endif