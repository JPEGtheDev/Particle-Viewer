if [ ! "$(ls -A /Users/travis/build/JPEGtheDev/glm)" ]; then
	pwd;
	cd /Users/travis/build/JPEGtheDev/;
	git clone https://github.com/g-truc/glm.git;
	cd /Users/travis/build/JPEGtheDev/Particle-Viewer;
fi
