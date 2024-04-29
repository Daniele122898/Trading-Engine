#!/bin/bash

cmake --build "${0%/*}/../build" --target tests -j 6
