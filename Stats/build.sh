#!/bin/bash

cmake --build "${0%/*}/../cmake-build-debug-wsl" --target Stats -j 6
