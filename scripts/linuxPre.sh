#!/usr/bin/env sh

if [ $(find "/home/travis/build/JPEGtheDev/SDL2_Build" -maxdepth 0 -type d -empty 2>/dev/null)]; then
  hg clone https://hg.libsdl.org/SDL SDL;
  cd SDL;
  mkdir build;
  cd build;
  ../configure --prefix=/home/travis/build/JPEGtheDev/SDL2_Build
  make;
  make install;
  cd /home/travis/build/JPEGtheDev/Particle-Viewer;
fi
