#!/usr/bin/env sh
#gcc build script
mkdir build;
cd build;
/home/travis/build/JPEGtheDev/cmake/bin/cmake ..;
make
