//includes
	#include <iostream>
	#include "glm/glm.hpp"
	#include "glm/gtc/matrix_transform.hpp"
	#include "glm/gtc/type_ptr.hpp"
	#include "glad/glad.h"
	#include "shader.hpp"
    #include "camera.hpp"
    #include "models/lowResSphere.h"
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
    void setupObjects();			
//end function prototypes

//variables
	const int SCREEN_FULLSCREEN = 0;
	const int SCREEN_WIDTH  = 1280;
	const int SCREEN_HEIGHT = 720;
	SDL_Window *window = NULL;
	SDL_GLContext maincontext;
	bool quit = false;
	bool highRes = true;
	uint32_t ticks,lastticks= 0;

    Shader highResShader;
    Shader lowResShader;
    GLuint transformLoc;
    Camera cam = Camera();
    int n = 27000;
    int defN = 27000;
    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;
    glm::mat4 view;
    glm::mat4 projection;
    GLuint quadVAO, quadVBO;
    GLuint instanceVBO;
    std::string highResVertexShader =
    #include "shaders/highResVertex.vs"
    ;
    std::string highResFragmentShader =
    #include "shaders/highResFragment.frag"
    ;
    std::string lowResVertexShader =
    #include "shaders/lowResVertex.vs"
    ;
    std::string lowResFragmentShader =
    #include "shaders/lowResFragment.frag"
    ;

//end variables