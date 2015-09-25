#!/usr/bin/env sh
#gcc build script
ls /home/travis/build/JPEGtheDev/Particle-Viewer/SDL2_Build/lib
g++ ./src/main.cpp -o ./bin/viewer -ldl -pthread  -I. -g -O0  /home/travis/build/JPEGtheDev/Particle-Viewer/SDL2_Build/lib/libSDL2.a /home/travis/build/JPEGtheDev/Particle-Viewer/SDL2_Build/lib/libSDL2main.a -I/home/travis/build/JPEGtheDev/Particle-Viewer/SDL2_Build/include  -lGLEW;
