#!/usr/bin/env sh
#gcc build script
g++ ./src/main.cpp -o ./bin/viewer -ldl -pthread  -I. -g -O0  -L/home/travis/build/JPEGtheDev/SDL2_Build/lib/ -I/home/travis/build/JPEGtheDev/SDL2_Build/include -I/home/travis/build/JPEGtheDev/glm/include  -lGLEW -lSDL2;
