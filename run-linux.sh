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

qemu-system-aarch64 \
    -M virt \
    -cpu cortex-a53 \
    -m 1024 \
    -kernel build/KERNEL.ELF \
    -display gtk \
    -serial vc \
    -monitor stdio