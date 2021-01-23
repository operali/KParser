#!/bin/bash

if [ ! -d build ]; then
    echo "mkdir build"
    mkdir build
fi
cd build
if [ ! -d centos ]; then
    echo "mkdir centos"
    mkdir centos    
fi
cd centos
cmake ../../
make -j16

cd ../..

