#!/usr/bin/sh

BUILD_DIR="${1:-build}"
TARGET_DIR="${2:-/data/local/tmp}"


adb push "$BUILD_DIR/Injector" "$TARGET_DIR" 
