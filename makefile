CC = gcc

PYTHON = python3
PY_CFLAGS = $(shell $(PYTHON)-config --includes)
PY_EXT = $(shell $(PYTHON)-config --extension-suffix)
PY_LDFLAGS = $(shell $(PYTHON)-config --ldflags)

CFLAGS = -std=c17 -Wall -Wextra -fPIC $(PY_CFLAGS)
LDFLAGS = -shared $(PY_LDFLAGS)
LIBS = -lzip -lm
SRC_DIR = skrash/vm/_native
TEST_DIR = tests
BUILD_DIR = build

TARGET = $(SRC_DIR)/_core$(PY_EXT)

SOURCES = $(shell find $(SRC_DIR) -name '*.c' ! -name 'module.c')
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

MODULE_OBJECT = $(BUILD_DIR)/module.o

C_TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
C_TESTS = $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%.bin,$(C_TEST_SOURCES))

.PHONY: all test ctest pytest clean

all: $(TARGET) $(C_TESTS)

$(TARGET): $(OBJECTS) $(MODULE_OBJECT)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(BUILD_DIR)/%.bin: $(TEST_DIR)/%.c $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) -std=c17 -Wall -Wextra -I$(SRC_DIR) $^ $(LIBS) -o $@

ctest: $(C_TESTS)
	@for test in $(C_TESTS); do \
		echo "=== $$test ==="; \
		./$$test || exit 1; \
	done

pytest: $(TARGET)
	$(PYTHON) -m pytest -s

test: all
	@$(MAKE) ctest
	@echo ""
	@echo "=== Python tests ==="
	$(PYTHON) -m pytest -s

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	find . -type d -name "__pycache__" -exec rm -rf {} +
	find . -type f -name "*.pyc" -delete
	find . -type f -name "*.pyo" -delete