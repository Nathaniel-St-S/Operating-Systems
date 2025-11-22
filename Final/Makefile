CC = gcc
CFLAGS = -Wall -O3 -Iinclude -pthread
LDFLAGS = -pthread

# Source files
SRC_FILES = $(wildcard src/*.c)
MAIN_SRC = src/main.c
ifneq ($(wildcard $(MAIN_SRC)),)
MAIN_OBJ = $(MAIN_SRC:.c=.o)
else
MAIN_OBJ =
endif
CORE_SRC = $(filter-out $(MAIN_SRC), $(SRC_FILES))
CORE_OBJS = $(CORE_SRC:.c=.o)

# Test program
TEST_SRC = $(wildcard test/*.c)
TEST_OBJ = $(TEST_SRC:.c=.o)

# Executables
DEMO = demo
TEST = tests

# Default target: build both demo and test
all: $(DEMO) $(TEST)

# Build demo executable
ifeq ($(MAIN_OBJ),)
$(DEMO):
	@echo "Demo target unavailable: src/main.c not found"
else
$(DEMO): $(CORE_OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CORE_OBJS) $(MAIN_OBJ) $(LDFLAGS)
	@echo "✓ Demo executable built successfully"
endif

# Build test executable
$(TEST): $(CORE_OBJS) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CORE_OBJS) $(TEST_OBJ) $(LDFLAGS)
	@echo "✓ Test executable built successfully"

# Compile source files
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile main.c
$(MAIN_OBJ): $(MAIN_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile tests
test/%.o: test/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the demo
run-demo: $(DEMO)
	@echo "Running Operating System Simulator Demo..."
	@echo ""
	./$(DEMO)

# Run the tests
run-test: $(TEST)
	@echo "Running Test Suite..."
	@echo ""
	./$(TEST)

# Run both
run-all: run-test run-demo

# Clean build artifacts

clean:
	rm -f $(CORE_OBJS) $(MAIN_OBJ) $(TEST_OBJ) $(DEMO) $(TEST)
	@echo "✓ Build artifacts cleaned"

# Help target
help:
	@echo "Available targets:"
	@echo "  make          - Build both demo and test executables"
	@echo "  make demo     - Build only the demo executable"
	@echo "  make test     - Build only the test executable"
	@echo "  make run-demo - Build and run the demo"
	@echo "  make run-test - Build and run the tests"
	@echo "  make run-all  - Run tests then demo"
	@echo "  make clean    - Remove all build artifacts"
	@echo "  make help     - Show this help message"

.PHONY: all clean run-demo run-test run-all help
