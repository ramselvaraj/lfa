#!/bin/sh
# Boot the LFA image on QEMU's ARM64 "virt" board. Exit QEMU with Ctrl-A then X.
set -e
LFA_WORK=${LFA_WORK:-$HOME/lfa-build}
IMG=$LFA_WORK/out/images
exec qemu-system-aarch64 \
	-M virt -cpu cortex-a72 -smp 2 -m 1G -nographic \
	-bios "$IMG/u-boot.bin" \
	-drive file="$IMG/disk.img",if=none,format=raw,id=hd0 \
	-device virtio-blk-pci,drive=hd0 \
	-netdev user,id=net0 -device virtio-net-pci,netdev=net0 \
	"$@"
