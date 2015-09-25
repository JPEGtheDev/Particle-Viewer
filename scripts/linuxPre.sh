#!/usr/bin/env sh
    hg clone https://hg.libsdl.org/SDL SDL;
    cd SDL;
    mkdir build;
    cd build;
    ../configure --prefix=/home/travis/build/JPEGtheDev/Particle-Viewer/SDL2_Build
    make;
    make install;
    cd /home/travis/build/JPEGtheDev/Particle-Viewer;
