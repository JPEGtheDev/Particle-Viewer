#include "clutter.hpp"

int main(int argc, char* argv[])
{
	init_screen("Particle-Viewer");
	SDL_Event event;
	ticks = SDL_GetTicks();
	gladLoadGLLoader(SDL_GL_GetProcAddress);
	part = new Particle();
	set->readPosVelFile(20,part,false); //loads the file
	int curFrame = 0;
	setupGLStuff();
	while (!quit) 
	{
		set->readPosVelFile(curFrame,part,false);
		readInput(event);
		beforeDraw();
		drawFunct();
		SDL_GL_SwapWindow(window);
		curFrame++;
		curFrame = curFrame % set->frames;
		//std::cout << curFrame << std::endl;
	}
	cleanup();
	return 0;
}

void beforeDraw()
{
	cam.update(deltaTime);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	manageFPS(ticks,lastticks);
	view = cam.setupCam();

	
}
void drawFunct()
{
	sphereShader.Use();
	part->pushVBO();
	glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, part->instanceVBO);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArraysInstanced(GL_POINTS,0,1,part->n);
	glBindVertexArray(0);
	
	//draw other stuff
}

void readInput(SDL_Event &event)
{
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	if(keystate[SDL_SCANCODE_W])
	{
		cam.moveForward();
	}
	if(keystate[SDL_SCANCODE_S])
	{
		cam.moveBackward();
	}
	if(keystate[SDL_SCANCODE_A])
	{
		cam.moveLeft();
	}
	if(keystate[SDL_SCANCODE_D])
	{
		cam.moveRight();
	}
	if(keystate[SDL_SCANCODE_I])
	{
		cam.lookUp();
	}
	if(keystate[SDL_SCANCODE_K])
	{
		cam.lookDown();
	}
	if(keystate[SDL_SCANCODE_J])
	{
		cam.lookLeft();
	}
	if(keystate[SDL_SCANCODE_L])
	{
		cam.lookRight();
	}

	while (SDL_PollEvent(&event)) 
	{
		if (event.type == SDL_QUIT) 
		{
			quit = true;
		}
		else if(event.type == SDL_KEYDOWN)
		{
			if(event.key.keysym.sym == SDLK_ESCAPE)
			{
				quit = true;
			}
			if(event.key.keysym.sym == SDLK_t)
			{
				highRes = !highRes;
			}
		}
	}
}

void setupGLStuff() 
{
	glEnable(GL_DEPTH_TEST);
	glEnable( GL_PROGRAM_POINT_SIZE );
	sphereShader = Shader(sphereVertexShader.c_str(),sphereFragmentShader.c_str());							//creates the shader to be used on the spheres
	projection = glm::perspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 1000.0f);	//basically tells the camera how to look
	
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

void cleanup()
{
	delete part;
}
