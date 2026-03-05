CC = gcc
CFLAGS = -std=c11 -Wall
TARGET = main

.PHONY: all run clean

all: $(TARGET)

$(TARGET): main.c tlv_parser.c tlv_parser.h tlv_def.h
	$(CC) $(CFLAGS) main.c tlv_parser.c -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	-rm -f $(TARGET) $(TARGET).exe *.o
