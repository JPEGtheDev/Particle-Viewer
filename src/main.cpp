#include "clutter.hpp"

int main(int argc, char* argv[])
{
  init_screen("Particle-Viewer");
  SDL_Event event;
  ticks = SDL_GetTicks();
  while (!quit) 
  {
  	readInput(event);
    beforeDraw();
    drawFunct();
    SDL_GL_SwapWindow(window);
  }
  return 0;
}

void beforeDraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  manageFPS(ticks,lastticks);
    
}

void drawFunct()
{
	//render here
}

void readInput(SDL_Event &event)
{
	while (SDL_PollEvent(&event)) 
  {
    if (event.type == SDL_QUIT) 
    {
      quit = true;
    }
  }
}

void manageFPS(uint32_t &ticks, uint32_t &lastticks)
{
	ticks = SDL_GetTicks();
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
