#include "particle.hpp"

Particle::Particle()
{
    n = 64000;
    translations = new glm::vec3 [n];

    for(int i =0; i < n; i++)
    {
        translations[i] = glm::vec3 (i%40 *.75,i%1600/40.0f * .75,i %64000/1600.0f *.75);
    }
    setUpInstanceBuffer();
}

Particle::~Particle()
{
    delete[] translations;
}

void Particle::pushVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * n, &translations[0],  GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particle::setUpInstanceBuffer()
{
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * n, &translations[0],  GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particle::setUpInstanceArray()
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glVertexAttribDivisor(1, 1); // Tell OpenGL this is an instanced vertex attribute.

}
//code to update N
//code to update translations
//figure out if anyhting else needs to be put in here