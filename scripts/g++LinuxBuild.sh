#!/usr/bin/env sh
#gcc build script
g++ ./src/main.cpp -o ./bin/viewer -ldl -pthread  -I. -g -O0  ./src/deps/lib/libSDL2.a ./src/deps/lib/libSDL2main.a  -lGLEW;
./bin/viewer
