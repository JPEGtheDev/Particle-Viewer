#!/usr/bin/env sh
#gcc build script
g++ ./src/main.cpp -o ./bin/viewer -ldl -pthread  -I. -I/Users/travis/build/JPEGtheDev/glm/glm -g -O0 -lGLEW -lSDL2;
