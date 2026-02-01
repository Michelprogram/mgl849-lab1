#define _GNU_SOURCE
#include "keyboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <strings.h> 

#define MAX_EVENT_DEVICES 20


int find_keyboard_device(void) {
    char device_path[256];
    char device_name[256];
    int fd;
    
    printf("Searching for keyboard device...\n");
    
    for (int i = 0; i < MAX_EVENT_DEVICES; i++) {
        snprintf(device_path, sizeof(device_path), "/dev/input/event%d", i);
        
        fd = open(device_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            continue;
        }
        
        if (ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name) >= 0) {

            if (strcasestr(device_name, "keyboard") != NULL) {
                printf("Found keyboard at %s: %s\n", device_path, device_name);
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
    
    fprintf(stderr, "Keyboard not found\n");
    return -1;
}

int keyboard_init(void) {
    return find_keyboard_device();
}

int keyboard_read_event(int fd, keyboard_event_t *event) {
    struct input_event ev;
    ssize_t bytes;
    
    if (fd < 0 || event == NULL) {
        return -1;
    }
    
    while (1) {
        bytes = read(fd, &ev, sizeof(struct input_event));
        
        if (bytes < (ssize_t)sizeof(struct input_event)) {
            if (bytes < 0) {
                fprintf(stderr, "Error reading keyboard: %s\n", strerror(errno));
            }
        }

        if (ev.type == EV_KEY && ev.value == MODIFIER_KEY_PRESSED && (ev.code == MODIFIER_KEY_PLUS || ev.code == MODIFIER_KEY_UP || ev.code == MODIFIER_KEY_KPPLUS)) {
            event->code = ev.code;
            event->value = ev.value;
            event->is_plus = 1;
            return 0;
        }

        if (ev.type == EV_KEY && ev.value == MODIFIER_KEY_PRESSED && (ev.code == MODIFIER_KEY_MINUS || ev.code == MODIFIER_KEY_DOWN || ev.code == MODIFIER_KEY_KPMINUS)) {
            event->code = ev.code;
            event->value = ev.value;
            event->is_plus = 0;
            return 0;
        }
    }
}

void keyboard_close(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}