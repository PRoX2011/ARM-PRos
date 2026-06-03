#!/bin/bash

# ==================================================================
# ARM-PRos - Linux build script for ARM-PRos kernel
# Copyright (C) 2026 PRoX2011
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

print_splitline "Starting ARM-PRos build..."

rm -f "$BIN_DIR"/*.o "$OUTPUT"

CC="clang"
AS="clang"
LD="ld.lld"
CFLAGS="--target=aarch64-none-elf -ffreestanding -nostdlib -Isrc/include"

print_info "Compiling Drivers..."
$CC $CFLAGS -c "$SRC_DIR/drivers/uart.c" -o "$BIN_DIR/uart.o"
check_error "Failed to compile uart.c"

$CC $CFLAGS -c "$SRC_DIR/drivers/mailbox.c" -o "$BIN_DIR/mailbox.o"
check_error "Failed to compile mailbox.c"

$CC $CFLAGS -c "$SRC_DIR/drivers/framebuffer.c" -o "$BIN_DIR/framebuffer.o"
check_error "Failed to compile framebuffer.c"

$CC $CFLAGS -c "$SRC_DIR/drivers/console.c" -o "$BIN_DIR/console.o"
check_error "Failed to compile console.c"

$CC $CFLAGS -c "$SRC_DIR/drivers/timer.c" -o "$BIN_DIR/timer.o"
check_error "Failed to compile timer.c"

$CC $CFLAGS -c "$SRC_DIR/drivers/input.c" -o "$BIN_DIR/input.o"
check_error "Failed to compile input.c"

$CC $CFLAGS -c "$SRC_DIR/drivers/spi.c" -o "$BIN_DIR/spi.o"
check_error "Failed to compile spi.c"

$CC $CFLAGS -c "$SRC_DIR/drivers/lcd/ili9486.c" -o "$BIN_DIR/ili9486.o"
check_error "Failed to compile ili9486.c"

print_info "Compiling USB stack..."
$CC $CFLAGS -c "$SRC_DIR/drivers/usb/usb.c" -o "$BIN_DIR/usb.o"
check_error "Failed to compile usb.c"

$CC $CFLAGS -c "$SRC_DIR/drivers/usb/keyboard.c" -o "$BIN_DIR/usb_kbd.o"
check_error "Failed to compile keyboard.c"

print_info "Compiling libc..."
$CC $CFLAGS -c "$SRC_DIR/lib/font8x8.c" -o "$BIN_DIR/font8x8.o"
check_error "Failed to compile font8x8.c"

$CC $CFLAGS -c "$SRC_DIR/lib/font8x16.c" -o "$BIN_DIR/font8x16.o"
check_error "Failed to compile font8x16.c"

$CC $CFLAGS -c "$SRC_DIR/lib/font4x6.c" -o "$BIN_DIR/font4x6.o"
check_error "Failed to compile font4x6.c"

$CC $CFLAGS -c "$SRC_DIR/lib/string.c" -o "$BIN_DIR/string.o"
check_error "Failed to compile string.c"

$CC $CFLAGS -c "$SRC_DIR/lib/stdlib.c" -o "$BIN_DIR/stdlib.o"
check_error "Failed to compile stdlib.c"

print_info "Compiling Shell..."
$CC $CFLAGS -c "$SRC_DIR/kernel/kshell.c" -o "$BIN_DIR/kshell.o"
check_error "Failed to compile kshell.c"

print_info "Compiling Kernel..."
$CC $CFLAGS -c "$SRC_DIR/kernel/log.c" -o "$BIN_DIR/log.o"
check_error "Failed to compile log.c"

$CC $CFLAGS -c "$SRC_DIR/kernel/power.c" -o "$BIN_DIR/power.o"
check_error "Failed to compile power.c"

${CROSS_COMPILE}gcc $CFLAGS -c "$SRC_DIR/kernel/interrupts.c" -o "$BIN_DIR/interrupts.o"
check_error "Failed to compile interrupts.c"

${CROSS_COMPILE}gcc $CFLAGS -c "$SRC_DIR/kernel/timer.c" -o "$BIN_DIR/timer.o"
check_error "Failed to compile timer.c"

${CROSS_COMPILE}gcc $CFLAGS -c "$SRC_DIR/kernel/pmm.c" -o "$BIN_DIR/pmm.o"
check_error "Failed to compile pmm.c"

${CROSS_COMPILE}gcc $CFLAGS -c "$SRC_DIR/kernel/malloc.c" -o "$BIN_DIR/malloc.o"
check_error "Failed to compile malloc.c"

${CROSS_COMPILE}gcc $CFLAGS -c "$SRC_DIR/kernel/kernel.c" -o "$BIN_DIR/kernel_c.o"
check_error "Failed to compile kernel.c"

print_info "Assembling Bootstrap..."
$AS --target=aarch64-none-elf -c "$SRC_DIR/arch/boot.S" -o "$BIN_DIR/boot.o"
check_error "Failed to assemble boot.S"

${CROSS_COMPILE}gcc -c "$SRC_DIR/arch/exception.S" -o "$BIN_DIR/exception.o"
check_error "Failed to assemble exception.S"

print_info "Linking..."
$LD -T "$SRC_DIR/kernel/linker.ld" \
    "$BIN_DIR/boot.o" \
    "$BIN_DIR/exception.o" \
    "$BIN_DIR/kernel_c.o" \
    "$BIN_DIR/boot_menu.o" \
    "$BIN_DIR/log.o" \
    "$BIN_DIR/power.o" \
    "$BIN_DIR/interrupts.o" \
    "$BIN_DIR/timer.o" \
    "$BIN_DIR/pmm.o" \
    "$BIN_DIR/malloc.o" \
    "$BIN_DIR/kshell.o" \
    "$BIN_DIR/console.o" \
    "$BIN_DIR/framebuffer.o" \
    "$BIN_DIR/mailbox.o" \
    "$BIN_DIR/uart.o" \
    "$BIN_DIR/timer.o" \
    "$BIN_DIR/input.o" \
    "$BIN_DIR/spi.o" \
    "$BIN_DIR/ili9486.o" \
    "$BIN_DIR/usb.o" \
    "$BIN_DIR/usb_kbd.o" \
    "$BIN_DIR/font8x8.o" \
    "$BIN_DIR/font8x16.o" \
    "$BIN_DIR/font4x6.o" \
    "$BIN_DIR/string.o" \
    "$BIN_DIR/stdlib.o" \
    -o "$OUTPUT"
check_error "Failed to link"

if [ -f "$OUTPUT" ]; then
    print_info "Creating kernel8.img (raw image for Raspberry Pi 3)..."
    llvm-objcopy -O binary "$OUTPUT" "${BUILD_DIR}/kernel8.img"
    check_error "Failed to create kernel8.img"

    size_bytes=$(stat -c%s "$OUTPUT" 2>/dev/null || echo "0")
    print_ok "Kernel compiled successfully: ${size_bytes} bytes"
    print_ok "Output: $OUTPUT"
    print_ok "Pi 3: flash ${BUILD_DIR}/sdcard.img (see scripts/mksdcard.sh) or copy files to boot FAT"

    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    if [ -x "${SCRIPT_DIR}/scripts/mksdcard.sh" ]; then
        print_info "Building bootable SD image (scripts/mksdcard.sh)..."
        if "${SCRIPT_DIR}/scripts/mksdcard.sh"; then
            print_ok "SD image: ${BUILD_DIR}/sdcard.img"
        elif [ $FLAG_QUIET_MODE == 0 ]; then
            echo -e "${YELLOW}[ WARN ]${NC} sdcard.img not built (need sfdisk, mkfs.fat, mcopy, curl). Run: ./scripts/mksdcard.sh"
        fi
    fi
else
    print_failed "Kernel ELF not created"
fi

print_splitline "Build completed successfully!"
