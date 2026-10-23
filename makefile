CC = gcc

PYTHON = python3
PY_CFLAGS = $(shell $(PYTHON)-config --includes)
PY_EXT = $(shell $(PYTHON)-config --extension-suffix)
PY_LDFLAGS = $(shell $(PYTHON)-config --ldflags)

CFLAGS = -std=c17 -Wall -Wextra -fPIC $(PY_CFLAGS) -D_POSIX_C_SOURCE=200809L
LDFLAGS = -shared $(PY_LDFLAGS)
LIBS = -lzip -lm
SRC_DIR = skrash/vm/_native
RENDER_DIR = skrash/renderer
TEST_DIR = tests
BUILD_DIR = build

TARGET = $(SRC_DIR)/_core$(PY_EXT)

# Sources de la lib : tous les .c sauf module.c et player.c
SOURCES = $(shell find $(SRC_DIR) -name '*.c' ! -name 'module.c' ! -name 'player.c')
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

MODULE_OBJECT = $(BUILD_DIR)/module.o

# Headers de la lib
HEADERS = $(shell find $(SRC_DIR) -name '*.h')

# Renderer : découverte dynamique
RENDER_SOURCES = $(shell find $(RENDER_DIR) -name '*.c')
RENDER_OBJECTS = $(patsubst $(RENDER_DIR)/%.c,$(BUILD_DIR)/renderer/%.o,$(RENDER_SOURCES))
RENDER_HEADERS = $(shell find $(RENDER_DIR) -name '*.h')

# Tests C
C_TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
C_TESTS = $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%.bin,$(C_TEST_SOURCES))

# Binaire renderer
PLAY_TARGET = $(BUILD_DIR)/skrash-play
PLAY_OBJECTS = $(OBJECTS) $(RENDER_OBJECTS)
PLAY_LIBS = $(LIBS) -lSDL2

.PHONY: all test ctest pytest clean play

all: $(TARGET) $(C_TESTS)

# ---------------- shared library (Python) ----------------

$(TARGET): $(OBJECTS) $(MODULE_OBJECT)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

# ---------------- objects lib ----------------

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

# ---------------- objects renderer ----------------

$(BUILD_DIR)/renderer/%.o: $(RENDER_DIR)/%.c $(HEADERS) $(RENDER_HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(RENDER_DIR) -c $< -o $@

# ---------------- C tests ----------------

$(BUILD_DIR)/%.bin: $(TEST_DIR)/%.c $(OBJECTS) $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -std=c17 -Wall -Wextra -I$(SRC_DIR) $(TEST_DIR)/$*.c $(OBJECTS) $(LIBS) -o $@

ctest: $(C_TESTS)
	@for test in $(C_TESTS); do \
		echo "=== $$test ==="; \
		./$$test || exit 1; \
	done

# ---------------- python tests ----------------

pytest: $(TARGET)
	$(PYTHON) -m pytest -s

test: all
	@$(MAKE) ctest
	@echo ""
	@echo "=== Python tests ==="
	$(PYTHON) -m pytest -s

# ---------------- renderer binary ----------------

$(PLAY_TARGET): $(PLAY_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(PLAY_OBJECTS) $(PLAY_LIBS) -o $@

play: $(PLAY_TARGET)

# ---------------- clean ----------------

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	find . -type d -name "__pycache__" -exec rm -rf {} +
	find . -type f -name "*.pyc" -delete
	find . -type f -name "*.pyo" -delete