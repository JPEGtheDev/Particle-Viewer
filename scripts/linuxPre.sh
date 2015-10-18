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

if [ ! "$(ls -A /home/travis/build/JPEGtheDev/glm)" ]; then
	
	cd /home/travis/build/JPEGtheDev/;
	mkdir glmBuild;
	cd glmBuild;
	git clone https://github.com/g-truc/glm.git;
	cd glm;
	cmake -DCMAKE_INSTALL_PREFIX=/home/travis/build/JPEGtheDev/glm .;
	make install;
	cd /home/travis/build/JPEGtheDev/Particle-Viewer;
fi

if [ ! "$(ls -A /home/travis/build/JPEGtheDev/assimp)" ]; then
	
	cd /home/travis/build/JPEGtheDev/;
	mkdir assBuild;
	cd assBuild;
	git clone https://github.com/assimp/assimp.git;
	cd assimp;
	cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX=/home/travis/build/JPEGtheDev/assimp .;
	make;
	make install;
	cd /home/travis/build/JPEGtheDev/Particle-Viewer;
fi
cmake -version;



