CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -li2c

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/lps25h.c $(SRC_DIR)/socket.c $(SRC_DIR)/joystick.c $(SRC_DIR)/i2c.c $(SRC_DIR)/hts221.c
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BIN_DIR)/lab1

# Default target
all: $(BUILD_DIR) $(BIN_DIR) $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	@echo "Clean complete"

# Rebuild everything
rebuild: clean all

run:
	$(TARGET)

# Phony targets
.PHONY: all clean rebuild