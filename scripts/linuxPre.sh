#!/usr/bin/env sh
if [ ! "$(ls -A /home/travis/build/JPEGtheDev/cmake)" ]; then
	
	cd /home/travis/build/JPEGtheDev/;
	mkdir cmakeBuild;
	cd cmakeBuild;
	wget --no-check-certificate "https://cmake.org/files/v3.3/cmake-3.3.2-Linux-x86_64.sh" ;
	mkdir /home/travis/build/JPEGtheDev/cmake;
	chmod 777 -R /home/travis/build/JPEGtheDev/cmake;
	sh ./cmake-3.3.2-Linux-x86_64.sh --prefix=/home/travis/build/JPEGtheDev/cmake --exclude-subdir;
	cd /home/travis/build/JPEGtheDev/Particle-Viewer;
fi

pip install --user cpp-coveralls


