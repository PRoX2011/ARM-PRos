#!/bin/bash

# ==================================================================
# ARM-PRos -- The run script for Linux
# Copyright (C) 2026 PRoX
# ==================================================================

RED='\033[31m'
GREEN='\033[32m'
NC='\033[0m'

print_msg() {
    local color="$1"
    local message="$2"
    echo -e "${color}${message}${NC}"
}

print_msg "$NC" ""
print_msg "$GREEN" "Starting ARM emulator..."

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KERNEL_ELF="${ROOT}/build/KERNEL.ELF"

if [ ! -f "$KERNEL_ELF" ]; then
    print_msg "$RED" "Missing ${KERNEL_ELF} — run ./build-linux.sh first."
    exit 1
fi

qemu-system-aarch64 \
    -M raspi3b \
    -kernel "$KERNEL_ELF" \
    -serial stdio \
    -display gtk