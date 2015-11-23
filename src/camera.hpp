#ifndef CAMERA_H
#define CAMERA_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"
class Camera
{
	private:
		GLfloat speed;
		GLfloat baseSpeed;
		GLfloat yaw;
		GLfloat pitch;
		void clampPitch();
	public:
		Camera();
		glm::mat4 setupCam();
		glm::vec3 cameraPos;
		glm::vec3 cameraFront;
		glm::vec3 cameraUp;
		void moveForward();
		void moveBackward();
		void moveRight();
		void moveLeft();
		void lookUp();
		void lookDown();
		void lookRight();
		void lookLeft();
		void updateSpeed(GLfloat deltaTime);
		void update(GLfloat deltaTime);

};

#endif