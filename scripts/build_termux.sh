#!/usr/bin/env sh
set -eu
pkg install -y clang cmake git make
cmake -B build
cmake --build build
