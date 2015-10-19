//Universal Includes
#include <iostream>

//Generally Accepted Includes
#include <SDL2/SDL.h>
#include "glad/glad.h"
#include <glm/glm.hpp>

//OS Specific Includes


//Asset Importer Includes
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static void sdl_die(const char * message); //finds SDL errors / kills the app if something doesn't init properly
void init_screen(const char * caption); //initializes the screen / window / context
void manageFPS(uint32_t &ticks, uint32_t &lastticks); //limits FPS to 60
void beforeDraw(); //the generic clear screen and other stuff before anything else has to be done
void drawFunct(); //Draws stuff to the screen
void readInput(SDL_Event &event); //takes in all input

static const int SCREEN_FULLSCREEN = 0;
static const int SCREEN_WIDTH  = 1024;
static const int SCREEN_HEIGHT = 768;
static SDL_Window *window = NULL;
static SDL_GLContext maincontext;

bool quit = false;
uint32_t ticks,lastticks=0;

int main() 
{
  init_screen("Particle-Viewer");
  SDL_Event event;
  ticks = SDL_GetTicks();

  while (!quit) 
  {
  	readInput(event);
    beforeDraw();
    drawFunct();
    SDL_GL_SwapWindow(window); // should almost always be last
  }
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
  // Initialize SDL 
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
