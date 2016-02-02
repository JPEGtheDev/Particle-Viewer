#ifndef CAMERA_H
#define CAMERA_H
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"
#include "shader.hpp"
#ifdef _WIN32 //Windows Includes
	#include <windows.h>
	#include <SDL.h>
#else //Includes for Linux and OSX
	#include <SDL2/SDL.h>
#endif
struct locData
{
	long frame;
	glm::vec3 position;
	glm::vec2 look;
};
class Camera
{
	private:
		GLfloat speed;
		GLfloat baseSpeed;
		GLfloat yaw;
		GLfloat pitch;
		GLfloat sphereYaw;
		GLfloat spherePitch;
		GLuint VAO;
		GLuint VBO;
		bool rotLock;
		glm::vec3 spherePos;
		Shader sphereShader;
		void clampPitch(GLfloat &);
		std::string vertShader;
    	std::string fragShader;
    	glm::vec3 calcSpherePos(GLfloat,GLfloat,glm::vec3);
	public:
		Camera(const int, const int);
		float renderDistance;
		glm::mat4 projection; //basically tells the camera how to look
		glm::mat4 setupCam();
		glm::vec3 cameraPos;
		glm::vec3 cameraFront;
		glm::vec3 cameraUp;
		std::vector<locData> camLocation;
		bool isPlayingBack;
		bool renderSphere;
		GLuint rotateState;
		glm::vec3 sphereColor;
		GLfloat sphereDistance;
		void moveForward();
		void moveBackward();
		void moveRight();
		void moveLeft();
		void lookUp(float);
		void lookDown(float);
		void lookRight(float);
		void lookLeft(float);
		void updateSpeed(GLfloat deltaTime);
		void update(GLfloat deltaTime);
		void setRenderDistance(float);
		void changeSpeed(float);
		void recordPosition(int);
		void MultiKeyEventReader(SDL_Event &event);
		void SingleKeyEventReader(SDL_Event &event);
		void RenderSphere();
		void initGL();
};

#endif