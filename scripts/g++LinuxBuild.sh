#!/usr/bin/env sh
#gcc build script
mkdir build;
cd build;
cmake .. -DGLM_INCLUDE_DIR=/home/travis/build/JPEGtheDev/glm/;
make;
#g++ ./src/main.cpp -o ./bin/viewer -I. -g -O0  -L/home/travis/build/JPEGtheDev/SDL2_Build/lib/ -L/home/travis/build/JPEGtheDev/assimp/lib -I/home/travis/build/JPEGtheDev/SDL2_Build/include -I/home/travis/build/JPEGtheDev/glm/include  -I/home/travis/build/JPEGtheDev/assimp/include -lGLEW -lSDL2 -lassimp;
