//includes
	#include <iostream>
	#include "glm/glm.hpp"
	#include "glm/gtc/matrix_transform.hpp"
	#include "glm/gtc/type_ptr.hpp"
	#include "glad/glad.h"
	#include "shader.hpp"
    #include "camera.hpp"
	#include "tinyFileDialogs/tinyfiledialogs.h"
	#include "particle.hpp"
	#include "settingsIO.hpp"
	#ifdef _WIN32 //Windows Includes
		#include <windows.h>
		#include <SDL.h>
	#else //Includes for Linux and OSX
		#include <SDL2/SDL.h>
	#endif
//end includes
	

//function prototypes
	static void sdl_die(const char * message); 				//finds SDL errors / kills the app if something doesn't init properly
	void init_screen(const char * caption);					//initializes the screen / window / context
	void manageFPS(uint32_t &ticks, uint32_t &lastticks);	//limits FPS to 60 and update deltaTime
	void beforeDraw(); 										//the generic clear screen and other stuff before anything else has to be done
	void drawFunct(); 										//Draws stuff to the screen
	void readInput(SDL_Event &event); 						//takes in all input
	void setupGLStuff();									//sets up the VAOs and the VBOs
	void cleanup();											//destroy it all with fire
    void setupRender();										//Updates the VBOs for position changes
    void seekFrame(int frame, bool isForward);				//skips frames
//end function prototypes

//variables
	const int SCREEN_FULLSCREEN = 0, SCREEN_WIDTH  = 1280, SCREEN_HEIGHT = 720;
	int curFrame = 0;
	bool quit = false, highRes = false;
	float sphereScale = 1.0;
	float sphereRadius = 250.0f;
    GLuint circleVAO, circleVBO;

    uint32_t ticks,lastticks = 0;
    GLfloat deltaTime = 0.0f, lastFrame = 0.0f;

	SDL_Window *window = NULL;
	SDL_GLContext maincontext;

    Shader sphereShader;
   
    Camera cam = Camera(SCREEN_WIDTH,SCREEN_HEIGHT);
    Particle* part;
    
    glm::mat4 view;
    
    std::string sphereVertexShader =
    #include "shaders/sphereVertex.vs"
    ,
    sphereFragmentShader =
    #include "shaders/sphereFragment.frag"
    ;
//end variables
    const std::string posLoc = "/Users/JPEG/Desktop/500kSlam/PosAndVel";
    const std::string setLoc = "/Users/JPEG/Desktop/500kSlam/RunSetup";
    SettingsIO *set = new SettingsIO();
//functions that should not be changed
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
	  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
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
//functions
