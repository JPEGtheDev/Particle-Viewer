#ifndef CAMERA_H
#define CAMERA_H
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"
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
		void clampPitch();
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

};

#endif