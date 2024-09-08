# Makefile for HMM Library

CC = gcc
CFLAGS = -Wall -Wextra -fPIC
LDFLAGS = -shared
TARGET = libhmm.so
SOURCES = HMM.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
    $(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
    $(CC) $(CFLAGS) -c -o $@ $<

clean:
    rm -f $(TARGET) $(OBJECTS)
