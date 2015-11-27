#!/usr/bin/env sh
#gcc build script
mkdir build;
cd build;
/home/travis/build/JPEGtheDev/cmake/bin/cmake ..  -DSDL2_INCLUDE_DIR=/home/travis/build/JPEGtheDev/SDL2_Build/include -DSDL2_LIBRARY=/home/travis/build/JPEGtheDev/SDL2_Build/lib/libSDL2.so -DSDL2MAIN_LIBRARY=/home/travis/build/JPEGtheDev/SDL2_Build/lib/libSDL2main.a
