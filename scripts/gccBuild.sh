#!/usr/bin/env sh
#gcc build script
gcc ./src/main.cpp -o ./bin/viewer -lGLEW -I. -g -O0 -Wall ./SDL/build/build/libSDL2main.a;
