#!/usr/bin/env sh
#gcc build script
g++ ./src/main.cpp -o ./bin/viewer -ldl -pthread  -I. -g -O0 -lGLEW -lSDL2 -lGLM;
