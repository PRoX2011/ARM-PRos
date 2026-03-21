#!/bin/bash

# ==================================================================
# Build a bootable Raspberry Pi SD image (MBR + FAT32 boot partition).
# Puts kernel8.img and official GPU firmware on the card — same layout
# as a real Pi 3 boot partition.
# Requires: sfdisk (util-linux), mkfs.fat (dosfstools), mcopy (mtools), curl
# ==================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${ROOT}/build"
IMG="${BUILD_DIR}/sdcard.img"
KERNEL="${BUILD_DIR}/kernel8.img"
FW_CACHE="${BUILD_DIR}/rpi-firmware"
BOOT_START=8192
BOOT_OFFSET=$((BOOT_START * 512))
SD_SIZE=$((64 * 1024 * 1024))

FW_BASE="https://raw.githubusercontent.com/raspberrypi/firmware/master/boot"
FW_FILES=(start.elf fixup.dat bootcode.bin LICENCE.broadcom)

die() {
	echo "mksdcard: $*" >&2
	exit 1
}

for cmd in sfdisk mkfs.fat mcopy curl; do
	command -v "$cmd" >/dev/null 2>&1 || die "missing required command: $cmd"
done

[[ -f "$KERNEL" ]] || die "kernel not found: $KERNEL (run build-linux.sh first)"

mkdir -p "$BUILD_DIR" "$FW_CACHE"

for f in "${FW_FILES[@]}"; do
	dst="${FW_CACHE}/${f}"
	if [[ ! -f "$dst" ]]; then
		echo "mksdcard: fetching ${f}..."
		curl -fsSL -o "$dst" "${FW_BASE}/${f}" || die "failed to download ${f}"
	fi
done

truncate -s "$SD_SIZE" "$IMG"

echo "8192, , c, *" | sfdisk "$IMG" >/dev/null

mkfs.fat -F 32 -n BOOT --offset="$BOOT_START" "$IMG" >/dev/null

mcopy_to_boot() {
	mcopy -i "${IMG}@@${BOOT_OFFSET}" "$@"
}

mcopy_to_boot "$KERNEL" ::/kernel8.img
for f in "${FW_FILES[@]}"; do
	mcopy_to_boot "${FW_CACHE}/${f}" "::/${f}"
done

CONFIG_TMP="$(mktemp)"
trap 'rm -f "$CONFIG_TMP"' EXIT
cat >"$CONFIG_TMP" <<'EOF'
# ARM-PRos — Raspberry Pi 3 (AArch64 bare-metal)
arm_64bit=1
kernel=kernel8.img
enable_uart=1
EOF
mcopy_to_boot "$CONFIG_TMP" ::/config.txt

echo "mksdcard: wrote ${IMG} ($(stat -c%s "$IMG") bytes)"
