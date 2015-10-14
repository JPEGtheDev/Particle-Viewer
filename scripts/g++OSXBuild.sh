#!/usr/bin/env sh
#gcc build script
g++ ./src/main.cpp -o ./bin/viewer -I. -I/Users/travis/build/JPEGtheDev/glm -g -O0 -lGLEW -lSDL2;
