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

test: $(TARGET)
	./$(TARGET)

debug: CFLAGS += -DRC_DEBUG_MODE
debug: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

clean:
	rm -f $(TARGET)

help:
	@echo "Available targets:"
	@echo "  all      - Build the test executable"
	@echo "  test     - Build and run tests"
	@echo "  debug    - Build and run tests with debug output"
	@echo "  valgrind - Run tests with memory leak detection"
	@echo "  clean    - Remove built files"
	@echo "  help     - Show this help message"

.PHONY: all test debug valgrind clean help
