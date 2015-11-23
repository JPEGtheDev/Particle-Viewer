#!/usr/bin/env sh
#gcc build script
mkdir build;
cd build;
/home/travis/build/JPEGtheDev/cmake/bin/cmake .. -DGLM_INCLUDE_DIR=/home/travis/build/JPEGtheDev/glm/ -DSDL2_INCLUDE_DIR=/home/travis/build/JPEGtheDev/SDL2_Build/include/SDL2 -DSDL2_LIBRARY=/home/travis/build/JPEGtheDev/SDL2_Build/lib/libSDL2.so -DSDL2MAIN_LIBRARY=/home/travis/build/JPEGtheDev/SDL2_Build/lib/libSDL2main.a -DCMAKE_CXX_FLAGS="-L/home/travis/build/JPEGtheDev/SDL2_Build/lib/ -I/home/travis/build/JPEGtheDev/SDL2_Build/include -I./src/glad -lSDL2";
make;
