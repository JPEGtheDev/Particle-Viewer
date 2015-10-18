#!/usr/bin/env sh
#gcc build script
mkdir build;
cd build;
cmake ..;
make;
#g++ ./src/main.cpp -o ./bin/viewer -I. -g -O0 -lGLEW -lSDL2 -lassimp;
