CC = gcc
# CFLAGS = -Wall -Wextra -Wshadow -Wformat=2  \
# 				 -Wpointer-arith -Wcast-qual -Wwrite-strings -fsanitize=address,undefined \
# 				 -g3 -DDEBUG -O0 -Iinclude -pthread
# these are annoying me -Wconversion -Wsign-conversion
# When we finish we can use these flags instead
CFLAGS = -Wall -Wextra -O2 -flto -march=native -fstack-protector-strong -D_FORTIFY_SOURCE=2
LDFLAGS = -pthread

DEBUG ?= 0

ifeq ($(DEBUG),1)
    CFLAGS += -g -O0 -DDEBUG
    BUILD_MODE = Debug
else
    BUILD_MODE = Release
endif

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

# Run comparison of all algorithms
compare: $(DEMO)
	@echo "Running All Scheduling Algorithms Comparison..."
	@echo ""
	./$(DEMO) --compare-all --export-csv comparison_results.csv programs/*.asm

# Run specific examples
example-fcfs: $(DEMO)
	@echo "Running FCFS Example..."
	./$(DEMO) --fcfs programs/hello_world.asm programs/factorial.asm programs/goodbye_planet.asm

example-rr: $(DEMO)
	@echo "Running Round Robin Example..."
	./$(DEMO) --round-robin programs/hello_world.asm programs/factorial.asm programs/goodbye_planet.asm

example-priority: $(DEMO)
	@echo "Running Priority Scheduling Example..."
	./$(DEMO) --priority programs/hello_world.asm programs/factorial.asm programs/goodbye_planet.asm

example-spn: $(DEMO)
	@echo "Running SPN Example..."
	./$(DEMO) --spn programs/hello_world.asm programs/factorial.asm programs/goodbye_planet.asm

# Clean build artifacts
clean:
	rm -f $(CORE_OBJS) $(MAIN_OBJ) $(TEST_OBJ) $(DEMO) $(TEST)
	rm -f *.csv
	@echo "✓ Build artifacts cleaned"

# Help target
help:
	@echo "Available targets:"
	@echo "  make              - Build both demo and test executables"
	@echo "  make demo         - Build only the demo executable"
	@echo "  make test         - Build only the test executable"
	@echo "  make run-demo     - Build and run the demo"
	@echo "  make run-test     - Build and run the tests"
	@echo "  make run-all      - Run tests then demo"
	@echo ""
	@echo "Performance Analysis:"
	@echo "  make compare      - Run all algorithms and generate comparison report"
	@echo "  make example-fcfs - Run FCFS with sample programs"
	@echo "  make example-rr   - Run Round Robin with sample programs"
	@echo "  make example-priority - Run Priority scheduling with sample programs"
	@echo "  make example-spn  - Run SPN with sample programs"
	@echo ""
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make help         - Show this help message"

.PHONY: all clean run-demo run-test run-all compare example-fcfs example-rr example-priority example-spn help
