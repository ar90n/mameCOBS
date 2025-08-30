CXX ?= g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic -O2 -Isrc
TEST_FLAGS = -Itests

# Output directories
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj

# Source files
SAMPLE_SRCS = samples/enc.cpp samples/dec.cpp
# Only use working tests for the new chunk-of-chunks architecture  
TEST_SRCS = tests/test_all.cpp tests/test_vector_free.cpp tests/test_incremental.cpp tests/test_encode.cpp tests/test_decode.cpp tests/test_roundtrip.cpp
SAMPLES = $(patsubst samples/%.cpp,$(BIN_DIR)/%,$(SAMPLE_SRCS))
TESTS = $(BIN_DIR)/test_runner

# Default target
all: samples tests

# Create directories
$(BUILD_DIR) $(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# Build samples
samples: $(SAMPLES)

$(BIN_DIR)/%: samples/%.cpp src/mameCOBS.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Build tests
tests: $(TESTS)

$(BIN_DIR)/test_runner: $(TEST_SRCS) src/mameCOBS.hpp tests/utest.h | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_FLAGS) -o $@ $(TEST_SRCS)

# Run tests
test: tests
	@echo "Running tests..."
	@$(TESTS)

# Format code
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Formatting code..."; \
		find src samples tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i; \
	else \
		echo "clang-format not found. Please install it."; \
	fi

# Check format
check-format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Checking code format..."; \
		find src samples tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run --Werror; \
	else \
		echo "clang-format not found. Please install it."; \
	fi

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)
	@rm -f simple hierarchical

# Help
help:
	@echo "mameCOBS Makefile targets:"
	@echo "  all          - Build everything (default)"
	@echo "  samples      - Build sample programs"
	@echo "  tests        - Build test suite"
	@echo "  test         - Run tests"
	@echo "  format       - Format code with clang-format"
	@echo "  check-format - Check code formatting"
	@echo "  clean        - Remove build artifacts"
	@echo "  help         - Show this help"

.PHONY: all samples tests test format check-format clean help