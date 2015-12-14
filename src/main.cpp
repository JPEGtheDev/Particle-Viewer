#include "clutter.hpp"

int main(int argc, char* argv[])
{
  init_screen("Particle-Viewer");
  SDL_Event event;
  ticks = SDL_GetTicks();
  gladLoadGLLoader(SDL_GL_GetProcAddress);
  part = new Particle();
  setupGLStuff();
  
  while (!quit) 
  {
  	readInput(event);
    beforeDraw();
    drawFunct();
    SDL_GL_SwapWindow(window);
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
  if(highRes)
  {
    highResShader.Use();
  }
  else
  { 
    lowResShader.Use();
  }
}
void drawFunct()
{
  
  part->pushVBO();
  glBindVertexArray(quadVAO);
  if(highRes)
  {
    glUniformMatrix4fv(glGetUniformLocation(highResShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(highResShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glDrawArraysInstanced(GL_TRIANGLES, 0, lowResSphereNumVerts, part->n);
  }
  else
  {
    glUniformMatrix4fv(glGetUniformLocation(lowResShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(lowResShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glDrawArraysInstanced(GL_POINTS,0,1,part->n);
  }
  glBindVertexArray(0);
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
  highResShader = Shader(highResVertexShader.c_str(),highResFragmentShader.c_str());
  lowResShader = Shader(lowResVertexShader.c_str(),lowResFragmentShader.c_str());
  projection = glm::perspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 1000.0f);

  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(lowResSphereVerts), lowResSphereVerts, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(1);
  part->setUpInstanceArray();
  glBindVertexArray(0);
}

void cleanup()
{
  delete part;
}
