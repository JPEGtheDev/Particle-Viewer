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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  manageFPS(ticks,lastticks);
    
}

void drawFunct()
{
  cam.update(deltaTime);
  shader.Use();
  
  




  glm::mat4 view;
  glm::mat4 projection;

  view = cam.setupCam();
  projection = glm::perspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

  GLint modelLoc = glGetUniformLocation(shader.Program, "model");
  GLint viewLoc = glGetUniformLocation(shader.Program, "view");
  GLint projLoc = glGetUniformLocation(shader.Program, "projection");

  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

  glBindVertexArray(VAO);
  
  for (GLuint i = 0; i < 10; i++)
  {
      // Calculate the model matrix for each object and pass it to shader before drawing
      glm::mat4 model;
      model = glm::translate(model, cubePositions[i]);
      GLfloat angle = 20.0f * i;
      model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

      glDrawArrays(GL_TRIANGLES, 0, 36);
  }








  glBindVertexArray(VAO);


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
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  // TexCoord attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}
void cleanup()
{
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}