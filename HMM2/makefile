# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -fPIC
LDFLAGS = -shared

# File Names
LIBRARY = libhmm
LIBRARY_SO = $(LIBRARY).so
OBJECTS = heap.o

# Default target
all: $(LIBRARY_SO)

# Build the shared library
$(LIBRARY_SO): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJECTS) $(LIBRARY_SO)

# Phony targets
.PHONY: all clean

