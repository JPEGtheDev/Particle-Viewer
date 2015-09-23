#!/usr/bin/env sh
#gcc build script
gcc ./src/main.cpp -o ./bin/viewer  -lSDL2 -lGLEW -I. -g -O0 -Wall;
