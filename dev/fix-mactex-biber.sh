#!/bin/sh
# fix-mactex-biber.sh - repair the biber shipped by MacTeX 2025 on
# Apple Silicon Macs running Xcode 27 / current Command Line Tools.
#
# Problem: /Library/TeX/texbin/biber is a universal FAT binary whose
# self-extraction stub calls "/usr/bin/lipo -extract_family ...". Current
# lipo removed -extract_family, so biber dies with:
#   "biber: extracting arm64 binary with lipo failed (wstatus=256)"
#
# Fix: replace it with a thin arm64 slice, which skips the lipo step.
# The original is kept as biber.universal.
#
# Run with sudo:  sudo sh dev/fix-mactex-biber.sh
set -e

BIBER=/Library/TeX/texbin/biber

if [ "$(id -u)" != "0" ]; then
    echo "Please run with sudo: sudo sh $0" >&2
    exit 1
fi

if [ ! -f "$BIBER" ]; then
    echo "no biber found at $BIBER" >&2
    exit 1
fi

if [ ! -f "$BIBER.universal" ]; then
    cp -p "$BIBER" "$BIBER.universal"
    echo "backed up original to $BIBER.universal"
fi

/usr/bin/lipo "$BIBER.universal" -thin arm64 -output "$BIBER.arm64"
chmod 755 "$BIBER.arm64"
codesign --force --sign - "$BIBER.arm64"
mv "$BIBER.arm64" "$BIBER"

"$BIBER" --version && echo "biber repaired successfully"
