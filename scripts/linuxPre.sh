#!/usr/bin/env sh
touch /home/travis/build/JPEGtheDev/SDL2_Build;

if [ ! "$(ls -A /home/travis/build/JPEGtheDev/SDL2_Build)" ]; then
	hg clone https://hg.libsdl.org/SDL SDL;
	cd SDL;
	mkdir build;
	cd build;
	if [ "$(id -u)" != "0" ]; then
	 ../configure --prefix=/home/travis/build/JPEGtheDev/SDL2_Build;
	else
	 ../configure
	fi
	
	make;
	make install;
	cd /home/travis/build/JPEGtheDev/Particle-Viewer;
fi

if [ ! "$(ls -A /home/travis/build/JPEGtheDev/glm)" ]; then
	
	cd /home/travis/build/JPEGtheDev/;
	mkdir glmBuild;
	cd glmBuild;
	git clone https://github.com/g-truc/glm.git;
	cd glm;
	if [ "$(id -u)" != "0" ]; then
	cmake -DCMAKE_INSTALL_PREFIX=/home/travis/build/JPEGtheDev/glm .;
	else
	cmake .;
	fi
	make install;
	cd /home/travis/build/JPEGtheDev/Particle-Viewer;
fi

if [ ! "$(ls -A /home/travis/build/JPEGtheDev/assimp)" ]; then
	
	cd /home/travis/build/JPEGtheDev/;
	mkdir assBuild;
	cd assBuild;
	git clone https://github.com/assimp/assimp.git;
	cd assimp;
	cd glm;
	if [ "$(id -u)" != "0" ]; then
	cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX=/home/travis/build/JPEGtheDev/assimp .;
	else
	cmake -G 'Unix Makefiles' .;
	fi

	make;
	make install;
	cd /home/travis/build/JPEGtheDev/Particle-Viewer;
fi
if [ ! "$(ls -A /home/travis/build/JPEGtheDev/cmake)" ]; then
	
	cd /home/travis/build/JPEGtheDev/;
	mkdir cmakeBuild;
	cd cmakeBuild;
	wget --no-check-certificate "https://cmake.org/files/v3.3/cmake-3.3.2-Linux-x86_64.sh" ;
	sh ./cmake-3.3.2-Linux-x86_64.sh --prefix=/home/travis/build/JPEGtheDev/cmake --exclude-subdir;
	cd /home/travis/build/JPEGtheDev/Particle-Viewer;
fi
ls /home/travis/build/JPEGtheDev/SDL2_Build/lib
