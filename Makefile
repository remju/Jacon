CC = gcc
CFLAGS = -Wall -Wextra -ggdb -Wswitch-enum

TARGET=jacon

LOG_DIR=./logs
LOG_FILE=$(LOG_DIR)/test.log

VALGRIND_CMD = valgrind --leak-check=full --log-file=$(LOG_DIR)/valgrind.log

REPO_URL=https://github.com/nst/JSONTestSuite.git
LOCAL_REPO_DIR=$(shell pwd)/JSONTestSuite

ACCEPTED_FILES=$(shell find $(LOCAL_REPO_DIR)/test_parsing/y_*.json)  # Accepted files
REJECTED_FILES=$(shell find $(LOCAL_REPO_DIR)/test_parsing/n_*.json)  # Rejected files
CANPASSORFAIL_FILES=$(shell find $(LOCAL_REPO_DIR)/test_parsing/i_*.json)  # Might be accepted or rejected files

all: $(TARGET)

jacon: jacon.c jacon.h
	$(CC) $(CFLAGS) -o $(TARGET) jacon.c

# Check if the repository is already downloaded
check_test_repo_exists:
	@if [ ! -d $(LOCAL_REPO_DIR) ]; then \
		echo "Repository not found. Cloning the JSONTestSuite into $(LOCAL_REPO_DIR)..."; \
		git clone $(REPO_URL) $(LOCAL_REPO_DIR) || { echo "Failed to clone repository."; exit 1; }; \
		echo "Test files downloaded successfully."; \
	else \
		echo "Test files already exist in $(LOCAL_REPO_DIR). Skipping download."; \
	fi

test: $(TARGET) check_test_repo_exists
	@mkdir -p $(LOG_DIR)  # Ensure the logs directory exists
	@echo "Starting tests..." > $(LOG_FILE)  # Create or overwrite the log file

	# Run tests for accepted files (should pass)
	@for file in $(ACCEPTED_FILES); do \
		echo "Running test on accepted file $$file"; \
		if ./$(TARGET) $$file >> $(LOG_FILE) 2>&1; then \
			echo "Test passed on $$file" >> $(LOG_FILE); \
		else \
			echo "Test failed on $$file" >> $(LOG_FILE); \
			exit 1; \
		fi; \
	done

	# Run tests for rejected files (should not pass with code 0)
	@for file in $(REJECTED_FILES); do \
		echo "Running test on rejected file $$file"; \
		if ./$(TARGET) $$file >> $(LOG_FILE) 2>&1; then \
			echo "Test unexpectedly passed on $$file" >> $(LOG_FILE); \
			exit 1; \
		else \
			echo "Test correctly failed on $$file (exit code $$?)" >> $(LOG_FILE); \
		fi; \
	done

	# Run tests for might-be-accepted or rejected files (can pass or fail)
	@for file in $(CANPASSORFAIL_FILES); do \
		echo "Running test on might-be-accepted file $$file"; \
		if ./$(TARGET) $$file >> $(LOG_FILE) 2>&1; then \
			echo "Test passed on $$file" >> $(LOG_FILE); \
		else \
			echo "Test failed on $$file (which is allowed to fail)" >> $(LOG_FILE); \
		fi; \
	done

	@echo "Tests completed. See $(LOG_FILE) for details." >> $(LOG_FILE)

valgrind_test: $(TARGET)
	@mkdir -p $(LOG_DIR)  # Ensure the logs directory exists
	@echo "Running tests with Valgrind..." > $(LOG_DIR)/valgrind.log

	# Run tests for accepted files (should pass)
	@for file in $(ACCEPTED_FILES); do \
		echo "Running test on accepted file $$file using Valgrind"; \
		$(VALGRIND_CMD) ./$(TARGET) $$file >> $(LOG_DIR)/valgrind.log 2>&1; \
		if grep -q "ERROR SUMMARY: 0 errors from 0 contexts" $(LOG_DIR)/valgrind.log; then \
			echo "Valgrind passed for $$file" >> $(LOG_DIR)/valgrind.log; \
		else \
			echo "Valgrind found errors for $$file" >> $(LOG_DIR)/valgrind.log; \
			exit 1; \
		fi; \
	done

	# Run tests for rejected files (should not pass with code 0)
	@for file in $(REJECTED_FILES); do \
		echo "Running test on rejected file $$file using Valgrind"; \
		$(VALGRIND_CMD) ./$(TARGET) $$file >> $(LOG_DIR)/valgrind.log 2>&1; \
		if grep -q "ERROR SUMMARY: 0 errors from 0 contexts" $(LOG_DIR)/valgrind.log; then \
			echo "Valgrind passed for $$file" >> $(LOG_DIR)/valgrind.log; \
		else \
			echo "Valgrind found errors for $$file" >> $(LOG_DIR)/valgrind.log; \
			exit 1; \
		fi; \
	done

	# Run tests for might-be-accepted or rejected files (can pass or fail)
	@for file in $(CANPASSORFAIL_FILES); do \
		echo "Running test on might-be-accepted file $$file using Valgrind"; \
		$(VALGRIND_CMD) ./$(TARGET) $$file >> $(LOG_DIR)/valgrind.log 2>&1; \
		if grep -q "ERROR SUMMARY: 0 errors from 0 contexts" $(LOG_DIR)/valgrind.log; then \
			echo "Valgrind passed for $$file" >> $(LOG_DIR)/valgrind.log; \
		else \
			echo "Valgrind found errors for $$file (which is allowed to fail)" >> $(LOG_DIR)/valgrind.log; \
		fi; \
	done

	@echo "Valgrind test completed. See $(LOG_DIR)/valgrind.log for details."

clean:
	@rm -f $(TARGET)
	@echo "Cleaning up temporary test files..."
	@rm -rf $(LOCAL_REPO_DIR)
	@echo "Temporary test files removed."
	@rm -rf $(LOG_DIR)