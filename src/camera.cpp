#include "camera.hpp"
Camera::Camera(const int SCREEN_WIDTH, const int SCREEN_HEIGHT)
{
	std::string exePath = ExePath();
	this->cameraPos 	= glm::vec3(0.0f, 0.0f,  3.0f);
	this->cameraFront 	= glm::vec3(0.0f, 0.0f, -1.0f);
	this->cameraUp		= glm::vec3(0.0f, 1.0f,  0.0f);
	this->baseSpeed		= 5.0f;
	this->speed 		= this->baseSpeed;
	this->yaw			= -90.0f;	
	this->pitch  		= 0.0f;
	this->renderDistance= 1000.0f;
	this->projection	= glm::perspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, renderDistance);
	this->isPlayingBack = false;
	this->renderSphere 	= false;
	this->rotateState	= 0;
	this->sphereColor  	= glm::vec3(0.0f,0.0f,0.0f);
	this->centerOfMass 	= glm::vec3(0.0f,0.0f,0.0f);
	this->sphereDistance= 5.0f;
	this->vertShader 	= exePath + "/Viewer-Assets/shaders/colorSphere.vs";
	this->fragShader 	= exePath + "/Viewer-Assets/shaders/colorSphere.frag";
	this->rotLock = false;
	this->comLock = false;
	this->distTweak = .125f;
	for (int c = 0; c < 4; c++)
	{
		for (int r = 0; r < 4; r++)
		{
			std::cout << projection[c][r] << " ";
		}
		std::cout << std::endl;
	}
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
	if(keys[GLFW_KEY_LEFT_SHIFT])
	{
		this->speed = this->baseSpeed * deltaTime * 20;
	}
	else
	{
		this->speed = this->baseSpeed * deltaTime;
	}
	
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
	clampPitch(this->pitch);
	glm::vec3 front;
	//front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	//front.y = sin(glm::radians(pitch));
	//front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	//cameraFront = glm::normalize(front);
	//distTweak = cameraPos.z / 21;
	updateSpeed(deltaTime);
}
void Camera::clampPitch(GLfloat &pitch)
{
	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}
	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}
}
void Camera::clampDegrees(GLfloat &in)
{
	in = fmod(in,360);
	if(in < 0.0)
	{
		in += 360.0f;
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

void Camera::KeyReader(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS)
  		keys[key] = true;
	else if(action == GLFW_RELEASE)
  		keys[key] = false;
	clampDegrees(this->yaw);
}
void Camera::setSphereCenter(glm::vec3 pos)
{
	centerOfMass = pos;
}
void Camera::Move()
{
	if(!rotLock)
	{
		if(keys[GLFW_KEY_W])
		{
			moveForward();
		}
		if(keys[GLFW_KEY_S])
		{
			moveBackward();
		}
		if(keys[GLFW_KEY_A])
		{
			moveLeft();
		}
		if(keys[GLFW_KEY_D])
		{
			moveRight();
		}
		if(keys[GLFW_KEY_I])
		{
			lookUp(2.5f);
		}
		if(keys[GLFW_KEY_K])
		{
			lookDown(2.5f);
		}
		if(keys[GLFW_KEY_J])
		{
			lookLeft(2.5f);
		}
		if(keys[GLFW_KEY_L])
		{
			lookRight(2.5f);
		}
	}
	clampDegrees(this->yaw);
}