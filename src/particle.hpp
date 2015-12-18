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
    Particle(long,glm::vec4*);
    ~Particle();				//destructor for the particles

    void pushVBO();				//pushes the VBO to the graphics card. Allows the positions to change.
	void setUpInstanceArray();	//sets up the instance array to be used in the shader
	void changeTranslations(long,glm::vec4*);
	void changeVelocities(glm::vec4*);
	long n;						//number of objects
	GLuint instanceVBO;			//the instance VBO, it contains a pointer to the translations
	glm::vec4 *translations;	//the positions of the particles
	glm::vec4 *velocities;		//the (should be) blank velocities vec4
	//include shader

private:

	void setUpInstanceBuffer(); //sets up the instanceVBO

};

#endif