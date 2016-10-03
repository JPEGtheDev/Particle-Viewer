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
	this->sphereYaw		= -90.0f;	
	this->spherePitch  	= 0.0f;
	this->renderDistance= 3000.0f;
	this->projection	= glm::perspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, renderDistance);
	this->isPlayingBack = false;
	this->renderSphere 	= false;
	this->rotateState	= 0;
	this->sphereColor  	= glm::vec3(0.0f,0.0f,0.0f);
	this->centerOfMass 	= glm::vec3(0.0f,0.0f,0.0f);
	this->sphereDistance= 5.0f;
	this->vertShader 	= exePath + "/Viewer-Assets/shaders/colorSphere.vs";
	this->fragShader 	= exePath + "/Viewer-Assets/shaders/colorSphere.frag";
	spherePos 			= calcSpherePos(this->yaw,this->pitch,this->cameraPos);
	this->rotLock = false;
	this->comLock = false;
	this->distTweak = .125f;
}
void Camera::upTweak()
{

	//distTweak += .125f;
	std::cout << "Tweak Distance: " <<  distTweak << std::endl;
	std::cout << "Camera Location: " << "(" << cameraPos.x << " , " << cameraPos.y 
		<< " , " << cameraPos.z << " ) "<< std::endl;
}
void Camera::downTweak()
{
	//distTweak -= .125f;
	std::cout << "Tweak Distance: " << distTweak << std::endl;
	std::cout << "Camera Location: " << "(" << cameraPos.x << " , " << cameraPos.y
		<< " , " << cameraPos.z << " ) " << std::endl;
}
glm::mat4 Camera::setupCam()
{
	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}
