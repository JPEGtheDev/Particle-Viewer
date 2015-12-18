#include "particle.hpp"

Particle::Particle()
{

    n = 64000;
    translations = new glm::vec4[n];
    velocities = new glm::vec4[1];
    for(int i =0; i < n; i++)
    {
        translations[i] = glm::vec4 (i%40 *.75,i%1600/40.0f * .75,i %64000/1600.0f *.75,500);
    }
    setUpInstanceBuffer();
}
Particle::Particle(long N, glm::vec4* trans)
{
    n = N;
    translations = trans;
    setUpInstanceBuffer();
}
Particle::~Particle()
{
    delete[] translations;
}
void Particle::changeTranslations(long N, glm::vec4 *newTrans)
{
    if(newTrans)
    {
        delete[] translations;
        n = N;
        translations = newTrans;
        setUpInstanceBuffer();
        return;
    }
    std::cout << "Error Loading New Translations" << std::endl;
}
void Particle::changeVelocities(glm::vec4 *newVel)
{
    if(newVel)
    {
        delete[] velocities;
        velocities = newVel;
        return;
    }
    std::cout << "Error Loading New Velocities" << std::endl;
}
void Particle::pushVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * n, &translations[0],  GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particle::setUpInstanceBuffer()
{
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * n, &translations[0],  GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particle::setUpInstanceArray()
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glVertexAttribDivisor(0, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    
}
//figure out if anyhting else needs to be put in here