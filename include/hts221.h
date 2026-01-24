#ifndef HTS221_H
#define HTS221_H

#define HUMIDITY_OUT_L     0x28
#define HUMIDITY_OUT_H     0x29

// Prototypes simplifiés
float hts221_read_humidity(int file);

#endif