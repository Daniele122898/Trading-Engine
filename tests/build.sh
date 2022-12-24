#!/bin/bash

cmake --build "${0%/*}/../cmake-build-debug-wsl" --target tests -j 6
