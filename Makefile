# Compiler settings
CC = gcc
# -Iinc tells the compiler to look for header files in the 'inc' directory
CFLAGS = -Wall -Wextra -O2 -Iinc
# Link the math library for pressure/altitude calculations
LDFLAGS = -lm

# Name of your final executable
TARGET = bmp180_test

# List of source files with their relative paths
SRCS = main.c \
       src/bmp180.c \
       src/i2c_bus.c

# Automatically generate a list of object files (.o) from source files (.c)
OBJS = $(SRCS:.c=.o)

# The default rule (first one) is what runs when you just type 'make'
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Pattern rule to compile .c files into .o files
# This handles files in both the root and the src/ directory
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean