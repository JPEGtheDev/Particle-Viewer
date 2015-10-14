if [ ! "$(ls -A /User/travis/build/JPEGtheDev/glm)" ]; then
	
	cd /User/travis/build/JPEGtheDev/;
	git clone https://github.com/g-truc/glm.git;
	cd /User/travis/build/JPEGtheDev/Particle-Viewer;
fi
