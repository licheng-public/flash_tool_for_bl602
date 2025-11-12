# === Top-Level Makefile for flash_tool_for_bl602 ===

# Compiler and flags
CC := gcc
CFLAGS := -Wall -O2 -Iinc -Icommon

# Common source files
COMMON_SRCS := $(wildcard common/*.c)
COMMON_SRCS := $(filter-out common/dump_%.c, $(COMMON_SRCS))
COMMON_OBJS := $(COMMON_SRCS:.c=.o)

# Output directory
BIN_DIR := bin

# Subdirectories (each builds an executable)
SUBDIRS := flash img_build partition

# Executable names
FLASH_EXE := $(BIN_DIR)/flash
IMG_BUILD_EXE := $(BIN_DIR)/img_gen
PARTITION_EXE := $(BIN_DIR)/partition_gen

# Image/config files to copy
IMAGE_CFG_SRC := $(wildcard image_and_config/*)
IMAGE_CFG_DST := $(patsubst image_and_config/%, $(BIN_DIR)/%, $(IMAGE_CFG_SRC))

# Collect all targets
TARGETS := $(FLASH_EXE) $(IMG_BUILD_EXE) $(PARTITION_EXE) $(IMAGE_CFG_DST)

.PHONY: all clean

all: $(BIN_DIR) $(TARGETS)
	@echo "Build complete. Executables and configs are in $(BIN_DIR)/"

# === Rules for each subproject ===
# Automatically exclude dump_*.c from each directory
FLASH_SRCS := $(filter-out flash/dump_%.c, $(wildcard flash/*.c))
IMG_BUILD_SRCS := $(filter-out img_build/dump_%.c, $(wildcard img_build/*.c))
PARTITION_SRCS := $(filter-out partition/dump_%.c, $(wildcard partition/*.c))

$(FLASH_EXE): $(COMMON_OBJS) $(FLASH_SRCS)
	$(CC) $(CFLAGS) $^ -o $@

$(IMG_BUILD_EXE): $(COMMON_OBJS) $(IMG_BUILD_SRCS)
	$(CC) $(CFLAGS) $^ -o $@

$(PARTITION_EXE): $(COMMON_OBJS) $(PARTITION_SRCS)
	$(CC) $(CFLAGS) $^ -o $@

# === Copy image_and_config files ===
$(BIN_DIR)/%: image_and_config/%
	cp $< $@

# Create bin directory if not exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean up all generated files
clean:
	rm -rf $(BIN_DIR) $(COMMON_OBJS) \
	       flash/*.o img_build/*.o partition/*.o
	@echo "Cleaned all build artifacts."
