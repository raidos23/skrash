CC = gcc

PYTHON = python3
PY_CFLAGS = $(shell $(PYTHON)-config --includes)
PY_EXT = $(shell $(PYTHON)-config --extension-suffix)
PY_LDFLAGS = $(shell $(PYTHON)-config --ldflags)

CFLAGS = -std=c17 -Wall -Wextra -fPIC $(PY_CFLAGS)
LDFLAGS = -shared $(PY_LDFLAGS)
LIBS = -lzip

SRC_DIR = skrash/vm/_native
BUILD_DIR = build
TARGET = $(SRC_DIR)/_core$(PY_EXT)

SOURCES = $(shell find $(SRC_DIR) -name '*.c')
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)

.PHONY: all clean