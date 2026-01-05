# Usage:
# make help      show help
# make        	 compile all binaries
# make run       compile and run program
# make tidy      static analysis
# make format    source format, see https://github.com/motine/cppstylelineup for style details
# make valgrind	 dynamic analysis
# make clean  	 remove all binaries and objects

BINARY := bin/magnify
SRCS := $(wildcard src/*.c)
INCLUDES := -Iinclude

# Flags
WARNINGS := -Wall -Wextra -Wwrite-strings -Wconversion -Wshadow -Wstrict-prototypes -Wundef
STD := -std=c11 -O3 -pedantic
CFLAGS := -g $(INCLUDES) $(WARNINGS) $(STD)
LFLAGS :=  -lX11 -D_GNU_SOURCE

# Tools
ANALYZE := scan-build
CC := clang
FORMAT := clang-format
TIDY := clang-tidy
VALGRIND := valgrind

# Default target
all: create_dirs compile

clean:
	@echo "Cleaning up..."
	@rm -f bin/$(BINARY) || true

compile:
	clear
	@echo "Compiling $(BINARY)..."
	@$(CC) $(CFLAGS) $(LFLAGS) -o $(BINARY) $(SRCS)
	@echo "Build complete."
	@echo "--------------------------------------------------------------------------------"

create_dirs:
	@echo "Creating directories..."
	@mkdir -p bin src

format:
	@echo "🖌️  Running clang-format..."
	@$(FORMAT) -i $(SRCS) -style="{BasedOnStyle: LLVM, IndentWidth: 4}"

help:
	@sed -n '1,8p' $(MAKEFILE_LIST)

run: all
	@./$(BINARY)

tidy:
	@echo "🔍 Running clang-tidy..."
	@$(TIDY) $(SRCS) -- $(CFLAGS)

valgrind: all
	@echo "🔍 Running Valgrind Dynamic Analyzer / Runtime Checker..."
	@$(VALGRIND) --leak-check=full --track-origins=yes ./$(BINARY)

