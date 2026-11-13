## Introduction

This is an injector for android platforms.

## How it works

1. Get the dlopen and dlsym in target runtime
2. Use `open` + `pread` to save the page in which instruction pointer is.
3. `pwrite` payload and required string into the page
4. Payload exiting by break pointer, then recover target

## Build

1. Set the environment value of NDK `export NDK=/path/to/ndk`

2. Set the environment value of ABIS, default all of ABIS

3. Run ./android-build.sh, output into ./build-<ABI> 

### Build-example

```sh
$ export NDK=$ANDROID_HOME/ndk/xx.xxx
$ export ABIS="arm64-v8a"
$ ./android-build.sh
```

## Usage

```sh
$ Injector -p <pid> -S </path/to/lib.so> [-F <function>] [-V]
```

## Status

- [~] arm64-v8a
- [-] armeabi-v7a
- [-] x86
- [-] x86_64
- [-] riscv64

