# LFA: Linux For ARM

A from-scratch ARM64 Linux bring-up on QEMU's `virt` board, with no board required.
Everything is built from source and described in this repo: the bootloader, the kernel
configuration, the root filesystem, and the disk image they live on.

## Boot chain

```
QEMU virt (Cortex-A72)
  └─ -bios u-boot.bin          U-Boot 2026.07, qemu_arm64 board
       └─ distro boot          scans the virtio disk, finds /boot/extlinux/extlinux.conf
            └─ /boot/Image     Linux 6.18.58 (LTS), arm64 defconfig + board fragment
                 └─ BusyBox    init, getty on the PL011 console (ttyAMA0)
```

QEMU hands U-Boot a device tree describing the `virt` machine. U-Boot passes the same
tree on to the kernel. There is no `-kernel` shortcut: the kernel is loaded from the disk
image by the bootloader, the way it would be on real hardware.

## Status

Boots to a login prompt on QEMU in about 1.2 s of kernel time, with a working network. See
[docs/boot.md](docs/boot.md) for a stage-by-stage walkthrough and
[docs/boot-log.txt](docs/boot-log.txt) for the captured serial log.

## Quick start

Host packages (Arch): `base-devel git wget ninja meson dtc bc swig python-pyelftools mtools ccache uboot-tools qemu-system-aarch64`

```sh
scripts/build.sh      # fetch Buildroot, build toolchain + U-Boot + kernel + rootfs + disk image
scripts/run.sh        # boot it; log in as root; leave QEMU with Ctrl-A then X
```

Sources and build output go to `~/lfa-build` (override with `LFA_WORK`). They have to live on a
Linux filesystem; a kernel build does not work reliably on NTFS.

## Layout

| Path | Contents |
|---|---|
| `buildroot/` | Buildroot external tree: `configs/lfa_defconfig`, kernel fragment, `genimage.cfg`, `extlinux.conf` |
| `scripts/` | `build.sh`, `run.sh`, `fetch.sh` |
| `docs/` | Design notes and boot logs |
| `cosim/`, `rtl/` | Parked: a Verilator co-simulation of the RAPTOR-SoC GPU (see below) |

## Versions

| Component | Version |
|---|---|
| Buildroot | 2026.08.1 |
| Linux | 6.18.58 (sha256 checked against kernel.org) |
| U-Boot | 2026.07 |
| QEMU | 11.x (distro package) |

## Parked: GPU co-simulation

`cosim/` wraps the real RAPTOR-SoC GPU RTL (git submodule in `rtl/`) in a Verilator harness and runs the
3x3 blur on it with 0 mismatches against the Python reference (`scripts/cosim_blur_test.py`). It is not
part of the boot flow yet. The plan is to attach it to the QEMU board as a memory-mapped device once the
base system is documented.
