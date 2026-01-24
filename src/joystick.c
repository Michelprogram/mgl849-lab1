#include "joystick.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/ioctl.h>

// Maximum event devices to check
#define MAX_EVENT_DEVICES 10

/**
 * Find the Sense HAT joystick device
 * Searches through /dev/input/event* devices to find the joystick
 */
static int find_joystick_device(void) {
    char device_path[256];
    char device_name[256];
    int fd;
    
    printf("Searching for Sense HAT joystick...\n");
    
    // Try event devices from 0 to MAX_EVENT_DEVICES
    for (int i = 0; i < MAX_EVENT_DEVICES; i++) {
        snprintf(device_path, sizeof(device_path), "/dev/input/event%d", i);
        
        fd = open(device_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            continue;  // Device doesn't exist or can't be opened
        }
        
        // Get device name
        if (ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name) >= 0) {
            printf("Checking %s: %s\n", device_path, device_name);
            
            // Check if this is the Sense HAT joystick
            if (strstr(device_name, "Sense HAT Joystick") != NULL ||
                strstr(device_name, "sensehat") != NULL) {
                printf("Found Sense HAT joystick at %s\n", device_path);
                
                // Reopen in blocking mode for reading events
                close(fd);
                fd = open(device_path, O_RDONLY);
                if (fd < 0) {
                    fprintf(stderr, "Failed to reopen %s: %s\n", 
                            device_path, strerror(errno));
                    return -1;
                }
                return fd;
            }
        }
        
        close(fd);
    }
    
    fprintf(stderr, "Sense HAT joystick not found\n");
    return -1;
}

int joystick_init(void) {
    return find_joystick_device();
}

int joystick_read_event(int fd, joystick_event_t *event) {
    struct input_event ev;
    ssize_t bytes;
    
    if (fd < 0 || event == NULL) {
        return -1;
    }
    
    // Read events until we get a key event
    while (1) {
        bytes = read(fd, &ev, sizeof(struct input_event));
        
        if (bytes < (ssize_t)sizeof(struct input_event)) {
            if (bytes < 0) {
                fprintf(stderr, "Error reading joystick: %s\n", strerror(errno));
            }
            return -1;
        }

        printf("Event: type=%d, code=%d, value=%d\n", ev.type, ev.code, ev.value);
        
        // We only care about key events (EV_KEY)
        if (ev.type == EV_KEY) {
            event->code = ev.code;
            event->value = ev.value;
            return 0;
        }
    }
}

const char* joystick_code_to_string(uint16_t code) {
    switch (code) {
        case JS_UP:    return "UP";
        case JS_DOWN:  return "DOWN";
        case JS_LEFT:  return "LEFT";
        case JS_RIGHT: return "RIGHT";
        case JS_ENTER: return "ENTER";
        default:       return "UNKNOWN";
    }
}

const char* joystick_value_to_string(int32_t value) {
    switch (value) {
        case JS_RELEASED: return "RELEASED";
        case JS_PRESSED:  return "PRESSED";
        case JS_HOLD:     return "HOLD";
        default:          return "UNKNOWN";
    }
}

void joystick_close(int fd) {
    if (fd >= 0) {
        close(fd);
        printf("Joystick closed\n");
    }
}