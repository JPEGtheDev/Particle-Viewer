#include "camera.hpp"

Camera::Camera(const int SCREEN_WIDTH, const int SCREEN_HEIGHT)
{
	this->cameraPos 	= glm::vec3(0.0f, 0.0f,  3.0f);
	this->cameraFront 	= glm::vec3(0.0f, 0.0f, -1.0f);
	this->cameraUp		= glm::vec3(0.0f, 1.0f,  0.0f);
	this->baseSpeed		= 0.01f;
	this->speed 		= this->baseSpeed;
	this->yaw			= -90.0f;	
	this->pitch  		= 0.0f;
	this->renderDistance= 3000.0f;
	this->projection	= glm::perspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, renderDistance);
	this->isPlayingBack = false;
}

glm::mat4 Camera::setupCam()
{
	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}
void Camera::moveForward()
{
	cameraPos += speed * cameraFront;
}
void Camera::moveBackward()
{
	cameraPos -= speed * cameraFront;
}
void Camera::moveRight()
{
	cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
}
void Camera::moveLeft()
{
	cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
}

void Camera::updateSpeed(GLfloat deltaTime)
{
	this->speed = this->baseSpeed * deltaTime;
}
void Camera::lookUp(float pitch)
{
	this->pitch += pitch;
}
void Camera::lookDown(float pitch)
{
	this->pitch -= pitch;
}
void Camera::lookRight(float yaw)
{
	this->yaw += yaw;
}
void Camera::lookLeft(float yaw)
{
	this->yaw -= yaw;
}
void Camera::update(GLfloat deltaTime)
{
	clampPitch();
 	glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
    updateSpeed(deltaTime);

}
void Camera::clampPitch()
{
	if (this->pitch > 89.0f)
	{
        this->pitch = 89.0f;
	}
    if (this->pitch < -89.0f)
    {
        this->pitch = -89.0f;
    }
}
void Camera::setRenderDistance(float renderDistance)
{
	this->renderDistance = renderDistance;
}
void Camera::changeSpeed(float speed)
{
	this->speed = speed;
}
void Camera::recordPosition(int frame)
{
	for(int i =0; i < camLocation.size();i++) //checks to make sure the element has not been used already
	{
		if(camLocation[i].frame == frame)
		{
				camLocation.erase(camLocation.begin() + i); //deletes the element if it already exists
		}
	}
	locData data;
	data.frame = frame;
	data.position = cameraPos;
	data.look = glm::vec2(yaw,pitch);
	camLocation.push_back(data);
	//sort data
}