#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <linux/input.h>
#include <stdbool.h>

#define MODIFIER_KEY_PLUS        78   // Numpad +
#define MODIFIER_KEY_MINUS       74   // Numpad -
#define MODIFIER_KEY_KPPLUS      78   // Keypad +
#define MODIFIER_KEY_KPMINUS     74   // Keypad -
#define MODIFIER_KEY_UP          103   // Up
#define MODIFIER_KEY_DOWN        108   // Down

#define MODIFIER_KEY_PRESSED     1

typedef struct {
    uint16_t code;
    int32_t value;
    bool is_plus;
} keyboard_event_t;

/**
 * Initialize keyboard - find and open the keyboard device
 * @return File descriptor, or -1 on failure
 */
int keyboard_init(void);

/**
 * Read next keyboard event (blocking)
 * @param fd Keyboard file descriptor
 * @param event Pointer to keyboard_event_t to fill
 * @return 0 on success, -1 on error
 */
int keyboard_read_event(int fd, keyboard_event_t *event);

/**
 * Close keyboard device
 * @param fd Keyboard file descriptor
 */
void keyboard_close(int fd);

#endif