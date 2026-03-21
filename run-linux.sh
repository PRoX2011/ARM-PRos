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
SD_IMG="${ROOT}/build/sdcard.img"
KERNEL_IMG="${ROOT}/build/kernel8.img"

if [ ! -f "$SD_IMG" ]; then
    print_msg "$RED" "Missing ${SD_IMG} — run ./build-linux.sh first (needs sfdisk, mkfs.fat, mcopy, curl)."
    exit 1
fi
if [ ! -f "$KERNEL_IMG" ]; then
    print_msg "$RED" "Missing ${KERNEL_IMG} — run ./build-linux.sh first."
    exit 1
fi

# Raspberry Pi 3
qemu-system-aarch64 \
    -M raspi3b \
    -drive file="$SD_IMG",format=raw,if=sd,index=0 \
    -kernel "$KERNEL_IMG" \
    -serial stdio \
    -display gtk