#!/usr/bin/bash

set -e

: ${ABIS:=arm64-v8a,armeabi-v7a,x86,x86_64,riscv64}

IFS=','

for ABI in $ABIS;do
    BUILD_DIR="build-$ABI"
    cmake -GNinja -B"$BUILD_DIR" \
        -DANDROID_PLATFORM=android-30 \
        -DANDROID_ABI="$ABI" \
        -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
        "$@"
    cmake --build "$BUILD_DIR"
done

