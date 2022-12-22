#!/bin/bash

cmake --build "${0%/*}/../cmake-build-debug-wsl" --target Server -j 6
