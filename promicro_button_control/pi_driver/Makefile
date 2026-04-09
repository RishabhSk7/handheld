SHELL := /bin/sh

TARGET := pi_driver
SRC := main.cpp
BUILD_DIR := build

CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
LDFLAGS ?=

AARCH64_CXX ?= aarch64-linux-gnu-g++

NATIVE_OUT := $(BUILD_DIR)/$(TARGET)
AARCH64_OUT := $(BUILD_DIR)/$(TARGET)-aarch64

.PHONY: all help native aarch64 clean run dirs check-toolchain

all: native

help:
	@echo "Targets:"
	@echo "  make native    - Build for current machine"
	@echo "  make aarch64   - Cross-compile for aarch64 (requires $(AARCH64_CXX))"
	@echo "  make run       - Build native binary and run it"
	@echo "  make clean     - Remove build artifacts"

dirs:
	@mkdir -p $(BUILD_DIR)

native: dirs $(NATIVE_OUT)

$(NATIVE_OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

check-toolchain:
	@command -v $(AARCH64_CXX) >/dev/null 2>&1 || { \
		echo "Error: $(AARCH64_CXX) not found in PATH."; \
		echo "Install an aarch64 cross-compiler or set AARCH64_CXX=<compiler>."; \
		exit 1; \
	}

aarch64: check-toolchain dirs $(AARCH64_OUT)

$(AARCH64_OUT): $(SRC)
	$(AARCH64_CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

run: native
	./$(NATIVE_OUT)

clean:
	rm -rf $(BUILD_DIR)