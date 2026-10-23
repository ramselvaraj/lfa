# Booting LFA

This document walks through what happens from `scripts/run.sh` to a login prompt, using the
captured serial log in [`boot-log.txt`](boot-log.txt) (U-Boot 2026.07, Linux 6.18.58, Buildroot 2026.08.1).

## The command

```sh
qemu-system-aarch64 -M virt -cpu cortex-a72 -smp 2 -m 1G -nographic \
    -bios out/images/u-boot.bin \
    -drive file=out/images/disk.img,if=none,format=raw,id=hd0 -device virtio-blk-pci,drive=hd0 \
    -netdev user,id=net0 -device virtio-net-pci,netdev=net0
```

- `-bios u-boot.bin`: QEMU maps this file as the machine's flash and starts the CPU there, so U-Boot is
  the first code that runs. QEMU generates a device tree for the `virt` machine and gives U-Boot its address.
- `-device virtio-blk-pci`: the disk U-Boot loads the kernel from and the kernel mounts as root.
- There is no `-kernel` or `-initrd`. Everything after U-Boot comes from `disk.img`.

## Stage 1: U-Boot

U-Boot comes up on the serial console, finds 1 GiB of DRAM and a 64 MiB flash, and detects the
virtio-net device. The log has a few lines that look alarming but are expected here:

| Message | Meaning |
|---|---|
| `Bloblist at 0 not found` | Nothing hands U-Boot a bloblist on this board; it is optional. |
| `bad CRC, using default environment` | The emulated flash is blank, so there is no saved environment. Defaults are used. |
| `efi_mgr` tried first, `Cannot load any image` | The EFI boot manager has no boot entries. U-Boot moves on to the next method. |
| `fw-cfg ... fatal: no kernel available` | No `-kernel` was passed, which is intended. |

## Stage 2: finding the kernel

After the 2-second autoboot delay, U-Boot's distro boot scans every boot device. On the virtio disk it finds
partition 1 (the ext4 root filesystem) and reads `/boot/extlinux/extlinux.conf`:

```
label lfa
	kernel /boot/Image
	append console=ttyAMA0 root=/dev/vda1 rootwait rw
```

U-Boot loads `/boot/Image` into RAM. No device tree is listed in `extlinux.conf`, so U-Boot hands the kernel
the tree it received from QEMU (`Booting using the fdt blob at 0x40000000`). The kernel's
`Machine model: linux,dummy-virt` line is that QEMU-generated tree.

## Stage 3: Linux

The kernel detects the Cortex-A72 (`0x410fd083`), enables KASLR, and probes the virtio devices. The root disk
shows up as `virtio_blk virtio0: [vda] ... (268 MB/256 MiB)` with one partition `vda1`. Because the
command line has `root=/dev/vda1 rootwait`, the kernel waits for that device and mounts it:

```
EXT4-fs (vda1): mounted filesystem ... r/w with ordered data mode.
VFS: Mounted root (ext4 filesystem) on device 254:1.
Run /sbin/init as init process
```

`check access for rdinit=/init failed: -2, ignoring` just means there is no initramfs; the kernel falls
through to the on-disk root.

## Stage 4: userspace

BusyBox `init` runs the Buildroot start-up scripts: syslogd, klogd, sysctl, the network (DHCP from QEMU's
user-mode network gives `10.0.2.15`), and crond. A getty then starts on `ttyAMA0`, the PL011 UART of the `virt`
board, and prints the `lfa login:` prompt. Log in as `root` (no password).

Checked after login:

```
# uname -a
Linux lfa 6.18.58-lfa #1 SMP PREEMPT ... aarch64 GNU/Linux
# ls /boot
Image     extlinux
# ip -4 addr show eth0
    inet 10.0.2.15/24 ...
```

## Timing

The kernel mounts the root filesystem about 1.15 s after it starts, and `init` is running at about 1.2 s.
U-Boot's 2-second autoboot delay and the firmware scan come before that.

## What is not here yet

- A custom device tree. The kernel uses the one QEMU generates.
- TF-A. QEMU's `virt` board provides PSCI itself, so U-Boot runs directly at EL2/EL1.
- Secure boot, A/B slots and an update path.
- Out-of-tree drivers. The GPU co-simulation in `cosim/` is not connected to this board yet.
