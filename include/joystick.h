#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include <linux/input.h>

#define JS_UP       103  // KEY_UP
#define JS_DOWN     108  // KEY_DOWN
#define JS_LEFT     105  // KEY_LEFT
#define JS_RIGHT    106  // KEY_RIGHT
#define JS_ENTER    28   // KEY_ENTER

#define JS_RELEASED 0
#define JS_PRESSED  1
#define JS_HOLD     2

/**
 * Structure to hold joystick event information
 */
typedef struct {
    uint16_t code;      // Key code (JS_UP, JS_DOWN, etc.)
    int32_t value;      // Event value (JS_PRESSED, JS_RELEASED, JS_HOLD)
} joystick_event_t;

/**
 * Initialize joystick connection
 * Searches for the Sense HAT joystick device and opens it
 * @return File descriptor for joystick device, or -1 on failure
 */
int joystick_init(void);

/**
 * Read next joystick event (blocking)
 * @param fd Joystick file descriptor
 * @param event Pointer to joystick_event_t structure to fill
 * @return 0 on success, -1 on error
 */
int joystick_read_event(int fd, joystick_event_t *event);

/**
 * Convert joystick code to human-readable string
 * @param code Joystick code (JS_UP, JS_DOWN, etc.)
 * @return String representation of the direction
 */
const char* joystick_code_to_string(uint16_t code);

/**
 * Convert event value to human-readable string
 * @param value Event value (JS_PRESSED, JS_RELEASED, JS_HOLD)
 * @return String representation of the state
 */
const char* joystick_value_to_string(int32_t value);

/**
 * Close joystick connection
 * @param fd Joystick file descriptor
 */
void joystick_close(int fd);

#endif