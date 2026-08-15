.PHONY: all build dev test run compiledb compiledb-commands check format clean help

MAKEFLAGS += -j

C_SOURCES := main.c src/rt.c
COMPILEDB_TARGETS := $(C_SOURCES:%=compiledb-%)
COMPILEDB_FLAGS := -std=c23 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wdouble-promotion -Wformat=2 -Wundef -I include

.PHONY: $(COMPILEDB_TARGETS)

all: dev

build:
	zig build -Doptimize=ReleaseFast

dev:
	zig build -Doptimize=Debug

test:
	zig build test

run:
	zig build run

compiledb:
	compiledb --overwrite make compiledb-commands

compiledb-commands: $(COMPILEDB_TARGETS)

$(COMPILEDB_TARGETS):
	clang $(COMPILEDB_FLAGS) -fsyntax-only $(patsubst compiledb-%,%,$@)

check:
	cppcheck --enable=all --suppress=missingIncludeSystem $(C_SOURCES)

format:
	clang-format -i $(C_SOURCES) include/*.h include/sigma/*.h

clean:
	rm -rf zig-out .zig-cache compile_commands.json .bear-fingerprints app app-dev

help:
	@echo "Available make targets:"
	@echo "  make build      - compile a release build"
	@echo "  make dev        - compile a debug build (default)"
	@echo "  make test       - run tests when added"
	@echo "  make run        - build and run the example"
	@echo "  make compiledb  - generate compile_commands.json"
	@echo "  make check      - run cppcheck"
	@echo "  make format     - format C sources and headers"
	@echo "  make clean      - remove build artifacts"
