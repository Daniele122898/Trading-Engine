#!/bin/bash

cmake --build "${0%/*}/../build" --target Server -j 6
