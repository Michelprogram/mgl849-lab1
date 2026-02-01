#include <stdint.h>

#define LPS25H_ADDR 0x5C

#define WHO_AM_I    0x0F
#define CTRL_REG1   0x20
#define STATUS_REG  0x27

#define LPS25H_ID   0xBD

#define CTRL_REG1_HTS221   0x20
#define HTS221_ADDR        0x5F
#define HTS221_ID          0xBC

/**
 * Initialize I2C bus
 * @return File descriptor for I2C device, or -1 on failure
 */
int i2c_init_lps25h(void);

/**
 * Initialize I2C bus for HTS221 sensor
 * @return File descriptor for I2C device, or -1 on failure
 */
int i2c_init_hts221(void);

/**
 * Close I2C bus
 * @param file File descriptor for I2C device
 */
void i2c_close(int file);
