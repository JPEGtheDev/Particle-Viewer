#ifndef PARTICLE_H
#define PARTICLE_H

#include <iostream>
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Particle
{
public:
	
    Particle();					//default cube constructor
    ~Particle();				//destructor for the particles

    void pushVBO();				//pushes the VBO to the graphics card. Allows the positions to change.
	void setUpInstanceArray();	//sets up the instance array to be used in the shader

	int n;						//number of objects
	GLuint instanceVBO;			//the instance VBO, it contains a pointer to the translations
	glm::vec3 *translations;	//the positions of the particles
	//include shader

private:

	void setUpInstanceBuffer(); //sets up the instanceVBO

};

#endif