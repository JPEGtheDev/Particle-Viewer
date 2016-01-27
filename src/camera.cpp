#include "camera.hpp"
//#include <iostream>
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
	this->renderSphere 	= false;
	this->rotateState	= 0;
	this->sphereColor  	= glm::vec3(0.0f,0.0f,0.0f);
	this->sphereDistance= 5.0f;
	this->vertShader 	= 
	#include "shaders/colorSphere.vs"
	;
	this->fragShader 	= 
	#include "shaders/colorSphere.frag"
	;
	spherePos 			= calcSpherePos();
	this->rotLock = false;
	
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
void Camera::MultiKeyEventReader(SDL_Event &event)
{
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	if(keystate[SDL_SCANCODE_W])
	{
		moveForward();
	}
	if(keystate[SDL_SCANCODE_S])
	{
		moveBackward();
	}
	if(keystate[SDL_SCANCODE_A])
	{
		moveLeft();
	}
	if(keystate[SDL_SCANCODE_D])
	{
		moveRight();
	}
	if(keystate[SDL_SCANCODE_I])
	{
		lookUp(2.5f);
	}
	if(keystate[SDL_SCANCODE_K])
	{
		lookDown(2.5f);
	}
	if(keystate[SDL_SCANCODE_J])
	{
		lookLeft(2.5f);
	}
	if(keystate[SDL_SCANCODE_L])
	{
		lookRight(2.5f);
	}
	if(renderSphere && !rotLock)
	{
		if(keystate[SDL_SCANCODE_LEFTBRACKET])
		{
			sphereDistance -= .25;
		}
		if(keystate[SDL_SCANCODE_RIGHTBRACKET])
		{
			sphereDistance += .25;
		}
	}
	
}
void Camera::SingleKeyEventReader(SDL_Event &event)
{
	if(event.type == SDL_KEYDOWN)
	{
		if(event.key.keysym.sym == SDLK_p)
		{
			rotateState++;
			rotateState = rotateState%3;
			if(rotateState == 0)
			{
				renderSphere = false;
				sphereColor  = glm::vec3(0.0f,0.0f,0.0f);
			}
			else if(rotateState == 1)
			{
				renderSphere = true;
				sphereColor  = glm::vec3(1.0f,0.0f,0.0f);
			}
			else if(rotateState == 2)
			{
				renderSphere = true;
				sphereColor  = glm::vec3(0.0f,1.0f,0.0f);
			}
		}
	}
}
void Camera::RenderSphere()
{
	if(renderSphere)
	{
		sphereShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(setupCam()));
		glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection));
		spherePos = calcSpherePos();
	    glUniform3fv(glGetUniformLocation(sphereShader.Program, "pos"), 1, glm::value_ptr(spherePos));
	    glUniform3fv(glGetUniformLocation(sphereShader.Program, "color"), 1, glm::value_ptr(sphereColor));
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, 1);
		glBindVertexArray(0);
	}
	

}
void Camera::initGL()
{
	this->sphereShader = Shader(vertShader.c_str(),fragShader.c_str());
	glGenVertexArrays(1, &VAO);
}
glm::vec3 Camera::calcSpherePos()
{
	if(sphereDistance < 1)
	{
		sphereDistance = 1;
	}
	return glm::vec3(cameraPos.x + cos(glm::radians(yaw)) * cos(glm::radians(pitch)) * sphereDistance,cameraPos.y + sin(glm::radians(pitch)) * sphereDistance,cameraPos.z + sin(glm::radians(yaw)) * cos(glm::radians(pitch)) * sphereDistance);
}