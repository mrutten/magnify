BIN_DIR := bin/
BINARY := magnify
INSTALL_DIR = /usr/local/bin/

SRCS := $(wildcard src/*.c)
INCLUDES := -Iinclude

# Flags
WARNINGS := -Wall -Wextra -Wwrite-strings -Wconversion -Wshadow -Wstrict-prototypes -Wundef
STD := -std=c11 -O3 -pedantic
CFLAGS := $(INCLUDES) $(WARNINGS) $(STD)
LFLAGS :=  -lX11 -D_GNU_SOURCE
DEBUG := -g

# Tools
ANALYZE := scan-build
CC := clang
FORMAT := clang-format
TIDY := clang-tidy
VALGRIND := valgrind

# Default target
all: compile

clean:
	@echo "Cleaning up $(BIN_DIR)$(BINARY)"
	@rm -f $(BIN_DIR)$(BINARY) || true

compile: create_dirs clean
	@clear
	@echo "Compiling $(SRCS)"
	@$(CC) $(CFLAGS) $(LFLAGS) -o $(BIN_DIR)$(BINARY) $(SRCS)
	@echo "Build of $(BIN_DIR)$(BINARY) complete"
	@echo "--------------------------------------------------------------------------------"

create_dirs:
	@echo "Creating directories"
	@mkdir -p bin src

debug: create_dirs clean
	@clear
	@echo "Compiling $(SRCS) with debug symbols"
	@$(CC) $(CFLAGS) $(DEBUG) $(LFLAGS) -o $(BIN_DIR)$(BINARY) $(SRCS)
	@echo "Build of $(BIN_DIR)$(BINARY) complete"
	@echo "--------------------------------------------------------------------------------"

format:
	@echo "Running clang-format"
	@$(FORMAT) -i $(SRCS) -style="{BasedOnStyle: LLVM, IndentWidth: 4}"

help:
	@echo "Usage:"
	@echo ""
	@echo "Installation:"
	@echo "make && sudo make install"
	@echo ""
	@echo "Options:"
	@echo "make           compilation"
	@echo "make clean     remove binary and objects"
	@echo "make debug     compile binary with debug symbols"
	@echo "make format    source format, see https://github.com/motine/cppstylelineup for style details"
	@echo "make help      show this help"
	@echo "make run       compile and run program"
	@echo "make tidy      static analysis"
	@echo "make valgrind  dynamic analysis"
	@echo ""

install:
	@echo "Copying $(BIN_DIR)$(BINARY) to $(INSTALL_DIR)$(BINARY)"
	@cp $(BIN_DIR)$(BINARY) $(INSTALL_DIR)

run: all
	@./$(BIN_DIR)$(BINARY)

tidy:
	@echo "Running clang-tidy"
	@$(TIDY) $(SRCS) -- $(CFLAGS)

valgrind: all
	@echo "Running Valgrind Dynamic Analyzer / Runtime Checker"
	@$(VALGRIND) --leak-check=full --track-origins=yes $(BINDIR)$(BINARY)

