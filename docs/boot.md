# Boot walkthrough

Based on [boot-log.txt](boot-log.txt).

## Command

```sh
qemu-system-aarch64 -M virt -cpu cortex-a72 -smp 2 -m 1G -nographic \
    -bios out/images/u-boot.bin \
    -drive file=out/images/disk.img,if=none,format=raw,id=hd0 -device virtio-blk-pci,drive=hd0 \
    -netdev user,id=net0 -device virtio-net-pci,netdev=net0
```

`-bios` makes U-Boot the first thing that runs. QEMU builds a device tree for the `virt` machine and passes
its address to U-Boot. The kernel and rootfs both come from `disk.img`.

## U-Boot

It finds 1 GiB of RAM, 64 MiB of flash and the virtio NIC. A few messages look like errors but aren't:

| Message | Why |
|---|---|
| `Bloblist at 0 not found` | Nothing provides one on this board. |
| `bad CRC, using default environment` | The emulated flash is empty. |
| `efi_mgr ... Cannot load any image` | No EFI boot entries; it moves on to the next method. |
| `fw-cfg ... no kernel available` | No `-kernel` was given. |

## Finding the kernel

After the 2 s autoboot delay, distro boot scans the virtio disk, finds partition 1 and reads
`/boot/extlinux/extlinux.conf`:

```
label lfa
	kernel /boot/Image
	append console=ttyAMA0 root=/dev/vda1 rootwait rw
```

No device tree is named there, so U-Boot passes along the one from QEMU (the kernel reports it as
`Machine model: linux,dummy-virt`).

## Kernel

It detects the Cortex-A72, brings up virtio, and finds the disk as `vda` with partition `vda1`. Then it mounts
it as root and starts init:

```
EXT4-fs (vda1): mounted filesystem ... r/w with ordered data mode.
VFS: Mounted root (ext4 filesystem) on device 254:1.
Run /sbin/init as init process
```

`rdinit=/init failed: -2` is only the kernel noticing there's no initramfs.

## Userspace

BusyBox init starts syslogd, klogd, networking (DHCP from QEMU gives 10.0.2.15) and crond, then a getty on
`ttyAMA0`. Log in as `root`, no password.

```
# uname -a
Linux lfa 6.18.58-lfa #1 SMP PREEMPT ... aarch64 GNU/Linux
```

## Not done

- Custom device tree (it uses QEMU's).
- TF-A. QEMU's `virt` already provides PSCI.
- A/B slots, secure boot.
