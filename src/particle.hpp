/*
 * particle.hpp
 *
 * Particle data structure containing positions and velocities for N-body visualization.
 * Uses std::vector for safe memory management of particle data.
 */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <iostream>
#include <vector>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Particle
{
  public:
    /*
     * Generates the default cube for graphics testing.
     * Creates a 40x40x40 grid of 64,000 particles.
     */
    Particle()
    {
        n = 64000;
        instanceVBO = 0;
        translations.resize(n);
        velocities.resize(1);
        for (int i = 0; i < n; i++) {
            translations[i] = glm::vec4(i % 40 * 1.25, i % 1600 / 40.0f * 1.25, i % 64000 / 1600.0f * 1.25, 500);
        }
        setUpInstanceBuffer();
    }

    /*
     * Creates a new particle structure containing an N number of particles.
     * Copies the provided position data into internal storage.
     */
    Particle(long count, const glm::vec4* positions)
    {
        n = count;
        instanceVBO = 0;
        translations.assign(positions, positions + count);
        velocities.resize(count);
        setUpInstanceBuffer();
    }

    ~Particle()
    {
        if (instanceVBO != 0) {
            glDeleteBuffers(1, &instanceVBO);
        }
    }

    // Prevent copying (owns GL resources)
    Particle(const Particle&) = delete;
    Particle& operator=(const Particle&) = delete;

    /*
     * Changes the translations in the particle structure.
     * Copies data from the provided array.
     */
    void changeTranslations(long count, const glm::vec4* new_positions)
    {
        if (new_positions) {
            n = count;
            translations.assign(new_positions, new_positions + count);
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
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * n, translations.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    /*
     * Sets up the memory structure for the particle data.
     */
    void setUpInstanceArray()
    {
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glVertexAttribDivisor(0, 1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    /*
     * Changes the velocities in the particle structure.
     * Copies data from the provided array.
     */
    void changeVelocities(const glm::vec4* new_velocities)
    {
        if (new_velocities) {
            velocities.assign(new_velocities, new_velocities + n);
            return;
        }
        std::cout << "Error Loading New Velocities" << std::endl;
    }

    long n;                              // number of objects
    GLuint instanceVBO;                  // the instance VBO for OpenGL rendering
    std::vector<glm::vec4> translations; // the positions of the particles
    std::vector<glm::vec4> velocities;   // the velocity data

  private:
    /*
     * Sets up the memory space (buffer) in OpenGL that streams the translations to the GPU.
     */
    void setUpInstanceBuffer()
    {
        glDeleteBuffers(1, &instanceVBO);
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * n, translations.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};

#endif