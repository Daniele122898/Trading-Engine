#!/bin/bash

export LDFLAGS="-L/opt/homebrew/Cellar/libpq/16.2_1/lib"
export CPPFLAGS="-I/opt/homebrew/Cellar/libpq/16.2_1/include"

mkdir build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang '-DCMAKE_CXX_COMPILER=clang++' -DCMAKE_MAKE_PROGRAM=ninja -G Ninja -S ./ -B ./build
