#!/usr/bin/env sh
    hg clone https://hg.libsdl.org/SDL SDL;
    cd SDL;
    mkdir build;
    cd build;
    ../configure --prefix="../../SDL2_Build";
    pwd;
    make;
    cd /home/travis/build/JPEGtheDev/Particle-Viewer;
