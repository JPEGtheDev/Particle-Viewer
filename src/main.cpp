#include "clutter.hpp"

int main(int argc, char* argv[])
{
	init_screen("Particle-Viewer");
	SDL_Event event;
	ticks = SDL_GetTicks();
	gladLoadGLLoader(SDL_GL_GetProcAddress);
	cam.initGL();
	part = new Particle();
	//set->readPosVelFile(0,part,false); //loads the file
	setupGLStuff();
	while (!quit) 
	{
		readInput(event);
		beforeDraw();
		drawFunct();
		SDL_GL_SwapWindow(window);
		
		if(set->frames > 1)
		{
			set->readPosVelFile(curFrame,part,false);
		}
		
		if(set->isPlaying)
		{
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

void readInput(SDL_Event &event)
{
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	cam.MultiKeyEventReader(event);
	if(keystate[SDL_SCANCODE_E])
	{
		seekFrame(3, true);
	}
	if(keystate[SDL_SCANCODE_Q])
	{
		seekFrame(3, false);
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
			if(event.key.keysym.sym == SDLK_SPACE)
			{
				set->togglePlay();
			}
			if(event.key.keysym.sym == SDLK_t)
			{
				set = set->loadFile(part,false);
				curFrame = 0;
			}
			if(event.key.keysym.sym == SDLK_RIGHT)
			{
				seekFrame(1,true);
			}
			if(event.key.keysym.sym == SDLK_LEFT)
			{
				seekFrame(1,false);
			}
			if(event.key.keysym.sym == SDLK_r)
			{
				if(!isRecording)
				{
					imageError = 0;
					std::string dialog = "Select Folder";
					const char* fol = tinyfd_selectFolderDialog (dialog.c_str() , "") ;
					
					std::string folder;
					if(fol != NULL)
					{
						folder = std::string(fol);
					}
					else
					{
						folder = "";
					}
					if(folder != "")
					{
						recordFolder = folder;
						isRecording = true;
						break;
					}
					std::cout << "Folder not selected" << std::endl;
					isRecording = false;
				}
				else
				{
					recordFolder = "";
					isRecording = false;
				}
				
			}
		}
		cam.SingleKeyEventReader(event);
	}
}

void setupGLStuff() 
{
	glEnable(GL_DEPTH_TEST);
	glEnable( GL_PROGRAM_POINT_SIZE );
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
	delete pixels;
	delete pixels2;
}
