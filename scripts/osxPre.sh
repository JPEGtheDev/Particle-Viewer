brew update && brew install sdl2 glew glm assimp boost-python;
if [ ! "$(ls -A /home/travis/build/JPEGtheDev/libRocket)" ]; then
	cd /Users/travis/build/JPEGtheDev/;
	mkdir rocketBuild;
	cd rocketBuild;
	git clone https://github.com/libRocket/libRocket.git;
	cd libRocket
	#sh ./Dependencies/osx-depends.sh;
	cd Build;
	cmake -DCMAKE_INSTALL_PREFIX=/home/travis/build/JPEGtheDev/libRocket .;
	make;
	make install;
	cd /Users/travis/build/JPEGtheDev/Particle-Viewer;
fi
