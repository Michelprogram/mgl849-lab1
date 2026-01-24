#include "hts221.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <stdint.h>


float hts221_read_humidity(int file) {
    uint8_t l = i2c_smbus_read_byte_data(file, HUMIDITY_OUT_L);
    uint8_t h = i2c_smbus_read_byte_data(file, HUMIDITY_OUT_H);
    
    // On combine les deux octets en un entier signé de 16 bits
    int16_t raw = (int16_t)((h << 8) | l);
    
    // Formule simplifiée pour le test
    return (float)raw / 655.36;
}