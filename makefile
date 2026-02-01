CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -Iinclude/drivers -Iinclude/network -Iinclude/sensors
LDFLAGS = -li2c -lpthread

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

SOURCES = $(shell find $(SRC_DIR) -name '*.c')

OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

BUILD_DIRS = $(sort $(dir $(OBJECTS)))

TARGET = $(BIN_DIR)/lab1

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

rebuild: clean all

IP ?= 127.0.0.1
PORT ?= 1234

run: $(TARGET)
	sudo $(TARGET) $(IP) $(PORT)

.PHONY: all clean rebuild run