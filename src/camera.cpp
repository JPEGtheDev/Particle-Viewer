#include "camera.hpp"

Camera::Camera()
{
	this->cameraPos 	= glm::vec3(0.0f, 0.0f,  3.0f);
	this->cameraFront 	= glm::vec3(0.0f, 0.0f, -1.0f);
	this->cameraUp		= glm::vec3(0.0f, 1.0f,  0.0f);
	this->baseSpeed		= 0.01f;
	this->speed 		= this->baseSpeed;
	this->yaw			= -90.0f;	
	this->pitch  		= 0.0f;
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
void Camera::lookUp()
{
	pitch += 2.5f;
}
void Camera::lookDown()
{
	pitch -= 2.5f;
}
void Camera::lookRight()
{
	yaw += 2.5f;
}
void Camera::lookLeft()
{
	yaw -= 2.5f;
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