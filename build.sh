#!/bin/bash

REAL_CWD=$(dirname $0)
EXE_NAME=$(basename $0)
REAL_CWD=$(realpath $REAL_CWD)
cd "${REAL_CWD}"

# Ideally will use a more platform specific build folder name - depending on input parameters
TARGET_BUILD_DIR="build"

# Need to determine this dynamically - find vcpkg.exe
VCPKG_PATH=$(dirname `which vcpkg`)
VCPKG_CMAKE_PATH="${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake"
TOOLCHAIN_PATH=${VCPKG_CMAKE_PATH}

mkdir -p ${TARGET_BUILD_DIR}

cmake -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_PATH}" -B"${TARGET_BUILD_DIR}" -S"${REAL_CWD}"