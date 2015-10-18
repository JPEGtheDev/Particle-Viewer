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



int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Delay(1000);
	std::cout << "Hello World" << std::endl;
	SDL_Quit();
	return 0;
}

