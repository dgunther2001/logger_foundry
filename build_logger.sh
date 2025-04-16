#!/usr/bin/env bash

rm -rf build
mkdir -p build
cd build
cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=$PYROXENE_ROOT_PATH > /dev/null
make > /dev/null
make install > /dev/null
