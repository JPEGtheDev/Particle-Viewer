/*
 * particle.hpp
 *
 * Particle data structure containing velocities and translations.
 *
 */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <iostream>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Particle
{
  public:
    /*
     * Generates the default cube for graphics testing.
     */
    Particle()
    {
        n = 64000;
        instanceVBO = 0; // Initialize before setUpInstanceBuffer to avoid undefined behavior
        translations = new glm::vec4[n];
        velocities = new glm::vec4[1];
        for (int i = 0; i < n; i++) {
            translations[i] = glm::vec4(i % 40 * 1.25, i % 1600 / 40.0f * 1.25, i % 64000 / 1600.0f * 1.25, 500);
        }
        setUpInstanceBuffer();
    }

    /*
     * Creates a new particle structure containing an N number of particles.
     * This function also sets up the buffer to be used with OpenGL calls.
     */
    Particle(long N, glm::vec4* trans)
    {
        n = N;
        instanceVBO = 0; // Initialize before setUpInstanceBuffer to avoid undefined behavior
        translations = trans;
        velocities = new glm::vec4[N];
        setUpInstanceBuffer();
    }
    /*
     * Cleans everything up
     */
    ~Particle()
    {
        delete[] translations;
        delete[] velocities;
    }
    /*
     * Changes the translations in the particle structure
     */
    void changeTranslations(long N, glm::vec4* newTrans)
    {
        if (newTrans) {
            delete[] translations;
            n = N;
            translations = new glm::vec4[N];
            for (int i = 0; i < n; i++) {
                translations[i] = newTrans[i];
            }
            setUpInstanceBuffer();
            return;
        }
        std::cout << "Error Loading New Translations" << std::endl;
    }

    /*
     * Pushes the translation data to OpenGL. Allows the translations to change.
     */
    void pushVBO()
    {
        // glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * n, &translations[0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    /*
     * Sets up the memory structure for the particle data
     */
    void setUpInstanceArray()
    {
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glVertexAttribDivisor(0, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    /*
     * Changes the velocities in the particle structure
     */
    void changeVelocities(glm::vec4* newVel)
    {
        if (newVel) {
            delete[] velocities;
            velocities = new glm::vec4[n];
            for (int i = 0; i < n; i++) {
                velocities[i] = newVel[i];
            }
            return;
        }
        std::cout << "Error Loading New Velocities" << std::endl;
    }

    long n;                  // number of objects
    GLuint instanceVBO;      // the instance VBO, it contains a pointer to the translations
    glm::vec4* translations; // the positions of the particles
    glm::vec4* velocities;   // the (should be) blank velocities vec4

  private:
    /*
     * Sets up the memory space (buffer) in OpenGL that streams the translations to the graphics card.
     */
    void setUpInstanceBuffer()
    {
        glDeleteBuffers(1, &instanceVBO);
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * n, &translations[0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};

#endif