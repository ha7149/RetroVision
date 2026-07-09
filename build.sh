#!/bin/bash
set -e
# Verify environment 
if [ -z "$VCPKG_ROOT" ]; then echo "VCPKG_ROOT missing"; exit 1; fi

rm -rf build
export CC=gcc
export CXX=g++
# Force paths to UCRT64 for compiler and linker
export CFLAGS="-I/ucrt64/include"
export LDFLAGS="-L/ucrt64/lib"
export PKG_CONFIG_PATH="/ucrt64/lib/pkgconfig:/ucrt64/share/pkgconfig"

cmake -B build -G "Ninja" \
	-DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
	-DVCPKG_TARGET_TRIPLET="x64-mingw-dynamic" \
	-DVCPKG_HOST_TRIPLET="x64-mingw-dynamic" \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DCMAKE_BUILD_TYPE=Release

cmake --build build
