#!/usr/bin/env sh
#gcc build script
ls /home/travis/build/JPEGtheDev/Particle-Viewer/SDL2_Build
g++ ./src/main.cpp -o ./bin/viewer -ldl -pthread  -I. -g -O0  -L/home/travis/build/JPEGtheDev/SDL2_Build/lib/ -I/home/travis/build/JPEGtheDev/SDL2_Build/include -I/home/travis/build/JPEGtheDev/glm/glm  -lGLEW -lSDL2;
