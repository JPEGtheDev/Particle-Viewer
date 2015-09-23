#!/usr/bin/env sh
    hg clone https://hg.libsdl.org/SDL SDL;
    cd SDL;
    mkdir build;
    cd build;
    ../configure;
    make;
    sudo make install;
    cd /home/travis/build/JPEGtheDev/Particle-Viewer;
