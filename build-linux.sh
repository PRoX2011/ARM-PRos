#!/bin/bash

# ==================================================================
# ARM-PRos -- The build script for Linux
# Copyright (C) 2026 PRoX
# ==================================================================

set -e

BUILD_DIR="build"
BIN_DIR="${BUILD_DIR}/bin"
SRC_DIR="src"
OUTPUT="${BUILD_DIR}/KERNEL.ELF"

FLAG_QUIET_MODE=0

for arg in "$@"; do
    if [ "$arg" == "-quiet" ]; then FLAG_QUIET_MODE=1; continue; fi
done

RED='\033[31m'
GREEN='\033[32m'
YELLOW='\033[33m'
CYAN='\033[36m'
NC='\033[0m'

print_info() {
    local message="$1"
    if [ $FLAG_QUIET_MODE == 0 ]; then echo -e "${CYAN}[ INFO ]${NC} ${message}"; fi
}

print_ok() {
    local message="$1"
    if [ $FLAG_QUIET_MODE == 0 ]; then echo -e "${GREEN}[  OK  ]${NC} ${message}"; fi
}

print_failed() {
    local message="$1"
    if [ $FLAG_QUIET_MODE == 0 ]; then echo -e "${RED}[ FAILED ]${NC} ${message}"; fi
    exit 1
}

print_splitline() {
    local message="$1"
    if [ $FLAG_QUIET_MODE == 0 ]; then
        echo -e "$NC"
        echo -e "$GREEN========== $message ==========$NC"
    fi
}

check_error() {
    if [ $? -ne 0 ]; then print_failed "$1"; fi
}

mkdir -p "$BIN_DIR"

print_splitline "Starting ARM x16-PRos build..."

rm -f "$BIN_DIR"/*.o "$OUTPUT"

CFLAGS="-ffreestanding -nostdlib -Isrc/include"
CROSS_COMPILE="aarch64-linux-gnu-"

print_info "Compiling Drivers..."
${CROSS_COMPILE}gcc $CFLAGS -c "$SRC_DIR/drivers/uart.c" -o "$BIN_DIR/uart.o"
check_error "Failed to compile uart.c"

print_info "Compiling Kernel..."
${CROSS_COMPILE}gcc $CFLAGS -c "$SRC_DIR/kernel/kernel.c" -o "$BIN_DIR/kernel_c.o"
check_error "Failed to compile kernel.c"

print_info "Assembling Bootstrap..."
${CROSS_COMPILE}gcc -c "$SRC_DIR/arch/boot.S" -o "$BIN_DIR/boot.o"
check_error "Failed to assemble boot.S"

print_info "Linking..."
${CROSS_COMPILE}ld -T "$SRC_DIR/kernel/linker.ld" "$BIN_DIR/boot.o" "$BIN_DIR/kernel_c.o" "$BIN_DIR/uart.o" -o "$OUTPUT"
check_error "Failed to link"

if [ -f "$OUTPUT" ]; then
    size_bytes=$(stat -c%s "$OUTPUT" 2>/dev/null || echo "0")
    print_ok "Kernel compiled successfully: ${size_bytes} bytes"
    print_ok "Output: $OUTPUT"
else
    print_failed "Kernel ELF not created"
fi

print_splitline "Build completed successfully!"
