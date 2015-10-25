#!/usr/bin/env sh
#gcc build script
mkdir build;
cd build;
/home/travis/build/JPEGtheDev/cmake/bin/cmake .. -DGLM_INCLUDE_DIR=/home/travis/build/JPEGtheDev/glm/ -DSDL2_INCLUDE_DIR=/home/travis/build/JPEGtheDev/SDL2_Build/include/SDL2 -DSDL2_LIBRARY=/home/travis/build/JPEGtheDev/SDL2_Build/lib/libSDL2.so -DSDL2MAIN_LIBRARY=/home/travis/build/JPEGtheDev/SDL2_Build/lib/libSDL2main.a -DLIBROCKET_INCLUDE_DIR=/home/travis/build/JPEGtheDev/libRocket/include -DLIBROCKET_LIBRARY=/home/travis/build/JPEGtheDev/libRocket/lib/libRocketCore.so -DCMAKE_CXX_FLAGS="-L/home/travis/build/JPEGtheDev/SDL2_Build/lib/ -L/home/travis/build/JPEGtheDev/assimp/lib -I/home/travis/build/JPEGtheDev/SDL2_Build/include -I/home/travis/build/JPEGtheDev/glm/include  -I/home/travis/build/JPEGtheDev/assimp/include -I./src/glad -lGLEW -lSDL2 -lassimp";
make;

#g++ ./src/main.cpp -o ./Viewer -I. -g -O0  -L/home/travis/build/JPEGtheDev/SDL2_Build/lib/ -L/home/travis/build/JPEGtheDev/assimp/lib -I/home/travis/build/JPEGtheDev/SDL2_Build/include -I/home/travis/build/JPEGtheDev/glm/include  -I/home/travis/build/JPEGtheDev/assimp/include -I./src/glad -lGLEW -lSDL2 -lassimp;
 # 
