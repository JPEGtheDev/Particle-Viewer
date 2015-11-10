#include "clutter.hpp"

int main(int argc, char* argv[])
{
  init_screen("Particle-Viewer");
  SDL_Event event;
  ticks = SDL_GetTicks();
  gladLoadGLLoader(SDL_GL_GetProcAddress);
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
  shader.Use();
  view = cam.setupCam();
  //update positions

  
}

glm::vec3 translations[10000];
void setupRender()
{
  GLfloat offset = 0.1f;
  int index = 0;
  for(GLint y = -100; y < 100; y += 2)
    {
        for(GLint x = -100; x < 100; x += 2)
        {
            glm::vec3 translation;
            translation.x = (GLfloat)x / 10.0f + offset;
            translation.y = (GLfloat)y / 10.0f + offset;
            translation.z = (GLfloat)y * (GLfloat)x * deltaTime / 10000.0f;
            //std::cout << deltaTime << std::endl;
            translations[index++] = translation;
        }
    }


      glGenBuffers(1, &instanceVBO);
      glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 10000, &translations[0],  GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glBindVertexArray(quadVAO);
      glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
      glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
      glBindBuffer(GL_ARRAY_BUFFER, 0); 
      glBindVertexArray(0);
}

void drawFunct()
{
  setupRender();
  glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
  glBindVertexArray(quadVAO);
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 10000);
  glBindVertexArray(0);
  glDeleteBuffers(1, &instanceVBO);
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
    }
  }
}

void manageFPS(uint32_t &ticks, uint32_t &lastticks)
{
	ticks = SDL_GetTicks();
  deltaTime = ticks - lastticks;
  if ( ((ticks*10-lastticks*10)) < 167 )
  {
    SDL_Delay( (167-((ticks*10-lastticks*10)))/10 );
  } 
  lastticks = SDL_GetTicks();

}

static void sdl_die(const char * message) 
{
  fprintf(stderr, "%s: %s\n", message, SDL_GetError());
  exit(2);
}
 
void init_screen(const char * title) 
{
  // Init SDL 
  if (SDL_Init(SDL_INIT_VIDEO) < 0) sdl_die("SDL Initialize Failed!");
  atexit (SDL_Quit);
  
  //loads base GL Libs
  SDL_GL_LoadLibrary(NULL);
  
  //set base GL stuff
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  //creates the window
  window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
  if (window == NULL) sdl_die("Failed to create window!");

  //creates the main GL context
  maincontext = SDL_GL_CreateContext(window);
  if (maincontext == NULL) sdl_die("Failed to create an OpenGL context!");

  gladLoadGLLoader(SDL_GL_GetProcAddress);

  // Use v-sync
  SDL_GL_SetSwapInterval(1);

  int w,h;
  SDL_GetWindowSize(window, &w, &h);
  glViewport(0, 0, w, h);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}


void setupGLStuff() 
{
 
  
  glEnable(GL_DEPTH_TEST);
  shader = Shader("../src/shaders/vertex.vs","../src/shaders/fragment.frag");
  projection = glm::perspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);
  glGenBuffers(1, &instanceVBO);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 10000, &translations[0],  GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
  // Also set instance data
  glEnableVertexAttribArray(2);


  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glVertexAttribDivisor(2, 1); // Tell OpenGL this is an instanced vertex attribute.
  glBindVertexArray(0);
}

void cleanup()
{
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}