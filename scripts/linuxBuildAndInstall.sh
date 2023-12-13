#!/usr/bin/env sh
mkdir build;
cmake -B ./build
cmake --build ./build
sudo cmake --install build