CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -g -O0
LDFLAGS = 

ifeq ($(shell uname),Linux)
    LDFLAGS = -latomic
endif

TARGET = test_crc
SOURCE = test_crc.c

all: $(TARGET)

$(TARGET): $(SOURCE) crc.h third-party/miniunit.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

test: clear clean $(TARGET)
	./$(TARGET)

debug: CFLAGS += -DRC_DEBUG_MODE
debug: $(TARGET)
	./$(TARGET)

asan: CFLAGS += -fsanitize=address
asan: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

clear:
	clear

clean:
	rm -f $(TARGET)

help:
	@echo "Available targets:"
	@echo "  all      - Build the test executable"
	@echo "  test     - Clear screen, clean, build and run tests"
	@echo "  debug    - Build and run tests with debug output"
	@echo "  asan     - Build and run tests with AddressSanitizer"
	@echo "  valgrind - Run tests with memory leak detection"
	@echo "  clear    - Clear the screen"
	@echo "  clean    - Remove built files"
	@echo "  help     - Show this help message"

.PHONY: all test debug asan valgrind clear clean help
