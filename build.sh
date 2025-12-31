#!/bin/sh

set -xe

CC="clang"
CFLAGS="-Wall -Wextra -Wpedantic -std=c2x -O0 -g -DDEBUG_MODE -I./src"
BUILD_DIR="./build"
PROGRAM_NAME="clox"

mkdir -p $BUILD_DIR

$CC $CFLAGS -o "$BUILD_DIR/$PROGRAM_NAME" src/main.c
