#!/bin/sh
# Download the Buildroot release. Buildroot fetches the kernel, U-Boot and toolchain itself.
set -e
LFA_WORK=${LFA_WORK:-$HOME/lfa-build}
BR_VERSION=${BR_VERSION:-2026.08.1}
mkdir -p "$LFA_WORK/src"
wget -c -O "$LFA_WORK/src/buildroot-$BR_VERSION.tar.xz" "https://buildroot.org/downloads/buildroot-$BR_VERSION.tar.xz"
