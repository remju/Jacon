CC = gcc
CFLAGS = -Wall -Wextra -ggdb -Wswitch-enum

TARGET=jacon

INPUT_FILES=$(shell find ./test-files/*.json)

all: $(TARGET)

jacon: jacon.c jacon.h
	$(CC) $(CFLAGS) -o $(TARGET) jacon.c

test: $(TARGET)
	@for file in $(INPUT_FILES); do \
		echo "Running test on $$file"; \
		./$(TARGET) $$file || { echo "Test failed on $$file"; exit 1; }; \
	done

valgrind: $(TARGET)
	@for file in $(INPUT_FILES); do \
		echo "Running valgrind on $$file"; \
		valgrind --leak-check=full ./$(TARGET) $$file; \
	done

clean:
	rm -f $(TARGET)