glm::mat4 Camera::setupLeftCam()
{
	return glm::lookAt(cameraPos - glm::vec3(distTweak ,0,0), cameraPos - glm::vec3(distTweak, 0, 0) + cameraFront, cameraUp);
}
glm::mat4 Camera::setupRightCam()
{
	return glm::lookAt(cameraPos + glm::vec3(distTweak, 0, 0), cameraPos + glm::vec3(distTweak, 0, 0) + cameraFront, cameraUp);
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
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
	distTweak = cameraPos.z / 21;
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
void Camera::KeyReader(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS)
  		keys[key] = true;
	else if(action == GLFW_RELEASE)
  		keys[key] = false;

  	if (action == GLFW_PRESS)
	{  
		if( key == GLFW_KEY_P)
		{
			rotateState++;
			rotateState = rotateState%3;
			if(rotateState == 0)
			{
				rotLock = false;
				comLock = false;
				renderSphere = false;
				sphereColor  = glm::vec3(0.0f,0.0f,0.0f);
			}
			else if(rotateState == 1)
			{
				rotLock = false;
				comLock = false;
				renderSphere = true;
				sphereColor  = glm::vec3(1.0f,0.0f,0.0f);
			}
			else if(rotateState == 2)
			{
				this->sphereYaw = yaw + 180;
				this->spherePitch = -pitch;
				renderSphere = true;
				rotLock = true;
				sphereColor  = glm::vec3(0.0f,1.0f,0.0f);
			}
		}
		if(key == GLFW_KEY_O)
		{
			if(rotLock)
			{
				comLock = !comLock;
			}
			
		}
		if(rotLock)
		{
			if(key == GLFW_KEY_1)
			{
				this->yaw= 90.0f;
				this->pitch = 0.0f;
				this->sphereYaw = -90.0f;
				this->spherePitch = 0.0f;
			}
			if(key == GLFW_KEY_2)
			{
				this->yaw= 180.0f;
				this->pitch = 0.0f;
				this->sphereYaw = 0.0f;
				this->spherePitch = 0.0f;
			}
			if(key == GLFW_KEY_3)
			{
				this->yaw= 270.0f;
				this->pitch = 0.0f;
				this->sphereYaw = 90.0f;
				this->spherePitch = 0.0f;
			}
			if(key == GLFW_KEY_4)
			{
				this->yaw= 0.0f;
				this->pitch = 0.0f;
				this->sphereYaw = 180.0f;
				this->spherePitch = 0.0f;
			}
			if(key == GLFW_KEY_5)
			{
				this->yaw= 90.0f;
				this->pitch = -89.0f;
				this->sphereYaw = 270.0f;
				this->spherePitch = 89.0f;
			}
			if(key == GLFW_KEY_6)
			{
				this->yaw= 90.0f;
				this->pitch = 89.0f;
				this->sphereYaw = 270.0f;
				this->spherePitch = -89.0f;
			}
		}
	}
	clampPitch(this->spherePitch);
	clampDegrees(this->sphereYaw);
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
	else
	{
		if(keys[GLFW_KEY_W])
		{
			spherePitch += 1.0;
			lookDown(1.0f);
		}
		if(keys[GLFW_KEY_S])
		{
			spherePitch -= 1.0;
			lookUp(1.0f);
		}
		if(keys[GLFW_KEY_A])
		{
			sphereYaw -= 1.0;
			lookLeft(1.0);
		}
		if(keys[GLFW_KEY_D])
		{
			sphereYaw += 1.0;
			lookRight(1.0);
		}
	}

	if(renderSphere)
	{
		if(keys[GLFW_KEY_LEFT_BRACKET])
		{
			sphereDistance -= .25;
		}
		if(keys[GLFW_KEY_RIGHT_BRACKET])
		{
			sphereDistance += .25;
		}
	}
	clampPitch(this->spherePitch);
	clampDegrees(this->sphereYaw);
	clampDegrees(this->yaw);
}
void Camera::RenderSphere()
{
	if(renderSphere)
	{
		sphereShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(setupCam()));
		glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection));
		
		if(rotLock && comLock)
		{
			cameraPos = calcSpherePos(this->sphereYaw,this->spherePitch,this->centerOfMass);
		}

		else if(!rotLock)
		{
			spherePos = calcSpherePos(this->yaw,this->pitch,this->cameraPos);
		}
		else if(rotLock)
		{
			cameraPos = calcSpherePos(this->sphereYaw,this->spherePitch,this->spherePos);
		}
		else
		{
			std::cout << "Yeah... Fix the rotLock and comLock if statements" << std::endl;
		}
		
	    glUniform3fv(glGetUniformLocation(sphereShader.Program, "pos"), 1, glm::value_ptr(spherePos));
	    glUniform3fv(glGetUniformLocation(sphereShader.Program, "color"), 1, glm::value_ptr(sphereColor));
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, 1);
		glBindVertexArray(0);
		if(comLock)
		{
			glUniform3fv(glGetUniformLocation(sphereShader.Program, "pos"), 1, glm::value_ptr(centerOfMass));
			glUniform3fv(glGetUniformLocation(sphereShader.Program, "color"), 1, glm::value_ptr(glm::vec3(0,0,1.0f)));
			glBindVertexArray(VAO2);
			glDrawArrays(GL_POINTS, 0, 1);
			glBindVertexArray(0);
		}
		
	}
}
void Camera::initGL()
{
	this->sphereShader = Shader(vertShader.c_str(),fragShader.c_str());
	glGenVertexArrays(1, &VAO);
	glGenVertexArrays(1, &VAO2);
}
glm::vec3 Camera::calcSpherePos(GLfloat yaw, GLfloat pitch,glm::vec3 pos)
{
	if(sphereDistance < 1)
	{
		sphereDistance = 1;
	}
	return glm::vec3(pos.x + cos(glm::radians(yaw)) * cos(glm::radians(pitch)) * sphereDistance,pos.y + sin(glm::radians(pitch)) * sphereDistance,pos.z + sin(glm::radians(yaw)) * cos(glm::radians(pitch)) * sphereDistance);
}