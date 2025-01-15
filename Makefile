CC = gcc
CFLAGS = -Wall -Wextra -ggdb -Wswitch-enum

TARGET=jacon

all: $(TARGET)

jacon: jacon.c jacon.h
	$(CC) $(CFLAGS) -o $(TARGET) jacon.c

test: $(TARGET)
	@for file in $(INPUT_FILES); do \
		echo "Running test on $$file"; \
		./$(TARGET) $$file || { echo "Test failed on $$file"; exit 1; }; \
	done

# Directories and file sets
VALIDATION_DIR=./testdata/JSONTestSuite/test_parsing
REPO_URL=https://github.com/nst/JSONTestSuite.git
LOCAL_REPO_DIR=$(shell pwd)/JSONTestSuite
ACCEPTED_FILES=$(shell find $(LOCAL_REPO_DIR)/test_parsing/y_*.json)  # Accepted files
REJECTED_FILES=$(shell find $(LOCAL_REPO_DIR)/test_parsing/n_*.json)  # Rejected files
CANPASSORFAIL_FILES=$(shell find $(LOCAL_REPO_DIR)/test_parsing/i_*.json)  # Might be accepted or rejected files

# Check if the repository is already downloaded
check_repo_exists:
	@if [ ! -d $(LOCAL_REPO_DIR) ]; then \
		echo "Repository not found. Cloning the JSONTestSuite into $(LOCAL_REPO_DIR)..."; \
		git clone $(REPO_URL) $(LOCAL_REPO_DIR) || { echo "Failed to clone repository."; exit 1; }; \
		echo "Test files downloaded successfully."; \
	else \
		echo "Test files already exist in $(LOCAL_REPO_DIR). Skipping download."; \
	fi

# Validation process
validation: $(TARGET) check_repo_exists
	@echo "Running validation tests..."

	# Run tests for accepted files (should pass)
	@for file in $(ACCEPTED_FILES); do \
		echo "Running test on accepted file $$file"; \
		./$(TARGET) $$file || { echo "Test failed on $$file"; exit 1; }; \
	done
	
	# Run tests for rejected files (should fail)
	@for file in $(REJECTED_FILES); do \
		echo "Running test on rejected file $$file"; \
		./$(TARGET) $$file && { echo "Test unexpectedly passed on $$file"; exit 1; }; \
	done
	
	# Run tests for might be accepted or rejected files (can pass or fail)
	@for file in $(CANPASSORFAIL_FILES); do \
		echo "Running test on might-be-accepted file $$file"; \
		./$(TARGET) $$file || { echo "Test failed on $$file (which is allowed to fail)"; }; \
	done

clean:
	@rm -f $(TARGET)
	@echo "Cleaning up temporary test files..."
	@rm -rf $(LOCAL_REPO_DIR)
	@echo "Temporary test files removed."