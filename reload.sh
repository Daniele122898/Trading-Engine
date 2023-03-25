#!/bin/bash
mkdir cmake-build-debug-wsl
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang-15 '-DCMAKE_CXX_COMPILER=clang++-15' -DCMAKE_MAKE_PROGRAM=ninja -G Ninja -S ./ -B ./cmake-build-debug-wsl
