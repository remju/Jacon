CC = gcc
CFLAGS = -Wall -Wextra -ggdb -Wswitch-enum

TARGET=jacon

LOG_DIR=./logs
TEST_DIR=./tests

TEST_TARGET=test

VALIDATION_LOG_FILE=$(LOG_DIR)/validation.log
VALGRIND_LOG_FILE=$(LOG_DIR)/valgrind.log
TEST_LOG_FILE=$(LOG_DIR)/test.log


VALGRIND_CMD = valgrind --leak-check=full --log-file=$(VALGRIND_LOG_FILE)

REPO_URL=https://github.com/nst/JSONTestSuite.git
LOCAL_REPO_DIR=$(shell pwd)/JSONTestSuite

ACCEPTED_FILES=$(shell find $(LOCAL_REPO_DIR)/test_parsing/y_*.json)  # Accepted files
REJECTED_FILES=$(shell find $(LOCAL_REPO_DIR)/test_parsing/n_*.json)  # Rejected files
CANPASSORFAIL_FILES=$(shell find $(LOCAL_REPO_DIR)/test_parsing/i_*.json)  # Might be accepted or rejected files

all: $(TARGET)

jacon: jacon.c jacon.h
	$(CC) $(CFLAGS) -o $(TARGET) jacon.c

test: test.c jacon.h
	$(CC) $(CFLAGS) -o $(TEST_TARGET) test.c
	./$(TEST_TARGET)

# Check if the repository is already downloaded
check_validation_repo_exists:
	@if [ ! -d $(LOCAL_REPO_DIR) ]; then \
		echo "Repository not found. Cloning the JSONTestSuite into $(LOCAL_REPO_DIR)..."; \
		git clone $(REPO_URL) $(LOCAL_REPO_DIR) || { echo "Failed to clone repository."; exit 1; }; \
		echo "Validation files downloaded successfully."; \
	else \
		echo "Validation files already exist in $(LOCAL_REPO_DIR). Skipping download."; \
	fi

validation: $(TARGET) check_validation_repo_exists
	@mkdir -p $(LOG_DIR)  # Ensure the logs directory exists
	@echo "Starting validations..." > $(VALIDATION_LOG_FILE)  # Create or overwrite the log file

	# Run validations for accepted files (should pass)
	@for file in $(ACCEPTED_FILES); do \
		echo "Running validation on accepted file $$file"; \
		if ./$(TARGET) $$file >> $(VALIDATION_LOG_FILE) 2>&1; then \
			echo "Validation passed on $$file" >> $(VALIDATION_LOG_FILE); \
		else \
			echo "Validation failed on $$file" >> $(VALIDATION_LOG_FILE); \
			exit 1; \
		fi; \
	done

	# Run validations for rejected files (should not pass with code 0)
	@for file in $(REJECTED_FILES); do \
		echo "Running validation on rejected file $$file"; \
		if ./$(TARGET) $$file >> $(VALIDATION_LOG_FILE) 2>&1; then \
			echo "Validation unexpectedly passed on $$file" >> $(VALIDATION_LOG_FILE); \
			exit 1; \
		else \
			echo "Validation correctly failed on $$file (exit code $$?)" >> $(VALIDATION_LOG_FILE); \
		fi; \
	done

	# Run validations for might-be-accepted or rejected files (can pass or fail)
	@for file in $(CANPASSORFAIL_FILES); do \
		echo "Running validation on might-be-accepted file $$file"; \
		if ./$(TARGET) $$file >> $(VALIDATION_LOG_FILE) 2>&1; then \
			echo "Validation passed on $$file" >> $(VALIDATION_LOG_FILE); \
		else \
			echo "Validation failed on $$file (which is allowed to fail)" >> $(VALIDATION_LOG_FILE); \
		fi; \
	done

	@echo "validations completed. See $(VALIDATION_LOG_FILE) for details." >> $(VALIDATION_LOG_FILE)

valgrind_validation: $(TARGET)
	@mkdir -p $(LOG_DIR)  # Ensure the logs directory exists
	@echo "Running validations with Valgrind..." > $(VALGRIND_LOG_FILE)

	# Run validations for accepted files (should pass)
	@for file in $(ACCEPTED_FILES); do \
		echo "Running validation on accepted file $$file using Valgrind"; \
		$(VALGRIND_CMD) ./$(TARGET) $$file >> $(VALGRIND_LOG_FILE) 2>&1; \
		if grep -q "ERROR SUMMARY: 0 errors from 0 contexts" $(VALGRIND_LOG_FILE); then \
			echo "Valgrind passed for $$file" >> $(VALGRIND_LOG_FILE); \
		else \
			echo "Valgrind found errors for $$file" >> $(VALGRIND_LOG_FILE); \
			exit 1; \
		fi; \
	done

	# Run validations for rejected files (should not pass with code 0)
	@for file in $(REJECTED_FILES); do \
		echo "Running validation on rejected file $$file using Valgrind"; \
		$(VALGRIND_CMD) ./$(TARGET) $$file >> $(VALGRIND_LOG_FILE) 2>&1; \
		if grep -q "ERROR SUMMARY: 0 errors from 0 contexts" $(VALGRIND_LOG_FILE); then \
			echo "Valgrind passed for $$file" >> $(VALGRIND_LOG_FILE); \
		else \
			echo "Valgrind found errors for $$file" >> $(VALGRIND_LOG_FILE); \
			exit 1; \
		fi; \
	done

	# Run validations for might-be-accepted or rejected files (can pass or fail)
	@for file in $(CANPASSORFAIL_FILES); do \
		echo "Running validation on might-be-accepted file $$file using Valgrind"; \
		$(VALGRIND_CMD) ./$(TARGET) $$file >> $(VALGRIND_LOG_FILE) 2>&1; \
		if grep -q "ERROR SUMMARY: 0 errors from 0 contexts" $(VALGRIND_LOG_FILE); then \
			echo "Valgrind passed for $$file" >> $(VALGRIND_LOG_FILE); \
		else \
			echo "Valgrind found errors for $$file (which is allowed to fail)" >> $(VALGRIND_LOG_FILE); \
		fi; \
	done

	@echo "Valgrind validation completed. See $(VALGRIND_LOG_FILE) for details."

clean:
	@echo "Cleaning up executable..."
	@rm -f $(TARGET)
	@echo "Executable removed."
	@echo "Cleaning up test executable..."
	@rm -f $(TEST_TARGET)
	@echo "Test executable removed."
	@echo "Cleaning up temporary validation files..."
	@rm -rf $(LOCAL_REPO_DIR)
	@echo "Temporary validation files removed."
	@echo "Cleaning up logs dir..."
	@rm -rf $(LOG_DIR)
	@echo "Logs dir removed."