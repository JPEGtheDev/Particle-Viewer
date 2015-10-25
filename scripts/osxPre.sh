brew update && brew install sdl2 glew glm assimp boost-python;

if [ ! "$(ls -A /Users/travis/build/JPEGtheDev/libRocket)" ]; then
	BUILD_FREETYPE2=YES
	BUILD_BOOST_PYTHON=YES
	BUILD_LUA=YES
	BUILD_PLATFORM=osx
	cd /Users/travis/build/JPEGtheDev/;
	mkdir rocketBuild;
	cd rocketBuild;
	git clone https://github.com/libRocket/libRocket.git;
	cd libRocket
	#sh ./Dependencies/osx-depends.sh;
	cd Build;
	cmake -LAH;
	cmake .;
	make;
	sudo make install;
	cd /Users/travis/build/JPEGtheDev/Particle-Viewer;
fi
