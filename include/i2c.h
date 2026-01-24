#include <stdint.h>

// LPS25H I2C Addresses
#define LPS25H_ADDR 0x5C

// Register addresses (from datasheet page 20)
#define WHO_AM_I    0x0F  // Should return 0xBD
#define CTRL_REG1   0x20  // Control register
#define STATUS_REG  0x27  // Status register

// Expected device ID
#define LPS25H_ID   0xBD

#define CTRL_REG1_HTS221   0x20
#define HTS221_ADDR        0x5F
#define HTS221_ID          0xBC

/**
 * Initialize I2C bus
 * @return File descriptor for I2C device, or -1 on failure
 */
int i2c_init_lps25h(void);

int i2c_init_hts221(void);

void i2c_close(int file);
