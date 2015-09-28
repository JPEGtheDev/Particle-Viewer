#!/usr/bin/env sh
touch /home/travis/build/JPEGtheDev/SDL2_Build;
if [ ! "$(ls -A /home/travis/build/JPEGtheDev/SDL2_Build)" ]; then
	hg clone https://hg.libsdl.org/SDL SDL;
	cd SDL;
	mkdir build;
	cd build;
	../configure --prefix=/home/travis/build/JPEGtheDev/SDL2_Build
	make;
	make install;
	cd /home/travis/build/JPEGtheDev/Particle-Viewer;
fi
