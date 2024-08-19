#!/bin/bash


set -e

cd "$(dirname "$(readlink -f "$0")")"

if [ -d build ]; then
    cd build
    make clean

    if [ "$1" = "clean" ]; then
        rm -rf build
        exit 0
    fi

else
    mkdir -p build
    cd build
fi

cmake ..
make 

