//includes
		#include <iostream>

	//Generally Accepted Includes
		#include <SDL2/SDL.h>
		#include "glad/glad.h"
		#include <glm/glm.hpp>
		#include <AntTweakBar.h>
	//Windows Includes
	
//end includes


//function prototypes
	static void sdl_die(const char * message); 				//finds SDL errors / kills the app if something doesn't init properly
	void init_screen(const char * caption);					//initializes the screen / window / context
	void manageFPS(uint32_t &ticks, uint32_t &lastticks);	//limits FPS to 60
	void beforeDraw(); 										//the generic clear screen and other stuff before anything else has to be done
	void drawFunct(); 										//Draws stuff to the screen
	void readInput(SDL_Event &event); 						//takes in all input
//end function prototypes

//variables
	static const int SCREEN_FULLSCREEN = 0;
	static const int SCREEN_WIDTH  = 1024;
	static const int SCREEN_HEIGHT = 768;
	static SDL_Window *window = NULL;
	static SDL_GLContext maincontext;
	static bool quit = false;
	static uint32_t ticks,lastticks= 0;
//end variables