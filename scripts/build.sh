#!/bin/sh
# Build the LFA image. Sources and build output live outside the repo (default ~/lfa-build)
# because the kernel and Buildroot need a Linux filesystem, not NTFS.
#   LFA_WORK   work dir          (default $HOME/lfa-build)
#   BR_VERSION Buildroot version (default 2026.08.1)
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
LFA_WORK=${LFA_WORK:-$HOME/lfa-build}
BR_VERSION=${BR_VERSION:-2026.08.1}
BR=$LFA_WORK/buildroot-$BR_VERSION
OUT=$LFA_WORK/out
export BR2_DL_DIR=$LFA_WORK/dl

mkdir -p "$LFA_WORK/dl"
if [ ! -d "$BR" ]; then
	"$ROOT/scripts/fetch.sh"
	tar -C "$LFA_WORK" -xf "$LFA_WORK/src/buildroot-$BR_VERSION.tar.xz"
fi
make -C "$BR" O="$OUT" BR2_EXTERNAL="$ROOT/buildroot" lfa_defconfig
make -C "$OUT" "$@"
