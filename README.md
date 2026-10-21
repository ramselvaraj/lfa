# raptor-linux

Embedded Linux bring-up for the [RAPTOR-SoC](https://github.com/ramselvaraj/RAPTOR-SoC) GPU, entirely under QEMU.

The RAPTOR GPU is modeled as a memory-mapped QEMU device on an ARM64 board. The project covers the whole stack:

1. Boot chain: TF-A, U-Boot, Linux, Buildroot rootfs
2. QEMU device model of the GPU register block, with a done interrupt
3. Device tree node and binding schema
4. Kernel platform driver (probe/remove, ioctl/mmap, IRQ)
5. Userspace test that runs the 3x3 blur through the driver and checks every pixel against a Python model
6. CI that boots the image and runs the test

## Status

Early scaffolding. Nothing boots yet.

## Layout

| Path | Contents |
|---|---|
| `qemu/` | QEMU device model and machine patches |
| `kernel/` | Kernel config, platform driver, DT binding |
| `buildroot/` | Buildroot external tree and defconfig |
| `userspace/` | Test program and Python golden model |
| `scripts/` | Build and run helpers |
| `docs/` | Register map and design notes |

## GPU register map

Base offsets from the RAPTOR-SoC `ADDRESS_MAP.md` (block size 0x20):

| Offset | Name | R/W | Description |
|---|---|---|---|
| `0x00` | `CTRL` | W | bit0 = start |
| `0x04` | `STATUS` | R | bit0 = busy, bit1 = done (sticky) |
| `0x08` | `THREADS` | W | thread count |
| `0x0C` | `SRC` | W | source framebuffer address |
| `0x10` | `DST` | W | destination framebuffer address |
| `0x14` | `PROG_BASE` | W | GPU program-memory offset |

Interrupt registers are not in the RTL yet; this project proposes them (see `docs/`).
