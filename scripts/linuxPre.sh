#!/usr/bin/env sh
#touch /home/travis/build/JPEGtheDev/SDL2_Build;
if [ ! "$(ls -A /home/travis/build/JPEGtheDev/cmake)" ]; then
	
	pushd /home/travis/build/JPEGtheDev/;
	mkdir cmakeBuild;
	cd cmakeBuild;
	wget --no-check-certificate "https://cmake.org/files/v3.3/cmake-3.3.2-Linux-x86_64.sh" ;
	mkdir /home/travis/build/JPEGtheDev/cmake;
	chmod 777 -R /home/travis/build/JPEGtheDev/cmake;
	sh ./cmake-3.3.2-Linux-x86_64.sh --prefix=/home/travis/build/JPEGtheDev/cmake --exclude-subdir;
	popd;
fi

if [ ! "$(ls -A /home/travis/build/JPEGtheDev/SDL2_Build)" ]; then
	hg clone https://hg.libsdl.org/SDL SDL;
	pushd SDL;
	mkdir build;
	cd build;
	/home/travis/build/JPEGtheDev/cmake/bin/cmake .. -DCMAKE_INSTALL_PREFIX="/home/travis/build/JPEGtheDev/SDL2_Build";
	make;
	make install;
	popd;
fi


