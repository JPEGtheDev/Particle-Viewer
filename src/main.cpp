#include "clutter.hpp"

int main(int argc, char* argv[])
{
	initPaths();
	init_screen("Particle-Viewer");
	cam.initGL();
	part = new Particle();
	setupGLStuff();
	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();
		cam.Move();
		//readInput(event);
		beforeDraw();
		drawFunct();
		glfwSwapBuffers(window);
        
		
		if(set->frames > 1)
		{
			set->readPosVelFile(curFrame,part,false);
		}
		
		if(set->isPlaying)
		{
			if(curFrame != 0)
			{
				calculateTime(curFrame, set->getDt(), 50.0, 2935.864063);
			}
			curFrame++;

		}
		if(curFrame > set->frames)
		{
			curFrame = set->frames;
		}
		if (curFrame < 0)
		{
			curFrame = 0;
		}
	}
	cleanup();
	return 0;
}
void beforeDraw()
{
	cam.update(deltaTime);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	upDeltaTime();
	view = cam.setupCam();
	
	
}
void drawFunct()
{
	set->getCOM(curFrame, com);
	cam.setSphereCenter(com);
	sphereShader.Use();
	part->pushVBO();
	glBindVertexArray(circleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, part->instanceVBO);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(cam.projection));
	glUniform1f(glGetUniformLocation(sphereShader.Program, "radius"), sphereRadius);
	glUniform1f(glGetUniformLocation(sphereShader.Program, "scale"), sphereScale);
	glDrawArraysInstanced(GL_POINTS,0,1,part->n);
	glBindVertexArray(0);
	//take screenshot
	if(set->isPlaying && isRecording)
	{
		glReadPixels(0,0,(int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		for (int i = 0; i < SCREEN_WIDTH * 3; i++)
		{
			for(int j = 0; j < SCREEN_HEIGHT; j++)
			{
				pixels2[i + SCREEN_WIDTH* 3 * j] = pixels[i+ SCREEN_WIDTH* 3 * (SCREEN_HEIGHT - j)];
			}
		}
		if(!stbi_write_tga(std::string(recordFolder+"/" + std::to_string(curFrame) + ".tga").c_str(), (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, 3, pixels2))
		{
			if(imageError < 5)
			{
				imageError++;
				std::cout << "Unable to save image: Error "<< imageError << std::endl;
			}
			else
			{
				std::cout << "Max Image Error Count Reached! Ending Recording!"<< std::endl;
				isRecording = false;
			}
		}
	}
	//draw GUI elements
	cam.RenderSphere();
}

void setupGLStuff() 
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE );
	glEnable(GL_MULTISAMPLE);  
	sphereShader = Shader(sphereVertexShader.c_str(),sphereFragmentShader.c_str());							//creates the shader to be used on the spheres

	glGenVertexArrays(1, &circleVAO);
	glGenBuffers(1, &circleVBO);
	glBindVertexArray(circleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	part->setUpInstanceArray();
	glBindVertexArray(0);
	//set up other object arrays
}
void seekFrame(int frame, bool isForward)
{
	if(isForward)
	{
		curFrame += frame;
	}
	else
	{
		curFrame -= frame;
	}
	
}
void cleanup()
{
	delete part;
	delete[] pixels;
	delete[] pixels2;
}
void calculateTime(long long frame, float dt, float recordRate, float unitTime)
{
	double calc = frame * dt * recordRate * unitTime;
	int seconds = fmod(calc, 60);
	int minutes = fmod((calc / 60), 60);
	int hours	= fmod((calc / 3600), 24);
	int days	= fmod((calc / 86400), 365);
	std::string secondS = ((seconds < 10) ? "0" + std::to_string(seconds) : std::to_string(seconds));
	std::string minuteS = ((minutes < 10) ? "0" + std::to_string(minutes) : std::to_string(minutes));
	std::string hourS   = ((hours   < 10) ? "0" + std::to_string(hours  ) : std::to_string(hours));

	std::cout << "\nReal Time: " << days << " D | " << hourS << " H | " << minuteS << " M | " << secondS << " S" << std::endl;
}
