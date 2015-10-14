if [ ! "$(ls -A /User/travis/build/JPEGtheDev/glm)" ]; then
	pwd;
	cd /User/travis/build/JPEGtheDev/;
	git clone https://github.com/g-truc/glm.git;
	cd /User/travis/build/JPEGtheDev/Particle-Viewer;
fi
