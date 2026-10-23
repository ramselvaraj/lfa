# LFA: Linux For ARM

ARM64 Linux bring-up on QEMU's `virt` board, built from source: U-Boot, kernel, Buildroot rootfs, disk image.

## Boot chain

```
QEMU virt (Cortex-A72)
  -bios u-boot.bin       U-Boot 2026.07 (qemu_arm64)
  extlinux.conf          found by U-Boot distro boot on the virtio disk
  /boot/Image            Linux 6.18.58
  BusyBox init           login on ttyAMA0
```

The kernel is loaded from the disk by U-Boot. No `-kernel` flag.

## Build and run

Needs: `base-devel git wget ninja meson dtc bc swig python-pyelftools mtools ccache uboot-tools qemu-system-aarch64`

```sh
scripts/build.sh    # first build takes a while
scripts/run.sh      # log in as root, quit with Ctrl-A X
```

Build output goes to `~/lfa-build` (set `LFA_WORK` to change it). It has to be on a Linux filesystem, not NTFS.

## Status

Boots to a login prompt, network up, about 1.2 s from kernel start to init.
Walkthrough: [docs/boot.md](docs/boot.md). Serial log: [docs/boot-log.txt](docs/boot-log.txt).

## Layout

- `buildroot/` Buildroot external tree (defconfig, kernel fragment, genimage config, extlinux.conf)
- `scripts/` build, run, fetch
- `docs/`

## Versions

Buildroot 2026.08.1, Linux 6.18.58, U-Boot 2026.07.
